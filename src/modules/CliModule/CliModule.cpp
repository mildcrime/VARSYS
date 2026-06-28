#include "CliModule.h"
#include "core/Logger.h"
#include "varsys_config.h"
#include "modules/RadioModule/RadioModule.h"
#include "modules/IrModule/IrModule.h"
#include "modules/WifiModule/WifiModule.h"
#include "modules/BleModule/BleModule.h"

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
        Serial.println("commands: ver | freq <kHz> | rssi | rec | replay | "
                        "ir tvoff | wifi scan | ble scan | reboot");
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
