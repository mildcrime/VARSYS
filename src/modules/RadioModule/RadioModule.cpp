#include "RadioModule.h"
#include "core/Logger.h"
#include "core/Settings.h"
#include "ui/UIManager.h"
#include "modules/StorageModule/StorageModule.h"
#include "modules/RadioModule/RfDecoder.h"
#include "modules/RadioModule/RfBrute.h"
#include "hal/board_pins.h"

static const char* TAG = "Radio";

RadioModule* RadioModule::_self = nullptr;

// Распространённые частоты sub-GHz (кГц).
const uint32_t RadioModule::PRESETS[] = {
    315000, 390000, 433920, 434420, 868350, 915000
};
const size_t RadioModule::PRESET_COUNT =
    sizeof(RadioModule::PRESETS) / sizeof(RadioModule::PRESETS[0]);

bool RadioModule::init() {
    _self = this;

    // Используем тот же объект SPI-шины, что и дисплей (как в Bruce).
    if (!_cc.begin(&UIManager::instance().display().spi())) {
        LOGW(TAG, "CC1101 init failed — module disabled");
        return false;
    }

    _cc.setFrequencyKhz(Settings::instance().subghzFreqKhz());
    LOGI(TAG, "Radio ready @ %lu kHz", (unsigned long)_cc.frequencyKhz());
    return true;
}

void RadioModule::setFreqKhz(uint32_t khz) {
    _cc.setFrequencyKhz(khz);
    Settings::instance().setSubghzFreqKhz(_cc.frequencyKhz());
}

void RadioModule::cycleFreqPreset() {
    uint32_t cur = _cc.frequencyKhz();
    size_t idx = 0;
    for (size_t i = 0; i < PRESET_COUNT; ++i) {
        if (PRESETS[i] == cur) { idx = (i + 1) % PRESET_COUNT; break; }
    }
    setFreqKhz(PRESETS[idx]);
}

void RadioModule::listen() { _cc.enterRx(); }
void RadioModule::idle()   { _cc.enterIdle(); }

uint32_t RadioModule::scan() {
    uint32_t bestFreq = _cc.frequencyKhz();
    int bestRssi = -200;
    for (size_t i = 0; i < PRESET_COUNT; ++i) {
        _cc.setFrequencyKhz(PRESETS[i]);
        _cc.enterRx();
        delay(20);
        int r = _cc.rssiDbm();
        LOGD(TAG, "scan %lu kHz -> %d dBm", (unsigned long)PRESETS[i], r);
        if (r > bestRssi) { bestRssi = r; bestFreq = PRESETS[i]; }
    }
    setFreqKhz(bestFreq);
    _cc.enterRx();
    LOGI(TAG, "scan best: %lu kHz (%d dBm)", (unsigned long)bestFreq, bestRssi);
    return bestFreq;
}

int RadioModule::sweep(uint32_t startKhz, uint32_t stepKhz, int count, int* out) {
    if (!_cc.present()) { for (int i = 0; i < count; ++i) out[i] = -110; return 0; }
    uint32_t saved = _cc.frequencyKhz();
    int peak = 0, peakRssi = -200;
    for (int i = 0; i < count; ++i) {
        _cc.setFrequencyKhz(startKhz + (uint32_t)i * stepKhz);
        _cc.enterRx();
        delay(2);
        int r = _cc.rssiDbm();
        out[i] = r;
        if (r > peakRssi) { peakRssi = r; peak = i; }
    }
    _cc.setFrequencyKhz(saved);
    _cc.enterRx();
    return peak;
}

size_t RadioModule::recordRaw() {
    if (!_cc.present()) return 0;
    // Аппаратный захват (RMT): ждём фрейм до 3 с или 512 импульсов.
    size_t n = _cc.captureRaw(_lastCapture, _startHigh, 3000, 512);
    LOGI(TAG, "recorded %u pulses (start=%s)", (unsigned)n, _startHigh ? "H" : "L");
    _cc.enterRx();
    return n;
}

bool RadioModule::replayLast() {
    if (!_cc.present() || _lastCapture.empty()) return false;
    LOGI(TAG, "replaying %u pulses @ %lu kHz",
         (unsigned)_lastCapture.size(), (unsigned long)_cc.frequencyKhz());
    _cc.transmitRaw(_lastCapture, _startHigh);
    _cc.enterRx();
    return true;
}

String RadioModule::decodeLast() {
    RfDecoded d = rfDecode(_lastCapture);
    if (!d.ok) return String("RAW ") + (int)_lastCapture.size();
    char buf[48];
    snprintf(buf, sizeof(buf), "%s %dbit\nkey 0x%llX",
             d.proto, d.bits, (unsigned long long)d.key);
    return String(buf);
}

int RadioModule::bruteProtoCount() const { return BRUTE_PROTOCOL_COUNT; }
const char* RadioModule::bruteProtoName(int i) const {
    if (i == BRUTE_PROTOCOL_COUNT) return "All";
    if (i < 0 || i > BRUTE_PROTOCOL_COUNT) return "";
    return brute_protocols[i].name;
}

// Один кадр протокола: pilot + биты MSB-first + stop, повтор repeats раз.
void RadioModule::sendFrame(const BruteProtocol& p, int code, int repeats) {
    for (int r = 0; r < repeats; ++r) {
        if (p.pilot[0] || p.pilot[1]) { _cc.txPulse(p.pilot[0]); _cc.txPulse(p.pilot[1]); }
        for (int j = p.bits - 1; j >= 0; --j) {
            const int* t = ((code >> j) & 1) ? p.one : p.zero;
            _cc.txPulse(t[0]); _cc.txPulse(t[1]);
        }
        if (p.stop[0] || p.stop[1]) { _cc.txPulse(p.stop[0]); _cc.txPulse(p.stop[1]); }
    }
}

// Один протокол. Возвращает true, если прервано кнопкой «назад».
bool RadioModule::runBruteProto(int idx, int repeats) {
    const BruteProtocol& p = brute_protocols[idx];
    const int total = 1 << p.bits;

    for (int code = 0; code < total; ++code) {
        // Окно кандидатов (на случай реакции оператора) — последние 48 кодов.
        _bruteRecent.push_back(((uint32_t)idx << 16) | (uint32_t)code);
        if (_bruteRecent.size() > 48) _bruteRecent.erase(_bruteRecent.begin());

        sendFrame(p, code, repeats);
        _bruteSent++;

        if ((code & 0x0F) == 0) {                  // watchdog + прерывание
            delay(1);
            if (digitalRead(PIN_BTN_BACK) == LOW) return true;
        }
    }
    return false;
}

int RadioModule::replayCandidates(int repeats, BruteProgressCb cb) {
    if (!_cc.present() || _bruteRecent.empty()) return -1;
    const int total = (int)_bruteRecent.size();
    int confirmed = -1;

    _cc.txBegin();
    for (int i = 0; i < total; ++i) {
        uint32_t v = _bruteRecent[i];
        int idx = v >> 16, code = v & 0xFFFF;
        if (idx < 0 || idx >= BRUTE_PROTOCOL_COUNT) continue;
        if (cb) cb(i, total, brute_protocols[idx].name, code);

        sendFrame(brute_protocols[idx], code, repeats);

        // Пауза ~1.5 с, чтобы шлагбаум успел отреагировать; «назад» = подтвердить.
        bool stop = false;
        for (int t = 0; t < 150; ++t) {
            delay(10);
            if (digitalRead(PIN_BTN_BACK) == LOW) { stop = true; break; }
        }
        if (stop) { confirmed = i; break; }
    }
    _cc.txEnd();
    _cc.enterRx();

    if (confirmed >= 0) saveConfirmed(confirmed);
    return confirmed;
}

void RadioModule::saveConfirmed(int idx) {
    if (idx < 0 || idx >= (int)_bruteRecent.size()) return;
    StorageModule& st = StorageModule::instance();
    fs::FS* fs = st.fs();
    if (!fs) return;
    if (!fs->exists("/brute")) fs->mkdir("/brute");

    uint32_t v = _bruteRecent[idx];
    int pi = v >> 16, code = v & 0xFFFF;
    String path = String("/brute/confirmed_") + (millis() & 0xFFFFF) + ".sub";
    File f = fs->open(path, FILE_WRITE);
    if (!f) return;
    f.printf("Filetype: VARSYS Confirmed\nProtocol: %s\nKey: 0x%X\nFrequency: %lu\n",
             brute_protocols[pi].name, code,
             (unsigned long)_cc.frequencyKhz() * 1000UL);
    f.close();
    LOGI(TAG, "confirmed %s 0x%X -> %s", brute_protocols[pi].name, code, path.c_str());
}

int RadioModule::bruteforce(int protoIdx, int repeats) {
    if (!_cc.present()) return 0;
    if (repeats < 1) repeats = 1;
    _bruteRecent.clear();
    _bruteSent = 0;
    _bruteAborted = false;

    _cc.txBegin();
    if (protoIdx >= BRUTE_PROTOCOL_COUNT) {         // режим All
        LOGI(TAG, "bruteforce ALL x%d", repeats);
        for (int i = 0; i < BRUTE_PROTOCOL_COUNT; ++i)
            if (runBruteProto(i, repeats)) { _bruteAborted = true; break; }
    } else if (protoIdx >= 0) {
        LOGI(TAG, "bruteforce %s x%d", brute_protocols[protoIdx].name, repeats);
        _bruteAborted = runBruteProto(protoIdx, repeats);
    }
    _cc.txEnd();
    _cc.enterRx();

    if (_bruteAborted) saveBruteFound();           // автосохранение кандидатов
    LOGI(TAG, "bruteforce sent %d (aborted=%d)", _bruteSent, _bruteAborted);
    return _bruteSent;
}

void RadioModule::saveBruteFound() {
    if (_bruteRecent.empty()) return;
    StorageModule& st = StorageModule::instance();
    fs::FS* fs = st.fs();
    if (!fs) return;
    if (!fs->exists("/brute")) fs->mkdir("/brute");

    String path = String("/brute/found_") + (millis() & 0xFFFFF) + ".txt";
    File f = fs->open(path, FILE_WRITE);
    if (!f) return;
    f.printf("VARSYS brute candidates @ %lu kHz (newest last)\n",
             (unsigned long)_cc.frequencyKhz());
    for (uint32_t v : _bruteRecent) {
        int idx = v >> 16, code = v & 0xFFFF;
        f.printf("%s  0x%X (%d)\n", brute_protocols[idx].name, code, code);
    }
    f.close();
    LOGI(TAG, "saved %u candidates to %s", (unsigned)_bruteRecent.size(), path.c_str());
}

bool RadioModule::saveLast() {
    if (_lastCapture.empty()) return false;
    SignalRecord rec;
    rec.name      = String("sig_") + (millis() & 0xFFFFF);
    rec.freqKhz   = _cc.frequencyKhz();
    rec.preset    = "OOK";
    rec.startHigh = _startHigh;
    rec.pulses    = _lastCapture;
    bool ok = StorageModule::instance().saveSignal(rec);
    LOGI(TAG, "saveLast %s -> %d", rec.name.c_str(), ok);
    return ok;
}

bool RadioModule::loadSignal(const String& name) {
    SignalRecord rec;
    if (!StorageModule::instance().loadSignal(name, rec)) return false;
    _lastCapture = rec.pulses;
    _startHigh   = rec.startHigh;
    setFreqKhz(rec.freqKhz);
    if (_cc.present()) _cc.enterRx();
    LOGI(TAG, "loaded %s (%u pulses @ %lu kHz)",
         name.c_str(), (unsigned)_lastCapture.size(), (unsigned long)rec.freqKhz);
    return true;
}
