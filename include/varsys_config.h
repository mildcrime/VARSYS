// ============================================================================
//  varsys_config.h — Глобальная конфигурация прошивки VARSYS
// ============================================================================
#pragma once

#ifndef VARSYS_VERSION
#define VARSYS_VERSION "0.0.0-dev"
#endif

#define VARSYS_NAME             "VARSYS"

// --- Тайминги планировщика ---
#define VARSYS_TICK_MS          5       // Период вызова Core::update() (мс)
#define VARSYS_LVGL_TICK_MS     5       // Период lv_timer_handler()

// --- Логирование ---
#define VARSYS_LOG_ENABLED      1
#define VARSYS_LOG_BAUD         115200

// --- UI ---
#define VARSYS_UI_ROTATION      3       // landscape 320x170 (как в Bruce на этой плате)
#define VARSYS_UI_BRIGHTNESS    200     // Стартовая яркость подсветки (0..255)
#define VARSYS_BOOT_SPLASH_MS   1200    // Длительность загрузочного экрана

// --- Питание / энергосбережение ---
#define VARSYS_BATTERY_POLL_MS  5000    // Период опроса топливомера BQ27220
#define VARSYS_DIM_MS           20000   // Бездействие до затемнения экрана
#define VARSYS_SLEEP_MS         60000   // Бездействие до выключения экрана
#define VARSYS_DIM_BRIGHTNESS   30      // Яркость в затемнённом режиме (0..255)

// --- Ввод (энкодер + кнопка) ---
#define VARSYS_ENCODER_REVERSED 0       // 1 — инвертировать направление вращения
#define VARSYS_BTN_ACTIVE_LOW   1       // Кнопка энкодера: нажата = LOW (с подтяжкой)
#define VARSYS_BTN_DEBOUNCE_MS  25      // Антидребезг кнопки (мс)
#define VARSYS_BTN_LONGPRESS_MS 600     // Порог долгого нажатия (мс)
