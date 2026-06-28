#include "CliModule.h"
#include "core/Logger.h"
#include "varsys_config.h"
#include "modules/RadioModule/RadioModule.h"
#include "modules/IrModule/IrModule.h"
#include "modules/WifiModule/WifiModule.h"
#include "modules/BleModule/BleModule.h"
#include "modules/NfcModule/NfcModule.h"
#include "modules/FmModule/FmModule.h"
#include "modules/NrfModule/NrfModule.h"
#include "modules/GpsModule/GpsModule.h"

static const char* TAG = "Cli";

bool CliModule::init() {
    Serial.println();
    Serial.println("VARSYS CLI — type 'help'");
    return true;
}

void CliModule::exec(const String& raw) {
    String line = raw; line.trim();
    if (line.isEmpty()) return;

    int sp = line.indexOf(' ');
    String cmd = (sp < 0) ? line : line.substring(0, sp);
    String arg = (sp < 0) ? "" : line.substring(sp + 1);

    if (cmd == "help") {
        Serial.println("commands: ver | hw | freq <kHz> | rssi | rec | replay | "
                        "ir tvoff | wifi scan | ble scan | reboot");
    } else if (cmd == "hw") {
        // Карта функциональность -> железо со статусом обнаружения.
        auto row = [](const char* feat, const char* hw, bool builtin, int det) {
            // det: -1 = н/д (встроенное), 0 = не найдено, 1 = обнаружено
            const char* st = builtin ? "built-in"
                           : det == 1 ? "DETECTED"
                           : "MISSING — connect it";
            Serial.printf("  %-10s %-22s %s\n", feat, hw, st);
        };
        Serial.println("Feature    Hardware               Status");
        row("Sub-GHz",  "CC1101",                true, -1);
        row("Infrared", "IR LED/RX",             true, -1);
        row("WiFi/BLE", "ESP32-S3",              true, -1);
        row("BadUSB",   "USB-C HID",             true, -1);
        row("RGB",      "onboard LEDs",          true, -1);
        row("Battery",  "BQ27220",               true, -1);
        row("SD",       "card slot",             true, -1);
        row("Audio",    "mic + NS4168",          true, -1);
        row("NFC",      "PN532 (I2C/QWIIC)",     false, NfcModule::instance().present());
        row("FM TX",    "Si4713 (I2C/QWIIC)",    false, FmModule::instance().present());
        row("NRF24",    "NRF24L01 (QWIIC/SPI)",  false, NrfModule::instance().present());
        row("GPS",      "GPS (QWIIC/UART)",      false, GpsModule::instance().charsRx() > 0);
        row("iButton",  "1-Wire probe (QWIIC)",  false, 0);
        Serial.println("note: GPS/NRF24/iButton share the QWIIC port (one at a time)");
    } else if (cmd == "ver") {
        Serial.printf("%s v%s\n", VARSYS_NAME, VARSYS_VERSION);
    } else if (cmd == "freq") {
        if (arg.length()) RadioModule::instance().setFreqKhz(arg.toInt());
        Serial.printf("freq=%lu kHz\n", (unsigned long)RadioModule::instance().freqKhz());
    } else if (cmd == "rssi") {
        Serial.printf("rssi=%d dBm\n", RadioModule::instance().rssi());
    } else if (cmd == "rec") {
        size_t n = RadioModule::instance().recordRaw();
        Serial.printf("recorded %u pulses (%s)\n", (unsigned)n,
                      RadioModule::instance().decodeLast().c_str());
    } else if (cmd == "replay") {
        Serial.println(RadioModule::instance().replayLast() ? "sent" : "empty");
    } else if (cmd == "ir" && arg == "tvoff") {
        IrModule::instance().sendTvOff();
        Serial.println("ir tv-off sent");
    } else if (cmd == "wifi" && arg == "scan") {
        int n = WifiModule::instance().scan();
        for (int i = 0; i < n; ++i) {
            const ApInfo& a = WifiModule::instance().aps()[i];
            Serial.printf("  %-24s ch%-2u %d dBm %s\n", a.ssid.c_str(),
                          a.channel, (int)a.rssi, a.locked ? "[L]" : "");
        }
    } else if (cmd == "ble" && arg == "scan") {
        int n = BleModule::instance().scan(4);
        for (int i = 0; i < n; ++i) {
            const BleDev& d = BleModule::instance().devices()[i];
            Serial.printf("  %-24s %s %d dBm\n", d.name.c_str(),
                          d.addr.c_str(), d.rssi);
        }
    } else if (cmd == "wifi" && arg == "hs") {
        Serial.printf("handshakes: %lu\n",
                      (unsigned long)WifiModule::instance().handshakeCount());
    } else if (cmd == "reboot") {
        Serial.println("rebooting...");
        delay(100);
        ESP.restart();
    } else {
        Serial.printf("unknown: %s (try 'help')\n", cmd.c_str());
    }
}

void CliModule::update(uint32_t) {
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (_buf.length()) { exec(_buf); _buf = ""; }
        } else if (_buf.length() < 160) {
            _buf += c;
        }
    }
}
