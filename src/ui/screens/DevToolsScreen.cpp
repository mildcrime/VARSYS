#include "DevToolsScreen.h"
#include "ui/UITheme.h"
#include "ui/Notify.h"
#include "core/Settings.h"
#include "hal/board_pins.h"
#include "modules/RadioModule/RadioModule.h"
#include <Wire.h>

using namespace ui;

lv_obj_t* DevToolsScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
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
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_t* lb = lv_label_create(row);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 30, 0);
    if (idx >= 0 && idx < kRows) _rows[idx] = row;
    return row;
}

void DevToolsScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Dev tools");

    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 168, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    _out = lv_label_create(left);
    lv_label_set_text(_out, "select a tool");
    lv_obj_set_style_text_color(_out, cText(), 0);
    lv_obj_set_style_text_font(_out, &varsys_12, 0);
    lv_label_set_long_mode(_out, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_out, 150);
    lv_obj_align(_out, LV_ALIGN_TOP_LEFT, 8, 8);

    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 122, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    makeAction(right, 0, ICON_INFO,     cBlue(),  "System",  true);
    makeAction(right, 1, ICON_SCAN,     cBlue(),  "I2C scan", true);
    makeAction(right, 2, ICON_ANTENNA,  cOrange(),"CC1101",  true);
    makeAction(right, 3, ICON_EXPERT,   cRed(),   "Reset",   false);
}

void DevToolsScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void DevToolsScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void DevToolsScreen::activateSelected() {
    switch (_selected) {
        case 0: {   // System
            char b[160];
            float t = temperatureRead();
            snprintf(b, sizeof(b),
                     "heap %u KB\npsram %u KB\nup %lus\ncpu %.0fC",
                     (unsigned)(ESP.getFreeHeap() / 1024),
                     (unsigned)(ESP.getFreePsram() / 1024),
                     (unsigned long)(millis() / 1000), t);
            lv_label_set_text(_out, b);
            break;
        }
        case 1: {   // I2C scan
            String s = "I2C:";
            int found = 0;
            for (uint8_t a = 1; a < 127; ++a) {
                Wire.beginTransmission(a);
                if (Wire.endTransmission() == 0) {
                    char h[8]; snprintf(h, sizeof(h), " %02X", a);
                    s += h; found++;
                }
            }
            if (!found) s += " none";
            lv_label_set_text(_out, s.c_str());
            break;
        }
        case 2:     // CC1101 registers
            lv_label_set_text(_out, RadioModule::instance().diag().c_str());
            break;
        case 3:     // Factory reset (немедленно, с перезагрузкой)
            Notify::toast("Factory reset", Notify::Warn);
            lv_refr_now(NULL);
            Settings::instance().factoryReset();
            break;
        default: break;
    }
}

void DevToolsScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
}

void DevToolsScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
