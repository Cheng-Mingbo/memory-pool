#include <iostream>
#include <thread>
#include <vector>
#include "memory_pool.h" // 引入ThreadCache头文件

void processMessages() {
    for (int i = 0; i < 10; ++i) {
        // 从当前线程的内存池中获取一个Message对象
        auto message = ThreadCache::acquire<Message>();
        
        // 处理消息
        message->process();
        
        // message对象在离开作用域时会自动被放回内存池
    }
}

int main() {
    const int numThreads = 4;
    std::vector<std::thread> threads;
    
    // 创建多个线程，每个线程处理一部分消息
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(processMessages);
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
