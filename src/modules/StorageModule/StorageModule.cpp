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

bool StorageModule::exists(const String& path) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;
    return _fs->exists(path);
}

bool StorageModule::saveSignal(const SignalRecord& rec) {
    if (!_fs) return false;
    hal::SpiBusGuard guard;

    String path = String(SIG_DIR) + "/" + rec.name + ".sub";
    File f = _fs->open(path, FILE_WRITE);
    if (!f) { LOGE(TAG, "open for write failed: %s", path.c_str()); return false; }

    f.println("Filetype: VARSYS Signal");
    f.printf("Frequency: %lu\n", (unsigned long)rec.freqKhz * 1000UL);
    f.printf("Preset: %s\n", rec.preset.c_str());
    f.printf("Start: %c\n", rec.startHigh ? 'H' : 'L');
    f.print("RAW_Data:");
    bool high = rec.startHigh;
    for (uint16_t d : rec.pulses) {
        f.printf(" %d", high ? (int)d : -(int)d);
        high = !high;
    }
    f.println();
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
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.startsWith("Frequency:")) {
            out.freqKhz = line.substring(10).toInt() / 1000UL;
        } else if (line.startsWith("Preset:")) {
            out.preset = line.substring(7); out.preset.trim();
        } else if (line.startsWith("Start:")) {
            out.startHigh = line.indexOf('H') >= 0;
        } else if (line.startsWith("RAW_Data:")) {
            int i = 9;
            while (i < (int)line.length()) {
                while (i < (int)line.length() && line[i] == ' ') i++;
                int start = i;
                while (i < (int)line.length() && line[i] != ' ') i++;
                if (i > start) {
                    int v = line.substring(start, i).toInt();
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
