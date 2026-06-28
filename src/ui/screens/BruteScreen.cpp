#include "BruteScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/RadioModule/RadioModule.h"

using namespace ui;

lv_obj_t* BruteScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
                                  lv_color_t color, const char* label, bool sep) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 29);
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

void BruteScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, tr(STR_BRUTE));

    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 158, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    lv_obj_t* cap = lv_label_create(left);
    lv_label_set_text(cap, "PROTOCOL");
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 12);

    _proto = lv_label_create(left);
    lv_obj_set_style_text_color(_proto, cText(), 0);
    lv_obj_set_style_text_font(_proto, &varsys_14, 0);
    lv_label_set_long_mode(_proto, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(_proto, 140);
    lv_obj_align(_proto, LV_ALIGN_CENTER, 0, -6);

    _status = lv_label_create(left);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -10);

    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 132, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    makeAction(right, 0, ICON_ANTENNA, cBlue(),  "Protocol", true);
    makeAction(right, 1, ICON_RECORD,  cBlue(),  "Repeats",  true);
    makeAction(right, 2, ICON_SIGNAL,  cGreen(), tr(STR_START), true);
    makeAction(right, 3, ICON_FOLDER,  cOrange(),"Candidates", false);

    refresh();
}

void BruteScreen::refresh() {
    RadioModule& r = RadioModule::instance();
    lv_label_set_text_fmt(_proto, "%s\nx%d  %.2f MHz",
                          r.bruteProtoName(_protoIdx), _repeats,
                          r.freqKhz() / 1000.0f);
}

void BruteScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void BruteScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void BruteScreen::activateSelected() {
    RadioModule& r = RadioModule::instance();
    if (_selected == 0) {                       // протокол (+ «All»)
        _protoIdx = (_protoIdx + 1) % (r.bruteProtoCount() + 1);
        refresh();
    } else if (_selected == 1) {                // повторы 1..5
        _repeats = (_repeats % 5) + 1;
        refresh();
    } else if (_selected == 2) {                // старт (блокирующий)
        if (!r.present()) { Notify::toast(tr(STR_NO_RADIO), Notify::Error); return; }
        lv_label_set_text(_status, "TX…  back=стоп");
        lv_obj_set_style_text_color(_status, cBlue(), 0);
        lv_refr_now(NULL);
        int sent = r.bruteforce(_protoIdx, _repeats);
        if (r.bruteAborted()) {                 // оператор остановил — код сохранён
            lv_label_set_text_fmt(_status, "stop @ %d\nкандидаты сохранены", sent);
            lv_obj_set_style_text_color(_status, cGreen(), 0);
            Notify::toast(tr(STR_SAVED), Notify::Success);
        } else {
            lv_label_set_text_fmt(_status, "done: %d", sent);
            lv_obj_set_style_text_color(_status, cText2(), 0);
            Notify::toast(tr(STR_SENT), Notify::Info);
        }
    } else {                                    // автопрогон кандидатов
        if (r.candidateCount() == 0) { Notify::toast(tr(STR_EMPTY), Notify::Warn); return; }
        lv_obj_t* st = _status;
        int conf = r.replayCandidates(4, [st](int i, int tot, const char* pr, int code) {
            lv_label_set_text_fmt(st, "%d/%d\n%s 0x%X\nback=это оно", i + 1, tot, pr, code);
            lv_obj_set_style_text_color(st, cBlue(), 0);
            lv_refr_now(NULL);
        });
        if (conf >= 0) {
            lv_label_set_text(_status, "confirmed!\nсохранён код");
            lv_obj_set_style_text_color(_status, cGreen(), 0);
            Notify::toast(tr(STR_SAVED), Notify::Success);
        } else {
            lv_label_set_text(_status, "done");
            lv_obj_set_style_text_color(_status, cText2(), 0);
        }
    }
}

void BruteScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
    refresh();
}

void BruteScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
