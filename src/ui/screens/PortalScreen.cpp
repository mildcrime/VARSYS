#include "PortalScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "core/Scheduler.h"
#include "modules/EvilPortalModule/EvilPortalModule.h"

using namespace ui;

void PortalScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Portal");

    lv_obj_t* c = card(_root);
    lv_obj_set_size(c, 300, 100);
    lv_obj_align(c, LV_ALIGN_CENTER, 0, 8);

    _btn = lv_label_create(c);
    lv_obj_set_style_text_font(_btn, &varsys_22, 0);
    lv_obj_align(_btn, LV_ALIGN_TOP_MID, 0, 14);

    _info = lv_label_create(c);
    lv_obj_set_style_text_color(_info, cText2(), 0);
    lv_obj_set_style_text_font(_info, &varsys_12, 0);
    lv_label_set_long_mode(_info, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_info, 280);
    lv_obj_align(_info, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void PortalScreen::refresh() {
    EvilPortalModule& p = EvilPortalModule::instance();
    if (p.active()) {
        lv_label_set_text(_btn, tr(STR_STOP));
        lv_obj_set_style_text_color(_btn, cRed(), 0);
        lv_label_set_text_fmt(_info, "%s\ncaptured: %d", p.ssid().c_str(), p.captured());
    } else {
        lv_label_set_text(_btn, tr(STR_START));
        lv_obj_set_style_text_color(_btn, cBlue(), 0);
        lv_label_set_text(_info, "Free WiFi (captive)");
    }
}

void PortalScreen::onShow() {
    refresh();
    if (_task) Scheduler::instance().cancel(_task);
    _task = Scheduler::instance().every(1000, [this] { refresh(); });
}

void PortalScreen::onEvent(const Event& e) {
    if (e.type == EventType::INPUT_BTN_CLICK) {
        EvilPortalModule& p = EvilPortalModule::instance();
        if (p.active()) { p.stop();  Notify::toast(tr(STR_STOP), Notify::Info); }
        else            { p.start(); Notify::toast(tr(STR_START), Notify::Warn); }
        refresh();
    }
}
