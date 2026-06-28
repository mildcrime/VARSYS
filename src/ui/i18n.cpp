#include "i18n.h"
#include "core/Settings.h"

// Порядок строк строго совпадает с enum StrId.
static const char* const RU[STR_COUNT] = {
    "Меню",         // STR_MENU
    "Файлы",        // STR_FILES
    "Настройки",    // STR_SETTINGS
    "ЧАСТОТА",      // STR_FREQUENCY
    "Сканировать",  // STR_SCAN
    "Записать",     // STR_RECORD
    "Сохранено",    // STR_SAVED
    "Звук",         // STR_SOUND
    "Вибро",        // STR_VIBRO
    "Тёмная",       // STR_DARK
    "Яркость",      // STR_BRIGHTNESS
    "Язык",         // STR_LANGUAGE
    "О системе",    // STR_ABOUT
    "Русский",      // STR_LANG_NAME
    "Частота",      // STR_FREQ
    "Воспроизвести",// STR_REPLAY
    "Записано",     // STR_RECORDED
    "Нет радио",    // STR_NO_RADIO
    "Нет сигнала",  // STR_NO_SIGNAL
    "Слушаю…",      // STR_LISTENING
    "Передано",     // STR_SENT
    "Эксперт",      // STR_EXPERT
    "Режим эксперта",// STR_EXPERT_MODE
    "Неизбирательно. Только авторизованная лаборатория.", // STR_EXPERT_WARN
    "Глушилка",     // STR_JAMMER
    "BLE-spam",     // STR_BLE_SPAM
    "Только лаборатория", // STR_LAB_ONLY
    "Захват",       // STR_CAPTURE
    "Отправить",    // STR_SEND
    "Выключить ТВ", // STR_TV_OFF
    "Сохранить",    // STR_SAVE
    "Библиотека",   // STR_LIBRARY
    "Загружено",    // STR_LOADED
    "Пусто",        // STR_EMPTY
    "Читать",       // STR_READ
    "Нет метки",    // STR_NO_TAG
    "Нет модуля",   // STR_NO_MODULE
    "Сканирую…",    // STR_SCANNING
    "Деаутент.",    // STR_DEAUTH
    "Стоп",         // STR_STOP
    "Спектр",       // STR_SPECTRUM
    "Запустить",    // STR_RUN
    "Старт",        // STR_START
    "Перебор",      // STR_BRUTE
    "Гашение экрана",// STR_SCREEN_TIMEOUT
    "Батарея",      // STR_BATTERY
    "Выкл",         // STR_OFF
    "Напряжение",   // STR_VOLTAGE
    "Ток",          // STR_CURRENT
    "Зарядка",      // STR_CHARGING
};

static const char* const EN[STR_COUNT] = {
    "Menu",         // STR_MENU
    "Files",        // STR_FILES
    "Settings",     // STR_SETTINGS
    "FREQUENCY",    // STR_FREQUENCY
    "Scan",         // STR_SCAN
    "Record",       // STR_RECORD
    "Saved",        // STR_SAVED
    "Sound",        // STR_SOUND
    "Vibration",    // STR_VIBRO
    "Dark",         // STR_DARK
    "Brightness",   // STR_BRIGHTNESS
    "Language",     // STR_LANGUAGE
    "About",        // STR_ABOUT
    "English",      // STR_LANG_NAME
    "Frequency",    // STR_FREQ
    "Replay",       // STR_REPLAY
    "Recorded",     // STR_RECORDED
    "No radio",     // STR_NO_RADIO
    "No signal",    // STR_NO_SIGNAL
    "Listening…",   // STR_LISTENING
    "Sent",         // STR_SENT
    "Expert",       // STR_EXPERT
    "Expert mode",  // STR_EXPERT_MODE
    "Indiscriminate. Authorized lab only.", // STR_EXPERT_WARN
    "Jammer",       // STR_JAMMER
    "BLE spam",     // STR_BLE_SPAM
    "Lab only",     // STR_LAB_ONLY
    "Capture",      // STR_CAPTURE
    "Send",         // STR_SEND
    "TV off",       // STR_TV_OFF
    "Save",         // STR_SAVE
    "Library",      // STR_LIBRARY
    "Loaded",       // STR_LOADED
    "Empty",        // STR_EMPTY
    "Read",         // STR_READ
    "No tag",       // STR_NO_TAG
    "No module",    // STR_NO_MODULE
    "Scanning…",    // STR_SCANNING
    "Deauth",       // STR_DEAUTH
    "Stop",         // STR_STOP
    "Spectrum",     // STR_SPECTRUM
    "Run",          // STR_RUN
    "Start",        // STR_START
    "Bruteforce",   // STR_BRUTE
    "Screen timeout",// STR_SCREEN_TIMEOUT
    "Battery",      // STR_BATTERY
    "Off",          // STR_OFF
    "Voltage",      // STR_VOLTAGE
    "Current",      // STR_CURRENT
    "Charging",     // STR_CHARGING
};

const char* tr(StrId id) {
    if (id < 0 || id >= STR_COUNT) return "";
    return (Settings::instance().language() == Lang::EN ? EN : RU)[id];
}
