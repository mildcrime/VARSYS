// ============================================================================
//  HomeScreen.h — Домашний экран: focus-карусель (LVGL)
//
//  Горизонтальная карусель крупных иконок: выбранная в центре увеличена и
//  подсвечена, соседние уменьшены и приглушены. Прокрутка энкодером плавная
//  (scroll-snap по центру + пересчёт масштаба по близости к центру).
// ============================================================================
#pragma once
#include "ui/Screen.h"

class HomeScreen : public Screen {
public:
    const char* name() const override { return "Home"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kMax = 16;

    struct Tile {
        lv_obj_t*   icon   = nullptr;
        const char* target = nullptr;
        const char* label  = nullptr;
    };

    void addTile(const char* sym, lv_color_t color, const char* label, const char* target);
    void applyFocus();                 // пересчёт масштаба/прозрачности/выбора
    void moveSelection(int delta);
    void openSelected();
    static void scrollCb(lv_event_t* e);

    lv_obj_t* _row  = nullptr;
    lv_obj_t* _name = nullptr;
    Tile      _tiles[kMax];
    int       _count    = 0;
    int       _selected = 0;
};
