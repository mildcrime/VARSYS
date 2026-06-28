#include "Display.h"
#include "core/Logger.h"
#include "varsys_config.h"

static const char* TAG = "Display";

// LEDC для подсветки.
static constexpr uint8_t  BL_CHANNEL = 7;
static constexpr uint32_t BL_FREQ    = 12000;
static constexpr uint8_t  BL_RES_BITS = 8;

bool Display::begin() {
    // Удержание питания платы — без этого панель/батарея отключатся.
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    _tft.init();
    _tft.setRotation(VARSYS_UI_ROTATION);
    _tft.fillScreen(TFT_BLACK);

    // Подсветка через LEDC (ШИМ-яркость).
    ledcSetup(BL_CHANNEL, BL_FREQ, BL_RES_BITS);
    ledcAttachPin(PIN_LCD_BL, BL_CHANNEL);
    setBrightness(VARSYS_UI_BRIGHTNESS);

    LOGI(TAG, "Panel ready: %dx%d", width(), height());
    return true;
}

void Display::setBrightness(uint8_t value) {
    ledcWrite(BL_CHANNEL, value);
}
