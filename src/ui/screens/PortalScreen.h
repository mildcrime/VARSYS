// ============================================================================
//  PortalScreen.h — Captive-портал: старт/стоп + счётчик (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class PortalScreen : public Screen {
public:
    const char* name() const override { return "Portal"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    void refresh();
    lv_obj_t* _btn  = nullptr;
    lv_obj_t* _info = nullptr;
    uint32_t  _task = 0;
};
