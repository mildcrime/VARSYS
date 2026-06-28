// ============================================================================
//  NrfScreen.h — NRF24 анализатор 2.4 ГГц (ландшафт 320×170)
// ============================================================================
#pragma once
#include "ui/Screen.h"

class NrfScreen : public Screen {
public:
    const char* name() const override { return "Nrf"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;

private:
    void refresh();
    lv_obj_t*          _chart = nullptr;
    lv_chart_series_t* _ser   = nullptr;
    lv_obj_t*          _info  = nullptr;
    uint32_t           _task  = 0;
};
