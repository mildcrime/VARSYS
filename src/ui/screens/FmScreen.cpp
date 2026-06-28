#include "FmScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/FmModule/FmModule.h"

using namespace ui;

lv_obj_t* FmScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
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

void FmScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    _hwPanel = nullptr;   // старый объект удалён lv_obj_clean при пересборке
    styleScreen(_root);
    header(_root, "FM");

    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 158, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    lv_obj_t* cap = lv_label_create(left);
    lv_label_set_text(cap, tr(STR_FREQUENCY));
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 12);

    _freq = lv_label_create(left);
    lv_obj_set_style_text_color(_freq, cText(), 0);
    lv_obj_set_style_text_font(_freq, &varsys_22, 0);
    lv_obj_align(_freq, LV_ALIGN_CENTER, 0, -2);

    _status = lv_label_create(left);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -12);

    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 132, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    makeAction(right, 0, ICON_ANTENNA, cBlue(),  tr(STR_FREQ), true);
    makeAction(right, 1, ICON_SIGNAL,  cGreen(), tr(STR_SEND), false);

    refresh();
}

void FmScreen::refresh() {
    FmModule& fm = FmModule::instance();
    lv_label_set_text_fmt(_freq, "%.2f", fm.freqKhz10() / 100.0f);
    if (!fm.present()) {
        lv_label_set_text(_status, tr(STR_NO_MODULE));
        lv_obj_set_style_text_color(_status, cRed(), 0);
    } else if (fm.txOn()) {
        lv_label_set_text(_status, "MHz · TX");
        lv_obj_set_style_text_color(_status, cGreen(), 0);
    } else {
        lv_label_set_text(_status, "MHz");
        lv_obj_set_style_text_color(_status, cText2(), 0);
    }
}

void FmScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void FmScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void FmScreen::activateSelected() {
    FmModule& fm = FmModule::instance();
    if (!fm.present()) { Notify::toast(tr(STR_NO_MODULE), Notify::Error); return; }
    if (_selected == 0) { fm.cyclePreset(); }
    else                { fm.setTx(!fm.txOn()); }
    refresh();
}

void FmScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
    if (!FmModule::instance().present()) {
        if (!_hwPanel)
            _hwPanel = ui::hwMissingPanel(_root, "Si4713 FM TX", "I2C / QWIIC");
    } else if (_hwPanel) {
        lv_obj_del(_hwPanel); _hwPanel = nullptr;
    }
    refresh();
}

void FmScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
