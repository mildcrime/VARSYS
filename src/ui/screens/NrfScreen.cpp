#include "NrfScreen.h"
#include "ui/UITheme.h"
#include "core/Scheduler.h"
#include "modules/NrfModule/NrfModule.h"

using namespace ui;

void NrfScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "NRF24");

    _chart = lv_chart_create(_root);
    lv_obj_set_size(_chart, 300, 96);
    lv_obj_align(_chart, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_style_bg_color(_chart, cCard(), 0);
    lv_obj_set_style_bg_opa(_chart, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_chart, 0, 0);
    lv_obj_set_style_radius(_chart, 12, 0);
    lv_chart_set_type(_chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(_chart, NrfModule::kChannels);
    lv_chart_set_range(_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 30);
    _ser = lv_chart_add_series(_chart, cBlue(), LV_CHART_AXIS_PRIMARY_Y);

    _info = lv_label_create(_root);
    lv_obj_set_style_text_color(_info, cText2(), 0);
    lv_obj_set_style_text_font(_info, &varsys_12, 0);
    lv_obj_align(_info, LV_ALIGN_TOP_MID, 0, 28);
}

void NrfScreen::refresh() {
    NrfModule& n = NrfModule::instance();
    if (!n.present()) { lv_label_set_text(_info, "no module"); return; }
    n.scanPass();
    const uint8_t* a = n.activity();
    int peak = 0;
    for (int i = 0; i < NrfModule::kChannels; ++i) {
        lv_chart_set_value_by_id(_chart, _ser, i, a[i]);
        if (a[i] > a[peak]) peak = i;
    }
    lv_chart_refresh(_chart);
    lv_label_set_text_fmt(_info, "peak ch%d  %d MHz", peak, 2400 + peak);
}

void NrfScreen::onShow() {
    if (!NrfModule::instance().present()) {
        if (!_hwPanel)
            _hwPanel = ui::hwMissingPanel(_root, "NRF24L01", "QWIIC (SPI)");
        return;   // без железа сканировать нечего
    }
    if (_hwPanel) { lv_obj_del(_hwPanel); _hwPanel = nullptr; }

    NrfModule::instance().resetScan();
    if (_task) Scheduler::instance().cancel(_task);
    refresh();
    _task = Scheduler::instance().every(250, [this] { refresh(); });
}

void NrfScreen::onHide() {
    if (_task) { Scheduler::instance().cancel(_task); _task = 0; }
}
