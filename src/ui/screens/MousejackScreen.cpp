#include "MousejackScreen.h"
#include "ui/UITheme.h"
#include "ui/i18n.h"
#include "ui/Notify.h"
#include "modules/NrfModule/NrfModule.h"

using namespace ui;

void MousejackScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    styleScreen(_root);
    header(_root, "Mousejack");

    _status = lv_label_create(_root);
    lv_label_set_text(_status, "");
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
}

void MousejackScreen::rebuild() {
    lv_obj_clean(_list);
    _rows.clear();

    NrfModule& n = NrfModule::instance();
    for (int i = 0; i < n.mjCount(); ++i) {
        const auto& d = n.mjDevice(i);
        lv_obj_t* row = lv_obj_create(_list);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 28);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);

        lv_obj_t* ic = lv_label_create(row);
        lv_label_set_text(ic, ICON_ANTENNA);
        lv_obj_set_style_text_color(ic, cBlue(), 0);
        lv_obj_set_style_text_font(ic, &varsys_16, 0);
        lv_obj_align(ic, LV_ALIGN_LEFT_MID, 8, 0);

        lv_obj_t* lb = lv_label_create(row);
        lv_label_set_text_fmt(lb, "%02X:%02X:%02X:%02X:%02X",
                              d.addr[0], d.addr[1], d.addr[2], d.addr[3], d.addr[4]);
        lv_obj_set_style_text_color(lb, cText(), 0);
        lv_obj_set_style_text_font(lb, &varsys_14, 0);
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 34, 0);

        lv_obj_t* meta = lv_label_create(row);
        lv_label_set_text_fmt(meta, "ch%u", d.channel);
        lv_obj_set_style_text_color(meta, cText2(), 0);
        lv_obj_set_style_text_font(meta, &varsys_12, 0);
        lv_obj_align(meta, LV_ALIGN_RIGHT_MID, -8, 0);

        _rows.push_back(row);
    }

    lv_label_set_text_fmt(_status, "%d dev", (int)_rows.size());
    _selected = 0;
    if (!_rows.empty()) select(0, true);
}

void MousejackScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= (int)_rows.size()) return;
    lv_obj_set_style_bg_color(_rows[idx], cTint(), 0);
    lv_obj_set_style_bg_opa(_rows[idx], on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    if (on) lv_obj_scroll_to_view(_rows[idx], LV_ANIM_ON);
}

void MousejackScreen::moveSelection(int delta) {
    if (_rows.empty()) return;
    select(_selected, false);
    _selected = (_selected + delta + (int)_rows.size()) % (int)_rows.size();
    select(_selected, true);
}

void MousejackScreen::activateSelected() {
    NrfModule& n = NrfModule::instance();
    if (_selected < 0 || _selected >= n.mjCount()) return;
    // Демо-инъекция безобидной строки в выбранное (своё) устройство.
    Notify::toast(tr(STR_RUN), Notify::Warn);
    int sent = n.mjInject(n.mjDevice(_selected), "VARSYS mousejack test\n");
    lv_label_set_text_fmt(_status, "sent %d", sent);
    Notify::toast(sent > 0 ? tr(STR_SENT) : tr(STR_NO_TAG),
                  sent > 0 ? Notify::Success : Notify::Warn);
}

void MousejackScreen::onShow() {
    NrfModule& n = NrfModule::instance();
    if (!n.present()) {
        if (!_hwPanel)
            _hwPanel = ui::hwMissingPanel(_root, "NRF24L01", "QWIIC (SPI)");
        return;
    }
    if (_hwPanel) { lv_obj_del(_hwPanel); _hwPanel = nullptr; }

    lv_label_set_text(_status, tr(STR_SCANNING));
    lv_refr_now(NULL);
    n.mjScan(2000);     // promiscuous-скан (блокирующий ~2 с)
    rebuild();
}

void MousejackScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        default: break;
    }
}
