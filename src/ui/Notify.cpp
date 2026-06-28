#include "Notify.h"
#include "ui/UITheme.h"
#include "core/Scheduler.h"

using namespace ui;

namespace Notify {

static lv_obj_t* s_toast = nullptr;
static lv_obj_t* s_label = nullptr;
static uint32_t  s_hideTask = 0;

static void ensure() {
    if (s_toast) return;
    s_toast = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_toast);
    lv_obj_set_style_radius(s_toast, 10, 0);
    lv_obj_set_style_pad_hor(s_toast, 14, 0);
    lv_obj_set_style_pad_ver(s_toast, 8, 0);
    lv_obj_set_style_bg_opa(s_toast, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_toast, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_toast, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_toast, LV_OBJ_FLAG_HIDDEN);

    s_label = lv_label_create(s_toast);
    lv_obj_set_style_text_font(s_label, &varsys_14, 0);
    lv_obj_center(s_label);
}

static void colorsFor(Type t, lv_color_t& bg, lv_color_t& fg) {
    switch (t) {
        case Success: bg = cGreen();              fg = lv_color_white(); break;
        case Warn:    bg = cOrange();             fg = lv_color_white(); break;
        case Error:   bg = cRed();                fg = lv_color_white(); break;
        default:      bg = lv_color_hex(0x2c2c2e); fg = lv_color_white(); break;
    }
}

void toast(const char* msg, Type type, uint32_t ms) {
    ensure();

    lv_color_t bg, fg;
    colorsFor(type, bg, fg);
    lv_obj_set_style_bg_color(s_toast, bg, 0);
    lv_obj_set_style_text_color(s_label, fg, 0);
    lv_label_set_text(s_label, msg);

    lv_obj_clear_flag(s_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_toast);
    lv_obj_align(s_toast, LV_ALIGN_BOTTOM_MID, 0, -10);

    if (s_hideTask) Scheduler::instance().cancel(s_hideTask);
    s_hideTask = Scheduler::instance().after(ms, [] {
        if (s_toast) lv_obj_add_flag(s_toast, LV_OBJ_FLAG_HIDDEN);
        s_hideTask = 0;
    });
}

} // namespace Notify
