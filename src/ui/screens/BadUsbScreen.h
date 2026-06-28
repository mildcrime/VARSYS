// ============================================================================
//  BadUsbScreen.h — BadUSB (USB-HID), ландшафт 320×170
// ============================================================================
#pragma once
#include "ui/Screen.h"

class BadUsbScreen : public Screen {
public:
    const char* name() const override { return "Badusb"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    lv_obj_t* _row    = nullptr;
    lv_obj_t* _status = nullptr;
};
