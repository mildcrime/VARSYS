#include "IButtonScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/IButtonModule/IButtonModule.h"
#include "modules/StorageModule/StorageModule.h"

using namespace ui;

lv_obj_t* IButtonScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
                                    lv_color_t color, const char* label, bool sep) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 38);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    if (sep) {
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);
    }
    lv_obj_t* ic = lv_label_create(row);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, color, 0);
    lv_obj_set_style_text_font(ic, &varsys_16, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t* lb = lv_label_create(row);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 34, 0);
    if (idx >= 0 && idx < kRows) _rows[idx] = row;
    return row;
}

void IButtonScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "iButton");

    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 168, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    lv_obj_t* cap = lv_label_create(left);
    lv_label_set_text(cap, "ROM");
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 12);

    _id = lv_label_create(left);
    lv_label_set_text(_id, "--");
    lv_obj_set_style_text_color(_id, cText(), 0);
    lv_obj_set_style_text_font(_id, &varsys_16, 0);
    lv_obj_align(_id, LV_ALIGN_CENTER, 0, 0);

    _status = lv_label_create(left);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -8);

    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 122, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    makeAction(right, 0, ICON_RECORD, cBlue(),  tr(STR_READ), true);
    makeAction(right, 1, ICON_SAVE,   cGreen(), tr(STR_SAVE), false);
}

void IButtonScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void IButtonScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void IButtonScreen::activateSelected() {
    IButtonModule& ib = IButtonModule::instance();
    if (_selected == 0) {                       // Читать
        String key;
        if (ib.readKey(key)) {
            lv_label_set_text(_id, key.c_str());
            lv_label_set_text(_status, "");
            Notify::toast(key.c_str(), Notify::Success);
        } else {
            lv_label_set_text(_status, tr(STR_NO_TAG));
            Notify::toast(tr(STR_NO_TAG), Notify::Warn);
        }
    } else {                                    // Сохранить
        if (ib.lastKey().isEmpty()) { Notify::toast(tr(STR_NO_TAG), Notify::Warn); return; }
        fs::FS* fs = StorageModule::instance().fs();
        if (fs) {
            if (!fs->exists("/ibutton")) fs->mkdir("/ibutton");
            File f = fs->open("/ibutton/" + ib.lastKey() + ".txt", FILE_WRITE);
            if (f) { f.printf("ROM: %s\n", ib.lastKey().c_str()); f.close(); }
            Notify::toast(tr(STR_SAVED), Notify::Success);
        }
    }
}

void IButtonScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
}

void IButtonScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
