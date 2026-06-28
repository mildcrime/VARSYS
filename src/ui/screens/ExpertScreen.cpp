#include "ExpertScreen.h"
#include "ui/UITheme.h"
#include "ui/UIManager.h"
#include "ui/i18n.h"
#include "ui/Notify.h"

using namespace ui;

static lv_obj_t* makeRow(lv_obj_t* parent, const char* sym, const char* label, bool sep) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 34);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    if (sep) {
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);
    }
    lv_obj_t* ic = lv_label_create(row);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, cRed(), 0);
    lv_obj_set_style_text_font(ic, &varsys_16, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t* lb = lv_label_create(row);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 34, 0);
    return row;
}

void ExpertScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, tr(STR_EXPERT));

    // Предупреждение.
    lv_obj_t* warn = lv_label_create(_root);
    lv_label_set_text_fmt(warn, ICON_EXPERT "  %s", tr(STR_EXPERT_WARN));
    lv_obj_set_style_text_color(warn, cRed(), 0);
    lv_obj_set_style_text_font(warn, &varsys_12, 0);
    lv_obj_set_width(warn, 300);
    lv_label_set_long_mode(warn, LV_LABEL_LONG_WRAP);
    lv_obj_align(warn, LV_ALIGN_TOP_MID, 0, 28);

    // Список гейтнутых инструментов.
    lv_obj_t* list = card(_root);
    lv_obj_set_size(list, 300, 100);
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    _rows[0] = makeRow(list, ICON_ANTENNA,   tr(STR_JAMMER),   true);
    _rows[1] = makeRow(list, ICON_BLUETOOTH, tr(STR_BLE_SPAM), true);
    _rows[2] = makeRow(list, ICON_WIFI,      "Captive portal", true);
    _rows[3] = makeRow(list, ICON_SETTINGS,  "Dev tools",      false);

    _status = lv_label_create(_root);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -4);
}

void ExpertScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void ExpertScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
    lv_label_set_text(_status, "");
}

void ExpertScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void ExpertScreen::activateSelected() {
    // Captive-портал — рабочий инструмент (для авторизованных фишинг-симуляций).
    if (_selected == 2) {
        UIManager::instance().pushScreen("Portal", LV_SCR_LOAD_ANIM_MOVE_LEFT);
        return;
    }
    // Инженерные инструменты.
    if (_selected == 3) {
        UIManager::instance().pushScreen("Dev", LV_SCR_LOAD_ANIM_MOVE_LEFT);
        return;
    }
    // Глушилка / массовый BLE-spam: атакующая нагрузка намеренно НЕ реализована
    // (точка расширения для лаборатории — подтверждение оператора, Фарадей,
    // региональные ограничения).
    lv_label_set_text(_status, tr(STR_LAB_ONLY));
    lv_obj_set_style_text_color(_status, cRed(), 0);
    Notify::toast(tr(STR_LAB_ONLY), Notify::Warn);
}

void ExpertScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;   // «назад» — централизованно в UIManager
    }
}
