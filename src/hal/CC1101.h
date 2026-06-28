// ============================================================================
//  CC1101.h — Драйвер sub-GHz трансивера CC1101 (HAL)
//
//  Работает на ОБЩЕЙ шине SPI с дисплеем (как в Bruce): использует ТОТ ЖЕ
//  объект Arduino SPIClass, что и TFT_eSPI (Display::spi()), разные CS.
//  Каждый доступ оборачивается в beginTransaction/endTransaction и
//  сериализуется через hal::SpiBusGuard.
//
//  Антенный переключатель SW0/SW1 переключается по диапазону частоты
//  (карта из проверенной прошивки Bruce для T-Embed CC1101).
//
//  ВНИМАНИЕ: конфигурация модема (ASK/OOK) и сырой захват/воспроизведение —
//  рабочая основа; финальную точность нужно проверить и подстроить на железе.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <vector>

namespace hal {

class CC1101 {
public:
    bool begin(SPIClass* spi);          // общий объект шины от дисплея
    bool present() const { return _present; }

    void     setFrequencyKhz(uint32_t khz);
    uint32_t frequencyKhz() const { return _khz; }

    int  rssiDbm();                     // текущий RSSI (в режиме RX)
    String diag();                      // дамп ключевых регистров (для dev-tools)

    void enterRx();                     // асинхронный приём (данные на GDO0)
    void enterIdle();

    // Сырой захват OOK через аппаратный RMT: длительности (мкс) импульсов на
    // GDO0 + стартовый уровень. Ждёт фрейм до timeoutMs / maxPulses.
    size_t captureRaw(std::vector<uint16_t>& out, bool& startHigh,
                      uint32_t timeoutMs, size_t maxPulses);

    // Воспроизведение сырого OOK на текущей частоте через RMT.
    void transmitRaw(const std::vector<uint16_t>& pulses, bool startHigh);

    // Низкоуровневая передача кадров (для перебора): подготовка TX, импульс
    // со знаком (мкс: >0 HIGH, <0 LOW), завершение. Бит-бэнг по GDO0.
    void txBegin();
    void txPulse(int durUs);
    void txEnd();

private:
    void reset();
    void baseConfig();                  // регистры ASK/OOK
    void applyAntenna(uint32_t khz);    // SW0/SW1 по диапазону

    void     strobe(uint8_t cmd);
    void     writeReg(uint8_t addr, uint8_t val);
    uint8_t  readReg(uint8_t addr);
    uint8_t  readStatusReg(uint8_t addr);

    SPIClass* _spi    = nullptr;        // общий с дисплеем объект шины
    uint32_t  _khz    = 433920;
    bool      _present = false;
    int       _antenna = -1;            // кэш текущего диапазона
};

} // namespace hal
