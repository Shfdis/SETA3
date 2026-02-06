#include "RandomStreamGen.h"
#include "HashFuncGen.h"
#include "HyperLogLog.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>

struct StatRow {
    size_t stream_size;
    int step;
    double t;
    size_t processed_size;
    double E_Ft0;
    double E_Nt;
    double sigma_Nt;
};

struct Graph1Row {
    size_t stream_size;
    int step;
    double t;
    size_t processed_size;
    size_t Ft0;
    uint64_t Nt;
};

static int runBSelectionExperiment(HashFuncGen& hf) {
    std::cout << "Эксперимент выбора B\n";
    const std::vector<int> B_values = {4, 6, 8, 10, 12};
    const int numRuns = 10;
    const size_t streamSize = 8000;
    std::vector<double> avgRelErr(B_values.size(), 0);

    for (int run = 0; run < numRuns; ++run) {
        RandomStreamGen gen(static_cast<unsigned>(1000 + run));
        auto stream = gen.generateStream(streamSize);
        size_t Ft0 = HyperLogLog::exactUniqueCount(stream);
        if (Ft0 == 0) continue;

        for (size_t bi = 0; bi < B_values.size(); ++bi) {
            HyperLogLog hll(B_values[bi], hf);
            for (const auto& s : stream) hll.add(s);
            uint64_t Nt = hll.estimate();
            double relErr = std::fabs(static_cast<double>(Nt) - Ft0) / Ft0;
            avgRelErr[bi] += relErr;
        }
    }

    int chosenB = B_values[0];
    double bestAvg = 1e9;
    for (size_t bi = 0; bi < B_values.size(); ++bi) {
        avgRelErr[bi] /= numRuns;
        std::cout << "B=" << B_values[bi] << "  ср_отн_ошибка=" << std::fixed
                  << std::setprecision(4) << avgRelErr[bi] << "\n";
        if (avgRelErr[bi] < bestAvg) {
            bestAvg = avgRelErr[bi];
            chosenB = B_values[bi];
        }
    }
    std::cout << "Выбран B=" << chosenB << "\n\n";
    return chosenB;
}

static std::vector<Graph1Row> generateGraph1Data(HashFuncGen& hf, int B) {
    std::vector<Graph1Row> rows;
    const std::vector<size_t> streamSizes = {1000, 5000, 10000, 20000};
    const std::vector<double> tValues = {0.10, 0.25, 0.50, 0.75, 1.0};
    const unsigned seed = 42;

    for (size_t totalSize : streamSizes) {
        for (int step = 0; step < 5; ++step) {
            double t = tValues[step];
            RandomStreamGen gen(seed);
            auto stream = gen.getStreamAtTime(t, totalSize);
            size_t Ft0 = HyperLogLog::exactUniqueCount(stream);
            HyperLogLog hll(B, hf);
            for (const auto& s : stream) hll.add(s);
            uint64_t Nt = hll.estimate();
            rows.push_back({totalSize, step, t, static_cast<size_t>(t * totalSize), Ft0, Nt});
        }
    }
    return rows;
}

static std::vector<StatRow> runStreamExperiments(HashFuncGen& hf, int B) {
    std::cout << "Эксперименты с потоками\n";
    std::vector<StatRow> stats;
    const std::vector<size_t> streamSizes = {1000, 5000, 10000, 20000};
    const std::vector<double> tValues = {0.10, 0.25, 0.50, 0.75, 1.0};
    const int K = 15;

    for (size_t totalSize : streamSizes) {
        for (int step = 0; step < 5; ++step) {
            double t = tValues[step];
            std::vector<uint64_t> NtSamples;
            std::vector<size_t> Ft0Samples;
            for (int run = 0; run < K; ++run) {
                RandomStreamGen gen(static_cast<unsigned>(42 + run * 7));
                auto stream = gen.getStreamAtTime(t, totalSize);
                size_t Ft0 = HyperLogLog::exactUniqueCount(stream);
                HyperLogLog hll(B, hf);
                for (const auto& s : stream) hll.add(s);
                uint64_t Nt = hll.estimate();
                Ft0Samples.push_back(Ft0);
                NtSamples.push_back(Nt);
            }

            double sumNt = 0, sumFt0 = 0;
            for (int i = 0; i < K; ++i) {
                sumNt += NtSamples[i];
                sumFt0 += Ft0Samples[i];
            }
            double E_Nt = sumNt / K;
            double E_Ft0 = sumFt0 / K;
            double var = 0;
            for (int i = 0; i < K; ++i) {
                double d = NtSamples[i] - E_Nt;
                var += d * d;
            }
            double sigma_Nt = (K > 1) ? std::sqrt(var / (K - 1)) : 0;

            stats.push_back({totalSize, step, t, static_cast<size_t>(t * totalSize), E_Ft0, E_Nt, sigma_Nt});

            std::cout << "Размер=" << totalSize << " t=" << std::fixed << std::setprecision(0)
                      << (t * 100) << "%: E(Ft0)=" << std::setprecision(1) << E_Ft0
                      << " E(Nt)=" << E_Nt << " sigma_Nt=" << sigma_Nt;
            std::cout << " Nt_samples=[";
            for (int i = 0; i < K && i < 5; ++i) std::cout << NtSamples[i] << (i < 4 ? "," : "");
            if (K > 5) std::cout << "...";
            std::cout << "]\n";
        }
        std::cout << "\n";
    }
    return stats;
}

static void writeCSV(const std::vector<Graph1Row>& g1, const std::vector<StatRow>& g2) {
    std::ofstream f1("graph1_data.csv");
    f1 << "stream_size,step,t,processed_size,Ft0,Nt\n";
    for (const auto& r : g1) {
        f1 << r.stream_size << "," << r.step << "," << r.t << "," << r.processed_size
           << "," << r.Ft0 << "," << r.Nt << "\n";
    }

    std::ofstream f2("graph2_data.csv");
    f2 << "stream_size,step,t,processed_size,E_Ft0,E_Nt,sigma_Nt\n";
    for (const auto& r : g2) {
        f2 << r.stream_size << "," << r.step << "," << r.t << "," << r.processed_size
           << "," << std::fixed << std::setprecision(2) << r.E_Ft0 << "," << r.E_Nt
           << "," << r.sigma_Nt << "\n";
    }
}

static void printPhase3Analysis(int B, const std::vector<StatRow>& stats) {
    size_t m = size_t(1) << B;
    double sigma_theory_104 = 1.04 / std::sqrt(static_cast<double>(m));
    double sigma_theory_132 = 1.32 / std::sqrt(static_cast<double>(m));

    std::cout << "Этап 3: Анализ\n";
    std::cout << "B=" << B << " m=" << m << "\n";
    std::cout << "Теор. относительная SE (1.04/sqrt(m)): " << std::fixed << std::setprecision(4)
              << sigma_theory_104 << "\n";
    std::cout << "Теор. относительная SE (1.32/sqrt(m)): " << sigma_theory_132 << "\n\n";

    std::cout << "Точность (|E(Nt)-E(Ft0)|/E(Ft0)):\n";
    for (const auto& r : stats) {
        if (r.E_Ft0 <= 0) continue;
        double relErr = std::fabs(r.E_Nt - r.E_Ft0) / r.E_Ft0;
        bool ok104 = (relErr <= sigma_theory_104);
        bool ok132 = (relErr <= sigma_theory_132);
        std::cout << "  размер=" << r.stream_size << " t=" << (int)(r.t * 100) << "%: отн_ошибка="
                  << std::fixed << std::setprecision(4) << relErr
                  << " в пределах 1.04/sqrt(m)=" << (ok104 ? "да" : "нет")
                  << " в пределах 1.32/sqrt(m)=" << (ok132 ? "да" : "нет") << "\n";
    }

    std::cout << "\nСтабильность (CV = sigma_Nt/E(Nt)):\n";
    for (const auto& r : stats) {
        if (r.E_Nt <= 0) continue;
        double cv = r.sigma_Nt / r.E_Nt;
        std::cout << "  размер=" << r.stream_size << " t=" << (int)(r.t * 100) << "%: CV="
                  << std::fixed << std::setprecision(4) << cv << "\n";
    }

    std::cout << "\nКонстанты: alpha_m для m=16,32,64; 0.7213/(1+1.079/m) для m>=128. "
              << "Linear counting при E_raw < 2.5*m (малая кардинальность).\n";
}

int main() {
    HashFuncGen hf(HashMethod::FNV1a);
    int B = runBSelectionExperiment(hf);

    auto g1 = generateGraph1Data(hf, B);
    auto g2 = runStreamExperiments(hf, B);
    writeCSV(g1, g2);

    printPhase3Analysis(B, g2);

    return 0;
}
