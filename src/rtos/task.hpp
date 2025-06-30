#pragma once
#include <functional>
#include <thread>
#include <atomic>

namespace rtos {

class Task {
public:
    using Job = std::function<void()>;

    explicit Task(Job job);
    ~Task();

    void start();
    void stop();

private:
    Job job_;
    std::thread worker_;
    std::atomic<bool> running_{false};
};

} // namespace rtos