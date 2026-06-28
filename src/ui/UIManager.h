// ============================================================================
//  UIManager.h — Модуль визуального интерфейса VARSYS
//
//  Отвечает за: инициализацию дисплея (HAL), привязку LVGL к панели,
//  регистрацию и переключение экранов, прокрутку lv_timer_handler().
//  Является модулем ядра (реализует IModule).
// ============================================================================
#pragma once
#include <lvgl.h>
#include <vector>
#include "core/Module.h"
#include "hal/Display.h"
#include "ui/Screen.h"
#include "ui/StatusOverlay.h"

class UIManager : public IModule {
public:
    // --- IModule ---
    const char* name() const override { return "UI"; }
    bool init() override;
    void start() override;
    void update(uint32_t now) override;
    void stop() override;

    // Регистрация экрана. Владение остаётся за UIManager после init().
    void addScreen(Screen* screen);

    // Переключение на экран верхнего уровня (сбрасывает историю навигации).
    // anim == LV_SCR_LOAD_ANIM_NONE — мгновенно (по умолчанию).
    bool setScreen(const char* name,
                   lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_NONE);

    // Открыть экран с заходом в стек истории (для кнопки «назад»).
    bool pushScreen(const char* name,
                    lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_MOVE_LEFT);

    // Вернуться на предыдущий экран из стека. true, если было куда вернуться.
    bool popScreen(lv_scr_load_anim_t anim = LV_SCR_LOAD_ANIM_MOVE_RIGHT);

    static UIManager& instance() { return *_self; }
    Display& display() { return _display; }

    // Пересобрать все экраны (после смены языка) и перезагрузить активный.
    void retranslateAll();

private:
    // Колбэк отрисовки LVGL -> панель.
    static void flushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* px);

    void buildScreens();        // создаёт стандартные экраны
    void loadScreen(Screen* s, lv_scr_load_anim_t anim);  // общий загрузчик
    Screen* findScreen(const char* name);

    static UIManager* _self;

    Display          _display;
    lv_disp_draw_buf_t _drawBuf;
    lv_disp_drv_t    _dispDrv;
    lv_color_t*      _buf1 = nullptr;
    lv_color_t*      _buf2 = nullptr;

    std::vector<Screen*> _screens;
    std::vector<Screen*> _navStack;     // история для кнопки «назад»
    Screen*  _active = nullptr;
    uint32_t _lastLvTick = 0;

    StatusOverlay _overlay;
};
