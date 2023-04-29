#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <memory>
#include <thread>
#include <unordered_map>

template <typename T>
class MemoryPool {
  public:
    using PtrType = std::shared_ptr<T>;
    
    explicit MemoryPool(size_t initialSize = 10) {
        for (size_t i = 0; i < initialSize; ++i) {
            auto ptr = new T();
            m_pool.push_back(ptr);
        }
    }
    
    ~MemoryPool() {
        for (auto ptr : m_pool) {
            delete ptr;
        }
    }
    
    PtrType acquire() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_pool.empty()) {
            return PtrType(new T(), [this](T* ptr) { this->release(ptr); });
        } else {
            auto ptr = m_pool.back();
            m_pool.pop_back();
            return PtrType(ptr, [this](T* ptr) { this->release(ptr); });
        }
    }
  
  private:
    void release(T* ptr) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pool.push_back(ptr);
    }
    
    std::vector<T*> m_pool;
    std::mutex m_mutex;
};

class ThreadCache {
  public:
    template <typename T>
    static std::shared_ptr<T> acquire() {
        auto tid = std::this_thread::get_id();
        auto it = m_threadCaches.find(tid);
        if (it == m_threadCaches.end()) {
            auto memPool = std::make_shared<MemoryPool<T>>();
            m_threadCaches[tid] = memPool;
            return memPool->acquire();
        } else {
            auto memPool = std::static_pointer_cast<MemoryPool<T>>(it->second);
            return memPool->acquire();
        }
    }
  
  private:
    static thread_local std::unordered_map<std::thread::id, std::shared_ptr<void>> m_threadCaches;
};

thread_local std::unordered_map<std::thread::id, std::shared_ptr<void>> ThreadCache::m_threadCaches;

class Message {
  public:
    Message() {
        std::cout << "Message constructed" << std::endl;
    }
    
    ~Message() {
        std::cout << "Message destructed" << std::endl;
    }
    
    void process() {
        // 处理消息的逻辑
    }
};


