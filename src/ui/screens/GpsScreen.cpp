#include "GpsScreen.h"
#include "ui/UITheme.h"
#include "core/Scheduler.h"
#include "modules/GpsModule/GpsModule.h"
#include "modules/WardriveModule/WardriveModule.h"

using namespace ui;

void GpsScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    _hwPanel = nullptr;   // старый объект удалён lv_obj_clean при пересборке
    styleScreen(_root);
    header(_root, "GPS");

    lv_obj_t* c = card(_root);
    lv_obj_set_size(c, 300, 110);
    lv_obj_align(c, LV_ALIGN_BOTTOM_MID, 0, -8);

    lv_obj_t* cap = lv_label_create(c);
    lv_label_set_text(cap, "SATELLITES");
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 10);

    _sats = lv_label_create(c);
    lv_obj_set_style_text_color(_sats, cText(), 0);
    lv_obj_set_style_text_font(_sats, &varsys_22, 0);
    lv_obj_align(_sats, LV_ALIGN_TOP_MID, 0, 26);

    _fix = lv_label_create(c);
    lv_obj_set_style_text_font(_fix, &varsys_14, 0);
    lv_obj_align(_fix, LV_ALIGN_CENTER, 0, 14);

    _coord = lv_label_create(c);
    lv_obj_set_style_text_color(_coord, cText2(), 0);
    lv_obj_set_style_text_font(_coord, &varsys_12, 0);
    lv_obj_align(_coord, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void GpsScreen::refresh() {
    GpsModule& g = GpsModule::instance();

    // Нет ни одного байта NMEA — модуль, вероятно, не подключён.
    if (g.charsRx() == 0) {
        if (!_hwPanel)
            _hwPanel = ui::hwMissingPanel(_root, "GPS module", "QWIIC (UART)");
        return;
    }
    if (_hwPanel) { lv_obj_del(_hwPanel); _hwPanel = nullptr; }

    lv_label_set_text_fmt(_sats, "%d", g.sats());
    if (g.hasFix()) {
        lv_label_set_text(_fix, "FIX");
        lv_obj_set_style_text_color(_fix, cGreen(), 0);
        lv_label_set_text_fmt(_coord, "%.5f, %.5f", g.lat(), g.lng());
    } else {
        lv_label_set_text(_fix, "no fix");
        lv_obj_set_style_text_color(_fix, cText2(), 0);
        lv_label_set_text_fmt(_coord, "rx %lu", (unsigned long)g.charsRx());
    }
}

void GpsScreen::onShow() {
    GpsModule::instance().acquire();   // занять QWIIC-порт (UART)
    refresh();
    if (_task) Scheduler::instance().cancel(_task);
    _task = Scheduler::instance().every(1000, [this] { refresh(); });
}

void GpsScreen::onHide() {
    if (_task) { Scheduler::instance().cancel(_task); _task = 0; }
    // Освобождаем QWIIC, только если wardrive не использует GPS в фоне.
    if (!WardriveModule::instance().active()) GpsModule::instance().release();
}
