//
// dzhiblavi
//

#ifndef TEST_GREPUI_FAULT_INJECTION_HPP
#define TEST_GREPUI_FAULT_INJECTION_HPP

#include <functional>

struct injected_fault : std::runtime_error {
    using std::runtime_error::runtime_error;
};

bool should_inject();
void injection_point();
void faulty_run(std::function<void()> const& func);

struct fault_injection_disable {
    fault_injection_disable();
    ~fault_injection_disable();

private:
    bool was_disabled;
};

#endif //TEST_GREPUI_FAULT_INJECTION_HPP