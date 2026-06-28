#include "EventBus.h"

std::vector<EventBus::Subscription> EventBus::_subs;
std::vector<Event>                  EventBus::_deferred;

void EventBus::subscribe(EventType type, EventHandler handler) {
    _subs.push_back({type, std::move(handler)});
}

void EventBus::publish(const Event& event) {
    for (auto& sub : _subs) {
        if (sub.type == event.type && sub.handler) {
            sub.handler(event);
        }
    }
}

void EventBus::publish(EventType type, int32_t i32, void* ptr) {
    Event e;
    e.type = type;
    e.i32  = i32;
    e.ptr  = ptr;
    publish(e);
}

void EventBus::publishDeferred(EventType type, int32_t i32, void* ptr) {
    Event e;
    e.type = type;
    e.i32  = i32;
    e.ptr  = ptr;
    _deferred.push_back(e);
}

void EventBus::dispatchDeferred() {
    if (_deferred.empty()) return;
    // Забираем пачку, чтобы события, добавленные обработчиками, ушли в
    // следующий цикл (а не зациклили текущий).
    std::vector<Event> batch;
    batch.swap(_deferred);
    for (const auto& e : batch) publish(e);
}
