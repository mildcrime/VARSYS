#include "SavedSignalsScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/RadioModule/RadioModule.h"
#include "modules/StorageModule/StorageModule.h"

using namespace ui;

void SavedSignalsScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, tr(STR_LIBRARY));

    _list = card(_root);
    lv_obj_set_size(_list, 300, 118);
    lv_obj_align(_list, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_flex_flow(_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(_list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_list, LV_SCROLLBAR_MODE_OFF);

    _empty = lv_label_create(_root);
    lv_label_set_text(_empty, tr(STR_EMPTY));
    lv_obj_set_style_text_color(_empty, cText2(), 0);
    lv_obj_set_style_text_font(_empty, &varsys_14, 0);
    lv_obj_center(_empty);
}

void SavedSignalsScreen::rebuild() {
    lv_obj_clean(_list);
    _rows.clear();
    _names = StorageModule::instance().listSignals();

    for (const String& n : _names) {
        lv_obj_t* row = lv_obj_create(_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 28);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);

        lv_obj_t* ic = lv_label_create(row);
        lv_label_set_text(ic, ICON_ANTENNA);
        lv_obj_set_style_text_color(ic, cOrange(), 0);
        lv_obj_set_style_text_font(ic, &varsys_16, 0);
        lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t* lb = lv_label_create(row);
        String disp = n; disp.replace(".sub", "");
        lv_label_set_text(lb, disp.c_str());
        lv_obj_set_style_text_color(lb, cText(), 0);
        lv_obj_set_style_text_font(lb, &varsys_14, 0);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 32, 0);

        _rows.push_back(row);
    }

    bool empty = _names.empty();
    if (empty) lv_obj_clear_flag(_empty, LV_OBJ_FLAG_HIDDEN);
    else       lv_obj_add_flag(_empty, LV_OBJ_FLAG_HIDDEN);
}

void SavedSignalsScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= (int)_rows.size()) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void SavedSignalsScreen::onShow() {
    rebuild();
    _selected = 0;
    if (!_rows.empty()) select(0, true);
}

void SavedSignalsScreen::moveSelection(int delta) {
    if (_rows.empty()) return;
    select(_selected, false);
    _selected = (_selected + delta + (int)_rows.size()) % (int)_rows.size();
    select(_selected, true);
    lv_obj_scroll_to_view(_rows[_selected], LV_ANIM_ON);
}

void SavedSignalsScreen::activateSelected() {
    if (_selected < 0 || _selected >= (int)_names.size()) return;
    RadioModule& radio = RadioModule::instance();
    if (radio.loadSignal(_names[_selected])) {
        Notify::toast(tr(STR_LOADED), Notify::Info);
        if (radio.replayLast()) Notify::toast(tr(STR_SENT), Notify::Success);
    }
}

void SavedSignalsScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
