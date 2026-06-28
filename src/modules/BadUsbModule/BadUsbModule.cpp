#include "BadUsbModule.h"
#include "core/Logger.h"
#include "core/Settings.h"
#include "hal/SpiBus.h"
#include "modules/StorageModule/StorageModule.h"
#include <FS.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

static const char* TAG = "BadUsb";
static const char* DUCKY_DIR = "/ducky";

static USBHIDKeyboard s_kbd;

// Модификаторы HID (битовая маска report.modifiers).
static constexpr uint8_t M_SHIFT = 0x02;
static constexpr uint8_t M_ALTGR = 0x40;   // правый Alt

BadUsbModule* BadUsbModule::_self = nullptr;

bool BadUsbModule::init() {
    _self = this;
    s_kbd.begin();
    USB.begin();
    _layout = (Layout)Settings::instance().badusbLayout();
    _ready = true;
    LOGI(TAG, "USB HID keyboard ready (layout %s)",
         _layout == LAYOUT_DE ? "DE" : "US");
    return true;
}

// Преобразование ASCII-символа в (модификатор, HID usage) для раскладки хоста.
// Возвращает false для непечатаемых/неподдерживаемых символов.
static bool charToUsage(char c, BadUsbModule::Layout lay,
                        uint8_t& mod, uint8_t& usage) {
    mod = 0; usage = 0;

    // Буквы: a-z = usage 0x04.. ; в DE меняются местами y<->z.
    if (c >= 'a' && c <= 'z') {
        char b = c;
        if (lay == BadUsbModule::LAYOUT_DE) { if (b == 'y') b = 'z'; else if (b == 'z') b = 'y'; }
        usage = 0x04 + (b - 'a');
        return true;
    }
    if (c >= 'A' && c <= 'Z') {
        char b = c;
        if (lay == BadUsbModule::LAYOUT_DE) { if (b == 'Y') b = 'Z'; else if (b == 'Z') b = 'Y'; }
        usage = 0x04 + (b - 'A'); mod = M_SHIFT;
        return true;
    }
    // Цифры (usage одинаковы в обеих раскладках для самих цифр).
    if (c >= '1' && c <= '9') { usage = 0x1E + (c - '1'); return true; }
    if (c == '0') { usage = 0x27; return true; }
    if (c == ' ') { usage = 0x2C; return true; }

    if (lay == BadUsbModule::LAYOUT_DE) {
        // Немецкая раскладка T1 (валидировать на хосте).
        switch (c) {
            case '!': mod=M_SHIFT; usage=0x1E; return true;
            case '"': mod=M_SHIFT; usage=0x1F; return true;
            case '$': mod=M_SHIFT; usage=0x21; return true;
            case '%': mod=M_SHIFT; usage=0x22; return true;
            case '&': mod=M_SHIFT; usage=0x23; return true;
            case '/': mod=M_SHIFT; usage=0x24; return true;
            case '(': mod=M_SHIFT; usage=0x25; return true;
            case ')': mod=M_SHIFT; usage=0x26; return true;
            case '=': mod=M_SHIFT; usage=0x27; return true;
            case '?': mod=M_SHIFT; usage=0x2D; return true;
            case '+': usage=0x30; return true;
            case '*': mod=M_SHIFT; usage=0x30; return true;
            case '~': mod=M_ALTGR; usage=0x30; return true;
            case '#': usage=0x32; return true;
            case '\'':mod=M_SHIFT; usage=0x32; return true;
            case '-': usage=0x38; return true;
            case '_': mod=M_SHIFT; usage=0x38; return true;
            case '.': usage=0x37; return true;
            case ':': mod=M_SHIFT; usage=0x37; return true;
            case ',': usage=0x36; return true;
            case ';': mod=M_SHIFT; usage=0x36; return true;
            case '<': usage=0x64; return true;
            case '>': mod=M_SHIFT; usage=0x64; return true;
            case '|': mod=M_ALTGR; usage=0x64; return true;
            case '@': mod=M_ALTGR; usage=0x14; return true;   // AltGr+Q
            case '{': mod=M_ALTGR; usage=0x24; return true;   // AltGr+7
            case '[': mod=M_ALTGR; usage=0x25; return true;   // AltGr+8
            case ']': mod=M_ALTGR; usage=0x26; return true;   // AltGr+9
            case '}': mod=M_ALTGR; usage=0x27; return true;   // AltGr+0
            case '\\':mod=M_ALTGR; usage=0x2D; return true;   // AltGr+ß
            case '^': usage=0x35; return true;                // dead key
            default: return false;
        }
    }

    // Раскладка US (по умолчанию).
    switch (c) {
        case '!': mod=M_SHIFT; usage=0x1E; return true;
        case '@': mod=M_SHIFT; usage=0x1F; return true;
        case '#': mod=M_SHIFT; usage=0x20; return true;
        case '$': mod=M_SHIFT; usage=0x21; return true;
        case '%': mod=M_SHIFT; usage=0x22; return true;
        case '^': mod=M_SHIFT; usage=0x23; return true;
        case '&': mod=M_SHIFT; usage=0x24; return true;
        case '*': mod=M_SHIFT; usage=0x25; return true;
        case '(': mod=M_SHIFT; usage=0x26; return true;
        case ')': mod=M_SHIFT; usage=0x27; return true;
        case '-': usage=0x2D; return true;
        case '_': mod=M_SHIFT; usage=0x2D; return true;
        case '=': usage=0x2E; return true;
        case '+': mod=M_SHIFT; usage=0x2E; return true;
        case '[': usage=0x2F; return true;
        case '{': mod=M_SHIFT; usage=0x2F; return true;
        case ']': usage=0x30; return true;
        case '}': mod=M_SHIFT; usage=0x30; return true;
        case '\\':usage=0x31; return true;
        case '|': mod=M_SHIFT; usage=0x31; return true;
        case ';': usage=0x33; return true;
        case ':': mod=M_SHIFT; usage=0x33; return true;
        case '\'':usage=0x34; return true;
        case '"': mod=M_SHIFT; usage=0x34; return true;
        case '`': usage=0x35; return true;
        case '~': mod=M_SHIFT; usage=0x35; return true;
        case ',': usage=0x36; return true;
        case '<': mod=M_SHIFT; usage=0x36; return true;
        case '.': usage=0x37; return true;
        case '>': mod=M_SHIFT; usage=0x37; return true;
        case '/': usage=0x38; return true;
        case '?': mod=M_SHIFT; usage=0x38; return true;
        default: return false;
    }
}

void BadUsbModule::typeChar(char c) {
    if (c == '\t') { s_kbd.write(KEY_TAB); return; }
    uint8_t mod, usage;
    if (!charToUsage(c, _layout, mod, usage)) return;   // пропускаем неизвестные
    KeyReport r; memset(&r, 0, sizeof(r));
    r.modifiers = mod; r.keys[0] = usage;
    s_kbd.sendReport(&r);
    delay(4);
    KeyReport e; memset(&e, 0, sizeof(e));
    s_kbd.sendReport(&e);
    delay(4);
}

void BadUsbModule::typeString(const String& s) {
    for (size_t i = 0; i < s.length(); ++i) typeChar(s[i]);
}

// Сопоставление имени клавиши Ducky -> код (для комбинаций).
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
        typeString(arg);
    } else if (cmd == "STRINGLN") {
        typeString(arg);
        s_kbd.write(KEY_RETURN);
    } else if (cmd == "ENTER") {
        s_kbd.write(KEY_RETURN);
    } else {
        // Комбинация модификаторов: "GUI r", "CTRL ALT DELETE".
        uint8_t keys[6]; int n = 0;
        String rest = s;
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
    }
}

void BadUsbModule::runScript(const String& script) {
    if (!_ready) return;
    _layout = (Layout)Settings::instance().badusbLayout();   // актуальная раскладка
    int start = 0;
    while (start < (int)script.length()) {
        int nl = script.indexOf('\n', start);
        if (nl < 0) nl = script.length();
        runLine(script.substring(start, nl));
        start = nl + 1;
    }
    LOGI(TAG, "script done");
}

bool BadUsbModule::runScriptFile(const String& name) {
    StorageModule& st = StorageModule::instance();
    if (!st.fs()) return false;

    String path = String(DUCKY_DIR) + "/" + name;
    hal::SpiBusGuard guard;
    File f = st.fs()->open(path, FILE_READ);
    if (!f) { LOGE(TAG, "open failed: %s", path.c_str()); return false; }
    String script = f.readString();
    f.close();

    runScript(script);
    return true;
}

void BadUsbModule::runDemo() {
    // Безопасное демо: просто печатает строку (без открытия системных окон).
    runScript("STRING VARSYS BadUSB demo\n");
}
