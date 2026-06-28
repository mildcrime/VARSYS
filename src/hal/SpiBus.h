// ============================================================================
//  SpiBus.h — Арбитраж общей шины SPI (дисплей + SD + CC1101)
//
//  На T-Embed CC1101 дисплей, SD-карта и радио сидят на одной шине SPI.
//  Любой доступ к шине (отрисовка LVGL, чтение CC1101, работа с SD) обязан
//  выполняться под блокировкой, иначе транзакции наложатся и собьются —
//  особенно когда радио уйдёт в фоновую FreeRTOS-задачу (этап 3+).
//
//  Используйте RAII-обёртку:
//      { hal::SpiBusGuard guard; ... доступ к SPI ... }
// ============================================================================
#pragma once

namespace hal {

// Создать мьютекс шины. Вызывается один раз в Core::begin() до старта модулей.
void spiBusInit();

void spiBusLock();
void spiBusUnlock();

// RAII: захватывает шину на время жизни объекта.
struct SpiBusGuard {
    SpiBusGuard()  { spiBusLock(); }
    ~SpiBusGuard() { spiBusUnlock(); }
    SpiBusGuard(const SpiBusGuard&) = delete;
    SpiBusGuard& operator=(const SpiBusGuard&) = delete;
};

} // namespace hal
