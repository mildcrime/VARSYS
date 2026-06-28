// ============================================================================
//  BatteryScreen.h — Детали батареи: заряд, напряжение, ток (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class BatteryScreen : public Screen {
public:
    const char* name() const override { return "Battery"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;

private:
    void refresh();
    lv_obj_t* _pct  = nullptr;
    lv_obj_t* _stat = nullptr;
    lv_obj_t* _volt = nullptr;
    lv_obj_t* _cur  = nullptr;
    uint32_t  _task = 0;
};
