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
    s_serial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    LOGI(TAG, "GPS UART rx=%d tx=%d @9600", PIN_GPS_RX, PIN_GPS_TX);
    return true;
}

void GpsModule::update(uint32_t) {
    while (s_serial.available()) s_gps.encode((char)s_serial.read());
}

bool   GpsModule::hasFix()  { return s_gps.location.isValid() && s_gps.satellites.value() > 0; }
int    GpsModule::sats()    { return (int)s_gps.satellites.value(); }
double GpsModule::lat()     { return s_gps.location.lat(); }
double GpsModule::lng()     { return s_gps.location.lng(); }
uint32_t GpsModule::charsRx() { return s_gps.charsProcessed(); }
