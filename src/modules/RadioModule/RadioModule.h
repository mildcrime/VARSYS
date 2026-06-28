// ============================================================================
//  RadioModule.h — Подсистема Sub-GHz (CC1101) VARSYS
//
//  Высокоуровневая обёртка над hal::CC1101: частота (с сохранением в Settings),
//  RSSI, сканирование пресетов, сырой захват/воспроизведение OOK.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>
#include "core/Module.h"
#include "hal/CC1101.h"

struct BruteProtocol;   // forward (определён в RfBrute.h)

class RadioModule : public IModule {
public:
    const char* name() const override { return "Radio"; }
    bool init() override;

    static RadioModule& instance() { return *_self; }

    bool     present() const { return _cc.present(); }
    uint32_t freqKhz() const { return _cc.frequencyKhz(); }
    int      rssi()    { return _cc.rssiDbm(); }
    String   diag()    { return _cc.diag(); }

    void setFreqKhz(uint32_t khz);     // применить + сохранить в Settings
    void cycleFreqPreset();            // следующий пресет частоты

    void listen();                     // включить приём (для живого RSSI)
    void idle();

    uint32_t scan();                   // свип пресетов, вернуть лучшую частоту

    // Свип RSSI по диапазону для спектра: count точек от startKhz с шагом
    // stepKhz, результат (dBm) в out[]. Возвращает индекс пика.
    int sweep(uint32_t startKhz, uint32_t stepKhz, int count, int* out);

    size_t recordRaw();                // захват, вернуть число импульсов
    bool   replayLast();               // воспроизвести последний захват
    size_t lastPulseCount() const { return _lastCapture.size(); }

    String decodeLast();               // многопротокольный декодер OOK
    bool   saveLast();                 // сохранить захват в библиотеку (Storage)
    bool   loadSignal(const String& name);  // загрузить из библиотеки + частота
    // Полнокадровый перебор фикс-кодов. protoIdx == bruteProtoCount() — режим
    // «All» (все протоколы подряд). Блокирующий; прерывается кнопкой «назад».
    // При прерывании автосохраняет окно последних кодов как кандидатов.
    int    bruteforce(int protoIdx, int repeats = 2);
    int    bruteProtoCount() const;
    const char* bruteProtoName(int i) const;     // i == count -> "All"
    bool   bruteAborted() const { return _bruteAborted; }

    // Автопрогон сохранённых кандидатов: по одному с паузой; cb(idx,total,proto,code)
    // вызывается перед каждым. Прерывание «назад» подтверждает текущий код
    // (сохраняется отдельно). Возврат — индекс подтверждённого или -1.
    using BruteProgressCb = std::function<void(int, int, const char*, int)>;
    int    replayCandidates(int repeats, BruteProgressCb cb);
    size_t candidateCount() const { return _bruteRecent.size(); }

    static const uint32_t  PRESETS[];
    static const size_t    PRESET_COUNT;

private:
    static RadioModule* _self;
    hal::CC1101 _cc;
    std::vector<uint16_t> _lastCapture;
    bool _startHigh = true;

    // Перебор: окно последних кодов (proto<<16 | code) и состояние прерывания.
    bool runBruteProto(int idx, int repeats);    // true, если прервано
    void sendFrame(const BruteProtocol& p, int code, int repeats);
    void saveBruteFound();
    void saveConfirmed(int idx);
    std::vector<uint32_t> _bruteRecent;
    int  _bruteSent     = 0;
    bool _bruteAborted  = false;
};
