#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <thread>
#include <set>
#include <queue>
#include <iostream>

#include <QObject>

#include "task.h"

class thread_pool // : QObject
{
private:
//    Q_OBJECT

public:
    thread_pool(size_t n_threads = 8);
    ~thread_pool();

    void enqueue(std::shared_ptr<task> task_);
    void abort();

    size_t queue_size() const;

private:
    bool exit_;
    std::mutex m_;
    std::atomic_bool cancel_;
    std::condition_variable work_cv_;

    std::list<std::shared_ptr<task>> queue_;
    std::vector<std::thread> th_;
};

#endif // THREAD_POOL_H
