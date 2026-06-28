#include "WardriveModule.h"
#include "core/Logger.h"
#include "core/Scheduler.h"
#include "modules/WifiModule/WifiModule.h"
#include "modules/GpsModule/GpsModule.h"
#include "modules/StorageModule/StorageModule.h"

static const char* TAG  = "Wardrive";
static const char* PATH = "/wardrive.csv";

WardriveModule* WardriveModule::_self = nullptr;

bool WardriveModule::init() {
    _self = this;
    LOGI(TAG, "Wardrive ready");
    return true;
}

void WardriveModule::start() {
    if (_active) return;
    _path = PATH;
    _seen.clear();
    _apCount = 0;

    StorageModule& st = StorageModule::instance();
    // Заголовок пишем только в новый файл (совместимо с WiGLE CSV).
    if (!st.exists(_path)) {
        st.appendLine(_path, "WigleWifi-1.4,appRelease=VARSYS");
        st.appendLine(_path, "MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,"
                             "CurrentLatitude,CurrentLongitude,AltitudeMeters,"
                             "AccuracyMeters,Type");
    }

    _active = true;
    if (_task) Scheduler::instance().cancel(_task);
    _task = Scheduler::instance().every(4000, [this] { tick(); });
    tick();   // первый проход сразу
    LOGI(TAG, "wardrive started -> %s", _path.c_str());
}

void WardriveModule::stop() {
    if (!_active) return;
    _active = false;
    if (_task) { Scheduler::instance().cancel(_task); _task = 0; }
    WifiModule::instance().radioOff();   // экономия энергии
    LOGI(TAG, "wardrive stopped (%lu APs)", (unsigned long)_apCount);
}

void WardriveModule::tick() {
    if (!_active) return;

    WifiModule& wifi = WifiModule::instance();
    GpsModule&  gps  = GpsModule::instance();
    StorageModule& st = StorageModule::instance();

    wifi.scan();   // блокирующий (~2 с)

    const bool fix = gps.hasFix();
    const double lat = fix ? gps.lat() : 0.0;
    const double lon = fix ? gps.lng() : 0.0;

    for (const auto& ap : wifi.aps()) {
        char mac[18];
        snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                 ap.bssid[0], ap.bssid[1], ap.bssid[2],
                 ap.bssid[3], ap.bssid[4], ap.bssid[5]);
        String key(mac);
        if (_seen.count(key)) continue;   // уже записана в этой сессии
        _seen.insert(key);
        _apCount++;

        // SSID без запятых/переводов строк, чтобы не ломать CSV.
        String ssid = ap.ssid;
        ssid.replace(",", " ");
        ssid.replace("\n", " ");
        ssid.replace("\r", " ");

        String line = key + "," + ssid + ","
                    + (ap.locked ? "[WPA2]" : "[OPEN]") + ","
                    + String((unsigned long)(millis() / 1000)) + ","
                    + String((int)ap.channel) + ","
                    + String((int)ap.rssi) + ","
                    + String(lat, 6) + "," + String(lon, 6) + ",0,"
                    + (fix ? "5" : "0") + ",WIFI";
        st.appendLine(_path, line);
    }
}
