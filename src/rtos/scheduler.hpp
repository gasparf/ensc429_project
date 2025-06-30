#pragma once
#include "task.hpp"
#include <vector>
#include <memory>

namespace rtos {

class Scheduler {
public:
    Scheduler() = default;
    ~Scheduler();

    template<class F>
    void addTask(F&& job) {
        tasks_.emplace_back(std::make_unique<Task>(std::forward<F>(job)));
    }

    void start();
    void stop();

private:
    std::vector<std::unique_ptr<Task>> tasks_;
};

} // namespace rtos