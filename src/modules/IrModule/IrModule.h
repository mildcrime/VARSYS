// ============================================================================
//  IrModule.h — Подсистема инфракрасного порта VARSYS
//
//  Обёртка над hal::InfraRed: захват/воспроизведение сырых сигналов, отправка
//  готовых кодов (NEC), заготовка универсального пульта (power-коды).
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>
#include "core/Module.h"
#include "hal/InfraRed.h"

class IrModule : public IModule {
public:
    const char* name() const override { return "Ir"; }
    bool init() override;

    static IrModule& instance() { return *_self; }

    size_t capture();                  // захват, число импульсов
    bool   replayLast();               // воспроизвести последний захват
    size_t lastPulseCount() const { return _last.size(); }

    void   sendNEC(uint8_t addr, uint8_t cmd) { _ir.sendNEC(addr, cmd); }
    void   sendTvOff();                // заготовка «выключить ТВ» (power-коды)

    // --- Универсальный пульт: одна функция -> sweep кодов топ-брендов ---
    enum UniFn : uint8_t { UNI_POWER, UNI_VOLUP, UNI_VOLDN, UNI_MUTE,
                           UNI_CHUP, UNI_CHDN, UNI_SOURCE, UNI_COUNT };
    int  sendUniversal(UniFn fn);              // число отправленных кодов
    static const char* uniName(UniFn fn);

private:
    static IrModule* _self;
    hal::InfraRed _ir;
    std::vector<uint16_t> _last;
};
