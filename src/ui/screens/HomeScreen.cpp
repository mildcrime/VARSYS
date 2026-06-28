#include "HomeScreen.h"
#include "ui/UITheme.h"
#include "ui/UIManager.h"
#include "ui/i18n.h"
#include "core/Settings.h"

using namespace ui;

void HomeScreen::addTile(const char* sym, lv_color_t color,
                         const char* label, const char* target) {
    if (_count >= kMax) return;
    lv_obj_t* ic = lv_obj_create(_row);
    lv_obj_remove_style_all(ic);
    lv_obj_set_size(ic, 72, 72);
    lv_obj_set_style_radius(ic, 20, 0);
    lv_obj_set_style_bg_color(ic, color, 0);
    lv_obj_set_style_bg_opa(ic, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ic, cBlue(), 0);
    lv_obj_clear_flag(ic, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_transform_pivot_x(ic, 36, 0);   // зум вокруг центра
    lv_obj_set_style_transform_pivot_y(ic, 36, 0);

    lv_obj_t* g = lv_label_create(ic);
    lv_label_set_text(g, sym);
    lv_obj_set_style_text_color(g, lv_color_white(), 0);
    lv_obj_set_style_text_font(g, &varsys_22, 0);
    lv_obj_center(g);

    _tiles[_count++] = { ic, target, label };
}

void HomeScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);

    _row = lv_obj_create(_root);
    lv_obj_remove_style_all(_row);
    lv_obj_set_size(_row, lv_pct(100), 132);
    lv_obj_align(_row, LV_ALIGN_CENTER, 0, -6);
    lv_obj_set_flex_flow(_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(_row, 24, 0);
    lv_obj_set_style_pad_left(_row, 124, 0);          // центрирование крайних
    lv_obj_set_style_pad_right(_row, 124, 0);
    lv_obj_set_scroll_dir(_row, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(_row, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(_row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(_row, scrollCb, LV_EVENT_SCROLL, this);

    _count = 0;
    addTile(ICON_ANTENNA,   lv_color_hex(0xFF9F0A), "Sub-GHz",        "SubGhz");
    addTile(ICON_INFRARED,  lv_color_hex(0xFF453A), "Infrared",       "Ir");
    addTile(ICON_NFC,       lv_color_hex(0x0A84FF), "NFC",            "Nfc");
    addTile(ICON_WIFI,      lv_color_hex(0x30B0C7), "WiFi",           "Wifi");
    addTile(ICON_BLUETOOTH, lv_color_hex(0x0A84FF), "Bluetooth",      "Ble");
    addTile(ICON_SIGNAL,    lv_color_hex(0x30D158), "GPS",            "Gps");
    addTile(ICON_ANTENNA,   lv_color_hex(0x5E5CE6), "NRF24",          "Nrf");
    addTile(ICON_NFC,       lv_color_hex(0xFFD60A), "iButton",        "IButton");
    addTile(ICON_SIGNAL,    lv_color_hex(0xFF375F), "FM",             "Fm");
    addTile(ICON_RECORD,    lv_color_hex(0xBF5AF2), "BadUSB",         "Badusb");
    addTile(ICON_LANGUAGE,  lv_color_hex(0x64D2FF), "WebUI",          "WebUi");
    addTile(ICON_FOLDER,    lv_color_hex(0x30D158), tr(STR_FILES),    nullptr);
    addTile(ICON_SETTINGS,  lv_color_hex(0x8E8E93), tr(STR_SETTINGS), "Settings");
    if (Settings::instance().expert())
        addTile(ICON_EXPERT, lv_color_hex(0xFF453A), tr(STR_EXPERT),  "Expert");

    _name = lv_label_create(_root);
    lv_obj_set_style_text_color(_name, cText(), 0);
    lv_obj_set_style_text_font(_name, &varsys_16, 0);
    lv_obj_align(_name, LV_ALIGN_BOTTOM_MID, 0, -2);
}

void HomeScreen::scrollCb(lv_event_t* e) {
    static_cast<HomeScreen*>(lv_event_get_user_data(e))->applyFocus();
}

void HomeScreen::applyFocus() {
    if (_count == 0) return;
    lv_area_t rc;
    lv_obj_get_coords(_row, &rc);
    int viewC = (rc.x1 + rc.x2) / 2;

    int best = 0, bestD = 1 << 30;
    for (int i = 0; i < _count; ++i) {
        lv_area_t tc;
        lv_obj_get_coords(_tiles[i].icon, &tc);
        int c = (tc.x1 + tc.x2) / 2;
        int d = c > viewC ? c - viewC : viewC - c;
        int nd = d > 150 ? 150 : d;
        int f = 150 - nd;                              // 0..150 (150 в центре)
        lv_obj_set_style_transform_zoom(_tiles[i].icon, 256 + f * 110 / 150, 0);
        lv_obj_set_style_opa(_tiles[i].icon, (lv_opa_t)(110 + f * 145 / 150), 0);
        if (d < bestD) { bestD = d; best = i; }
    }
    for (int i = 0; i < _count; ++i)
        lv_obj_set_style_border_width(_tiles[i].icon, i == best ? 3 : 0, 0);

    _selected = best;
    if (_name) lv_label_set_text(_name, _tiles[best].label);
}

void HomeScreen::onShow() {
    lv_obj_update_layout(_root);
    if (_selected >= _count) _selected = 0;
    lv_obj_scroll_to_view(_tiles[_selected].icon, LV_ANIM_OFF);
    applyFocus();
}

void HomeScreen::moveSelection(int delta) {
    int t = _selected + delta;
    if (t < 0) t = 0;
    if (t >= _count) t = _count - 1;
    lv_obj_scroll_to_view(_tiles[t].icon, LV_ANIM_ON);
}

void HomeScreen::openSelected() {
    const char* target = _tiles[_selected].target;
    if (target) UIManager::instance().pushScreen(target, LV_SCR_LOAD_ANIM_MOVE_LEFT);
}

void HomeScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1); break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1); break;
        case EventType::INPUT_BTN_CLICK:   openSelected();    break;
        default: break;
    }
}
