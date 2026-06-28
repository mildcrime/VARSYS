#include "IrScreen.h"
#include "ui/UITheme.h"
#include "ui/UIManager.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/IrModule/IrModule.h"

using namespace ui;

lv_obj_t* IrScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
                               lv_color_t color, const char* label, bool sep) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 28);
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

void IrScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Infrared");

    // Левая карточка — состояние.
    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 142, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    lv_obj_t* cap = lv_label_create(left);
    lv_label_set_text(cap, "IR");
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 12);

    _count = lv_label_create(left);
    lv_obj_set_style_text_color(_count, cText(), 0);
    lv_obj_set_style_text_font(_count, &varsys_22, 0);
    lv_obj_align(_count, LV_ALIGN_CENTER, 0, -6);

    _status = lv_label_create(left);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -12);

    // Правая карточка — действия.
    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 148, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);

    makeAction(right, 0, ICON_RECORD,   cRed(),    tr(STR_CAPTURE), true);
    makeAction(right, 1, ICON_SIGNAL,   cGreen(),  tr(STR_REPLAY),  true);
    makeAction(right, 2, ICON_INFRARED, cOrange(), tr(STR_TV_OFF),  true);
    makeAction(right, 3, ICON_APPS,     cBlue(),   "Universal",     false);

    refreshInfo();
}

void IrScreen::refreshInfo() {
    lv_label_set_text_fmt(_count, "%u", (unsigned)IrModule::instance().lastPulseCount());
}

void IrScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void IrScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void IrScreen::activateSelected() {
    IrModule& ir = IrModule::instance();
    switch (_selected) {
        case 0: {   // захват
            lv_label_set_text(_status, tr(STR_LISTENING));
            lv_refr_now(NULL);
            size_t n = ir.capture();
            refreshInfo();
            if (n > 0) {
                lv_label_set_text_fmt(_status, "%s: %u", tr(STR_RECORDED), (unsigned)n);
                Notify::toast(tr(STR_RECORDED), Notify::Success);
            } else {
                lv_label_set_text(_status, tr(STR_NO_SIGNAL));
                Notify::toast(tr(STR_NO_SIGNAL), Notify::Warn);
            }
            break;
        }
        case 1:     // воспроизведение
            if (ir.replayLast()) Notify::toast(tr(STR_SENT), Notify::Success);
            else                 Notify::toast(tr(STR_NO_SIGNAL), Notify::Warn);
            break;
        case 2:     // выключить ТВ
            ir.sendTvOff();
            Notify::toast(tr(STR_SENT), Notify::Success);
            break;
        case 3:     // универсальный пульт
            UIManager::instance().pushScreen("IrRemote");
            break;
        default: break;
    }
}

void IrScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
    refreshInfo();
}

void IrScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
