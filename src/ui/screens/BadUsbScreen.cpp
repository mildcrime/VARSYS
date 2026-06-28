#include "BadUsbScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/BadUsbModule/BadUsbModule.h"

using namespace ui;

void BadUsbScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "BadUSB");

    lv_obj_t* c = card(_root);
    lv_obj_set_size(c, 300, 80);
    lv_obj_align(c, LV_ALIGN_CENTER, 0, 10);

    _row = lv_obj_create(c);
    lv_obj_remove_style_all(_row);
    lv_obj_set_size(_row, lv_pct(100), 40);
    lv_obj_align(_row, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_bg_color(_row, cTint(), 0);
    lv_obj_set_style_bg_opa(_row, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(_row, 8, 0);
    lv_obj_clear_flag(_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* ic = lv_label_create(_row);
    lv_label_set_text(ic, ICON_RECORD);
    lv_obj_set_style_text_color(ic, cBlue(), 0);
    lv_obj_set_style_text_font(ic, &varsys_16, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t* lb = lv_label_create(_row);
    lv_label_set_text_fmt(lb, "%s demo", tr(STR_RUN));
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 38, 0);

    _status = lv_label_create(c);
    lv_label_set_text(_status, "USB HID");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void BadUsbScreen::onShow() {}

void BadUsbScreen::onEvent(const Event& e) {
    if (e.type == EventType::INPUT_BTN_CLICK) {
        BadUsbModule::instance().runDemo();
        Notify::toast(tr(STR_SENT), Notify::Success);
    }
}
