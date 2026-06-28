#include "NrfModule.h"
#include "core/Logger.h"
#include "hal/SpiBus.h"
#include "hal/board_pins.h"
#include "ui/UIManager.h"
#include <RF24.h>

static const char* TAG = "Nrf";

static RF24 s_radio(PIN_NRF_CE, PIN_NRF_CSN);

NrfModule* NrfModule::_self = nullptr;

bool NrfModule::init() {
    _self = this;

    hal::SpiBusGuard guard;
    // Общая шина SPI с дисплеем (как у SD/CC1101).
    bool ok = s_radio.begin(&UIManager::instance().display().spi());
    _present = ok && s_radio.isChipConnected();
    if (!_present) {
        LOGW(TAG, "NRF24 not detected");
        return true;   // не фатально
    }
    // Режим пассивного прослушивания несущей.
    s_radio.setAutoAck(false);
    s_radio.disableCRC();
    s_radio.setDataRate(RF24_1MBPS);
    s_radio.startListening();
    s_radio.stopListening();
    LOGI(TAG, "NRF24 ready");
    return true;
}

void NrfModule::resetScan() {
    memset(_activity, 0, sizeof(_activity));
}

void NrfModule::scanPass() {
    if (!_present) return;
    hal::SpiBusGuard guard;
    for (int ch = 0; ch < kChannels; ++ch) {
        s_radio.setChannel(ch);
        s_radio.startListening();
        delayMicroseconds(140);
        s_radio.stopListening();
        if (s_radio.testCarrier() && _activity[ch] < 99) _activity[ch]++;
    }
}
