// ============================================================================
//  GpsModule.h — GPS-приёмник (TinyGPS++ по UART), как в Bruce
//
//  Внешний модуль на порт QWIIC (UART 43/44). Парсит NMEA, отдаёт фикс,
//  число спутников и координаты. Питается в update() из главного цикла.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class GpsModule : public IModule {
public:
    const char* name() const override { return "Gps"; }
    bool init() override;
    void update(uint32_t now) override;

    static GpsModule& instance() { return *_self; }

    void acquire();    // поднять UART на QWIIC (экран GPS / wardrive)
    void release();    // освободить пины QWIIC
    bool active() const { return _active; }

    bool   hasFix();
    int    sats();
    double lat();
    double lng();
    uint32_t charsRx();

private:
    static GpsModule* _self;
    bool _active = false;
};
