// ============================================================================
//  NrfModule.h — NRF24L01 анализатор 2.4 ГГц (RF24), как в Bruce
//
//  Внешний модуль на порт QWIIC (общая шина SPI, CE 43 / CSN 44). Сканер
//  несущей по 126 каналам (2400..2525 МГц). Глушилка — в разделе «Эксперт».
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class NrfModule : public IModule {
public:
    const char* name() const override { return "Nrf"; }
    bool init() override;

    static NrfModule& instance() { return *_self; }

    bool present() const { return _present; }
    static constexpr int kChannels = 126;

    void resetScan();
    void scanPass();                         // один проход по каналам
    const uint8_t* activity() const { return _activity; }

private:
    static NrfModule* _self;
    bool    _present = false;
    uint8_t _activity[kChannels] = {0};
};
