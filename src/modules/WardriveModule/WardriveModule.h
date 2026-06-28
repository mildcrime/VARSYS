// ============================================================================
//  WardriveModule.h — Wardriving: лог WiFi-точек с GPS-координатами в CSV
//
//  Периодически сканирует WiFi (WifiModule) и для каждой НОВОЙ точки (по BSSID)
//  пишет строку в CSV на SD/LittleFS с текущей позицией GPS (GpsModule).
//  Формат совместим с WiGLE (MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,Lat,Lon,...).
//  Только для авторизованного аудита радиоэфира.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <set>
#include "core/Module.h"

class WardriveModule : public IModule {
public:
    const char* name() const override { return "Wardrive"; }
    bool init() override;

    static WardriveModule& instance() { return *_self; }

    void activate();              // открыть новый CSV и начать лог (НЕ lifecycle start())
    void deactivate();            // остановить лог
    bool active() const { return _active; }

    uint32_t apCount() const { return _apCount; }   // уникальных точек за сессию
    const String& path() const { return _path; }

private:
    void tick();                  // один проход скан+лог
    static WardriveModule* _self;

    bool     _active  = false;
    uint32_t _task    = 0;
    uint32_t _apCount = 0;
    String   _path;
    std::set<String> _seen;       // уже записанные BSSID
};
