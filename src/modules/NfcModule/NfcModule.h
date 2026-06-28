// ============================================================================
//  NfcModule.h — Подсистема NFC/RFID 13.56 МГц на PN532 (I2C)
//
//  PN532 подключён по I2C (SDA 8 / SCL 18, Wire поднимает PowerModule), IRQ 17,
//  RESET 45. Используем проверенную библиотеку Adafruit_PN532 (как Bruce —
//  опирается на готовый драйвер чипа). Чтение UID/типа метки ISO14443A.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_PN532.h>
#include "core/Module.h"
#include "hal/board_pins.h"

class NfcModule : public IModule {
public:
    const char* name() const override { return "Nfc"; }
    bool init() override;

    static NfcModule& instance() { return *_self; }

    bool present() const { return _present; }

    // Однократная попытка чтения метки. true — метка найдена.
    bool readTag(String& uidOut, String& typeOut, uint16_t timeoutMs = 500);

    // Запись блока Mifare Classic (auth ключом A по умолчанию FF..FF).
    bool writeBlock(uint8_t block, const uint8_t data[16]);

    const String& lastUid() const { return _lastUid; }

    // --- Полный дамп Mifare Classic 1K (16 секторов × 4 блока) ---
    static constexpr int kSectors = 16;
    static constexpr int kBlocks  = 64;
    struct ClassicDump {
        uint8_t uid[7]    = {0};
        uint8_t uidLen    = 0;
        uint8_t block[kBlocks][16] = {{0}};
        bool    blockOk[kBlocks]   = {false};
        uint8_t keyA[kSectors][6]  = {{0}};
        uint8_t keyB[kSectors][6]  = {{0}};
        bool    keyAok[kSectors]   = {false};
        bool    keyBok[kSectors]   = {false};
        int     blocksRead = 0;
    };

    // Полный дамп с подбором ключей по словарю. true — метка прочитана.
    bool dumpClassic(ClassicDump& out);
    // Сохранить дамп в /nfc/<uid>.dump (текстовый hex). pathOut — итоговый путь.
    bool saveDump(const ClassicDump& d, String& pathOut);
    // Загрузить дамп из /nfc/<name>.
    bool loadDump(const String& name, ClassicDump& d);
    // Записать дата-блоки дампа обратно на карту. Возврат — число записанных.
    int  cloneDump(const ClassicDump& d);

private:
    static NfcModule* _self;
    Adafruit_PN532 _nfc{PN532_IRQ, PN532_RF_REST};   // конструктор I2C
    bool   _present = false;
    String _lastUid;
};
