// ============================================================================
//  BadUsbModule.h — BadUSB (USB-HID клавиатура), встроенный USB ESP32-S3
//
//  Эмулирует USB-клавиатуру (TinyUSB) и исполняет Ducky-подобные скрипты.
//  Поддержка раскладок хоста (US/DE) через сырые HID-репорты. Скрипты — из
//  /ducky/*.txt (Storage). Для авторизованного тестирования.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class BadUsbModule : public IModule {
public:
    enum Layout : uint8_t { LAYOUT_US = 0, LAYOUT_DE = 1 };

    const char* name() const override { return "Badusb"; }
    bool init() override;

    static BadUsbModule& instance() { return *_self; }

    // Выполнить Ducky-скрипт (многострочный). Поддержка: REM, DELAY, STRING,
    // STRINGLN, ENTER, GUI/WINDOWS, CTRL, ALT, SHIFT, TAB, ESC и комбинации.
    void runScript(const String& script);
    bool runScriptFile(const String& name);   // из /ducky/<name>
    void runDemo();

    void   setLayout(Layout l) { _layout = l; }
    Layout layout() const { return _layout; }

private:
    static BadUsbModule* _self;
    void runLine(const String& line);
    void typeString(const String& s);
    void typeChar(char c);

    Layout _layout = LAYOUT_US;
    bool   _ready  = false;
};
