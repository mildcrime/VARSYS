#include "NfcScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/NfcModule/NfcModule.h"
#include "modules/StorageModule/StorageModule.h"

using namespace ui;

lv_obj_t* NfcScreen::makeAction(lv_obj_t* parent, int idx, const char* sym,
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

void NfcScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    _hwPanel = nullptr;   // старый объект удалён lv_obj_clean при пересборке
    styleScreen(_root);
    header(_root, "NFC");

    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 158, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);

    lv_obj_t* cap = lv_label_create(left);
    lv_label_set_text(cap, "UID");
    lv_obj_set_style_text_color(cap, cText2(), 0);
    lv_obj_set_style_text_font(cap, &varsys_12, 0);
    lv_obj_align(cap, LV_ALIGN_TOP_MID, 0, 12);

    _uid = lv_label_create(left);
    lv_label_set_text(_uid, "--");
    lv_obj_set_style_text_color(_uid, cText(), 0);
    lv_obj_set_style_text_font(_uid, &varsys_22, 0);
    lv_obj_align(_uid, LV_ALIGN_CENTER, 0, -6);

    _type = lv_label_create(left);
    lv_label_set_text(_type, "");
    lv_obj_set_style_text_color(_type, cText2(), 0);
    lv_obj_set_style_text_font(_type, &varsys_12, 0);
    lv_obj_align(_type, LV_ALIGN_BOTTOM_MID, 0, -22);

    _status = lv_label_create(left);
    lv_label_set_text(_status, "");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_BOTTOM_MID, 0, -6);

    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 132, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    makeAction(right, 0, ICON_NFC,    cBlue(),   tr(STR_READ), true);
    makeAction(right, 1, ICON_SAVE,   cGreen(),  "Dump",       true);
    makeAction(right, 2, ICON_FOLDER, cOrange(), "Clone",      true);
    makeAction(right, 3, ICON_RECORD, cRed(),    "Write",      false);
}

void NfcScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= kRows || !_rows[idx]) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void NfcScreen::moveSelection(int delta) {
    select(_selected, false);
    _selected = (_selected + delta + kRows) % kRows;
    select(_selected, true);
}

void NfcScreen::activateSelected() {
    NfcModule& nfc = NfcModule::instance();
    if (!nfc.present()) {
        lv_label_set_text(_status, tr(STR_NO_MODULE));
        lv_obj_set_style_text_color(_status, cRed(), 0);
        Notify::toast(tr(STR_NO_MODULE), Notify::Error);
        return;
    }

    if (_selected == 0) {           // Читать
        lv_label_set_text(_status, tr(STR_LISTENING));
        lv_refr_now(NULL);
        String uid, type;
        if (nfc.readTag(uid, type)) {
            lv_label_set_text(_uid, uid.c_str());
            lv_label_set_text(_type, type.c_str());
            lv_label_set_text(_status, "");
            Notify::toast(uid.c_str(), Notify::Success);
        } else {
            lv_label_set_text(_status, tr(STR_NO_TAG));
            Notify::toast(tr(STR_NO_TAG), Notify::Warn);
        }
    } else if (_selected == 1) {    // Полный дамп со словарём -> файл
        lv_label_set_text(_status, tr(STR_LISTENING));
        lv_refr_now(NULL);
        NfcModule::ClassicDump d;
        if (!nfc.dumpClassic(d)) {
            lv_label_set_text(_status, tr(STR_NO_TAG));
            Notify::toast(tr(STR_NO_TAG), Notify::Warn);
            return;
        }
        String path;
        bool saved = nfc.saveDump(d, path);
        lv_label_set_text_fmt(_status, "%d/64 blk", d.blocksRead);
        lv_obj_set_style_text_color(_status, d.blocksRead == 64 ? cGreen() : cOrange(), 0);
        Notify::toast(saved ? tr(STR_SAVED) : tr(STR_NO_MODULE),
                      saved ? Notify::Success : Notify::Error);
    } else if (_selected == 2) {    // Клон: записать последний дамп на карту
        if (nfc.lastUid().isEmpty()) { Notify::toast(tr(STR_NO_TAG), Notify::Warn); return; }
        NfcModule::ClassicDump d;
        if (!nfc.loadDump(nfc.lastUid() + ".dump", d)) {
            Notify::toast(tr(STR_EMPTY), Notify::Warn);
            return;
        }
        lv_label_set_text(_status, tr(STR_LISTENING));
        lv_refr_now(NULL);
        int w = nfc.cloneDump(d);
        lv_label_set_text_fmt(_status, "wr %d blk", w);
        lv_obj_set_style_text_color(_status, w > 0 ? cGreen() : cRed(), 0);
        Notify::toast(w > 0 ? tr(STR_SAVED) : tr(STR_NO_TAG),
                      w > 0 ? Notify::Success : Notify::Warn);
    } else {                        // Записать демо-блок (Mifare block 4)
        uint8_t demo[16] = {0};
        memcpy(demo, "VARSYS", 6);
        bool ok = nfc.writeBlock(4, demo);
        lv_label_set_text(_status, ok ? "block 4 written" : tr(STR_NO_TAG));
        lv_obj_set_style_text_color(_status, ok ? cGreen() : cRed(), 0);
        Notify::toast(ok ? tr(STR_SAVED) : tr(STR_NO_TAG), ok ? Notify::Success : Notify::Warn);
    }
}

void NfcScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < kRows; ++i) select(i, false);
    select(_selected, true);
    if (!NfcModule::instance().present()) {
        if (!_hwPanel)
            _hwPanel = ui::hwMissingPanel(_root, "PN532 NFC", "I2C / QWIIC");
    } else if (_hwPanel) {
        lv_obj_del(_hwPanel); _hwPanel = nullptr;
    }
}

void NfcScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
