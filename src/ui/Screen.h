// ============================================================================
//  Screen.h — Базовый класс экрана UI (LVGL)
//
//  Каждый экран владеет собственным корневым объектом LVGL (lv_obj_t root).
//  UIManager управляет жизненным циклом и переключением экранов.
// ============================================================================
#pragma once
#include <lvgl.h>
#include "core/EventBus.h"

class Screen {
public:
    virtual ~Screen() = default;

    virtual const char* name() const = 0;

    // Создание виджетов экрана внутри переданного родителя.
    virtual void onCreate(lv_obj_t* parent) = 0;

    // Экран стал активным / ушёл в фон.
    virtual void onShow() {}
    virtual void onHide() {}

    // Периодическое обновление (анимации, данные).
    virtual void onUpdate(uint32_t now) {}

    // Обработка системного события (ввод и т.п.).
    virtual void onEvent(const Event& e) {}

    lv_obj_t* root() const { return _root; }

protected:
    lv_obj_t* _root = nullptr;
};
