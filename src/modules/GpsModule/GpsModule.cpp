#include "GpsModule.h"
#include "core/Logger.h"
#include "hal/board_pins.h"
#include <TinyGPS++.h>

static const char* TAG = "Gps";

static TinyGPSPlus   s_gps;
static HardwareSerial s_serial(1);

GpsModule* GpsModule::_self = nullptr;

bool GpsModule::init() {
    _self = this;
    // Пины 43/44 — общий QWIIC-порт (GPS UART / NRF24 / iButton). НЕ занимаем
    // их при загрузке, иначе модули затирают друг друга. UART поднимается
    // лениво в acquire() (экран GPS / wardrive), освобождается в release().
    LOGI(TAG, "GPS ready (UART lazy on QWIIC %d/%d)", PIN_GPS_RX, PIN_GPS_TX);
    return true;
}

void GpsModule::acquire() {
    if (_active) return;
    s_serial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    _active = true;
    LOGI(TAG, "GPS UART up");
}

void GpsModule::release() {
    if (!_active) return;
    s_serial.end();
    pinMode(PIN_GPS_RX, INPUT);    // освобождаем общий QWIIC-порт
    pinMode(PIN_GPS_TX, INPUT);
    _active = false;
}

void GpsModule::update(uint32_t) {
    if (!_active) return;
    while (s_serial.available()) s_gps.encode((char)s_serial.read());
}

bool   GpsModule::hasFix()  { return s_gps.location.isValid() && s_gps.satellites.value() > 0; }
int    GpsModule::sats()    { return (int)s_gps.satellites.value(); }
double GpsModule::lat()     { return s_gps.location.lat(); }
double GpsModule::lng()     { return s_gps.location.lng(); }
uint32_t GpsModule::charsRx() { return s_gps.charsProcessed(); }
