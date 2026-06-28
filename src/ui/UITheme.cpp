#include "UITheme.h"
#include "ui/i18n.h"

namespace ui {

bool darkMode = false;

void styleScreen(lv_obj_t* root) {
    lv_obj_remove_style_all(root);
    lv_obj_set_style_bg_color(root, cBg(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t* statusBar(lv_obj_t* parent, const char* title) {
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, lv_pct(100), 22);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    // Слева — заголовок. Время/батарея рисует глобальный StatusOverlay.
    lv_obj_t* t = lv_label_create(bar);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, cText(), 0);
    lv_obj_set_style_text_font(t, &varsys_14, 0);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 12, 0);
    return bar;
}

lv_obj_t* header(lv_obj_t* parent, const char* title) {
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, lv_pct(100), 24);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* back = lv_label_create(bar);
    lv_label_set_text_fmt(back, ICON_BACK " %s", tr(STR_MENU));
    lv_obj_set_style_text_color(back, cBlue(), 0);
    lv_obj_set_style_text_font(back, &varsys_14, 0);
    lv_obj_align(back, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t* t = lv_label_create(bar);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_color(t, cText(), 0);
    lv_obj_set_style_text_font(t, &varsys_14, 0);
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);
    // Время/батарея — глобальный StatusOverlay (верхний правый угол).
    return bar;
}

lv_obj_t* card(lv_obj_t* parent) {
    lv_obj_t* c = lv_obj_create(parent);
    lv_obj_remove_style_all(c);
    lv_obj_set_style_bg_color(c, cCard(), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, 14, 0);
    lv_obj_set_style_pad_all(c, 0, 0);
    lv_obj_set_style_clip_corner(c, true, 0);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    return c;
}

lv_obj_t* hwMissingPanel(lv_obj_t* parent, const char* hw, const char* port) {
    lv_obj_t* c = card(parent);
    lv_obj_set_size(c, 300, 110);
    lv_obj_align(c, LV_ALIGN_BOTTOM_MID, 0, -8);

    lv_obj_t* ic = lv_label_create(c);
    lv_label_set_text(ic, ICON_EXPERT);          // знак внимания
    lv_obj_set_style_text_color(ic, cOrange(), 0);
    lv_obj_set_style_text_font(ic, &varsys_22, 0);
    lv_obj_align(ic, LV_ALIGN_TOP_MID, 0, 12);

    // «Требуется: <железо>»
    lv_obj_t* req = lv_label_create(c);
    lv_label_set_text_fmt(req, "%s: %s", tr(STR_REQUIRES), hw);
    lv_obj_set_style_text_color(req, cText(), 0);
    lv_obj_set_style_text_font(req, &varsys_16, 0);
    lv_label_set_long_mode(req, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(req, 280);
    lv_obj_set_style_text_align(req, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(req, LV_ALIGN_CENTER, 0, 4);

    // «подключите к <порт>»
    lv_obj_t* hint = lv_label_create(c);
    lv_label_set_text_fmt(hint, "%s %s", tr(STR_CONNECT_TO), port);
    lv_obj_set_style_text_color(hint, cText2(), 0);
    lv_obj_set_style_text_font(hint, &varsys_12, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);

    return c;
}

} // namespace ui
