// ============================================================================
//  PowerModule.h — Питание VARSYS: батарея, энергосбережение, выключение
//
//  - Читает топливомер BQ27220 по I2C (заряд %, напряжение, зарядка).
//  - Следит за бездействием: затемняет, затем гасит экран; пробуждает по вводу.
//  - Держит линию питания платы (PIN_POWER_ON) и умеет корректно выключать.
//
//  Данные батареи кэшируются и читаются статус-баром (StatusOverlay).
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class PowerModule : public IModule {
public:
    const char* name() const override { return "Power"; }
    bool init() override;
    void update(uint32_t now) override;

    static PowerModule& instance() { return *_self; }

    int      batteryPercent() const { return _pct; }   // -1 если недоступно
    uint16_t batteryMv()      const { return _mv; }
    int16_t  batteryMa()      const { return _ma; }    // >0 заряд, <0 разряд
    bool     charging()       const { return _charging; }

    // Корректное выключение устройства (отпускает линию питания).
    void powerOff();

private:
    void pollBattery();
    void noteActivity();
    void wake();

    static PowerModule* _self;

    int      _pct      = -1;
    uint16_t _mv       = 0;
    int16_t  _ma       = 0;
    bool     _charging = false;

    uint32_t _lastActivity = 0;
    bool     _dimmed   = false;
    bool     _screenOff = false;
    bool     _lowPower = false;   // CPU понижен до VARSYS_CPU_MHZ_IDLE
};
