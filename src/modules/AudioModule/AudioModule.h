// ============================================================================
//  AudioModule.h — Звук через динамик NS4168 (I2S)
//
//  Тон/бип на встроенном I2S (без внешних библиотек). I2S ставится только на
//  время воспроизведения и снимается после — чтобы освободить пин WS (40),
//  который на этой плате совпадает с TFT_RST.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class AudioModule : public IModule {
public:
    const char* name() const override { return "Audio"; }
    bool init() override;

    static AudioModule& instance() { return *_self; }

    void tone(uint32_t freqHz, uint32_t ms);
    void beep();

private:
    static AudioModule* _self;
};
