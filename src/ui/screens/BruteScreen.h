// ============================================================================
//  BruteScreen.h — Перебор фикс-кодов шлагбаумов (ландшафт 320×170)
//
//  Выбор протокола и числа повторов, запуск полнокадрового перебора.
//  Перебор блокирующий; прерывается кнопкой «назад».
// ============================================================================
#pragma once
#include "ui/Screen.h"

class BruteScreen : public Screen {
public:
    const char* name() const override { return "Brute"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 4;     // Протокол, Повторы, Старт, Кандидаты

    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();
    void refresh();

    lv_obj_t* _proto  = nullptr;
    lv_obj_t* _status = nullptr;
    lv_obj_t* _rows[kRows] = {};
    int _selected = 0;
    int _protoIdx = 0;
    int _repeats  = 2;
};
