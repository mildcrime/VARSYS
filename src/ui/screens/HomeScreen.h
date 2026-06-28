// ============================================================================
//  HomeScreen.h — Главный экран VARSYS (спрингборд iOS, ландшафт 320×170)
//
//  Ряд плиток-иконок с горизонтальной прокруткой. Навигация энкодером:
//  поворот — перемещение выбора, нажатие — открытие подсистемы.
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
    static constexpr int kMaxTiles = 16;

    struct Tile {
        lv_obj_t*   wrap   = nullptr;   // обёртка (иконка + подпись)
        lv_obj_t*   square = nullptr;   // цветная плитка
        const char* target = nullptr;   // имя экрана для открытия (или nullptr)
    };

    void buildTile(lv_obj_t* row, int idx, const char* sym,
                   lv_color_t color, const char* label, const char* target);
    void moveSelection(int delta);
    void highlight(int idx, bool on);
    void openSelected();

    lv_obj_t* _row = nullptr;
    Tile      _tiles[kMaxTiles];
    int       _count    = 0;
    int       _selected = 0;
};
