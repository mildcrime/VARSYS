#include "SpectrumScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "core/Scheduler.h"
#include "modules/RadioModule/RadioModule.h"

using namespace ui;

void SpectrumScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, tr(STR_SPECTRUM));

    _chart = lv_chart_create(_root);
    lv_obj_set_size(_chart, 300, 96);
    lv_obj_align(_chart, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(_chart, cCard(), 0);
    lv_obj_set_style_bg_opa(_chart, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_chart, 0, 0);
    lv_obj_set_style_radius(_chart, 12, 0);
    lv_chart_set_type(_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(_chart, kPoints);
    lv_chart_set_range(_chart, LV_CHART_AXIS_PRIMARY_Y, -110, -30);
    lv_chart_set_div_line_count(_chart, 4, 0);
    lv_obj_set_style_line_color(_chart, cBlue(), LV_PART_ITEMS);
    lv_obj_set_style_size(_chart, 0, LV_PART_INDICATOR);   // без точек

    _ser = lv_chart_add_series(_chart, cBlue(), LV_CHART_AXIS_PRIMARY_Y);

    _peak = lv_label_create(_root);
    lv_label_set_text(_peak, "");
    lv_obj_set_style_text_color(_peak, cText2(), 0);
    lv_obj_set_style_text_font(_peak, &varsys_12, 0);
    lv_obj_align(_peak, LV_ALIGN_TOP_MID, 0, 28);
}

void SpectrumScreen::doSweep() {
    int rssi[kPoints];
    uint32_t cur = RadioModule::instance().freqKhz();
    uint32_t step = 60;                          // 60 кГц/точку (~1.9 МГц окно)
    uint32_t start = (cur > (uint32_t)(kPoints / 2) * step)
                     ? cur - (kPoints / 2) * step : cur;
    int peak = RadioModule::instance().sweep(start, step, kPoints, rssi);

    for (int i = 0; i < kPoints; ++i)
        lv_chart_set_value_by_id(_chart, _ser, i, rssi[i]);
    lv_chart_refresh(_chart);

    float peakMhz = (start + (uint32_t)peak * step) / 1000.0f;
    lv_label_set_text_fmt(_peak, "%.2f MHz  %d dBm", peakMhz, rssi[peak]);
}

void SpectrumScreen::onShow() {
    if (_task) Scheduler::instance().cancel(_task);
    doSweep();
    _task = Scheduler::instance().every(400, [this] { doSweep(); });
}

void SpectrumScreen::onHide() {
    if (_task) { Scheduler::instance().cancel(_task); _task = 0; }
}
