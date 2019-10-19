#ifndef TASK_H
#define TASK_H

#include <functional>
#include <vector>
#include <thread>

class thread_pool;

class task
{
    friend class thread_pool;

public:
    virtual ~task() = default;

    void cancel();

protected:
    bool is_cancelled() const;

private:
    virtual void run() = 0;

    std::atomic_bool canc_;
    std::function<bool()> cancelled_;
};

#endif // TASK_H
