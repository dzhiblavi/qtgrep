#ifndef TASK_H
#define TASK_H

#include <functional>
#include <vector>
#include <thread>

#include "thread_pool.h"

class task {
protected:
    thread_pool& tp_;

public:
    task(thread_pool& tp);
    virtual ~task() = default;

    virtual void run() noexcept = 0;
    virtual void prepare() noexcept;
};

#endif // TASK_H
