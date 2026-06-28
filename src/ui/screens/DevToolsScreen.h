// ============================================================================
//  DevToolsScreen.h — Инженерные инструменты (раздел «Эксперт»)
//
//  Диагностика системы, сканер шины I2C, дамп регистров CC1101, factory-reset.
//  Слева — область результата, справа — список действий (энкодер).
// ============================================================================
#pragma once
#include "ui/Screen.h"

class DevToolsScreen : public Screen {
public:
    const char* name() const override { return "Dev"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 4;     // System, I2C, CC1101, Reset

    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _out = nullptr;
    lv_obj_t* _rows[kRows] = {};
    int       _selected = 0;
};
