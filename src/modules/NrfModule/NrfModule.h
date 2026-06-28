// ============================================================================
//  NrfModule.h — NRF24L01 анализатор 2.4 ГГц (RF24), как в Bruce
//
//  Внешний модуль на порт QWIIC (общая шина SPI, CE 43 / CSN 44). Сканер
//  несущей по 126 каналам (2400..2525 МГц). Глушилка — в разделе «Эксперт».
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class NrfModule : public IModule {
public:
    const char* name() const override { return "Nrf"; }
    bool init() override;

    static NrfModule& instance() { return *_self; }

    bool present() const { return _present; }
    static constexpr int kChannels = 126;

    void acquire();    // поднять RF24 на QWIIC (экран NRF/Mousejack)
    void release();    // освободить пины QWIIC

    void resetScan();
    void scanPass();                         // один проход по каналам
    const uint8_t* activity() const { return _activity; }

    // --- Mousejack (HID-инъекция в беспроводные мыши/клавиатуры) ---
    //  Только для авторизованного тестирования своих/разрешённых устройств.
    struct MjDevice { uint8_t addr[5]; uint8_t channel; };
    static constexpr int kMaxDevices = 12;

    // Promiscuous-поиск адресов уязвимых приёмников (best-effort, ~ms на канал).
    int  mjScan(uint32_t msPerSweep = 2000);
    int  mjCount() const { return _mjCount; }
    const MjDevice& mjDevice(int i) const { return _mj[i]; }

    // Найти активный канал устройства (ping с ACK). true + chOut при успехе.
    bool mjPing(const uint8_t addr[5], uint8_t& chOut);
    // Инъекция строки (US-раскладка) кадрами Logitech Unifying unencrypted.
    // Возврат — число успешно отправленных символов.
    int  mjInject(const MjDevice& d, const String& text);

private:
    static NrfModule* _self;
    bool    _present = false;
    bool    _begun   = false;
    uint8_t _activity[kChannels] = {0};

    void     mjBeginRx(uint8_t ch);                  // promiscuous-настройка RX
    bool     mjSendFrame(const uint8_t* buf, uint8_t len);
    MjDevice _mj[kMaxDevices];
    int      _mjCount = 0;
};
