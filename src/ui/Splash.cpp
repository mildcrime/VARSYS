#include "Splash.h"
#include "ui/UITheme.h"
#include "ui/fonts/varsys_fonts.h"
#include "core/Scheduler.h"

using namespace ui;

namespace Splash {

static lv_obj_t* s_root = nullptr;
static lv_obj_t* s_word = nullptr;
static lv_obj_t* s_line = nullptr;
static uint32_t  s_task = 0;

static void onFadeDone(lv_anim_t*) {
    if (s_root) lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);
}

void create() {
    if (s_root) return;
    s_root = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(s_root);
    lv_obj_set_size(s_root, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(s_root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(s_root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(s_root, LV_OBJ_FLAG_HIDDEN);

    s_word = lv_label_create(s_root);
    lv_label_set_text(s_word, "VARSYS");
    lv_obj_set_style_text_color(s_word, lv_color_white(), 0);
    lv_obj_set_style_text_font(s_word, &varsys_22, 0);
    lv_obj_align(s_word, LV_ALIGN_CENTER, 0, -10);

    s_line = lv_obj_create(s_root);
    lv_obj_remove_style_all(s_line);
    lv_obj_set_size(s_line, 0, 3);
    lv_obj_set_style_radius(s_line, 2, 0);
    lv_obj_set_style_bg_color(s_line, cBlue(), 0);
    lv_obj_set_style_bg_opa(s_line, LV_OPA_COVER, 0);
    lv_obj_align(s_line, LV_ALIGN_CENTER, 0, 18);
}

void play(uint32_t ms) {
    if (!s_root) create();

    // Поверх всего и полностью непрозрачно.
    lv_obj_clear_flag(s_root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(s_root);
    lv_obj_set_style_opa(s_root, LV_OPA_COVER, 0);

    // Сброс начального состояния слова/линии.
    lv_obj_set_style_opa(s_word, LV_OPA_TRANSP, 0);
    lv_obj_set_style_translate_y(s_word, 18, 0);
    lv_obj_set_style_text_letter_space(s_word, 12, 0);
    lv_obj_set_width(s_line, 0);
    lv_obj_align(s_line, LV_ALIGN_CENTER, 0, 18);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_word);
    lv_anim_set_time(&a, 650);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_exec_cb(&a, [](void* o, int32_t v) { lv_obj_set_style_opa((lv_obj_t*)o, (lv_opa_t)v, 0); });
    lv_anim_start(&a);
    lv_anim_set_values(&a, 18, 0);
    lv_anim_set_exec_cb(&a, [](void* o, int32_t v) { lv_obj_set_style_translate_y((lv_obj_t*)o, v, 0); });
    lv_anim_start(&a);
    lv_anim_set_values(&a, 12, 2);
    lv_anim_set_exec_cb(&a, [](void* o, int32_t v) { lv_obj_set_style_text_letter_space((lv_obj_t*)o, v, 0); });
    lv_anim_start(&a);

    lv_anim_t b;
    lv_anim_init(&b);
    lv_anim_set_var(&b, s_line);
    lv_anim_set_time(&b, 420);
    lv_anim_set_delay(&b, 650);
    lv_anim_set_path_cb(&b, lv_anim_path_ease_out);
    lv_anim_set_values(&b, 0, 150);
    lv_anim_set_exec_cb(&b, [](void* o, int32_t v) {
        lv_obj_set_width((lv_obj_t*)o, v);
        lv_obj_align((lv_obj_t*)o, LV_ALIGN_CENTER, 0, 18);
    });
    lv_anim_start(&b);

    // По истечении ms — плавно погасить оверлей и скрыть.
    if (s_task) Scheduler::instance().cancel(s_task);
    s_task = Scheduler::instance().after(ms, [] {
        lv_anim_t f;
        lv_anim_init(&f);
        lv_anim_set_var(&f, s_root);
        lv_anim_set_time(&f, 350);
        lv_anim_set_values(&f, 255, 0);
        lv_anim_set_exec_cb(&f, [](void* o, int32_t v) { lv_obj_set_style_opa((lv_obj_t*)o, (lv_opa_t)v, 0); });
        lv_anim_set_ready_cb(&f, onFadeDone);
        lv_anim_start(&f);
        s_task = 0;
    });
}

} // namespace Splash
