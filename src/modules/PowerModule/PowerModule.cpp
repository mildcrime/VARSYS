#include "PowerModule.h"
#include "core/EventBus.h"
#include "core/Logger.h"
#include "core/Scheduler.h"
#include "core/Settings.h"
#include "hal/board_pins.h"
#include "ui/UIManager.h"
#include "varsys_config.h"
#include <Wire.h>

static const char* TAG = "Power";

// Команды BQ27220 (стандартные регистры топливомера).
static constexpr uint8_t BQ_VOLTAGE = 0x08;   // мВ
static constexpr uint8_t BQ_CURRENT = 0x0C;   // мА (signed)
static constexpr uint8_t BQ_SOC     = 0x2C;   // заряд, %

PowerModule* PowerModule::_self = nullptr;

static bool bqRead16(uint8_t reg, uint16_t& out) {
    Wire.beginTransmission(BQ27220_I2C_ADDRESS);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)BQ27220_I2C_ADDRESS, 2) != 2) return false;
    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();
    out = (uint16_t)(hi << 8) | lo;
    return true;
}

bool PowerModule::init() {
    _self = this;

    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);   // удержание питания платы

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    _lastActivity = millis();

    // Любой ввод — признак активности (сброс таймеров энергосбережения).
    auto activity = [](const Event&) { PowerModule::instance().noteActivity(); };
    EventBus::subscribe(EventType::INPUT_ENCODER_CW,  activity);
    EventBus::subscribe(EventType::INPUT_ENCODER_CCW, activity);
    EventBus::subscribe(EventType::INPUT_BTN_CLICK,   activity);
    EventBus::subscribe(EventType::INPUT_BTN_LONG,    activity);
    EventBus::subscribe(EventType::INPUT_BACK,        activity);

    // Периодический опрос батареи.
    Scheduler::instance().every(VARSYS_BATTERY_POLL_MS, [] {
        PowerModule::instance().pollBattery();
    });
    pollBattery();

    LOGI(TAG, "Power ready (BQ27220 @0x%02X)", BQ27220_I2C_ADDRESS);
    return true;
}

void PowerModule::pollBattery() {
    uint16_t soc = 0, mv = 0, cur = 0;
    bool ok = bqRead16(BQ_SOC, soc);
    if (ok) {
        bqRead16(BQ_VOLTAGE, mv);
        bqRead16(BQ_CURRENT, cur);
        _pct = (soc > 100) ? 100 : (int)soc;
        _mv  = mv;
        _charging = ((int16_t)cur > 0);   // ток в плату = заряд
        EventBus::publish(EventType::POWER_CHANGED, _pct);
    } else {
        _pct = -1;   // топливомер недоступен
        LOGW(TAG, "BQ27220 not responding");
    }
}

void PowerModule::noteActivity() {
    _lastActivity = millis();
    if (_dimmed || _screenOff) wake();
}

void PowerModule::wake() {
    UIManager::instance().display().setBrightness(Settings::instance().brightness());
    _dimmed = false;
    _screenOff = false;
}

void PowerModule::update(uint32_t now) {
    const uint32_t idle = now - _lastActivity;

    if (!_screenOff && idle > VARSYS_SLEEP_MS) {
        UIManager::instance().display().setBrightness(0);
        _screenOff = true;
        LOGD(TAG, "Screen off (idle)");
    } else if (!_dimmed && idle > VARSYS_DIM_MS) {
        UIManager::instance().display().setBrightness(VARSYS_DIM_BRIGHTNESS);
        _dimmed = true;
        LOGD(TAG, "Screen dimmed (idle)");
    }
}

void PowerModule::powerOff() {
    LOGI(TAG, "Powering off");
    delay(50);
    digitalWrite(PIN_POWER_ON, LOW);   // отпускаем линию питания
}
