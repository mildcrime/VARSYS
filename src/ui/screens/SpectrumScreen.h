// ============================================================================
//  SpectrumScreen.h — Спектр Sub-GHz (живой свип RSSI на LVGL-графике)
//
//  Свипует окно вокруг текущей частоты, рисует линию RSSI, отмечает пик.
//  Обновляется по таймеру, пока экран активен.
// ============================================================================
#pragma once
#include "ui/Screen.h"

class SpectrumScreen : public Screen {
public:
    const char* name() const override { return "Spectrum"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;

private:
    static constexpr int kPoints = 32;

    void doSweep();

    lv_obj_t*           _chart = nullptr;
    lv_chart_series_t*  _ser   = nullptr;
    lv_obj_t*           _peak  = nullptr;
    uint32_t            _task  = 0;
};
