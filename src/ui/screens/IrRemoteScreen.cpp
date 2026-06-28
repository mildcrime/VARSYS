#include "IrRemoteScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/IrModule/IrModule.h"

using namespace ui;

void IrRemoteScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Universal");

    _status = lv_label_create(_root);
    lv_label_set_text(_status, "TV");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_TOP_RIGHT, -70, 6);

    _list = card(_root);
    lv_obj_set_size(_list, 300, 122);
    lv_obj_align(_list, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_flex_flow(_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(_list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_list, LV_SCROLLBAR_MODE_OFF);

    for (int f = 0; f < IrModule::UNI_COUNT; ++f) {
        lv_obj_t* row = lv_obj_create(_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 28);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);

        lv_obj_t* ic = lv_label_create(row);
        lv_label_set_text(ic, ICON_INFRARED);
        lv_obj_set_style_text_color(ic, cRed(), 0);
        lv_obj_set_style_text_font(ic, &varsys_16, 0);
        lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t* lb = lv_label_create(row);
        lv_label_set_text(lb, IrModule::uniName((IrModule::UniFn)f));
        lv_obj_set_style_text_color(lb, cText(), 0);
        lv_obj_set_style_text_font(lb, &varsys_14, 0);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 34, 0);

        _rows.push_back(row);
    }
    if (!_rows.empty()) select(0, true);
}

void IrRemoteScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= (int)_rows.size()) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    if (on) lv_obj_scroll_to_view(_rows[idx], LV_ANIM_ON);
}

void IrRemoteScreen::moveSelection(int delta) {
    if (_rows.empty()) return;
    select(_selected, false);
    _selected = (_selected + delta + (int)_rows.size()) % (int)_rows.size();
    select(_selected, true);
}

void IrRemoteScreen::activateSelected() {
    auto fn = (IrModule::UniFn)_selected;
    Notify::toast(IrModule::uniName(fn), Notify::Info);
    lv_label_set_text(_status, tr(STR_SENT));
    lv_refr_now(NULL);
    int n = IrModule::instance().sendUniversal(fn);
    lv_label_set_text_fmt(_status, "%d codes", n);
    Notify::toast(tr(STR_SENT), Notify::Success);
}

void IrRemoteScreen::onShow() {
    _selected = 0;
    for (size_t i = 0; i < _rows.size(); ++i) select(i, false);
    if (!_rows.empty()) select(0, true);
}

void IrRemoteScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
