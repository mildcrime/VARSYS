#include "WebUiScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/WebUiModule/WebUiModule.h"

using namespace ui;

void WebUiScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "WebUI");

    lv_obj_t* c = card(_root);
    lv_obj_set_size(c, 300, 96);
    lv_obj_align(c, LV_ALIGN_CENTER, 0, 8);

    _btn = lv_obj_create(c);
    lv_obj_remove_style_all(_btn);
    lv_obj_set_size(_btn, lv_pct(100), 38);
    lv_obj_align(_btn, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_bg_color(_btn, cTint(), 0);
    lv_obj_set_style_bg_opa(_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(_btn, 8, 0);
    lv_obj_clear_flag(_btn, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* ic = lv_label_create(_btn);
    lv_label_set_text(ic, ICON_WIFI);
    lv_obj_set_style_text_color(ic, cBlue(), 0);
    lv_obj_set_style_text_font(ic, &varsys_16, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 12, 0);

    lv_obj_t* lb = lv_label_create(_btn);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 38, 0);
    lv_obj_set_user_data(_btn, lb);

    _info = lv_label_create(c);
    lv_obj_set_style_text_color(_info, cText2(), 0);
    lv_obj_set_style_text_font(_info, &varsys_12, 0);
    lv_label_set_long_mode(_info, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_info, 280);
    lv_obj_align(_info, LV_ALIGN_BOTTOM_MID, 0, -8);
}

void WebUiScreen::refresh() {
    WebUiModule& w = WebUiModule::instance();
    lv_obj_t* lb = (lv_obj_t*)lv_obj_get_user_data(_btn);
    lv_label_set_text(lb, w.active() ? tr(STR_STOP) : tr(STR_START));
    if (w.active())
        lv_label_set_text_fmt(_info, "%s\nhttp://%s  (pass: varsys1234)",
                              w.ssid().c_str(), w.ip().c_str());
    else
        lv_label_set_text(_info, "AP + http");
}

void WebUiScreen::onShow() { refresh(); }

void WebUiScreen::onEvent(const Event& e) {
    if (e.type == EventType::INPUT_BTN_CLICK) {
        WebUiModule& w = WebUiModule::instance();
        if (w.active()) { w.stop();  Notify::toast(tr(STR_STOP),  Notify::Info); }
        else            { w.start(); Notify::toast(tr(STR_START), Notify::Success); }
        refresh();
    }
}
