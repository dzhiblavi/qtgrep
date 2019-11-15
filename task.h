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

    virtual void run() = 0;
    virtual void prepare();
};

#endif // TASK_H
