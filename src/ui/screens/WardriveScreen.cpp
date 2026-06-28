#include "WardriveScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "core/Scheduler.h"
#include "modules/WardriveModule/WardriveModule.h"
#include "modules/GpsModule/GpsModule.h"

using namespace ui;

void WardriveScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Wardrive");

    lv_obj_t* c = card(_root);
    lv_obj_set_size(c, 300, 110);
    lv_obj_align(c, LV_ALIGN_BOTTOM_MID, 0, -8);

    _state = lv_label_create(c);
    lv_obj_set_style_text_font(_state, &varsys_22, 0);
    lv_obj_align(_state, LV_ALIGN_TOP_MID, 0, 10);

    _aps = lv_label_create(c);
    lv_obj_set_style_text_color(_aps, cText(), 0);
    lv_obj_set_style_text_font(_aps, &varsys_16, 0);
    lv_obj_align(_aps, LV_ALIGN_CENTER, 0, 4);

    _gps = lv_label_create(c);
    lv_obj_set_style_text_color(_gps, cText2(), 0);
    lv_obj_set_style_text_font(_gps, &varsys_12, 0);
    lv_obj_align(_gps, LV_ALIGN_BOTTOM_MID, 0, -10);

    _hint = lv_label_create(_root);
    lv_label_set_text(_hint, tr(STR_START));
    lv_obj_set_style_text_color(_hint, cText2(), 0);
    lv_obj_set_style_text_font(_hint, &varsys_12, 0);
    lv_obj_align(_hint, LV_ALIGN_TOP_RIGHT, -70, 6);
}

void WardriveScreen::refresh() {
    WardriveModule& w = WardriveModule::instance();
    GpsModule& g = GpsModule::instance();

    if (w.active()) {
        lv_label_set_text(_state, tr(STR_RUN));
        lv_obj_set_style_text_color(_state, cGreen(), 0);
        lv_label_set_text(_hint, tr(STR_STOP));
    } else {
        lv_label_set_text(_state, tr(STR_STOP));
        lv_obj_set_style_text_color(_state, cText2(), 0);
        lv_label_set_text(_hint, tr(STR_START));
    }

    lv_label_set_text_fmt(_aps, "%lu AP", (unsigned long)w.apCount());
    lv_label_set_text_fmt(_gps, "GPS: %s · %d sat",
                          g.hasFix() ? "fix" : "no fix", g.sats());
}

void WardriveScreen::onShow() {
    refresh();
    if (_task) Scheduler::instance().cancel(_task);
    _task = Scheduler::instance().every(1000, [this] { refresh(); });
}

void WardriveScreen::onHide() {
    if (_task) { Scheduler::instance().cancel(_task); _task = 0; }
}

void WardriveScreen::onEvent(const Event& e) {
    if (e.type != EventType::INPUT_BTN_CLICK) return;
    WardriveModule& w = WardriveModule::instance();
    if (w.active()) {
        w.deactivate();
        Notify::toast(tr(STR_STOP), Notify::Info);
    } else {
        w.activate();
        Notify::toast(tr(STR_START), Notify::Warn);
    }
    refresh();
}
