#include "LedModule.h"
#include "core/Logger.h"
#include "core/Scheduler.h"
#include "core/EventBus.h"
#include "hal/board_pins.h"
#include <FastLED.h>

static const char* TAG = "Led";

static CRGB s_leds[NEOPIXEL_COUNT];

LedModule* LedModule::_self = nullptr;

bool LedModule::init() {
    _self = this;
    FastLED.addLeds<WS2812B, PIN_NEOPIXEL, GRB>(s_leds, NEOPIXEL_COUNT);
    FastLED.setBrightness(40);
    off();

    // Зелёная вспышка по окончании загрузки.
    EventBus::subscribe(EventType::SYS_BOOT_DONE, [](const Event&) {
        LedModule::instance().flash(0, 120, 0, 400);
    });
    // Красная индикация при низком заряде.
    EventBus::subscribe(EventType::POWER_CHANGED, [](const Event& e) {
        if (e.i32 >= 0 && e.i32 <= 15) LedModule::instance().flash(120, 0, 0, 300);
    });

    LOGI(TAG, "RGB ready (%d LEDs)", NEOPIXEL_COUNT);
    return true;
}

void LedModule::fill(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < NEOPIXEL_COUNT; ++i) s_leds[i] = CRGB(r, g, b);
    FastLED.show();
}

void LedModule::off() {
    fill(0, 0, 0);
}

void LedModule::flash(uint8_t r, uint8_t g, uint8_t b, uint32_t ms) {
    fill(r, g, b);
    Scheduler::instance().after(ms, [] { LedModule::instance().off(); });
}
