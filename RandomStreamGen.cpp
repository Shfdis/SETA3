#include "RandomStreamGen.h"

const std::string RandomStreamGen::CHARSET =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789-";

RandomStreamGen::RandomStreamGen(unsigned seed) : seed_(seed), rng_(seed) {}

std::string RandomStreamGen::generateString() {
    size_t len = len_dist_(rng_);
    std::string result;
    result.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        result += CHARSET[char_dist_(rng_)];
    }
    return result;
}

std::vector<std::string> RandomStreamGen::generateStream(size_t n) {
    std::vector<std::string> stream;
    stream.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        stream.push_back(generateString());
    }
    return stream;
}

std::vector<std::string> RandomStreamGen::getStreamAtTime(double t, size_t totalSize) {
    if (t <= 0.0 || totalSize == 0) return {};
    
    size_t count = static_cast<size_t>(t * totalSize);
    if (count == 0) return {};
    if (count >= totalSize) count = totalSize;
    
    std::mt19937 temp_rng(seed_);
    std::uniform_int_distribution<size_t> len_dist(1, MAX_STRING_LEN);
    std::uniform_int_distribution<size_t> char_dist(0, CHARSET.size() - 1);
    
    std::vector<std::string> stream;
    stream.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        size_t len = len_dist(temp_rng);
        std::string s;
        s.reserve(len);
        for (size_t j = 0; j < len; ++j) {
            s += CHARSET[char_dist(temp_rng)];
        }
        stream.push_back(std::move(s));
    }
    return stream;
}

void RandomStreamGen::setSeed(unsigned seed) {
    seed_ = seed;
    rng_.seed(seed);
}
