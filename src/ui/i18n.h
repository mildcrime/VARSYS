// ============================================================================
//  i18n.h — Локализация интерфейса VARSYS (RU / EN)
//
//  Строки задаются идентификаторами StrId; tr(id) возвращает перевод для
//  текущего языка (Settings::language()). Смена языка публикует LANG_CHANGED,
//  по которому UIManager пересобирает экраны (см. UIManager::retranslateAll()).
//
//  Имена собственные (Sub-GHz, Bluetooth, Infrared) не переводятся — задаются
//  в экранах литералами.
// ============================================================================
#pragma once

enum StrId : int {
    STR_MENU = 0,       // «Меню» (кнопка назад)
    STR_FILES,
    STR_SETTINGS,
    STR_FREQUENCY,
    STR_SCAN,
    STR_RECORD,
    STR_SAVED,
    STR_SOUND,
    STR_VIBRO,
    STR_DARK,
    STR_BRIGHTNESS,
    STR_LANGUAGE,
    STR_ABOUT,
    STR_LANG_NAME,      // собственное название текущего языка
    STR_FREQ,           // «Частота» (строка списка)
    STR_REPLAY,
    STR_RECORDED,
    STR_NO_RADIO,
    STR_NO_SIGNAL,
    STR_LISTENING,
    STR_SENT,
    STR_EXPERT,         // название раздела/плитки
    STR_EXPERT_MODE,    // тумблер в настройках
    STR_EXPERT_WARN,    // предупреждение
    STR_JAMMER,
    STR_BLE_SPAM,
    STR_LAB_ONLY,       // «только для лаборатории»
    STR_CAPTURE,
    STR_SEND,
    STR_TV_OFF,
    STR_SAVE,
    STR_LIBRARY,
    STR_LOADED,
    STR_EMPTY,
    STR_READ,
    STR_NO_TAG,
    STR_NO_MODULE,
    STR_SCANNING,
    STR_DEAUTH,
    STR_STOP,
    STR_SPECTRUM,
    STR_RUN,
    STR_START,
    STR_BRUTE,
    STR_SCREEN_TIMEOUT, // таймаут гашения экрана
    STR_BATTERY,        // раздел/строка «Батарея»
    STR_OFF,            // «Выкл» (значение таймаута = никогда)
    STR_VOLTAGE,        // напряжение
    STR_CURRENT,        // ток
    STR_CHARGING,       // идёт зарядка
    STR_REQUIRES,       // «Требуется» (нет нужного железа)
    STR_CONNECT_TO,     // «подключите к» (порт)
    STR_COUNT
};

// Перевод по текущему языку из Settings.
const char* tr(StrId id);
