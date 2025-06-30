#include "scheduler.hpp"

namespace rtos {

Scheduler::~Scheduler() {
    stop();
}

void Scheduler::start() {
    for (auto& t : tasks_) {
        t->start();
    }
}

void Scheduler::stop() {
    for (auto& t : tasks_) {
        t->stop();
    }
}

} // namespace rtos