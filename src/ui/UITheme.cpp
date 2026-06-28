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

} // namespace ui
