#include "HomeScreen.h"
#include "ui/UITheme.h"
#include "ui/UIManager.h"
#include "ui/i18n.h"
#include "core/Settings.h"

using namespace ui;

void HomeScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);

    statusBar(_root, "VARSYS");

    // Горизонтально прокручиваемый ряд плиток.
    _row = lv_obj_create(_root);
    lv_obj_remove_style_all(_row);
    lv_obj_set_size(_row, lv_pct(100), 132);
    lv_obj_align(_row, LV_ALIGN_BOTTOM_MID, 0, -8);
    lv_obj_set_flex_flow(_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(_row, 4, 0);
    lv_obj_set_style_pad_hor(_row, 12, 0);
    lv_obj_set_scroll_dir(_row, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(_row, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(_row, LV_SCROLLBAR_MODE_OFF);

    // Пункты меню: иконка, цвет плитки, подпись, целевой экран.
    int i = 0;
    buildTile(_row, i++, ICON_ANTENNA,   cOrange(), "Sub-GHz",   "SubGhz");
    buildTile(_row, i++, ICON_INFRARED,  cRed(),    "Infrared",  "Ir");
    buildTile(_row, i++, ICON_NFC,       cBlue(),   "NFC",       "Nfc");
    buildTile(_row, i++, ICON_WIFI,      cBlue(),   "WiFi",      "Wifi");
    buildTile(_row, i++, ICON_SIGNAL,    cGreen(),  "GPS",       "Gps");
    buildTile(_row, i++, ICON_ANTENNA,   cBlue(),   "NRF24",     "Nrf");
    buildTile(_row, i++, ICON_NFC,       cOrange(), "iButton",   "IButton");
    buildTile(_row, i++, ICON_SIGNAL,    cRed(),    "FM",        "Fm");
    buildTile(_row, i++, ICON_BLUETOOTH, cBlue(),   "Bluetooth", "Ble");
    buildTile(_row, i++, ICON_RECORD,    cOrange(), "BadUSB",    "Badusb");
    buildTile(_row, i++, ICON_LANGUAGE,  cBlue(),   "WebUI",     "WebUi");
    buildTile(_row, i++, ICON_FOLDER,    cGreen(),  tr(STR_FILES),    nullptr);
    buildTile(_row, i++, ICON_SETTINGS,  cGray(),   tr(STR_SETTINGS), "Settings");

    // Раздел «Эксперт» виден только при включённом режиме эксперта.
    if (Settings::instance().expert()) {
        buildTile(_row, i++, ICON_EXPERT, cRed(), tr(STR_EXPERT), "Expert");
    }
}

void HomeScreen::buildTile(lv_obj_t* row, int idx, const char* sym,
                           lv_color_t color, const char* label,
                           const char* target) {
    if (idx >= kMaxTiles) return;

    // Обёртка-колонка (плитка + подпись).
    lv_obj_t* wrap = lv_obj_create(row);
    lv_obj_remove_style_all(wrap);
    lv_obj_set_size(wrap, 70, 92);
    lv_obj_set_flex_flow(wrap, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(wrap, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(wrap, 6, 0);
    lv_obj_set_style_radius(wrap, 16, 0);
    lv_obj_clear_flag(wrap, LV_OBJ_FLAG_SCROLLABLE);

    // Цветная плитка-иконка.
    lv_obj_t* sq = lv_obj_create(wrap);
    lv_obj_remove_style_all(sq);
    lv_obj_set_size(sq, 50, 50);
    lv_obj_set_style_bg_color(sq, color, 0);
    lv_obj_set_style_bg_opa(sq, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(sq, 13, 0);
    lv_obj_clear_flag(sq, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* ic = lv_label_create(sq);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, lv_color_white(), 0);
    lv_obj_set_style_text_font(ic, &varsys_22, 0);
    lv_obj_center(ic);

    // Подпись.
    lv_obj_t* lb = lv_label_create(wrap);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_12, 0);

    _tiles[idx] = { wrap, sq, target };
    _count = idx + 1;
}

void HomeScreen::highlight(int idx, bool on) {
    if (idx < 0 || idx >= _count) return;
    Tile& t = _tiles[idx];
    if (on) {
        lv_obj_set_style_bg_color(t.wrap, cTint(), 0);
        lv_obj_set_style_bg_opa(t.wrap, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(t.square, 3, 0);
        lv_obj_set_style_border_color(t.square, cBlue(), 0);
    } else {
        lv_obj_set_style_bg_opa(t.wrap, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(t.square, 0, 0);
    }
}

void HomeScreen::onShow() {
    highlight(_selected, true);
}

void HomeScreen::moveSelection(int delta) {
    if (_count == 0) return;
    highlight(_selected, false);
    _selected = (_selected + delta + _count) % _count;
    highlight(_selected, true);
    lv_obj_scroll_to_view(_tiles[_selected].wrap, LV_ANIM_ON);
}

void HomeScreen::openSelected() {
    const char* target = _tiles[_selected].target;
    if (target) {
        UIManager::instance().pushScreen(target, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    }
}

void HomeScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1); break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1); break;
        case EventType::INPUT_BTN_CLICK:   openSelected();    break;
        default: break;
    }
}
