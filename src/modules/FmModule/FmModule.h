// ============================================================================
//  FmModule.h — FM-передатчик Si4713 (I2C), как в Bruce
//
//  Si4713 на общей шине I2C (SDA 8 / SCL 18). Маломощный FM-вещатель: выбор
//  частоты и включение/выключение передачи.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_Si4713.h>
#include "core/Module.h"

class FmModule : public IModule {
public:
    const char* name() const override { return "Fm"; }
    bool init() override;

    static FmModule& instance() { return *_self; }

    bool present() const { return _present; }
    uint16_t freqKhz10() const { return _freq; }      // в единицах 10 кГц
    void setFreqKhz10(uint16_t f);                     // напр. 10230 = 102.30 МГц
    void cyclePreset();
    bool txOn() const { return _tx; }
    void setTx(bool on);

private:
    static FmModule* _self;
    Adafruit_Si4713 _radio{(int8_t)-1};   // без отдельного reset-пина
    bool     _present = false;
    bool     _tx      = false;
    uint16_t _freq    = 8750;             // 87.50 МГц
};
