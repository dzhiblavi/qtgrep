#include "task.h"

void task::cancel()
{
    canc_.store(true);
}

bool task::is_cancelled() const
{
    return cancelled_();
}
