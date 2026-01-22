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