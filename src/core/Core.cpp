#include "Core.h"
#include "Logger.h"
#include "EventBus.h"
#include "Settings.h"
#include "Scheduler.h"
#include "Clock.h"
#include "hal/SpiBus.h"
#include "varsys_config.h"

// --- Подключаемые модули системы ---
#include "modules/InputModule/InputModule.h"
#include "modules/PowerModule/PowerModule.h"
#include "modules/RadioModule/RadioModule.h"
#include "modules/StorageModule/StorageModule.h"
#include "modules/IrModule/IrModule.h"
#include "modules/NfcModule/NfcModule.h"
#include "modules/WifiModule/WifiModule.h"
#include "modules/BleModule/BleModule.h"
#include "modules/BadUsbModule/BadUsbModule.h"
#include "modules/CliModule/CliModule.h"
#include "modules/WebUiModule/WebUiModule.h"
#include "modules/LedModule/LedModule.h"
#include "modules/GpsModule/GpsModule.h"
#include "modules/NrfModule/NrfModule.h"
#include "modules/AudioModule/AudioModule.h"
#include "modules/EvilPortalModule/EvilPortalModule.h"
#include "modules/IButtonModule/IButtonModule.h"
#include "modules/FmModule/FmModule.h"
#include "ui/UIManager.h"
// На следующих этапах сюда добавятся: IrModule, WifiModule и т.д.

static const char* TAG = "Core";

// Статические экземпляры модулей (живут всё время работы прошивки).
static InputModule   s_input;
static UIManager     s_ui;
static StorageModule s_storage;
static PowerModule   s_power;
static RadioModule   s_radio;
static IrModule      s_ir;
static NfcModule     s_nfc;
static WifiModule    s_wifi;
static BleModule     s_ble;
static BadUsbModule  s_badusb;
static CliModule     s_cli;
static WebUiModule   s_webui;
static LedModule     s_led;
static GpsModule     s_gps;
static NrfModule     s_nrf;
static AudioModule   s_audio;
static EvilPortalModule s_portal;
static IButtonModule s_ibutton;
static FmModule      s_fm;

Core& Core::instance() {
    static Core core;
    return core;
}

void Core::registerModules() {
    // Порядок регистрации = порядок инициализации и обновления.
    // Ввод раньше UI, чтобы события вращения/нажатия обрабатывались
    // активным экраном в том же тике главного цикла. Power после UI —
    // его димминг обращается к дисплею через UIManager.
    _modules.add(&s_input);
    _modules.add(&s_ui);
    // Storage и Radio после UI: используют шину SPI дисплея (Display::begin).
    _modules.add(&s_storage);
    _modules.add(&s_power);
    _modules.add(&s_radio);
    _modules.add(&s_ir);
    _modules.add(&s_nfc);   // PN532 по I2C (Wire поднят PowerModule)
    _modules.add(&s_wifi);
    _modules.add(&s_ble);
    _modules.add(&s_badusb);
    _modules.add(&s_cli);
    _modules.add(&s_webui);
    _modules.add(&s_led);
    _modules.add(&s_gps);
    _modules.add(&s_nrf);
    _modules.add(&s_audio);
    _modules.add(&s_portal);
    _modules.add(&s_ibutton);
    _modules.add(&s_fm);
}

void Core::begin() {
    Logger::begin();
    Logger::setLevel(LogLevel::DEBUG);

    LOGI(TAG, "==============================================");
    LOGI(TAG, " %s firmware v%s", VARSYS_NAME, VARSYS_VERSION);
    LOGI(TAG, " Target: T-Embed CC1101 Plus S3 (ESP32-S3)");
    LOGI(TAG, "==============================================");

    // Базовые сервисы до модулей: арбитраж шины, часы, настройки.
    hal::spiBusInit();
    Clock::begin();
    // Рантайм-настройки грузим до модулей: UI поднимется уже с сохранёнными
    // яркостью, ориентацией и языком.
    Settings::instance().begin();

    registerModules();

    if (_modules.initAll()) {
        LOGI(TAG, "All modules initialized (%u)", (unsigned)_modules.count());
    } else {
        LOGW(TAG, "Some modules failed to init — running degraded");
    }

    _modules.startAll();
    _booted = true;
    EventBus::publish(EventType::SYS_BOOT_DONE);
    LOGI(TAG, "Boot complete");
}

void Core::loop() {
    const uint32_t now = millis();
    if (now - _lastTick >= VARSYS_TICK_MS) {
        _lastTick = now;
        EventBus::dispatchDeferred();        // доставка отложенных событий
        Scheduler::instance().update(now);   // таймеры
        _modules.updateAll(now);
    }
}
