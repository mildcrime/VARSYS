#include "FmModule.h"
#include "core/Logger.h"

static const char* TAG = "Fm";

// Распространённые свободные частоты (единицы 10 кГц).
static const uint16_t PRESETS[] = { 8750, 9550, 10230, 10770 };
static const int PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);

FmModule* FmModule::_self = nullptr;

bool FmModule::init() {
    _self = this;
    // Wire уже поднят PowerModule (SDA 8 / SCL 18).
    _present = _radio.begin();
    if (!_present) { LOGW(TAG, "Si4713 not found"); return true; }
    LOGI(TAG, "Si4713 ready");
    return true;
}

void FmModule::setFreqKhz10(uint16_t f) {
    _freq = f;
    if (_present && _tx) {
        _radio.tuneFM(_freq);
    }
}

void FmModule::cyclePreset() {
    int idx = 0;
    for (int i = 0; i < PRESET_COUNT; ++i)
        if (PRESETS[i] == _freq) { idx = (i + 1) % PRESET_COUNT; break; }
    setFreqKhz10(PRESETS[idx]);
}

void FmModule::setTx(bool on) {
    if (!_present) return;
    _tx = on;
    if (on) {
        _radio.setTXpower(115);
        _radio.tuneFM(_freq);
    } else {
        _radio.setTXpower(0);
    }
    LOGI(TAG, "TX %s @ %u", on ? "on" : "off", _freq);
}
