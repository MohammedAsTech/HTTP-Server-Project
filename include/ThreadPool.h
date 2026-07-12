#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();
    void submit(std::function<void()> task);
    void shutdown();

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_taskQueue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stop = false;
    void workerLoop();
};
#endif