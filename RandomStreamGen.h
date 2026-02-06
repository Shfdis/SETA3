#pragma once

#include <vector>
#include <string>
#include <random>

class RandomStreamGen {
public:
    explicit RandomStreamGen(unsigned seed = std::random_device{}());
    
    std::string generateString();
    
    std::vector<std::string> generateStream(size_t n);
    
    std::vector<std::string> getStreamAtTime(double t, size_t totalSize);
    
    void setSeed(unsigned seed);

private:
    static constexpr size_t MAX_STRING_LEN = 30;
    static const std::string CHARSET;
    
    unsigned seed_;
    std::mt19937 rng_;
    std::uniform_int_distribution<size_t> len_dist_{1, MAX_STRING_LEN};
    std::uniform_int_distribution<size_t> char_dist_{0, CHARSET.size() - 1};
};
