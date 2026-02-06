#include "HashFuncGen.h"
static constexpr uint32_t P_POLY = 31;
static constexpr uint32_t FNV_OFFSET = 2166136261u;
static constexpr uint32_t FNV_PRIME = 16777619u;

HashFuncGen::HashFuncGen(HashMethod method) : method_(method) {}

uint32_t HashFuncGen::operator()(const std::string& s) const {
    return hash(s);
}

uint32_t HashFuncGen::hash(const std::string& s) const {
    switch (method_) {
        case HashMethod::Polynomial:
            return hashPolynomial(s);
        case HashMethod::FNV1a:
            return hashFNV1a(s);
    }
    return 0;
}

uint32_t HashFuncGen::hashPolynomial(const std::string& s) const {
    uint32_t h = 0;
    for (unsigned char c : s) {
        h = h * P_POLY + c;
    }
    return h;
}

uint32_t HashFuncGen::hashFNV1a(const std::string& s) const {
    uint32_t h = FNV_OFFSET;
    for (unsigned char c : s) {
        h ^= c;
        h *= FNV_PRIME;
    }
    return h;
}

void HashFuncGen::setMethod(HashMethod method) {
    method_ = method;
}

double HashFuncGen::uniformityScore(const std::vector<std::string>& samples,
                                    const HashFuncGen& h,
                                    uint32_t numBuckets) {
    if (samples.empty() || numBuckets == 0) return 0.0;

    std::vector<uint32_t> buckets(numBuckets, 0);
    for (const auto& s : samples) {
        uint32_t hv = h(s);
        buckets[hv % numBuckets]++;
    }

    double expected = static_cast<double>(samples.size()) / numBuckets;
    double chi2 = 0.0;
    for (uint32_t count : buckets) {
        double diff = count - expected;
        chi2 += (diff * diff) / expected;
    }
    double p_value_approx = 1.0 / (1.0 + chi2 / numBuckets);
    return p_value_approx;
}
