// ============================================================================
//  IrScreen.h — Экран инфракрасного порта (ландшафт 320×170)
//
//  Действия энкодером: захват, воспроизведение, «выключить ТВ» (универсальный
//  пульт). Статус и счётчик импульсов слева.
// ============================================================================
#pragma once
#include "ui/Screen.h"

class IrScreen : public Screen {
public:
    const char* name() const override { return "Ir"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 3;     // Захват, Воспроизв., ТВ выкл

    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();
    void refreshInfo();

    lv_obj_t* _count  = nullptr;
    lv_obj_t* _status = nullptr;
    lv_obj_t* _rows[kRows] = {};
    int       _selected = 0;
};
