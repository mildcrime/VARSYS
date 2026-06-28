#include "StorageModule.h"
#include "core/Logger.h"
#include "hal/SpiBus.h"
#include "hal/board_pins.h"
#include "ui/UIManager.h"
#include <SD.h>
#include <LittleFS.h>

static const char* TAG = "Storage";
static const char* SIG_DIR = "/signals";

StorageModule* StorageModule::_self = nullptr;

bool StorageModule::init() {
    _self = this;

    bool lfs = LittleFS.begin(true);   // format on first run
    if (!lfs) LOGW(TAG, "LittleFS mount failed");

    // SD на общей шине дисплея (как в Bruce), CS 13.
    {
        hal::SpiBusGuard guard;
        _sd = SD.begin(PIN_SD_CS, UIManager::instance().display().spi(), 20000000);
    }

    if (_sd)       { _fs = &SD;       LOGI(TAG, "SD mounted"); }
    else if (lfs)  { _fs = &LittleFS; LOGI(TAG, "Using LittleFS (no SD)"); }
    else           { _fs = nullptr;   LOGE(TAG, "No filesystem available"); }

    if (_fs) ensureDir(SIG_DIR);
    return true;   // не фатально, если ФС нет
}

void StorageModule::ensureDir(const char* path) {
    if (!_fs) return;
    hal::SpiBusGuard guard;
    if (!_fs->exists(path)) _fs->mkdir(path);
}

bool StorageModule::appendLine(const String& path, const String& line) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;
    File f = _fs->open(path, FILE_APPEND);
    if (!f) { LOGE(TAG, "append open failed: %s", path.c_str()); return false; }
    f.println(line);
    f.close();
    return true;
}

bool StorageModule::writeFile(const String& path, const String& content) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;
    File f = _fs->open(path, FILE_WRITE);
    if (!f) { LOGE(TAG, "write open failed: %s", path.c_str()); return false; }
    f.print(content);
    f.close();
    return true;
}

String StorageModule::readFile(const String& path) {
    String out;
    if (!_fs) return out;
    hal::SpiBusGuard guard;
    File f = _fs->open(path, FILE_READ);
    if (!f) return out;
    out = f.readString();
    f.close();
    return out;
}

bool StorageModule::exists(const String& path) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;
    return _fs->exists(path);
}

std::vector<String> StorageModule::listDir(const String& dir, const char* ext) {
    std::vector<String> names;
    if (!_fs) return names;
    hal::SpiBusGuard guard;

    File d = _fs->open(dir);
    if (!d || !d.isDirectory()) return names;
    for (File e = d.openNextFile(); e; e = d.openNextFile()) {
        String n = e.name();
        int slash = n.lastIndexOf('/');
        if (slash >= 0) n = n.substring(slash + 1);
        if (!ext || n.endsWith(ext)) names.push_back(n);
        e.close();
    }
    d.close();
    return names;
}

bool StorageModule::saveSignal(const SignalRecord& rec) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;

    String path = String(SIG_DIR) + "/" + rec.name + ".sub";
    File f = _fs->open(path, FILE_WRITE);
    if (!f) { LOGE(TAG, "open for write failed: %s", path.c_str()); return false; }

    // Формат, совместимый с Flipper Zero (RAW .sub): файлы открываются и на
    // Flipper, и в нашей библиотеке. RAW_Data дробится на строки (≤512 значений).
    f.println("Filetype: Flipper SubGhz RAW File");
    f.println("Version: 1");
    f.printf("Frequency: %lu\n", (unsigned long)rec.freqKhz * 1000UL);
    f.println("Preset: FuriHalSubGhzPresetOok650Async");
    f.println("Protocol: RAW");

    bool high = rec.startHigh;
    int  col  = 0;
    for (size_t i = 0; i < rec.pulses.size(); ++i) {
        if (col == 0) f.print("RAW_Data:");
        f.printf(" %d", high ? (int)rec.pulses[i] : -(int)rec.pulses[i]);
        high = !high;
        if (++col >= 512) { f.println(); col = 0; }
    }
    if (col > 0) f.println();
    f.close();
    LOGI(TAG, "saved %s (%u pulses)", path.c_str(), (unsigned)rec.pulses.size());
    return true;
}

bool StorageModule::loadSignal(const String& name, SignalRecord& out) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;

    String path = String(SIG_DIR) + "/" + name;
    if (!path.endsWith(".sub")) path += ".sub";
    File f = _fs->open(path, FILE_READ);
    if (!f) return false;

    out = SignalRecord{};
    out.name = name;
    bool firstRaw = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.startsWith("Frequency:")) {
            out.freqKhz = line.substring(10).toInt() / 1000UL;
        } else if (line.startsWith("Preset:")) {
            out.preset = line.substring(7); out.preset.trim();
        } else if (line.startsWith("Start:")) {
            out.startHigh = line.indexOf('H') >= 0;
        } else if (line.startsWith("RAW_Data:")) {
            // Несколько строк RAW_Data (как у Flipper) накапливаются подряд.
            int i = 9;
            while (i < (int)line.length()) {
                while (i < (int)line.length() && line[i] == ' ') i++;
                int start = i;
                while (i < (int)line.length() && line[i] != ' ') i++;
                if (i > start) {
                    int v = line.substring(start, i).toInt();
                    // Знак ПЕРВОГО значения задаёт стартовый уровень.
                    if (firstRaw) { out.startHigh = (v >= 0); firstRaw = false; }
                    out.pulses.push_back((uint16_t)abs(v));
                }
            }
        }
    }
    f.close();
    return true;
}

std::vector<String> StorageModule::listSignals() {
    std::vector<String> names;
    if (!_fs) return names;
    hal::SpiBusGuard guard;

    File dir = _fs->open(SIG_DIR);
    if (!dir || !dir.isDirectory()) return names;
    for (File e = dir.openNextFile(); e; e = dir.openNextFile()) {
        String n = e.name();
        int slash = n.lastIndexOf('/');
        if (slash >= 0) n = n.substring(slash + 1);
        if (n.endsWith(".sub")) names.push_back(n);
        e.close();
    }
    dir.close();
    return names;
}
