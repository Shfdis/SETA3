#include "HyperLogLog.h"
#include "HashFuncGen.h"
#include <unordered_set>
#include <cmath>

static int rho(uint32_t w, int bits) {
    if (w == 0) return bits + 1;
    int leading_zeros = 0;
    uint32_t mask = 1u << (bits - 1);
    while (mask && (w & mask) == 0) {
        leading_zeros++;
        mask >>= 1;
    }
    return leading_zeros + 1;
}

static const double ALPHA_16 = 0.673;
static const double ALPHA_32 = 0.697;
static const double ALPHA_64 = 0.709;

double HyperLogLog::alpha(size_t m) {
    if (m == 16) return ALPHA_16;
    if (m == 32) return ALPHA_32;
    if (m == 64) return ALPHA_64;
    return 0.7213 / (1.0 + 1.079 / m);
}

HyperLogLog::HyperLogLog(int B, const HashFuncGen& hash_func)
    : B_(B), m_(size_t(1) << B), hash_func_(&hash_func) {
    registers_.resize(m_, 0);
}

void HyperLogLog::add(const std::string& s) {
    uint32_t h = (*hash_func_)(s);
    int bits_remaining = 32 - B_;
    size_t j = (h >> bits_remaining) & ((size_t(1) << B_) - 1);
    uint32_t w = h & ((1u << bits_remaining) - 1);
    int r = rho(w, bits_remaining);
    if (r > registers_[j]) {
        registers_[j] = static_cast<uint8_t>(r);
    }
}

uint64_t HyperLogLog::estimate() const {
    double sum = 0;
    int zeros = 0;
    for (size_t i = 0; i < m_; ++i) {
        sum += std::pow(2.0, -static_cast<double>(registers_[i]));
        if (registers_[i] == 0) zeros++;
    }
    double z = 1.0 / sum;
    double e_raw = alpha(m_) * m_ * m_ * z;

    if (e_raw <= 2.5 * m_ && zeros > 0) {
        return static_cast<uint64_t>(m_ * std::log(static_cast<double>(m_) / zeros) + 0.5);
    }
    return static_cast<uint64_t>(e_raw + 0.5);
}

size_t HyperLogLog::exactUniqueCount(const std::vector<std::string>& stream) {
    std::unordered_set<std::string> unique(stream.begin(), stream.end());
    return unique.size();
}
