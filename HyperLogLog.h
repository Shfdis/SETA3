#pragma once

#include <vector>
#include <string>
#include <cstdint>

class HashFuncGen;

class HyperLogLog {
public:
    explicit HyperLogLog(int B, const HashFuncGen& hash_func);

    void add(const std::string& s);
    uint64_t estimate() const;

    static size_t exactUniqueCount(const std::vector<std::string>& stream);

private:
    int B_;
    size_t m_;
    std::vector<uint8_t> registers_;
    const HashFuncGen* hash_func_;

    static double alpha(size_t m);
};
