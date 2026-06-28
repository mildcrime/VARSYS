#include "IButtonModule.h"
#include "core/Logger.h"
#include "hal/board_pins.h"
#include <OneWire.h>

static const char* TAG = "IButton";

static OneWire s_ow(PIN_IBUTTON);

IButtonModule* IButtonModule::_self = nullptr;

bool IButtonModule::init() {
    _self = this;
    LOGI(TAG, "1-Wire on pin %d", PIN_IBUTTON);
    return true;
}

bool IButtonModule::readKey(String& out) {
    if (!s_ow.reset()) return false;          // нет присутствия — ключа нет
    uint8_t rom[8];
    s_ow.write(0x33);                          // Read ROM
    for (int i = 0; i < 8; ++i) rom[i] = s_ow.read();

    if (OneWire::crc8(rom, 7) != rom[7]) {     // проверка CRC
        LOGW(TAG, "bad CRC");
        return false;
    }

    char buf[24];
    int p = 0;
    for (int i = 0; i < 8; ++i)
        p += snprintf(buf + p, sizeof(buf) - p, "%02X", rom[i]);
    out = buf;
    _last = out;
    LOGI(TAG, "key %s", out.c_str());
    return true;
}
