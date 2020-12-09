#pragma once

#include <stdint.h>
#include <vector>

namespace appsk {
namespace utils {

inline uint64_t byteArray2Ux(const std::vector<uint8_t> vec, int len)
{
    uint64_t v = 0;
    for (uint8_t j = 0; j < len; j++) {
        v += ((vec.at(j)&0xFFULL)<<((len-j-1)*8));
    }
    return v;
}

inline std::vector<uint8_t> ux2ByteArray(uint64_t v, int len)
{
    std::vector<uint8_t> vec(len, 0);
    for (uint8_t j = 0; j < len; j++) {
        vec[j] = (v >> ((len-j-1)*8))&0xFF;
    }
    return vec;
}

inline uint64_t byteArray2Ux(const uint8_t (array)[], int len)
{
    uint64_t v = 0;
    for (uint8_t j = 0; j < len; j++) {
        v += ((array[j]&0xFFULL)<<((len-j-1)*8));
    }
    return v;
}

} // namespace utils
} // namespace appsk
