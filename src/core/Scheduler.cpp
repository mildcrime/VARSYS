#include "Scheduler.h"
#include <algorithm>

Scheduler& Scheduler::instance() {
    static Scheduler s;
    return s;
}

uint32_t Scheduler::every(uint32_t periodMs, Callback cb) {
    uint32_t id = _nextId++;
    _tasks.push_back({id, periodMs, millis() + periodMs, true, false, std::move(cb)});
    return id;
}

uint32_t Scheduler::after(uint32_t delayMs, Callback cb) {
    uint32_t id = _nextId++;
    _tasks.push_back({id, delayMs, millis() + delayMs, false, false, std::move(cb)});
    return id;
}

void Scheduler::cancel(uint32_t id) {
    for (auto& t : _tasks) {
        if (t.id == id) t.cancelled = true;
    }
}

void Scheduler::update(uint32_t now) {
    // Итерируемся по индексам: колбэк может добавить новые задачи (они
    // обработаются в следующем проходе), вектор не инвалидируется до erase.
    const size_t count = _tasks.size();
    for (size_t i = 0; i < count; ++i) {
        Task& t = _tasks[i];
        if (t.cancelled) continue;
        if ((int32_t)(now - t.next) >= 0) {       // wrap-safe сравнение времени
            t.cb();
            if (t.repeat) t.next = now + t.period;
            else          t.cancelled = true;
        }
    }
    // Уборка завершённых/отменённых.
    _tasks.erase(
        std::remove_if(_tasks.begin(), _tasks.end(),
                       [](const Task& t) { return t.cancelled; }),
        _tasks.end());
}
