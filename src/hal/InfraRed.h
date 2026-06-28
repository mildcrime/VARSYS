// ============================================================================
//  InfraRed.h — ИК приёмопередатчик на аппаратном RMT
//
//  RX (пин 1): сырой захват длительностей через RMT RX (приёмник отдаёт
//  демодулированную огибающую). TX (пин 2): воспроизведение с несущей 38 кГц
//  через RMT TX (carrier_en). Длительности хранятся «mark-first» (первый
//  импульс — засветка/несущая включена), уровни чередуются.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>

namespace hal {

class InfraRed {
public:
    void begin();

    // Сырой захват: длительности (мкс), начиная с mark. Блокирующий.
    size_t captureRaw(std::vector<uint16_t>& out, uint32_t timeoutMs,
                      size_t maxPulses);

    // Воспроизведение сырого сигнала с несущей carrierHz.
    void sendRaw(const std::vector<uint16_t>& pulses, uint32_t carrierHz = 38000);

    // Готовый код NEC (32 бита): адрес/команда.
    void sendNEC(uint8_t addr, uint8_t cmd);

private:
    bool _ready = false;
};

} // namespace hal
