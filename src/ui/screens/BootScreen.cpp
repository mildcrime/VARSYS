#include "BootScreen.h"
#include "ui/UIManager.h"
#include "ui/fonts/varsys_fonts.h"
#include "varsys_config.h"

void BootScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    lv_obj_set_style_bg_color(_root, lv_color_black(), 0);

    // Логотип / название
    lv_obj_t* title = lv_label_create(_root);
    lv_label_set_text(title, VARSYS_NAME);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &varsys_22, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -20);

    // Версия
    lv_obj_t* ver = lv_label_create(_root);
    lv_label_set_text(ver, "v" VARSYS_VERSION);
    lv_obj_set_style_text_color(ver, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(ver, &varsys_14, 0);
    lv_obj_align(ver, LV_ALIGN_CENTER, 0, 10);

    // Индикатор загрузки
    _bar = lv_bar_create(_root);
    lv_obj_set_size(_bar, 120, 6);
    lv_obj_align(_bar, LV_ALIGN_CENTER, 0, 40);
    lv_bar_set_range(_bar, 0, 100);
    lv_bar_set_value(_bar, 0, LV_ANIM_OFF);
}

void BootScreen::onShow() {
    _shownAt = millis();
}

void BootScreen::onUpdate(uint32_t now) {
    const uint32_t elapsed = now - _shownAt;
    int pct = (int)((elapsed * 100) / VARSYS_BOOT_SPLASH_MS);
    if (pct > 100) pct = 100;
    lv_bar_set_value(_bar, pct, LV_ANIM_OFF);

    if (elapsed >= VARSYS_BOOT_SPLASH_MS) {
        UIManager::instance().setScreen("Home", LV_SCR_LOAD_ANIM_FADE_ON);
    }
}
