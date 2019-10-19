#ifndef FACTORIZATION_TASK_H
#define FACTORIZATION_TASK_H

#include "task.h"

class factorization_task : public task
{
public:
    factorization_task(uint64_t n);
    std::pair<bool, std::vector<uint64_t>> get_result() const;

private:
    void run() override;

    mutable std::mutex m_;
    uint64_t n_;
    bool incomplete_ = false;
    std::vector<uint64_t> res_;
};


#endif // FACTORIZATION_TASK_H
