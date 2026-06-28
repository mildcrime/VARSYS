// ============================================================================
//  LedModule.h — Адресный RGB-светодиод (WS2812B x8, пин 14), FastLED
//
//  Встроенная лента из 8 диодов. Используется для индикации (загрузка, низкий
//  заряд, обратная связь). FastLED — как в Bruce.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class LedModule : public IModule {
public:
    const char* name() const override { return "Led"; }
    bool init() override;

    static LedModule& instance() { return *_self; }

    void fill(uint8_t r, uint8_t g, uint8_t b);
    void off();
    void flash(uint8_t r, uint8_t g, uint8_t b, uint32_t ms = 200);

    // Постоянная подсветка (цвет пункта меню). Хранится и восстанавливается
    // после вспышек. Учитывает вкл/выкл и яркость из Settings.
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    // Применить настройки (вкл/выкл, яркость) и перерисовать.
    void applySettings();

private:
    static LedModule* _self;
    void render();                 // вывести ambient (или чёрный, если выкл)
    uint8_t _ar = 0, _ag = 0, _ab = 0;   // текущий ambient-цвет
};
