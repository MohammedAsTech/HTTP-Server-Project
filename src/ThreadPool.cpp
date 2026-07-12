#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; i++) {
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::submit(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_taskQueue.push(task);
    }
    m_cv.notify_one();
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] {
                return !m_taskQueue.empty() || m_stop;
            });

            if (m_stop && m_taskQueue.empty()) return;

            task = m_taskQueue.front();
            m_taskQueue.pop();
        }
        task();
    }
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_cv.notify_all();
    for (auto& worker : m_workers) {
        if (worker.joinable()) worker.join();
    }
}