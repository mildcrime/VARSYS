#include "BadUsbScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "core/Settings.h"
#include "modules/BadUsbModule/BadUsbModule.h"
#include "modules/StorageModule/StorageModule.h"

using namespace ui;

void BadUsbScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "BadUSB");

    _status = lv_label_create(_root);
    lv_label_set_text(_status, "USB HID");
    lv_obj_set_style_text_color(_status, cText2(), 0);
    lv_obj_set_style_text_font(_status, &varsys_12, 0);
    lv_obj_align(_status, LV_ALIGN_TOP_RIGHT, -70, 6);

    _list = card(_root);
    lv_obj_set_size(_list, 300, 122);
    lv_obj_align(_list, LV_ALIGN_BOTTOM_MID, 0, -6);
    lv_obj_set_flex_flow(_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(_list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_list, LV_SCROLLBAR_MODE_OFF);

    rebuild();
}

static lv_obj_t* makeRow(lv_obj_t* list, const char* sym, lv_color_t color,
                         const char* label, const char* value, lv_obj_t** outVal) {
    lv_obj_t* row = lv_obj_create(list);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 28);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(row, cSep(), 0);
    lv_obj_set_style_border_width(row, 1, 0);

    lv_obj_t* ic = lv_label_create(row);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, color, 0);
    lv_obj_set_style_text_font(ic, &varsys_16, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t* lb = lv_label_create(row);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_label_set_long_mode(lb, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lb, 200);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 34, 0);

    lv_obj_t* v = nullptr;
    if (value) {
        v = lv_label_create(row);
        lv_label_set_text(v, value);
        lv_obj_set_style_text_color(v, cBlue(), 0);
        lv_obj_set_style_text_font(v, &varsys_14, 0);
        lv_obj_align(v, LV_ALIGN_RIGHT_MID, -10, 0);
    }
    if (outVal) *outVal = v;
    return row;
}

void BadUsbScreen::rebuild() {
    lv_obj_clean(_list);
    _items.clear();

    // 1) Раскладка хоста.
    const char* lay = (Settings::instance().badusbLayout() == BadUsbModule::LAYOUT_DE)
                          ? "DE" : "US";
    lv_obj_t* val = nullptr;
    lv_obj_t* r0 = makeRow(_list, ICON_LANGUAGE, cBlue(), "Layout", lay, &val);
    _items.push_back({ K_LAYOUT, "", r0, val });

    // 2) Демо.
    lv_obj_t* r1 = makeRow(_list, ICON_RECORD, cGreen(), "Demo", nullptr, nullptr);
    _items.push_back({ K_DEMO, "", r1, nullptr });

    // 3) Скрипты /ducky/*.txt.
    auto scripts = StorageModule::instance().listDir("/ducky", ".txt");
    for (auto& s : scripts) {
        lv_obj_t* r = makeRow(_list, ICON_FOLDER, cOrange(), s.c_str(), nullptr, nullptr);
        _items.push_back({ K_SCRIPT, s, r, nullptr });
    }

    lv_label_set_text_fmt(_status, "%u scripts", (unsigned)scripts.size());

    _selected = 0;
    for (size_t i = 0; i < _items.size(); ++i) select(i, false);
    if (!_items.empty()) select(0, true);
}

void BadUsbScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= (int)_items.size()) return;
    lv_obj_t* row = _items[idx].row;
    lv_obj_set_style_bg_color(row, cTint(), 0);
    lv_obj_set_style_bg_opa(row, on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    if (on) lv_obj_scroll_to_view(row, LV_ANIM_ON);
}

void BadUsbScreen::moveSelection(int delta) {
    if (_items.empty()) return;
    select(_selected, false);
    _selected = (_selected + delta + (int)_items.size()) % (int)_items.size();
    select(_selected, true);
}

void BadUsbScreen::activateSelected() {
    if (_selected < 0 || _selected >= (int)_items.size()) return;
    Item& it = _items[_selected];
    Settings& s = Settings::instance();

    switch (it.kind) {
        case K_LAYOUT: {
            uint8_t next = s.badusbLayout() == BadUsbModule::LAYOUT_US
                               ? BadUsbModule::LAYOUT_DE : BadUsbModule::LAYOUT_US;
            s.setBadusbLayout(next);
            BadUsbModule::instance().setLayout((BadUsbModule::Layout)next);
            if (it.val) lv_label_set_text(it.val, next == BadUsbModule::LAYOUT_DE ? "DE" : "US");
            break;
        }
        case K_DEMO:
            BadUsbModule::instance().runDemo();
            Notify::toast(tr(STR_SENT), Notify::Success);
            break;
        case K_SCRIPT:
            Notify::toast(tr(STR_RUN), Notify::Warn);
            if (BadUsbModule::instance().runScriptFile(it.name))
                Notify::toast(tr(STR_SENT), Notify::Success);
            else
                Notify::toast(tr(STR_EMPTY), Notify::Error);
            break;
    }
}

void BadUsbScreen::onShow() {
    rebuild();   // подхватить новые скрипты с SD
}

void BadUsbScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
