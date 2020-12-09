#pragma once

#include "std_id.h"

#include <memory>
#include <mutex>
#include <map>
#include <limits>

namespace appsk {
namespace utils {

template<typename T>
class HandleStorage {
public:
    static const unsigned long m_handleIdLimit = 1024;//(std::numeric_limits<int>::max)();
    static const unsigned long INVALID_HANDLE_VAL = m_handleIdLimit;

    unsigned long put(const std::shared_ptr<T> &ptr)
    {
        std::lock_guard<std::mutex> lk(m_storageMutex);

        if (m_ptrStorage.size() >= m_handleIdLimit) {
            return INVALID_HANDLE_VAL;
        }

        unsigned long handle = m_idStorage.getId();

        if (contains(handle)) {
            return INVALID_HANDLE_VAL;
        }

        m_ptrStorage.insert(std::pair<unsigned long, std::shared_ptr<T>>(handle, ptr));

        return handle;
    }

    std::shared_ptr<T> get(unsigned long handle)
    {
        std::lock_guard<std::mutex> lk(m_storageMutex);

        if (!contains(handle)) {
            return nullptr;
        }
        return m_ptrStorage[handle];
    }

    std::map<unsigned long, std::shared_ptr<T>> getItems()
    {
        std::lock_guard<std::mutex> lk(m_storageMutex);
        return m_ptrStorage;
    }

    bool remove(unsigned long handle)
    {
        std::lock_guard<std::mutex> lk(m_storageMutex);

        if (!contains(handle)) {
            return false;
        } 
        m_ptrStorage.erase(handle);
        m_idStorage.releaseId(handle);

        return true;
    }

protected:
    bool contains(unsigned long handle)
    {
        return m_ptrStorage.find(handle) != m_ptrStorage.end();
    }

    std::mutex m_storageMutex;
    std::map<unsigned long, std::shared_ptr<T>> m_ptrStorage;
    SimpleIdStorage<m_handleIdLimit> m_idStorage;
    //unsigned long m_handle = INVALID_HANDLE_VAL; // 0 is invalid
};

} // namespace utils
} // namespace appsk
