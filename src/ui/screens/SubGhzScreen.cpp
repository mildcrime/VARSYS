#include "SubGhzScreen.h"
#include "ui/UITheme.h"
#include "ui/UIManager.h"
#include "ui/i18n.h"
#include "core/Scheduler.h"
#include "ui/Notify.h"
#include "modules/RadioModule/RadioModule.h"

using namespace ui;

lv_obj_t* SubGhzScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
                                   lv_color_t color, const char* label, bool sep) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 28);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_flex_grow(row, 0, 0);
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
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 32, 0);

    if (idx >= 0 && idx < kRows) _rows[idx] = row;
    return row;
}

void SubGhzScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Sub-GHz");

    // --- Левая карточка: частота + RSSI ---
    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 142, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    lv_obj_t* cap = lv_label_create(left);
    lv_label_set_text(cap, tr(STR_FREQUENCY));
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 10);

    _freq = lv_label_create(left);
    lv_obj_set_style_text_color(_freq, cText(), 0);
    lv_obj_set_style_text_font(_freq, &varsys_22, 0);
    lv_obj_align(_freq, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t* unit = lv_label_create(left);
    lv_label_set_text(unit, "MHz");
    lv_obj_set_style_text_color(unit, cText2(), 0);
    lv_obj_set_style_text_font(unit, &varsys_12, 0);
    lv_obj_align(unit, LV_ALIGN_TOP_MID, 0, 56);

    _bar = lv_bar_create(left);
    lv_obj_set_size(_bar, 116, 6);
    lv_obj_align(_bar, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_bar_set_range(_bar, 0, 100);
    lv_bar_set_value(_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(_bar, cGreen(), LV_PART_INDICATOR);

    _dbm = lv_label_create(left);
    lv_label_set_text(_dbm, "-- dBm");
    lv_obj_set_style_text_color(_dbm, cText2(), 0);
    lv_obj_set_style_text_font(_dbm, &varsys_12, 0);
    lv_obj_align(_dbm, LV_ALIGN_BOTTOM_MID, 0, -12);

    _status = lv_label_create(left);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cBlue(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -2);

    // --- Правая карточка: действия ---
    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 148, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(right, LV_OBJ_FLAG_SCROLLABLE);      // 6 пунктов — прокрутка
    lv_obj_set_scroll_dir(right, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(right, LV_SCROLLBAR_MODE_OFF);

    makeAction(right, 0, ICON_ANTENNA, cBlue(),  tr(STR_FREQ),    true);
    makeAction(right, 1, ICON_SCAN,    cBlue(),  tr(STR_SCAN),    true);
    makeAction(right, 2, ICON_RECORD,  cRed(),   tr(STR_RECORD),  true);
    makeAction(right, 3, ICON_SIGNAL,  cGreen(), tr(STR_REPLAY),  true);
    makeAction(right, 4, ICON_SAVE,    cBlue(),  tr(STR_SAVE),    true);
    makeAction(right, 5, ICON_FOLDER,  cGreen(), tr(STR_LIBRARY),  true);
    makeAction(right, 6, ICON_SCAN,    cBlue(),  tr(STR_SPECTRUM), true);
    makeAction(right, 7, ICON_RECORD,  cRed(),   tr(STR_BRUTE),    false);

    refreshFreq();
}

void SubGhzScreen::refreshFreq() {
    if (!_freq) return;
    float mhz = RadioModule::instance().freqKhz() / 1000.0f;
    lv_label_set_text_fmt(_freq, "%.2f", mhz);
}

void SubGhzScreen::refreshRssi() {
    RadioModule& radio = RadioModule::instance();
    if (!radio.present()) {
        lv_label_set_text(_status, tr(STR_NO_RADIO));
        lv_obj_set_style_text_color(_status, cRed(), 0);
        lv_bar_set_value(_bar, 0, LV_ANIM_OFF);
        lv_label_set_text(_dbm, "-- dBm");
        return;
    }
    int r = radio.rssi();
    int pct = (r + 100) * 100 / 70;             // -100..-30 dBm -> 0..100%
    if (pct < 0) pct = 0; else if (pct > 100) pct = 100;
    lv_bar_set_value(_bar, pct, LV_ANIM_OFF);
    lv_label_set_text_fmt(_dbm, "%d dBm", r);
}

void SubGhzScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void SubGhzScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
    if (_rows[_selected]) lv_obj_scroll_to_view(_rows[_selected], LV_ANIM_ON);
}

void SubGhzScreen::activateSelected() {
    RadioModule& radio = RadioModule::instance();
    switch (_selected) {
        case 0:     // частота
            radio.cycleFreqPreset();
            refreshFreq();
            lv_label_set_text(_status, "");
            break;
        case 1:     // скан
            radio.scan();
            refreshFreq();
            lv_label_set_text(_status, "");
            break;
        case 2: {   // запись (блокирующая — показываем «Слушаю…» заранее)
            lv_label_set_text(_status, tr(STR_LISTENING));
            lv_obj_set_style_text_color(_status, cText2(), 0);
            lv_refr_now(NULL);                       // принудительно отрисовать
            size_t n = radio.recordRaw();
            if (n > 0) {
                // Декодер: краткая классификация сигнала.
                String dec = radio.decodeLast();
                lv_label_set_text_fmt(_status, "%s  (%s)", tr(STR_RECORDED), dec.c_str());
                lv_obj_set_style_text_color(_status, cBlue(), 0);
                Notify::toast(dec.c_str(), Notify::Success);
            } else {
                lv_label_set_text(_status, tr(STR_NO_SIGNAL));
                lv_obj_set_style_text_color(_status, cRed(), 0);
                Notify::toast(tr(STR_NO_SIGNAL), Notify::Warn);
            }
            break;
        }
        case 3:     // воспроизведение
            if (radio.replayLast()) {
                lv_label_set_text(_status, tr(STR_SENT));
                lv_obj_set_style_text_color(_status, cGreen(), 0);
                Notify::toast(tr(STR_SENT), Notify::Success);
            }
            break;
        case 4:     // сохранить в библиотеку
            if (radio.saveLast()) Notify::toast(tr(STR_SAVED), Notify::Success);
            else                  Notify::toast(tr(STR_NO_SIGNAL), Notify::Warn);
            break;
        case 5:     // библиотека записей
            UIManager::instance().pushScreen("Saved", LV_SCR_LOAD_ANIM_MOVE_LEFT);
            break;
        case 6:     // спектр
            UIManager::instance().pushScreen("Spectrum", LV_SCR_LOAD_ANIM_MOVE_LEFT);
            break;
        case 7:     // перебор фикс-кодов (выбор протокола на отдельном экране)
            UIManager::instance().pushScreen("Brute", LV_SCR_LOAD_ANIM_MOVE_LEFT);
            break;
        default: break;
    }
}

void SubGhzScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);

    refreshFreq();
    if (RadioModule::instance().present()) RadioModule::instance().listen();

    // Живой RSSI, пока экран активен.
    if (_rssiTask) Scheduler::instance().cancel(_rssiTask);
    _rssiTask = Scheduler::instance().every(250, [this] { refreshRssi(); });
    refreshRssi();
}

void SubGhzScreen::onHide() {
    if (_rssiTask) { Scheduler::instance().cancel(_rssiTask); _rssiTask = 0; }
    if (RadioModule::instance().present()) RadioModule::instance().idle();
}

void SubGhzScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        // «Назад» обрабатывает UIManager.
        default: break;
    }
}
