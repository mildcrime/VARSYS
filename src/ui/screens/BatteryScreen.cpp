#include "BatteryScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "core/Scheduler.h"
#include "modules/PowerModule/PowerModule.h"

using namespace ui;

void BatteryScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, tr(STR_BATTERY));

    lv_obj_t* c = card(_root);
    lv_obj_set_size(c, 300, 110);
    lv_obj_align(c, LV_ALIGN_BOTTOM_MID, 0, -8);

    // Крупный процент заряда.
    _pct = lv_label_create(c);
    lv_obj_set_style_text_color(_pct, cText(), 0);
    lv_obj_set_style_text_font(_pct, &varsys_22, 0);
    lv_obj_align(_pct, LV_ALIGN_TOP_MID, 0, 12);

    // Статус (зарядка / разряд).
    _stat = lv_label_create(c);
    lv_obj_set_style_text_font(_stat, &varsys_12, 0);
    lv_obj_align(_stat, LV_ALIGN_TOP_MID, 0, 42);

    // Напряжение и ток.
    _volt = lv_label_create(c);
    lv_obj_set_style_text_color(_volt, cText2(), 0);
    lv_obj_set_style_text_font(_volt, &varsys_14, 0);
    lv_obj_align(_volt, LV_ALIGN_BOTTOM_LEFT, 16, -12);

    _cur = lv_label_create(c);
    lv_obj_set_style_text_color(_cur, cText2(), 0);
    lv_obj_set_style_text_font(_cur, &varsys_14, 0);
    lv_obj_align(_cur, LV_ALIGN_BOTTOM_RIGHT, -16, -12);
}

void BatteryScreen::refresh() {
    PowerModule& p = PowerModule::instance();
    int pct = p.batteryPercent();

    if (pct < 0) {
        lv_label_set_text(_pct, "--");
        lv_label_set_text(_stat, tr(STR_NO_MODULE));
        lv_obj_set_style_text_color(_stat, cText2(), 0);
        lv_label_set_text(_volt, "");
        lv_label_set_text(_cur, "");
        return;
    }

    lv_label_set_text_fmt(_pct, "%d%%", pct);

    if (p.charging()) {
        lv_label_set_text(_stat, tr(STR_CHARGING));
        lv_obj_set_style_text_color(_stat, cGreen(), 0);
    } else {
        lv_label_set_text(_stat, "—");
        lv_obj_set_style_text_color(_stat, cText2(), 0);
    }

    lv_label_set_text_fmt(_volt, "%s: %.2f V",
                          tr(STR_VOLTAGE), p.batteryMv() / 1000.0f);
    lv_label_set_text_fmt(_cur, "%s: %d mA",
                          tr(STR_CURRENT), (int)p.batteryMa());
}

void BatteryScreen::onShow() {
    refresh();
    if (_task) Scheduler::instance().cancel(_task);
    _task = Scheduler::instance().every(1000, [this] { refresh(); });
}

void BatteryScreen::onHide() {
    if (_task) { Scheduler::instance().cancel(_task); _task = 0; }
}
