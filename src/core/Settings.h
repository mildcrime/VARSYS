// ============================================================================
//  Settings.h — Рантайм-конфигурация VARSYS (персистентность в NVS)
//
//  Хранит пользовательские настройки между перезагрузками (яркость, ориентация,
//  язык, звук и т.д.) в энергонезависимой памяти ESP32 (NVS, Preferences).
//  Синглтон: инициализируется в Core::begin() до создания модулей, чтобы UI
//  поднимался уже с сохранёнными значениями.
//
//  Каждый сеттер сразу пишет в NVS и публикует событие SETTINGS_CHANGED
//  (а смена языка — ещё и LANG_CHANGED), чтобы UI мог отреагировать вживую.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <Preferences.h>

enum class Lang : uint8_t { RU = 0, EN = 1 };

class Settings {
public:
    static Settings& instance();

    // Загрузка значений из NVS (с дефолтами из varsys_config.h).
    void begin();

    uint8_t brightness() const { return _brightness; }
    void    setBrightness(uint8_t v);

    uint8_t rotation() const { return _rotation; }
    void    setRotation(uint8_t v);

    Lang    language() const { return _lang; }
    void    setLanguage(Lang l);
    void    toggleLanguage();

    bool    sound() const { return _sound; }
    void    setSound(bool v);

    bool    vibro() const { return _vibro; }
    void    setVibro(bool v);

    bool    darkTheme() const { return _dark; }
    void    setDarkTheme(bool v);

    // Экспертный режим: открывает раздел с неизбирательными инструментами
    // (глушилки, массовый BLE-spam). Выключен по умолчанию.
    bool    expert() const { return _expert; }
    void    setExpert(bool v);

    uint32_t subghzFreqKhz() const { return _subghzKhz; }
    void     setSubghzFreqKhz(uint32_t khz);

    void     factoryReset();           // очистить NVS и перезагрузиться

private:
    Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    Preferences _prefs;

    uint8_t  _brightness;
    uint8_t  _rotation;
    Lang     _lang;
    bool     _sound;
    bool     _vibro;
    bool     _dark;
    bool     _expert;
    uint32_t _subghzKhz;
};
