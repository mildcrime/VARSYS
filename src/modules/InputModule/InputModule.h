// ============================================================================
//  InputModule.h — Модуль ввода VARSYS (энкодер вращения + кнопка)
//
//  Опрашивает энкодер (квадратурный декодер) и кнопку (антидребезг + детекция
//  короткого/долгого нажатия) и публикует события в EventBus:
//    INPUT_ENCODER_CW / INPUT_ENCODER_CCW — поворот на один щелчок (детент)
//    INPUT_BTN_CLICK / INPUT_BTN_LONG     — нажатия кнопки
//
//  UI ничего не знает об источнике — подписан только на события.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "core/Module.h"

class InputModule : public IModule {
public:
    // --- IModule ---
    const char* name() const override { return "Input"; }
    bool init() override;
    void update(uint32_t now) override;

private:
    void pollEncoder();
    void pollButton(uint32_t now);
    void pollBack(uint32_t now);

    // --- Энкодер: табличный полношаговый декодер (Ben Buxton) ---
    uint8_t _encState = 0;          // текущее состояние конечного автомата

    // --- Кнопка ---
    bool     _btnStable   = false;  // отфильтрованное состояние (true = нажата)
    bool     _btnLastRaw  = false;  // последнее «сырое» чтение
    uint32_t _btnChangeAt = 0;      // время последнего изменения «сырого» уровня
    uint32_t _btnPressAt  = 0;      // время фиксации нажатия
    bool     _longFired   = false;  // долгое нажатие уже отправлено

    // --- Боковая кнопка «назад» ---
    bool     _backStable   = false;
    bool     _backLastRaw  = false;
    uint32_t _backChangeAt = 0;
};
