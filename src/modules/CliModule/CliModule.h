// ============================================================================
//  CliModule.h — Командная строка по Serial (USB-CDC)
//
//  Управление прошивкой текстовыми командами (как serial-интерфейс Bruce):
//  help, ver, freq, rssi, rec, replay, ir, wifi, ble, reboot.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class CliModule : public IModule {
public:
    const char* name() const override { return "Cli"; }
    bool init() override;
    void update(uint32_t now) override;

private:
    void exec(const String& line);
    String _buf;
};
