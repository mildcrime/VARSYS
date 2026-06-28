// ============================================================================
//  StorageModule.h — Хранилище VARSYS (SD + LittleFS) и библиотека сигналов
//
//  SD-карта на ОБЩЕЙ шине SPI (CS 13), доступ под hal::SpiBusGuard. LittleFS —
//  во внутренней flash (раздел spiffs). Предоставляет файловый доступ и
//  каталог записанных сигналов (Sub-GHz/ИК) с метаданными.
//
//  Формат сигнала — текстовый, .sub-подобный (совместим по духу с Flipper):
//    Filetype / Frequency / Preset / Start / RAW_Data (знаковые длительности).
// ============================================================================
#pragma once
#include <Arduino.h>
#include <FS.h>
#include <vector>
#include "core/Module.h"

struct SignalRecord {
    String   name;
    uint32_t freqKhz   = 433920;
    String   preset    = "OOK";
    bool     startHigh = true;
    std::vector<uint16_t> pulses;   // длительности (мкс), уровни чередуются
};

class StorageModule : public IModule {
public:
    const char* name() const override { return "Storage"; }
    bool init() override;

    static StorageModule& instance() { return *_self; }

    bool sdMounted()  const { return _sd; }
    bool fsReady()    const { return _fs != nullptr; }

    // Активная ФС: SD при наличии, иначе LittleFS.
    fs::FS* fs() { return _fs; }

    // Дозапись строки в файл (создаёт при отсутствии). Доступ к SD на общей
    // шине обёрнут SpiBusGuard. Для CSV-логов (wardriving и т.п.).
    bool appendLine(const String& path, const String& line);
    bool exists(const String& path);
    // Список имён файлов в каталоге (опц. фильтр по расширению, напр. ".txt").
    std::vector<String> listDir(const String& dir, const char* ext = nullptr);

    // --- Библиотека сигналов ---
    bool saveSignal(const SignalRecord& rec);
    bool loadSignal(const String& name, SignalRecord& out);
    std::vector<String> listSignals();

private:
    static StorageModule* _self;
    bool    _sd = false;
    fs::FS* _fs = nullptr;     // указывает на SD или LittleFS
    void ensureDir(const char* path);
};
