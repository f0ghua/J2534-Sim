#pragma once

#include <bitset>

namespace appsk {
namespace utils {

template<int N, int B = 0>
class SimpleIdStorage {
public:
    std::bitset<N> ids;

    bool isIdExist()
    {
        return !ids.all();
    }

    int getId()
    {
        if (!isIdExist()) {
            return -1;
        }

        for (int i = 0; i < N; ++i) {
            if (!ids.test(static_cast<size_t>(i))) {
                ids.set(static_cast<size_t>(i));
                return i + B;
            }
        }
        return -1;
    }

    void releaseId(int i)
    {
        ids.reset(static_cast<size_t>(i - B));
    }

    void releaseAll()
    {
        ids.reset();
    }
};

} // namespace utils

} // namespace appsk
