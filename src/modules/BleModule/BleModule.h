// ============================================================================
//  BleModule.h — BLE-разведка VARSYS (NimBLE)
//
//  Активный скан BLE-устройств: имя, адрес, RSSI. HID-клавиатура (Ducky) и
//  массовый spam — отдельно (spam гейтится в разделе «Эксперт»).
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>
#include "core/Module.h"

struct BleDev {
    String name;
    String addr;
    int    rssi;
};

class BleModule : public IModule {
public:
    const char* name() const override { return "Ble"; }
    bool init() override;

    static BleModule& instance() { return *_self; }

    int scan(uint32_t seconds = 4);
    const std::vector<BleDev>& devices() const { return _devs; }

    void ensureReady();                      // лениво поднять контроллер BLE
    void radioOff();                         // выключить радио (экономия)

private:
    static BleModule* _self;
    std::vector<BleDev> _devs;
    bool _ready = false;
};
