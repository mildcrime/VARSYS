#include "StatusOverlay.h"
#include "ui/UITheme.h"
#include "core/Clock.h"
#include "modules/PowerModule/PowerModule.h"

using namespace ui;

void StatusOverlay::create() {
    _label = lv_label_create(lv_layer_top());
    lv_obj_remove_style_all(_label);
    lv_obj_set_style_text_color(_label, cText(), 0);
    lv_obj_set_style_text_font(_label, &varsys_14, 0);
    lv_obj_align(_label, LV_ALIGN_TOP_RIGHT, -10, 4);
    lv_obj_clear_flag(_label, LV_OBJ_FLAG_CLICKABLE);
    // Заглушка до первого обновления по таймеру (PowerModule ещё не готов).
    lv_label_set_text(_label, "--:-- " ICON_BATTERY);
}

void StatusOverlay::update() {
    if (!_label) return;

    char timebuf[8];
    Clock::timeString(timebuf, sizeof(timebuf));

    int pct = PowerModule::instance().batteryPercent();
    bool chg = PowerModule::instance().charging();

    if (pct < 0) {
        lv_label_set_text_fmt(_label, "%s   -- %s", timebuf, ICON_BATTERY);
    } else {
        lv_label_set_text_fmt(_label, "%s   %d%% %s", timebuf, pct, ICON_BATTERY);
    }
    lv_obj_set_style_text_color(_label, chg ? cGreen() : cText(), 0);
    lv_obj_align(_label, LV_ALIGN_TOP_RIGHT, -10, 4);
}

void StatusOverlay::setVisible(bool v) {
    if (!_label) return;
    if (v) lv_obj_clear_flag(_label, LV_OBJ_FLAG_HIDDEN);
    else   lv_obj_add_flag(_label, LV_OBJ_FLAG_HIDDEN);
}
