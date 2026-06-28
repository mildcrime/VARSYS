// ============================================================================
//  WardriveScreen.h — Wardriving: старт/стоп лога WiFi+GPS (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class WardriveScreen : public Screen {
public:
    const char* name() const override { return "Wardrive"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;
    void onEvent(const Event& e) override;

private:
    void refresh();
    lv_obj_t* _state = nullptr;
    lv_obj_t* _aps   = nullptr;
    lv_obj_t* _gps   = nullptr;
    lv_obj_t* _hint  = nullptr;
    uint32_t  _task  = 0;
};
