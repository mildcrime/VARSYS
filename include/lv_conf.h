// ============================================================================
//  lv_conf.h — Конфигурация LVGL 8.x для VARSYS
//  Подключается через -DLV_CONF_INCLUDE_SIMPLE (см. platformio.ini)
// ============================================================================
#pragma once

#define LV_CONF_INCLUDE_SIMPLE 1

// --- Глубина цвета: ST7789 -> RGB565 ---
// Свап делает TFT_eSPI в pushColors(..., true), поэтому здесь 0.
#define LV_COLOR_DEPTH      16
#define LV_COLOR_16_SWAP    0

// --- Память LVGL (используем кучу ESP32, в т.ч. PSRAM) ---
#define LV_MEM_CUSTOM       1
#define LV_MEM_CUSTOM_INCLUDE   <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC     malloc
#define LV_MEM_CUSTOM_FREE      free
#define LV_MEM_CUSTOM_REALLOC   realloc

// --- Источник тиков: Arduino millis() ---
#define LV_TICK_CUSTOM      1
#define LV_TICK_CUSTOM_INCLUDE      "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

// --- Производительность / отрисовка ---
#define LV_DPI_DEF          130
#define LV_DRAW_COMPLEX     1

// --- Шрифты ---
// Встроенные Montserrat отключены: используем собственные шрифты VARSYS
// (Inter + кириллица + иконки Tabler), см. src/ui/fonts/.
#define LV_FONT_MONTSERRAT_12   0
#define LV_FONT_MONTSERRAT_14   0
#define LV_FONT_MONTSERRAT_16   0
#define LV_FONT_MONTSERRAT_22   0
#define LV_FONT_CUSTOM_DECLARE  LV_FONT_DECLARE(varsys_14)
#define LV_FONT_DEFAULT         &varsys_14

// --- Виджеты (минимально необходимый набор для этапа 1) ---
#define LV_USE_LABEL    1
#define LV_USE_BTN      1
#define LV_USE_LIST     1
#define LV_USE_BAR      1
#define LV_USE_ARC      1
#define LV_USE_IMG      1
#define LV_USE_LINE     1

// --- Логирование LVGL (по желанию) ---
#define LV_USE_LOG      0
