#pragma once
#include "Config/pch.h"


class ThreadPool {
public:
    ThreadPool(size_t threadCount = std::thread::hardware_concurrency()) {
        if (threadCount == 0) threadCount = 1;

        for (size_t i = 0; i < threadCount; ++i) {
            m_workers.emplace_back([this]() { WorkerLoop(); });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
        }

        m_cv.notify_all();

        for (auto& t : m_workers)
            if (t.joinable())
                t.join();
    }

    template<typename Iterator, typename Func>
    void ParallelFor(Iterator begin, Iterator end, Func&& func) {
        auto length = std::distance(begin, end);
        if (length == 0) return;

        size_t threadCount = m_workers.size();
        size_t blockSize = (length + threadCount - 1) / threadCount;

        for (size_t i = 0; i < threadCount; ++i) {
            auto blockStart = begin;
            std::advance(blockStart, i * blockSize);

            if (blockStart == end) break;

            auto blockEnd = blockStart;
            size_t dist = std::distance(blockStart, end);
            if (dist > blockSize) dist = blockSize;
            std::advance(blockEnd, dist);

            Enqueue([blockStart, blockEnd, &func]() {
                for (auto it = blockStart; it != blockEnd; ++it) {
                    func(*it);
                }
                });
        }

        Wait();
    }

    // Enqueue a job (system, task, lambda, etc.)
    template<typename F>
    void Enqueue(F&& job) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_jobs.emplace(std::forward<F>(job));
        }
        m_cv.notify_one();
    }

    // Wait until all queued jobs are finished
    void Wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_finishedCV.wait(lock, [this]() {
            return m_jobs.empty() && m_activeWorkers == 0;
            });
    }

    size_t ThreadCount() const { return m_workers.size(); }

private:
    void WorkerLoop() {
        while (true) {
            std::function<void()> job;

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this]() {
                    return m_stop || !m_jobs.empty();
                    });

                if (m_stop && m_jobs.empty())
                    return;

                job = std::move(m_jobs.front());
                m_jobs.pop();
                ++m_activeWorkers;
            }

            // Execute job outside lock
            job();

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                --m_activeWorkers;

                if (m_jobs.empty() && m_activeWorkers == 0)
                    m_finishedCV.notify_all();
            }
        }
    }

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_jobs;

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable m_finishedCV;

    std::atomic<bool> m_stop = false;
    size_t m_activeWorkers = 0;
};