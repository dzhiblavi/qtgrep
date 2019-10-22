#include "factorization_task.h"

factorization_task::factorization_task(uint64_t n)
    : n_(n)
{}

void factorization_task::run()
{
    uint64_t num = n_;

    {
        std::unique_lock<std::mutex> lg(m_);
        incomplete_ = true;
    }
    if (num == 1)
    {
        {
            std::unique_lock<std::mutex> lg(m_);
            incomplete_ = false;
        }
        return;
    }

    for (uint64_t i = 2; i <= num; ++i)
    {
        if (is_cancelled())
        {
            return;
        }
        while (num % i == 0)
        {
            if (is_cancelled())
            {
                return;
            }
            {
                std::unique_lock<std::mutex> lg(m_);
                res_.push_back(i);
            }
            num /= i;
        }
    }
    if (num != 1)
    {
        {
            std::unique_lock<std::mutex> lg(m_);
            res_.push_back(num);
        }
    }
    {
        std::unique_lock<std::mutex> lg(m_);
        incomplete_ = false;
    }
}

std::pair<bool, std::vector<uint64_t>> factorization_task::get_result() const
{
    std::unique_lock<std::mutex> lg(m_);
    return { incomplete_, res_ };
}

void factorization_task::prepare()
{
    canc_.store(false);
    res_.clear();
    incomplete_ = true;
}
