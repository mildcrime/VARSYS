#include "FilesScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "modules/StorageModule/StorageModule.h"
#include <FS.h>

using namespace ui;

static const char* DIRS[] = { "/signals", "/brute", "/nfc", "/ibutton", "/ducky" };

void FilesScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, tr(STR_FILES));

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

void FilesScreen::rebuild() {
    lv_obj_clean(_list);
    _rows.clear();
    _count = 0;

    fs::FS* fs = StorageModule::instance().fs();
    if (fs) {
        for (const char* d : DIRS) {
            File dir = fs->open(d);
            if (!dir || !dir.isDirectory()) { if (dir) dir.close(); continue; }
            for (File e = dir.openNextFile(); e; e = dir.openNextFile()) {
                String n = e.name();
                int slash = n.lastIndexOf('/');
                if (slash >= 0) n = n.substring(slash + 1);
                size_t sz = e.size();
                e.close();

                lv_obj_t* row = lv_obj_create(_list);
                lv_obj_remove_style_all(row);
                lv_obj_set_size(row, lv_pct(100), 26);
                lv_obj_set_style_radius(row, 8, 0);
                lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
                lv_obj_set_style_border_color(row, cSep(), 0);
                lv_obj_set_style_border_width(row, 1, 0);

                lv_obj_t* ic = lv_label_create(row);
                lv_label_set_text(ic, ICON_FOLDER);
                lv_obj_set_style_text_color(ic, cText2(), 0);
                lv_obj_set_style_text_font(ic, &varsys_14, 0);
                lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);

                lv_obj_t* lb = lv_label_create(row);
                lv_label_set_text_fmt(lb, "%s/%s", d + 1, n.c_str());
                lv_obj_set_style_text_color(lb, cText(), 0);
                lv_obj_set_style_text_font(lb, &varsys_12, 0);
                lv_label_set_long_mode(lb, LV_LABEL_LONG_DOT);
                lv_obj_set_width(lb, 210);
                lv_obj_align(lb, LV_ALIGN_LEFT_MID, 28, 0);

                lv_obj_t* meta = lv_label_create(row);
                lv_label_set_text_fmt(meta, "%uB", (unsigned)sz);
                lv_obj_set_style_text_color(meta, cText2(), 0);
                lv_obj_set_style_text_font(meta, &varsys_12, 0);
                lv_obj_align(meta, LV_ALIGN_RIGHT_MID, -8, 0);

                _rows.push_back(row);
                _count++;
            }
            dir.close();
        }
    }

    bool empty = (_count == 0);
    if (empty) lv_obj_clear_flag(_empty, LV_OBJ_FLAG_HIDDEN);
    else       lv_obj_add_flag(_empty, LV_OBJ_FLAG_HIDDEN);
}

void FilesScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= (int)_rows.size()) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

void FilesScreen::onShow() {
    rebuild();
    _selected = 0;
    if (_count) select(0, true);
}

void FilesScreen::moveSelection(int delta) {
    if (_count == 0) return;
    select(_selected, false);
    _selected = (_selected + delta + _count) % _count;
    select(_selected, true);
    lv_obj_scroll_to_view(_rows[_selected], LV_ANIM_ON);
}

void FilesScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1); break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1); break;
        default: break;
    }
}
