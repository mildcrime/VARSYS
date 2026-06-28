#include "IrModule.h"
#include "core/Logger.h"

static const char* TAG = "Ir";

IrModule* IrModule::_self = nullptr;

// Небольшой набор power-кодов NEC (адрес, команда) для разных ТВ — заготовка
// универсального пульта. Полную базу TV-B-Gone подключим как ресурс позже.
struct TvCode { uint8_t addr; uint8_t cmd; };
static const TvCode TV_OFF_CODES[] = {
    {0x00, 0x02},   // Samsung-подобный (пример)
    {0x04, 0x08},
    {0x40, 0x0C},
    {0x10, 0x12},
};
static const size_t TV_OFF_COUNT = sizeof(TV_OFF_CODES) / sizeof(TV_OFF_CODES[0]);

bool IrModule::init() {
    _self = this;
    _ir.begin();
    LOGI(TAG, "IR module ready");
    return true;
}

size_t IrModule::capture() {
    size_t n = _ir.captureRaw(_last, 4000, 512);   // до 4 с ожидания кадра
    LOGI(TAG, "captured %u pulses", (unsigned)n);
    return n;
}

bool IrModule::replayLast() {
    if (_last.empty()) return false;
    _ir.sendRaw(_last);
    LOGI(TAG, "replayed %u pulses", (unsigned)_last.size());
    return true;
}

void IrModule::sendTvOff() {
    for (size_t i = 0; i < TV_OFF_COUNT; ++i) {
        _ir.sendNEC(TV_OFF_CODES[i].addr, TV_OFF_CODES[i].cmd);
        delay(40);
    }
    LOGI(TAG, "TV-off sweep: %u codes", (unsigned)TV_OFF_COUNT);
}
