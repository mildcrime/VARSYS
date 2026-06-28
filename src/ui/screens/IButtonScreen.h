// ============================================================================
//  IButtonScreen.h — iButton/Dallas (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class IButtonScreen : public Screen {
public:
    const char* name() const override { return "IButton"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 2;     // Читать, Сохранить
    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _id     = nullptr;
    lv_obj_t* _status = nullptr;
    lv_obj_t* _rows[kRows] = {};
    int       _selected = 0;
};
