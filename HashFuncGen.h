#pragma once

#include <string>
#include <cstdint>
#include <vector>

enum class HashMethod {
    Polynomial,
    FNV1a
};

class HashFuncGen {
public:
    explicit HashFuncGen(HashMethod method = HashMethod::Polynomial);

    uint32_t operator()(const std::string& s) const;
    uint32_t hash(const std::string& s) const;

    void setMethod(HashMethod method);

    static double uniformityScore(const std::vector<std::string>& samples,
                                  const HashFuncGen& h,
                                  uint32_t numBuckets = 256);

private:
    HashMethod method_;

    uint32_t hashPolynomial(const std::string& s) const;
    uint32_t hashFNV1a(const std::string& s) const;
};
