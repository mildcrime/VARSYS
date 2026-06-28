#include "InputModule.h"
#include "core/EventBus.h"
#include "core/Logger.h"
#include "hal/board_pins.h"
#include "varsys_config.h"

static const char* TAG = "Input";

// ----------------------------------------------------------------------------
//  Полношаговый квадратурный декодер (state machine, Ben Buxton).
//  Даёт ровно одно событие на детент, устойчив к дребезгу контактов.
// ----------------------------------------------------------------------------
#define R_START     0x0
#define R_CW_FINAL  0x1
#define R_CW_BEGIN  0x2
#define R_CW_NEXT   0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT  0x6

#define DIR_NONE    0x0
#define DIR_CW      0x10
#define DIR_CCW     0x20

static const uint8_t kTable[7][4] = {
    // R_START
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
    // R_CW_FINAL
    {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
    // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START},
    // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START | DIR_CCW},
};

bool InputModule::init() {
    pinMode(PIN_ENCODER_A,   INPUT_PULLUP);
    pinMode(PIN_ENCODER_B,   INPUT_PULLUP);
    pinMode(PIN_ENCODER_BTN, INPUT_PULLUP);
    pinMode(PIN_BTN_BACK,    INPUT_PULLUP);

    _encState   = R_START;
    _btnStable  = false;
    _btnLastRaw = false;

    LOGI(TAG, "Encoder A=%d B=%d BTN=%d BACK=%d ready",
         PIN_ENCODER_A, PIN_ENCODER_B, PIN_ENCODER_BTN, PIN_BTN_BACK);
    return true;
}

void InputModule::pollEncoder() {
    // Формируем 2-битный код пинов (A — старший бит).
    uint8_t pins = (digitalRead(PIN_ENCODER_A) << 1) | digitalRead(PIN_ENCODER_B);

    _encState = kTable[_encState & 0x0F][pins];
    uint8_t dir = _encState & 0x30;
    if (dir == DIR_NONE) return;

    bool cw = (dir == DIR_CW);
#if VARSYS_ENCODER_REVERSED
    cw = !cw;
#endif
    EventBus::publish(cw ? EventType::INPUT_ENCODER_CW
                         : EventType::INPUT_ENCODER_CCW, cw ? +1 : -1);
}

void InputModule::pollButton(uint32_t now) {
    int level = digitalRead(PIN_ENCODER_BTN);
#if VARSYS_BTN_ACTIVE_LOW
    bool raw = (level == LOW);
#else
    bool raw = (level == HIGH);
#endif

    // Антидребезг: реагируем только когда «сырой» уровень устойчив.
    if (raw != _btnLastRaw) {
        _btnLastRaw  = raw;
        _btnChangeAt = now;
        return;
    }
    if ((now - _btnChangeAt) < VARSYS_BTN_DEBOUNCE_MS) return;
    if (raw == _btnStable) {
        // Уровень не менялся — проверяем удержание для долгого нажатия.
        if (_btnStable && !_longFired &&
            (now - _btnPressAt) >= VARSYS_BTN_LONGPRESS_MS) {
            _longFired = true;
            EventBus::publish(EventType::INPUT_BTN_LONG);
        }
        return;
    }

    // Зафиксирован новый устойчивый уровень кнопки.
    _btnStable = raw;
    if (raw) {
        // Нажатие — запоминаем время, ждём отпускания/удержания.
        _btnPressAt = now;
        _longFired  = false;
    } else {
        // Отпускание — если долгое нажатие не сработало, это короткий клик.
        if (!_longFired) {
            EventBus::publish(EventType::INPUT_BTN_CLICK);
        }
    }
}

void InputModule::pollBack(uint32_t now) {
    // Боковая кнопка «назад», активный уровень LOW. Публикуем INPUT_BACK
    // на фронте нажатия (с антидребезгом).
    bool raw = (digitalRead(PIN_BTN_BACK) == LOW);
    if (raw != _backLastRaw) {
        _backLastRaw  = raw;
        _backChangeAt = now;
        return;
    }
    if ((now - _backChangeAt) < VARSYS_BTN_DEBOUNCE_MS) return;
    if (raw == _backStable) return;

    _backStable = raw;
    if (raw) EventBus::publish(EventType::INPUT_BACK);
}

void InputModule::update(uint32_t now) {
    pollEncoder();
    pollButton(now);
    pollBack(now);
}
