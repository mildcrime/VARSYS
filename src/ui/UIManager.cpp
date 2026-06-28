#include "UIManager.h"
#include "ui/UITheme.h"
#include "core/Logger.h"
#include "core/Settings.h"
#include "core/Scheduler.h"
#include "hal/SpiBus.h"
#include "varsys_config.h"
#include <string.h>

// Экраны
#include "ui/Splash.h"
#include "ui/screens/HomeScreen.h"
#include "ui/screens/SubGhzScreen.h"
#include "ui/screens/SettingsScreen.h"
#include "ui/screens/ExpertScreen.h"
#include "ui/screens/IrScreen.h"
#include "ui/screens/SavedSignalsScreen.h"
#include "ui/screens/NfcScreen.h"
#include "ui/screens/WifiScreen.h"
#include "ui/screens/BleScreen.h"
#include "ui/screens/SpectrumScreen.h"
#include "ui/screens/BadUsbScreen.h"
#include "ui/screens/WebUiScreen.h"
#include "ui/screens/GpsScreen.h"
#include "ui/screens/NrfScreen.h"
#include "ui/screens/PortalScreen.h"
#include "ui/screens/IButtonScreen.h"
#include "ui/screens/FmScreen.h"
#include "ui/screens/DevToolsScreen.h"
#include "ui/screens/BruteScreen.h"
#include "ui/screens/FilesScreen.h"
#include "ui/screens/BatteryScreen.h"

static const char* TAG = "UI";

UIManager* UIManager::_self = nullptr;

// Размер буфера отрисовки: 1/10 экрана на двойную буферизацию.
static constexpr uint32_t kBufLines = BOARD_TFT_HEIGHT / 10;

void UIManager::flushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* px) {
    auto* self = static_cast<UIManager*>(drv->user_data);
    const uint32_t w = area->x2 - area->x1 + 1;
    const uint32_t h = area->y2 - area->y1 + 1;

    // Захватываем общую шину SPI на время записи (делится с CC1101).
    hal::SpiBusGuard busGuard;
    TFT_eSPI& tft = self->_display.tft();
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(reinterpret_cast<uint16_t*>(px), w * h, true);  // swap = true
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

bool UIManager::init() {
    _self = this;

    if (!_display.begin()) {
        LOGE(TAG, "Display HAL init failed");
        return false;
    }

    // Применяем сохранённые настройки до настройки LVGL (важно для rotation,
    // т.к. от него зависят hor_res/ver_res буфера отрисовки).
    _display.setRotation(Settings::instance().rotation());
    _display.setBrightness(Settings::instance().brightness());

    lv_init();

    // Буферы отрисовки. Пытаемся разместить в PSRAM при наличии.
    const size_t bufPx = _display.width() * kBufLines;
    _buf1 = static_cast<lv_color_t*>(heap_caps_malloc(bufPx * sizeof(lv_color_t), MALLOC_CAP_DMA));
    _buf2 = static_cast<lv_color_t*>(heap_caps_malloc(bufPx * sizeof(lv_color_t), MALLOC_CAP_DMA));
    if (!_buf1) {
        LOGE(TAG, "Failed to alloc LVGL draw buffer");
        return false;
    }
    lv_disp_draw_buf_init(&_drawBuf, _buf1, _buf2, bufPx);

    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res   = _display.width();
    _dispDrv.ver_res   = _display.height();
    _dispDrv.flush_cb  = UIManager::flushCb;
    _dispDrv.draw_buf  = &_drawBuf;
    _dispDrv.user_data = this;
    lv_disp_drv_register(&_dispDrv);

    // Применяем сохранённую тему до создания экранов.
    ui::darkMode = Settings::instance().darkTheme();

    // Глобальный статус-бар (поверх всех экранов), скрыт до конца загрузки.
    _overlay.create();
    _overlay.setVisible(false);

    // Заставка-оверлей (всегда поверх всего, в т.ч. статус-бара).
    Splash::create();

    buildScreens();

    LOGI(TAG, "LVGL initialized (%dx%d), screens: %u",
         _display.width(), _display.height(), (unsigned)_screens.size());
    return true;
}

void UIManager::buildScreens() {
    addScreen(new HomeScreen());
    addScreen(new SubGhzScreen());
    addScreen(new SettingsScreen());
    addScreen(new ExpertScreen());
    addScreen(new IrScreen());
    addScreen(new SavedSignalsScreen());
    addScreen(new NfcScreen());
    addScreen(new WifiScreen());
    addScreen(new BleScreen());
    addScreen(new SpectrumScreen());
    addScreen(new BadUsbScreen());
    addScreen(new WebUiScreen());
    addScreen(new GpsScreen());
    addScreen(new NrfScreen());
    addScreen(new PortalScreen());
    addScreen(new IButtonScreen());
    addScreen(new FmScreen());
    addScreen(new DevToolsScreen());
    addScreen(new BruteScreen());
    addScreen(new FilesScreen());
    addScreen(new BatteryScreen());
}

void UIManager::addScreen(Screen* screen) {
    if (!screen) return;
    // Каждый экран — отдельный корневой объект, изначально скрыт.
    lv_obj_t* root = lv_obj_create(nullptr);
    screen->onCreate(root);
    _screens.push_back(screen);
}

Screen* UIManager::findScreen(const char* name) {
    for (auto* s : _screens) {
        if (strcmp(s->name(), name) == 0) return s;
    }
    return nullptr;
}

void UIManager::loadScreen(Screen* s, lv_scr_load_anim_t anim) {
    if (_active) _active->onHide();
    _active = s;
    if (anim == LV_SCR_LOAD_ANIM_NONE) {
        lv_scr_load(s->root());
    } else {
        // auto_del = false: экраны живут всё время работы прошивки.
        lv_scr_load_anim(s->root(), anim, 220, 0, false);
    }
    s->onShow();
    EventBus::publish(EventType::UI_SCREEN_CHANGED);
    LOGI(TAG, "Screen -> %s", s->name());
}

bool UIManager::setScreen(const char* name, lv_scr_load_anim_t anim) {
    Screen* s = findScreen(name);
    if (!s) { LOGW(TAG, "Screen '%s' not found", name); return false; }
    _navStack.clear();          // экран верхнего уровня — сброс истории
    loadScreen(s, anim);
    return true;
}

bool UIManager::pushScreen(const char* name, lv_scr_load_anim_t anim) {
    Screen* s = findScreen(name);
    if (!s) { LOGW(TAG, "Screen '%s' not found", name); return false; }
    if (_active) _navStack.push_back(_active);
    loadScreen(s, anim);
    return true;
}

bool UIManager::popScreen(lv_scr_load_anim_t anim) {
    if (_navStack.empty()) return false;
    Screen* s = _navStack.back();
    _navStack.pop_back();
    loadScreen(s, anim);
    return true;
}

void UIManager::start() {
    // Дом — стартовый экран; заставка-оверлей играет поверх него.
    setScreen("Home");
    Splash::play(2000);

    // Маршрутизация событий ввода к активному экрану.
    // Модуль ввода (этап 2) будет публиковать эти события в EventBus.
    auto forward = [](const Event& e) {
        Screen* s = UIManager::instance()._active;
        if (s) s->onEvent(e);
    };
    EventBus::subscribe(EventType::INPUT_ENCODER_CW,  forward);
    EventBus::subscribe(EventType::INPUT_ENCODER_CCW, forward);
    EventBus::subscribe(EventType::INPUT_BTN_CLICK,   forward);
    EventBus::subscribe(EventType::INPUT_BTN_LONG,    forward);

    // Кнопка «назад» и долгое нажатие энкодера — возврат по стеку навигации.
    auto goBack = [](const Event&) { UIManager::instance().popScreen(); };
    EventBus::subscribe(EventType::INPUT_BACK,     goBack);
    EventBus::subscribe(EventType::INPUT_BTN_LONG, goBack);

    // Смена языка или режима эксперта — пересобрать экраны.
    auto rebuild = [](const Event&) {
        ui::darkMode = Settings::instance().darkTheme();   // применить тему
        UIManager::instance().retranslateAll();
    };
    EventBus::subscribe(EventType::LANG_CHANGED, rebuild);
    EventBus::subscribe(EventType::UI_REBUILD,   rebuild);

    // Показать статус-бар после загрузки и обновлять его раз в секунду.
    EventBus::subscribe(EventType::SYS_BOOT_DONE, [](const Event&) {
        UIManager::instance()._overlay.setVisible(true);
    });
    Scheduler::instance().every(1000, [] {
        UIManager::instance()._overlay.update();
    });
}

void UIManager::retranslateAll() {
    for (auto* s : _screens) {
        lv_obj_clean(s->root());      // удалить виджеты, корень оставить
        s->onCreate(s->root());       // пересоздать с новыми строками
    }
    if (_active) {
        lv_scr_load(_active->root());
        _active->onShow();
    }
}

void UIManager::update(uint32_t now) {
    if (now - _lastLvTick >= VARSYS_LVGL_TICK_MS) {
        _lastLvTick = now;
        lv_timer_handler();
    }
    if (_active) _active->onUpdate(now);
}

void UIManager::stop() {
    if (_active) _active->onHide();
}
