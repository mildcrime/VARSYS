// ============================================================================
//  FmScreen.h — FM-передатчик (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class FmScreen : public Screen {
public:
    const char* name() const override { return "Fm"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 2;     // Частота, Передача
    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();
    void refresh();

    lv_obj_t* _freq   = nullptr;
    lv_obj_t* _status = nullptr;
    lv_obj_t* _hwPanel = nullptr;   // панель «нет железа»
    lv_obj_t* _rows[kRows] = {};
    int       _selected = 0;
};
