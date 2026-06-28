#include "SettingsScreen.h"
#include "ui/UITheme.h"
#include "ui/UIManager.h"
#include "ui/i18n.h"
#include "core/Settings.h"
#include "varsys_config.h"

using namespace ui;

lv_obj_t* SettingsScreen::makeRow(lv_obj_t* parent, const char* sym,
                                  lv_color_t color, const char* label,
                                  bool hasSwitch, bool swOn,
                                  const char* value, bool sep, Action act) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), 29);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    if (sep) {
        lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(row, cSep(), 0);
        lv_obj_set_style_border_width(row, 1, 0);
    }

    lv_obj_t* ic = lv_label_create(row);
    lv_label_set_text(ic, sym);
    lv_obj_set_style_text_color(ic, color, 0);
    lv_obj_set_style_text_font(ic, &varsys_16, 0);
    lv_obj_align(ic, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t* lb = lv_label_create(row);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, cText(), 0);
    lv_obj_set_style_text_font(lb, &varsys_14, 0);
    lv_obj_align(lb, LV_ALIGN_LEFT_MID, 34, 0);

    lv_obj_t* sw  = nullptr;
    lv_obj_t* val = nullptr;
    if (hasSwitch) {
        sw = lv_switch_create(row);
        lv_obj_set_size(sw, 40, 22);
        lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_set_style_bg_color(sw, cGreen(), LV_PART_INDICATOR | LV_STATE_CHECKED);
        if (swOn) lv_obj_add_state(sw, LV_STATE_CHECKED);
        lv_obj_clear_flag(sw, LV_OBJ_FLAG_CLICKABLE);   // управляем энкодером
    } else if (value) {
        val = lv_label_create(row);
        lv_label_set_text(val, value);
        // Интерактивные значения (язык/яркость/таймаут) подсвечиваем синим.
        bool interactive = (act == ACT_LANG || act == ACT_BRIGHT ||
                            act == ACT_TIMEOUT || act == ACT_BATTERY);
        lv_obj_set_style_text_color(val, interactive ? cBlue() : cText2(), 0);
        lv_obj_set_style_text_font(val, &varsys_14, 0);
        lv_obj_align(val, LV_ALIGN_RIGHT_MID, -10, 0);
    }

    if (_count < kMaxRows) _rows[_count++] = { row, sw, val, act };
    return row;
}

// Шаги яркости (0..255) и таймаута экрана (сек, 0 = выкл).
static const uint8_t  kBrightSteps[]  = { 51, 102, 153, 204, 255 };
static const uint16_t kTimeoutSteps[] = { 15, 30, 60, 120, 0 };

static uint8_t nextBright(uint8_t cur) {
    // Находим ближайший текущий шаг и берём следующий по кругу.
    int best = 0, bestd = 999;
    for (int i = 0; i < (int)(sizeof(kBrightSteps)); ++i) {
        int d = abs((int)kBrightSteps[i] - (int)cur);
        if (d < bestd) { bestd = d; best = i; }
    }
    return kBrightSteps[(best + 1) % (int)sizeof(kBrightSteps)];
}

static uint16_t nextTimeout(uint16_t cur) {
    const int n = sizeof(kTimeoutSteps) / sizeof(kTimeoutSteps[0]);
    for (int i = 0; i < n; ++i)
        if (kTimeoutSteps[i] == cur) return kTimeoutSteps[(i + 1) % n];
    return kTimeoutSteps[0];
}

static void fmtBright(char* buf, size_t n, uint8_t v) {
    snprintf(buf, n, "%d%%", (v * 100 + 127) / 255);
}

static void fmtTimeout(char* buf, size_t n, uint16_t sec) {
    if (sec == 0) snprintf(buf, n, "%s", tr(STR_OFF));
    else          snprintf(buf, n, "%us", sec);
}

void SettingsScreen::onCreate(lv_obj_t* parent) {
    _root = parent;
    _count = 0;
    styleScreen(_root);
    header(_root, tr(STR_SETTINGS));

    Settings& s = Settings::instance();

    // Левая колонка — тумблеры (прокручиваемая).
    lv_obj_t* left = card(_root);
    lv_obj_set_size(left, 142, 122);
    lv_obj_align(left, LV_ALIGN_BOTTOM_LEFT, 10, -8);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(left, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(left, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(left, LV_SCROLLBAR_MODE_OFF);
    makeRow(left, ICON_SOUND, cOrange(), tr(STR_SOUND), true, s.sound(), nullptr, true,  ACT_SOUND);
    makeRow(left, ICON_VIBRO, cRed(),    tr(STR_VIBRO), true, s.vibro(), nullptr, true,  ACT_VIBRO);
    makeRow(left, ICON_MOON,  lv_color_hex(0x5E5CE6), tr(STR_DARK), true, s.darkTheme(),
            nullptr, true, ACT_DARK);
    makeRow(left, ICON_SIGNAL, cGreen(), "LED", true, s.ledOn(), nullptr, true, ACT_LED);
    makeRow(left, ICON_EXPERT, cRed(), tr(STR_EXPERT_MODE), true, s.expert(),
            nullptr, false, ACT_EXPERT);

    // Правая колонка — значения (прокручиваемая: строк больше, чем влезает).
    lv_obj_t* right = card(_root);
    lv_obj_set_size(right, 148, 122);
    lv_obj_align(right, LV_ALIGN_BOTTOM_RIGHT, -10, -8);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_flag(right, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(right, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(right, LV_SCROLLBAR_MODE_OFF);

    char bright[8];   fmtBright(bright, sizeof(bright), s.brightness());
    char timeout[12]; fmtTimeout(timeout, sizeof(timeout), s.screenTimeoutSec());
    char ledbr[8];    fmtBright(ledbr, sizeof(ledbr), s.ledBrightness());

    makeRow(right, ICON_BRIGHTNESS, cGray(), tr(STR_BRIGHTNESS), false, false, bright,
            true, ACT_BRIGHT);
    makeRow(right, ICON_SIGNAL, cGreen(), "LED", false, false, ledbr, true, ACT_LEDBRIGHT);
    makeRow(right, ICON_MOON, lv_color_hex(0x5E5CE6), tr(STR_SCREEN_TIMEOUT), false, false,
            timeout, true, ACT_TIMEOUT);
    makeRow(right, ICON_LANGUAGE, cBlue(), tr(STR_LANGUAGE), false, false, tr(STR_LANG_NAME),
            true, ACT_LANG);
    makeRow(right, ICON_BATTERY, cGreen(), tr(STR_BATTERY), false, false, "›",
            true, ACT_BATTERY);
    makeRow(right, ICON_INFO, cGray(), tr(STR_ABOUT), false, false, "v" VARSYS_VERSION,
            false, ACT_NONE);
}

void SettingsScreen::select(int idx, bool on) {
    if (idx < 0 || idx >= _count) return;
    lv_obj_t* row = _rows[idx].obj;
    lv_obj_set_style_bg_color(row, cTint(), 0);
    lv_obj_set_style_bg_opa(row, on ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    if (on) lv_obj_scroll_to_view(row, LV_ANIM_ON);   // докрутить, если за краем
}

void SettingsScreen::onShow() {
    _selected = 0;
    for (int i = 0; i < _count; ++i) select(i, false);
    select(_selected, true);
}

void SettingsScreen::moveSelection(int delta) {
    if (_count == 0) return;
    select(_selected, false);
    _selected = (_selected + delta + _count) % _count;
    select(_selected, true);
}

void SettingsScreen::activateSelected() {
    Row& r = _rows[_selected];
    Settings& s = Settings::instance();
    switch (r.act) {
        case ACT_SOUND:  s.setSound(!s.sound());         break;
        case ACT_VIBRO:  s.setVibro(!s.vibro());         break;
        case ACT_DARK:   s.setDarkTheme(!s.darkTheme()); break;
        case ACT_EXPERT: s.setExpert(!s.expert());       break;
        case ACT_LED:    s.setLedOn(!s.ledOn());         break;
        case ACT_LANG:
            // Публикует LANG_CHANGED -> UIManager пересоберёт экраны.
            s.toggleLanguage();
            return;
        case ACT_BRIGHT: {
            uint8_t v = nextBright(s.brightness());
            s.setBrightness(v);
            UIManager::instance().display().setBrightness(v);   // применить сразу
            if (r.val) {
                char buf[8]; fmtBright(buf, sizeof(buf), v);
                lv_label_set_text(r.val, buf);
            }
            return;
        }
        case ACT_TIMEOUT: {
            uint16_t t = nextTimeout(s.screenTimeoutSec());
            s.setScreenTimeoutSec(t);
            if (r.val) {
                char buf[12]; fmtTimeout(buf, sizeof(buf), t);
                lv_label_set_text(r.val, buf);
            }
            return;
        }
        case ACT_LEDBRIGHT: {
            uint8_t v = nextBright(s.ledBrightness());
            s.setLedBrightness(v);   // LedModule применит через SETTINGS_CHANGED
            if (r.val) {
                char buf[8]; fmtBright(buf, sizeof(buf), v);
                lv_label_set_text(r.val, buf);
            }
            return;
        }
        case ACT_BATTERY:
            UIManager::instance().pushScreen("Battery");
            return;
        default: return;
    }
    // Обновляем визуальное состояние тумблера из источника истины.
    if (r.sw) {
        bool on = (r.act == ACT_SOUND)  ? s.sound()
                : (r.act == ACT_VIBRO)  ? s.vibro()
                : (r.act == ACT_EXPERT) ? s.expert()
                : (r.act == ACT_LED)    ? s.ledOn()
                : s.darkTheme();
        if (on) lv_obj_add_state(r.sw, LV_STATE_CHECKED);
        else    lv_obj_clear_state(r.sw, LV_STATE_CHECKED);
    }
}

void SettingsScreen::onEvent(const Event& e) {
    switch (e.type) {
        case EventType::INPUT_ENCODER_CW:  moveSelection(+1);  break;
        case EventType::INPUT_ENCODER_CCW: moveSelection(-1);  break;
        case EventType::INPUT_BTN_CLICK:   activateSelected(); break;
        // «Назад» (INPUT_BACK / долгое нажатие) обрабатывает UIManager.
        default: break;
    }
}
