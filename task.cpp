#include "task.h"

task::task(thread_pool& tp)
    : tp_(tp)
{}

void task::prepare() noexcept {}
