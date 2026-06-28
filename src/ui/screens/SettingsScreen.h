// ============================================================================
//  SettingsScreen.h — Экран настроек, ландшафт 320×170, две колонки
//
//  Навигация энкодером по строкам; короткое нажатие активирует строку
//  (переключает тумблер / меняет язык), долгое — возврат в меню.
//  Значения читаются и сохраняются через Settings (NVS).
// ============================================================================
#pragma once
#include "ui/Screen.h"

class SettingsScreen : public Screen {
public:
    const char* name() const override { return "Settings"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kMaxRows = 8;

    // Действие строки при нажатии.
    enum Action { ACT_NONE, ACT_SOUND, ACT_VIBRO, ACT_DARK, ACT_LANG, ACT_EXPERT };

    struct Row {
        lv_obj_t* obj = nullptr;    // строка (для подсветки выбора)
        lv_obj_t* sw  = nullptr;    // тумблер (если есть)
        Action    act = ACT_NONE;
    };

    lv_obj_t* makeRow(lv_obj_t* parent, const char* sym, lv_color_t color,
                      const char* label, bool hasSwitch, bool swOn,
                      const char* value, bool sep, Action act);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    Row _rows[kMaxRows];
    int _count    = 0;
    int _selected = 0;
};
