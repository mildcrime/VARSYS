// ============================================================================
//  WebUiModule.h — Веб-интерфейс VARSYS (точка доступа + HTTP)
//
//  Поднимает свою Wi-Fi точку доступа и веб-сервер: статус устройства и
//  доступ к библиотеке записанных сигналов (просмотр/скачивание).
//  Включается/выключается с экрана (режим AP, отдельный от WiFi-скана).
// ============================================================================
#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "core/Module.h"

class WebUiModule : public IModule {
public:
    const char* name() const override { return "WebUi"; }
    bool init() override;
    void update(uint32_t now) override;

    static WebUiModule& instance() { return *_self; }

    void start();
    void stop();
    bool active() const { return _active; }
    String ssid() const { return _ssid; }
    String ip()   const { return _ip; }

private:
    static WebUiModule* _self;
    WebServer _server{80};
    bool   _active = false;
    String _ssid;
    String _ip;

    void routes();
    void handleRoot();
    void handleSignals();
    void handleDownload();
};
