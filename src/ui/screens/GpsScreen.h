// ============================================================================
//  GpsScreen.h — GPS: фикс, спутники, координаты (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class GpsScreen : public Screen {
public:
    const char* name() const override { return "Gps"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;

private:
    void refresh();
    lv_obj_t* _sats = nullptr;
    lv_obj_t* _coord = nullptr;
    lv_obj_t* _fix  = nullptr;
    uint32_t  _task = 0;
};
