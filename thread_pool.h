#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <thread>
#include <set>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <list>
#include <atomic>

class task;

class thread_pool {
public:
    thread_pool(size_t n_threads = 8);
    ~thread_pool();

    void enqueue(std::shared_ptr<task> task_);
    size_t queue_size() const noexcept;

    void abort();
    bool is_aborted() const noexcept;

private:
    void create_threads_(size_t n_threads);
    void stop_all_threads_();

    bool exit_;
    std::mutex m_;
    std::atomic_bool cancel_;
    std::condition_variable work_cv_;

    std::list<std::shared_ptr<task>> queue_;
    std::vector<std::thread> th_;
};

#endif // THREAD_POOL_H
