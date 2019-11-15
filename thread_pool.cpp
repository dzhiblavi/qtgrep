#include "thread_pool.h"
#include "task.h"

std::mutex outm;

thread_pool::thread_pool(size_t n_threads)
    : exit_(false)
    , cancel_(false) {
    create_threads_(n_threads);
}

// basic
void thread_pool::create_threads_(size_t n_threads) {
    th_.clear();
    try {
        for (size_t id = 0; id < n_threads; ++id) {
            th_.emplace_back([this, id] {
                for (;;) {
                    std::unique_lock<std::mutex> lg(m_);
                    work_cv_.wait(lg, [this] {
                        return !queue_.empty() || exit_;
                    });
                    if (exit_) {
                        break;
                    }
                    auto ts = queue_.back();
                    queue_.pop_back();
                    lg.unlock();
                    try {
                        ts->run();
                    } catch (...) {
                        // ...
                    }
                    lg.lock();
                }
            });
        }
    } catch (...) {
        stop_all_threads_();
        th_.clear();
        throw;
    }
}

void thread_pool::stop_all_threads_() noexcept {
    cancel_.store(true);
    {
        std::unique_lock<std::mutex> lg(m_);
        exit_ = true;
    }
    work_cv_.notify_all();
    for (auto& t : th_) {
        t.join();
    }
}

thread_pool::~thread_pool() {
    stop_all_threads_();
}

void thread_pool::abort() {
    stop_all_threads_();
    cancel_.store(false);
    {
        std::unique_lock<std::mutex> lg(m_);
        queue_.clear();
        exit_ = false;
    }
    create_threads_(th_.size());
}

bool thread_pool::is_aborted() const noexcept {
    return cancel_.load();
}

void thread_pool::enqueue(std::shared_ptr<task> const& task) {
    task->prepare();
    {
        std::unique_lock<std::mutex> lg(m_);
        queue_.push_front(task);
    }
    work_cv_.notify_all();
}

size_t thread_pool::queue_size() const noexcept {
    return queue_.size();
}



