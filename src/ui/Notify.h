// ============================================================================
//  Notify.h — Всплывающие уведомления (toast)
//
//  Глобальный тост на верхнем слое LVGL поверх всех экранов, авто-скрытие
//  по таймеру (Scheduler). Используется любыми модулями/экранами.
//    Notify::toast("Сохранено", Notify::Success);
// ============================================================================
#pragma once
#include <lvgl.h>

namespace Notify {

enum Type { Info, Success, Warn, Error };

void toast(const char* msg, Type type = Info, uint32_t ms = 1800);

} // namespace Notify
