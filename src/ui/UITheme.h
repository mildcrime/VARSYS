// ============================================================================
//  UITheme.h — Тема оформления VARSYS в стиле iOS 17 (светлая, лаконичная)
//
//  Палитра и фабрики типовых элементов: фон экрана, статус-бар, шапка
//  с кнопкой «назад», белая карточка. Используется всеми экранами,
//  чтобы оформление было единым.
//
//  ПРИМЕЧАНИЕ: встроенные шрифты Montserrat в LVGL содержат только латиницу
//  и символы — поэтому подписи в прошивке на английском. Для кириллицы
//  потребуется собрать пользовательский шрифт (этап позже).
// ============================================================================
#pragma once
#include <lvgl.h>
#include "ui/fonts/varsys_fonts.h"   // шрифты varsys_* + макросы ICON_*

namespace ui {

// Текущий режим темы (true — тёмная). Устанавливается из Settings в UIManager.
extern bool darkMode;

// --- Палитра iOS, светлая/тёмная (RGB565 через lv_color_hex) ---
static inline lv_color_t cBg()    { return lv_color_hex(darkMode ? 0x000000 : 0xF2F2F7); }
static inline lv_color_t cCard()  { return lv_color_hex(darkMode ? 0x1C1C1E : 0xFFFFFF); }
static inline lv_color_t cText()  { return lv_color_hex(darkMode ? 0xFFFFFF : 0x1C1C1E); }
static inline lv_color_t cText2() { return lv_color_hex(0x8E8E93); }
static inline lv_color_t cSep()   { return lv_color_hex(darkMode ? 0x38383A : 0xD8D8DC); }
static inline lv_color_t cBlue()  { return lv_color_hex(darkMode ? 0x0A84FF : 0x007AFF); }
static inline lv_color_t cGreen() { return lv_color_hex(darkMode ? 0x30D158 : 0x34C759); }
static inline lv_color_t cOrange(){ return lv_color_hex(darkMode ? 0xFF9F0A : 0xFF9500); }
static inline lv_color_t cRed()   { return lv_color_hex(darkMode ? 0xFF453A : 0xFF3B30); }
static inline lv_color_t cGray()  { return lv_color_hex(0x8E8E93); }
static inline lv_color_t cTint()  { return lv_color_hex(darkMode ? 0x0A3A66 : 0xD6E7FB); }

// Базовое оформление экрана (фон, без прокрутки/полей).
void styleScreen(lv_obj_t* root);

// Верхний статус-бар: слева заголовок, справа время + батарея.
lv_obj_t* statusBar(lv_obj_t* parent, const char* title);

// Шапка детального экрана: «< Menu» слева, заголовок по центру, время справа.
lv_obj_t* header(lv_obj_t* parent, const char* title);

// Белая карточка iOS (скругление, без рамки, без прокрутки).
lv_obj_t* card(lv_obj_t* parent);

// Панель «нет нужного железа»: предупреждение + «Требуется: <hw>» +
// «подключите к <port>». Для экранов внешних модулей, когда железо не найдено.
// Возвращает созданную карточку (можно удалить при появлении железа).
lv_obj_t* hwMissingPanel(lv_obj_t* parent, const char* hw, const char* port);

} // namespace ui
