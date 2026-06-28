// ============================================================================
//  WebUiScreen.h — Управление веб-интерфейсом (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class WebUiScreen : public Screen {
public:
    const char* name() const override { return "WebUi"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    void refresh();
    lv_obj_t* _btn    = nullptr;
    lv_obj_t* _info   = nullptr;
};
