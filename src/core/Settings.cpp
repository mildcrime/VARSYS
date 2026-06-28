#include "Settings.h"
#include "EventBus.h"
#include "Logger.h"
#include "varsys_config.h"

static const char* TAG = "Settings";
static const char* NS  = "varsys";   // пространство имён NVS

// Ключи NVS (макс. 15 символов).
static const char* K_BRIGHT = "bright";
static const char* K_ROT    = "rotation";
static const char* K_LANG   = "lang";
static const char* K_SOUND  = "sound";
static const char* K_VIBRO  = "vibro";
static const char* K_DARK   = "dark";
static const char* K_EXPERT = "expert";
static const char* K_FREQ   = "subghz_khz";
static const char* K_SLEEP  = "sleep_sec";

Settings& Settings::instance() {
    static Settings s;
    return s;
}

void Settings::begin() {
    _prefs.begin(NS, false);   // read/write

    _brightness = _prefs.getUChar(K_BRIGHT, VARSYS_UI_BRIGHTNESS);
    _rotation   = _prefs.getUChar(K_ROT,    VARSYS_UI_ROTATION);
    _lang       = (Lang)_prefs.getUChar(K_LANG, (uint8_t)Lang::EN);   // по умолчанию English
    _sound      = _prefs.getBool(K_SOUND, true);
    _vibro      = _prefs.getBool(K_VIBRO, false);
    _dark       = _prefs.getBool(K_DARK,  false);
    _expert     = _prefs.getBool(K_EXPERT, false);
    _subghzKhz  = _prefs.getUInt(K_FREQ,  433920);
    _sleepSec   = _prefs.getUShort(K_SLEEP, VARSYS_SLEEP_MS / 1000);

    LOGI(TAG, "Loaded: bright=%u rot=%u lang=%s sound=%d freq=%lukHz",
         _brightness, _rotation, _lang == Lang::EN ? "EN" : "RU",
         _sound, (unsigned long)_subghzKhz);
}

void Settings::setBrightness(uint8_t v) {
    if (v == _brightness) return;
    _brightness = v;
    _prefs.putUChar(K_BRIGHT, v);
    EventBus::publish(EventType::SETTINGS_CHANGED);
}

void Settings::setRotation(uint8_t v) {
    if (v == _rotation) return;
    _rotation = v;
    _prefs.putUChar(K_ROT, v);
    EventBus::publish(EventType::SETTINGS_CHANGED);
}

void Settings::setLanguage(Lang l) {
    if (l == _lang) return;
    _lang = l;
    _prefs.putUChar(K_LANG, (uint8_t)l);
    EventBus::publish(EventType::SETTINGS_CHANGED);
    // Отложенно: пересборка экранов произойдёт вне обработки ввода.
    EventBus::publishDeferred(EventType::LANG_CHANGED);
    LOGI(TAG, "Language -> %s", l == Lang::EN ? "EN" : "RU");
}

void Settings::toggleLanguage() {
    setLanguage(_lang == Lang::RU ? Lang::EN : Lang::RU);
}

void Settings::setSound(bool v) {
    if (v == _sound) return;
    _sound = v;
    _prefs.putBool(K_SOUND, v);
    EventBus::publish(EventType::SETTINGS_CHANGED);
}

void Settings::setVibro(bool v) {
    if (v == _vibro) return;
    _vibro = v;
    _prefs.putBool(K_VIBRO, v);
    EventBus::publish(EventType::SETTINGS_CHANGED);
}

void Settings::setDarkTheme(bool v) {
    if (v == _dark) return;
    _dark = v;
    _prefs.putBool(K_DARK, v);
    EventBus::publish(EventType::SETTINGS_CHANGED);
    EventBus::publishDeferred(EventType::UI_REBUILD);   // применить тему
}

void Settings::factoryReset() {
    _prefs.clear();
    delay(100);
    ESP.restart();
}

void Settings::setExpert(bool v) {
    if (v == _expert) return;
    _expert = v;
    _prefs.putBool(K_EXPERT, v);
    EventBus::publish(EventType::SETTINGS_CHANGED);
    // Отложенно: пересборка экранов покажет/скроет раздел «Эксперт».
    EventBus::publishDeferred(EventType::UI_REBUILD);
}

void Settings::setSubghzFreqKhz(uint32_t khz) {
    if (khz == _subghzKhz) return;
    _subghzKhz = khz;
    _prefs.putUInt(K_FREQ, khz);
    EventBus::publish(EventType::SETTINGS_CHANGED);
}

void Settings::setScreenTimeoutSec(uint16_t sec) {
    if (sec == _sleepSec) return;
    _sleepSec = sec;
    _prefs.putUShort(K_SLEEP, sec);
    EventBus::publish(EventType::SETTINGS_CHANGED);
}
