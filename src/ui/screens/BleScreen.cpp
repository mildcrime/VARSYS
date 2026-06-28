#include "BleScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/BleModule/BleModule.h"

using namespace ui;

void BleScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Bluetooth");

    _status = lv_label_create(_root);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_TOP_RIGHT, -70, 6);

    _list = card(_root);
    lv_obj_set_size(_list, 300, 116);
    lv_obj_align(_list, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_flex_flow(_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(_list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_list, LV_SCROLLBAR_MODE_OFF);
}

void BleScreen::rebuild() {
    lv_obj_clean(_list);
    _rows.clear();
    const auto& devs = BleModule::instance().devices();

    for (const auto& d : devs) {
        lv_obj_t* row = lv_obj_create(_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 27);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);

        lv_obj_t* ic = lv_label_create(row);
        lv_label_set_text(ic, ICON_BLUETOOTH);
        lv_obj_set_style_text_color(ic, cBlue(), 0);
        lv_obj_set_style_text_font(ic, &varsys_16, 0);
        lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t* lb = lv_label_create(row);
        lv_label_set_text(lb, d.name.c_str());
        lv_obj_set_style_text_color(lb, cText(), 0);
        lv_obj_set_style_text_font(lb, &varsys_14, 0);
        lv_label_set_long_mode(lb, LV_LABEL_LONG_DOT);
        lv_obj_set_width(lb, 180);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 30, 0);

        lv_obj_t* meta = lv_label_create(row);
        lv_label_set_text_fmt(meta, "%d", d.rssi);
        lv_obj_set_style_text_color(meta, cText2(), 0);
        lv_obj_set_style_text_font(meta, &varsys_12, 0);
        lv_obj_align(meta, LV_ALIGN_RIGHT_MID, -8, 0);

        _rows.push_back(row);
    }
}

void BleScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= (int)_rows.size()) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void BleScreen::onShow() {
    lv_label_set_text(_status, tr(STR_SCANNING));
    lv_refr_now(NULL);

    BleModule::instance().scan(4);
    rebuild();
    _selected = 0;
    if (!_rows.empty()) select(0, true);
    lv_label_set_text_fmt(_status, "%u dev", (unsigned)_rows.size());
}

void BleScreen::onHide() {
    BleModule::instance().radioOff();   // выключаем контроллер BLE (экономия)
}

void BleScreen::moveSelection(int delta) {
    if (_rows.empty()) return;
    select(_selected, false);
    _selected = (_selected + delta + (int)_rows.size()) % (int)_rows.size();
    select(_selected, true);
    lv_obj_scroll_to_view(_rows[_selected], LV_ANIM_ON);
}

void BleScreen::activateSelected() {
    const auto& devs = BleModule::instance().devices();
    if (_selected < 0 || _selected >= (int)devs.size()) return;
    Notify::toast(devs[_selected].addr.c_str(), Notify::Info);
}

void BleScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
