// ============================================================================
//  BadUsbModule.h — BadUSB (USB-HID клавиатура), встроенный USB ESP32-S3
//
//  Эмулирует USB-клавиатуру (TinyUSB) и исполняет Ducky-подобные скрипты.
//  Для авторизованного тестирования. Полные скрипты — из /ducky/*.txt (Storage).
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class BadUsbModule : public IModule {
public:
    const char* name() const override { return "Badusb"; }
    bool init() override;

    static BadUsbModule& instance() { return *_self; }

    // Выполнить Ducky-скрипт (многострочный). Поддержка: REM, DELAY, STRING,
    // STRINGLN, ENTER, GUI/WINDOWS, CTRL, ALT, SHIFT, TAB, ESC и комбинации.
    void runScript(const String& script);
    void runDemo();

private:
    static BadUsbModule* _self;
    void runLine(const String& line);
    bool _ready = false;
};
