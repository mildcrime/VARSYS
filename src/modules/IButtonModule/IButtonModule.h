// ============================================================================
//  IButtonModule.h — iButton / Dallas 1-Wire (как в Bruce)
//
//  Внешний пробник на порт QWIIC (1-Wire data, пин 44). Чтение ROM-кода
//  ключа (8 байт: family + serial + CRC).
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class IButtonModule : public IModule {
public:
    const char* name() const override { return "IButton"; }
    bool init() override;

    static IButtonModule& instance() { return *_self; }

    bool readKey(String& out);            // true, если ключ приложен и валиден
    const String& lastKey() const { return _last; }

private:
    static IButtonModule* _self;
    String _last;
};
