#ifndef TASK_H
#define TASK_H

#include <functional>
#include <vector>
#include <thread>

class thread_pool;

class task {
    friend class thread_pool;

public:
    virtual ~task() = default;
    void cancel();
    bool is_cancelled() const;

protected:
    std::atomic_bool canc_;

private:
    virtual void run() = 0;
    virtual void prepare() {}

    std::function<bool()> cancelled_;
};

#endif // TASK_H
