#include "thread_pool.h"

thread_pool::thread_pool(size_t n_threads)
    : exit_(false)
{
    for (size_t id = 0; id < n_threads; ++id)
    {
        th_.emplace_back(std::thread([this]
        {
            for (;;)
            {
                std::unique_lock<std::mutex> lg(m_);
                work_cv_.wait(lg, [this]
                {
                    return !queue_.empty() || exit_;
                });

                if (exit_)
                {
                    break;
                }

                auto ts = queue_.back();
                queue_.pop_back();

                ts->canc_.store(false);
                ts->cancelled_ = [ts, this]() -> bool
                {
                    return ts->canc_.load() || cancel_.load();
                };

                lg.unlock();
                try {
                    ts->run();
                } catch (...) {
                    // ...
                }
                lg.lock();

                // ...
            }
        }));
    }
}

thread_pool::~thread_pool()
{
    cancel_.store(true);
    {
        std::unique_lock<std::mutex> lg(m_);
        exit_ = true;
    }
    work_cv_.notify_all();
    for (auto& t : th_)
    {
        t.join();
    }
}

void thread_pool::abort()
{
    {
        std::unique_lock<std::mutex> lg(m_);
        std::list<std::shared_ptr<task>>().swap(queue_);
    }
    cancel_.store(true);
}

void thread_pool::enqueue(std::shared_ptr<task> task_)
{
    cancel_.store(false);
    {
        std::unique_lock<std::mutex> lg(m_);
        queue_.push_front(task_);
    }
    work_cv_.notify_all();
}

size_t thread_pool::queue_size() const
{
    return queue_.size();
}



