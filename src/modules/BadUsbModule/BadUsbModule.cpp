#include "BadUsbModule.h"
#include "core/Logger.h"
#include "USB.h"
#include "USBHIDKeyboard.h"

static const char* TAG = "BadUsb";

static USBHIDKeyboard s_kbd;

BadUsbModule* BadUsbModule::_self = nullptr;

bool BadUsbModule::init() {
    _self = this;
    s_kbd.begin();
    USB.begin();
    _ready = true;
    LOGI(TAG, "USB HID keyboard ready");
    return true;
}

// Сопоставление имени клавиши Ducky -> код.
static uint8_t keyFor(const String& k) {
    if (k == "ENTER")  return KEY_RETURN;
    if (k == "TAB")    return KEY_TAB;
    if (k == "ESC" || k == "ESCAPE") return KEY_ESC;
    if (k == "SPACE")  return ' ';
    if (k == "GUI" || k == "WINDOWS") return KEY_LEFT_GUI;
    if (k == "CTRL" || k == "CONTROL") return KEY_LEFT_CTRL;
    if (k == "ALT")    return KEY_LEFT_ALT;
    if (k == "SHIFT")  return KEY_LEFT_SHIFT;
    if (k == "DELETE" || k == "DEL") return KEY_DELETE;
    if (k == "UP")     return KEY_UP_ARROW;
    if (k == "DOWN")   return KEY_DOWN_ARROW;
    if (k == "LEFT")   return KEY_LEFT_ARROW;
    if (k == "RIGHT")  return KEY_RIGHT_ARROW;
    if (k.length() == 1) return (uint8_t)k[0];
    return 0;
}

void BadUsbModule::runLine(const String& line) {
    String s = line; s.trim();
    if (s.isEmpty() || s.startsWith("REM")) return;

    int sp = s.indexOf(' ');
    String cmd = (sp < 0) ? s : s.substring(0, sp);
    String arg = (sp < 0) ? "" : s.substring(sp + 1);

    if (cmd == "DELAY") {
        delay(arg.toInt());
    } else if (cmd == "STRING") {
        s_kbd.print(arg);
    } else if (cmd == "STRINGLN") {
        s_kbd.print(arg);
        s_kbd.write(KEY_RETURN);
    } else if (cmd == "ENTER") {
        s_kbd.write(KEY_RETURN);
    } else {
        // Возможно комбинация модификаторов: "GUI r", "CTRL ALT DELETE".
        uint8_t keys[6]; int n = 0;
        String tok = cmd; int idx = 0; String rest = s;
        while (rest.length() && n < 6) {
            int p = rest.indexOf(' ');
            String t = (p < 0) ? rest : rest.substring(0, p);
            rest = (p < 0) ? "" : rest.substring(p + 1);
            uint8_t k = keyFor(t);
            if (k) keys[n++] = k;
        }
        for (int i = 0; i < n; ++i) s_kbd.press(keys[i]);
        delay(10);
        s_kbd.releaseAll();
        (void)idx; (void)tok;
    }
}

void BadUsbModule::runScript(const String& script) {
    if (!_ready) return;
    int start = 0;
    while (start < (int)script.length()) {
        int nl = script.indexOf('\n', start);
        if (nl < 0) nl = script.length();
        runLine(script.substring(start, nl));
        start = nl + 1;
    }
    LOGI(TAG, "script done");
}

void BadUsbModule::runDemo() {
    // Безопасное демо: просто печатает строку (без открытия системных окон).
    runScript("STRING VARSYS BadUSB demo\n");
}
