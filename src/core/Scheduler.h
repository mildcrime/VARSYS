// ============================================================================
//  Scheduler.h — Лёгкий планировщик отложенных/периодических задач VARSYS
//
//  Позволяет модулям выполнять колбэки по таймеру, не заводя собственные
//  переменные millis(). Кооперативный: колбэки выполняются в главном цикле
//  (Scheduler::update() вызывается из Core::loop()), поэтому НЕ блокируют.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <functional>
#include <vector>

class Scheduler {
public:
    static Scheduler& instance();

    using Callback = std::function<void()>;

    // Периодический вызов каждые periodMs. Возвращает id для cancel().
    uint32_t every(uint32_t periodMs, Callback cb);

    // Одноразовый вызов через delayMs. Возвращает id.
    uint32_t after(uint32_t delayMs, Callback cb);

    // Отмена задачи по id.
    void cancel(uint32_t id);

    // Прокрутка таймеров. Вызывается из Core::loop().
    void update(uint32_t now);

private:
    Scheduler() = default;

    struct Task {
        uint32_t id;
        uint32_t period;
        uint32_t next;
        bool     repeat;
        bool     cancelled;
        Callback cb;
    };

    std::vector<Task> _tasks;
    uint32_t _nextId = 1;
};
