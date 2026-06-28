#include "NfcModule.h"
#include "core/Logger.h"
#include "hal/SpiBus.h"
#include "modules/StorageModule/StorageModule.h"

static const char* TAG = "Nfc";

// Словарь распространённых ключей Mifare Classic (как в Flipper/mfoc).
static const uint8_t DICT[][6] = {
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, {0x00,0x00,0x00,0x00,0x00,0x00},
    {0xA0,0xA1,0xA2,0xA3,0xA4,0xA5}, {0xD3,0xF7,0xD3,0xF7,0xD3,0xF7},
    {0xA0,0xB0,0xC0,0xD0,0xE0,0xF0}, {0xB0,0xB1,0xB2,0xB3,0xB4,0xB5},
    {0x4D,0x3A,0x99,0xC3,0x51,0xDD}, {0x1A,0x98,0x2C,0x7E,0x45,0x9A},
    {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF}, {0x71,0x4C,0x5C,0x88,0x6E,0x97},
    {0x58,0x7E,0xE5,0xF9,0x35,0x0F}, {0xA0,0x47,0x8C,0xC3,0x90,0x91},
    {0x53,0x3C,0xB6,0xC7,0x23,0xF6}, {0x8F,0xD0,0xA4,0xF2,0x56,0xE9},
};
static constexpr int DICT_N = sizeof(DICT) / 6;

static void hex2(char* p, uint8_t b) {
    static const char* H = "0123456789ABCDEF";
    p[0] = H[b >> 4]; p[1] = H[b & 0xF];
}

NfcModule* NfcModule::_self = nullptr;

bool NfcModule::init() {
    _self = this;

    // Wire уже поднят PowerModule на SDA 8 / SCL 18.
    _nfc.begin();
    uint32_t ver = _nfc.getFirmwareVersion();
    _present = (ver != 0);
    if (!_present) {
        LOGW(TAG, "PN532 not found");
        return true;     // не фатально — экран покажет «нет модуля»
    }
    _nfc.SAMConfig();
    LOGI(TAG, "PN532 ready (fw 0x%08lX)", (unsigned long)ver);
    return true;
}

bool NfcModule::readTag(String& uidOut, String& typeOut, uint16_t timeoutMs) {
    if (!_present) return false;

    uint8_t uid[7] = {0};
    uint8_t uidLen = 0;
    bool ok = _nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, timeoutMs);
    if (!ok || uidLen == 0) return false;

    char buf[24];
    int p = 0;
    for (uint8_t i = 0; i < uidLen && p < (int)sizeof(buf) - 3; ++i)
        p += snprintf(buf + p, sizeof(buf) - p, "%02X", uid[i]);
    uidOut = buf;

    // Грубая классификация по длине UID.
    typeOut = (uidLen == 4) ? "Mifare Classic"
            : (uidLen == 7) ? "Mifare Ultralight/NTAG"
            : "ISO14443A";
    _lastUid = uidOut;
    LOGI(TAG, "tag UID=%s (%s)", uidOut.c_str(), typeOut.c_str());
    return true;
}

bool NfcModule::writeBlock(uint8_t block, const uint8_t data[16]) {
    if (!_present) return false;
    uint8_t uid[7] = {0}, uidLen = 0;
    if (!_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 800)) return false;

    uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (!_nfc.mifareclassic_AuthenticateBlock(uid, uidLen, block, 0, keyA)) {
        LOGW(TAG, "auth failed block %u", block);
        return false;
    }
    bool ok = _nfc.mifareclassic_WriteDataBlock(block, (uint8_t*)data);
    LOGI(TAG, "write block %u -> %d", block, ok);
    return ok;
}

bool NfcModule::dumpClassic(ClassicDump& out) {
    if (!_present) return false;
    out = ClassicDump{};

    uint8_t uidLen = 0;
    if (!_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, out.uid, &uidLen, 1000) ||
        uidLen == 0)
        return false;
    out.uidLen = uidLen;

    for (int s = 0; s < kSectors; ++s) {
        const uint8_t first = s * 4;     // первый блок сектора

        // Подбор ключа A по словарю (auth на первый блок сектора).
        for (int k = 0; k < DICT_N; ++k) {
            if (_nfc.mifareclassic_AuthenticateBlock(out.uid, uidLen, first, 0,
                                                     (uint8_t*)DICT[k])) {
                memcpy(out.keyA[s], DICT[k], 6); out.keyAok[s] = true; break;
            }
        }
        // Если ключ A найден — читаем 4 блока сектора (auth уже активна).
        if (out.keyAok[s]) {
            for (int b = 0; b < 4; ++b) {
                int idx = first + b;
                if (_nfc.mifareclassic_ReadDataBlock(idx, out.block[idx])) {
                    out.blockOk[idx] = true; out.blocksRead++;
                }
            }
        }
        // Подбор ключа B (для полноты / записи), отдельная auth.
        for (int k = 0; k < DICT_N; ++k) {
            if (_nfc.mifareclassic_AuthenticateBlock(out.uid, uidLen, first, 1,
                                                     (uint8_t*)DICT[k])) {
                memcpy(out.keyB[s], DICT[k], 6); out.keyBok[s] = true; break;
            }
        }
    }
    LOGI(TAG, "dump: %d/%d blocks read", out.blocksRead, kBlocks);
    return true;
}

bool NfcModule::saveDump(const ClassicDump& d, String& pathOut) {
    StorageModule& st = StorageModule::instance();
    if (!st.fs()) return false;

    char uid[16] = {0}; int p = 0;
    for (int i = 0; i < d.uidLen && p < 14; ++i) { hex2(uid + p, d.uid[i]); p += 2; }

    String body = "Filetype: VARSYS NFC dump\nType: Mifare Classic 1K\nUID: ";
    body += uid; body += "\n";

    char line[80];
    for (int s = 0; s < kSectors; ++s) {
        int n = snprintf(line, sizeof(line), "Sector %d KeyA:", s);
        for (int i = 0; i < 6; ++i) { hex2(line + n, d.keyAok[s] ? d.keyA[s][i] : 0); n += 2; }
        n += snprintf(line + n, sizeof(line) - n, " KeyB:");
        for (int i = 0; i < 6; ++i) { hex2(line + n, d.keyBok[s] ? d.keyB[s][i] : 0); n += 2; }
        line[n] = 0; body += line; body += "\n";
    }
    for (int b = 0; b < kBlocks; ++b) {
        int n = snprintf(line, sizeof(line), "Block %02d: ", b);
        if (d.blockOk[b]) for (int i = 0; i < 16; ++i) { hex2(line + n, d.block[b][i]); n += 2; }
        else             { memcpy(line + n, "??", 2); n += 2; }   // нечитанный
        line[n] = 0; body += line; body += "\n";
    }

    if (!st.exists("/nfc")) { hal::SpiBusGuard g; st.fs()->mkdir("/nfc"); }
    pathOut = String("/nfc/") + uid + ".dump";
    return st.writeFile(pathOut, body);
}

static int hexNib(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool NfcModule::loadDump(const String& name, ClassicDump& d) {
    String path = name.startsWith("/") ? name : String("/nfc/") + name;
    String body = StorageModule::instance().readFile(path);
    if (body.isEmpty()) return false;
    d = ClassicDump{};

    int pos = 0;
    while (pos < (int)body.length()) {
        int nl = body.indexOf('\n', pos);
        if (nl < 0) nl = body.length();
        String ln = body.substring(pos, nl);
        pos = nl + 1;

        if (ln.startsWith("Sector ")) {
            int s = ln.substring(7).toInt();
            int a = ln.indexOf("KeyA:"); int b = ln.indexOf("KeyB:");
            if (s >= 0 && s < kSectors && a >= 0) {
                for (int i = 0; i < 6; ++i) {
                    int hi = hexNib(ln[a + 5 + i*2]), lo = hexNib(ln[a + 6 + i*2]);
                    if (hi >= 0 && lo >= 0) d.keyA[s][i] = (hi << 4) | lo;
                }
                d.keyAok[s] = true;
                if (b >= 0) {
                    for (int i = 0; i < 6; ++i) {
                        int hi = hexNib(ln[b + 5 + i*2]), lo = hexNib(ln[b + 6 + i*2]);
                        if (hi >= 0 && lo >= 0) d.keyB[s][i] = (hi << 4) | lo;
                    }
                    d.keyBok[s] = true;
                }
            }
        } else if (ln.startsWith("Block ")) {
            int blk = ln.substring(6).toInt();
            int colon = ln.indexOf(": ");
            if (blk >= 0 && blk < kBlocks && colon >= 0 && ln.indexOf("??") < 0) {
                int h = colon + 2; bool ok = true;
                for (int i = 0; i < 16; ++i) {
                    int hi = hexNib(ln[h + i*2]), lo = hexNib(ln[h + 1 + i*2]);
                    if (hi < 0 || lo < 0) { ok = false; break; }
                    d.block[blk][i] = (hi << 4) | lo;
                }
                if (ok) { d.blockOk[blk] = true; d.blocksRead++; }
            }
        }
    }
    return d.blocksRead > 0;
}

int NfcModule::cloneDump(const ClassicDump& d) {
    if (!_present) return 0;
    uint8_t uid[7] = {0}, uidLen = 0;
    if (!_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 1000)) return 0;

    int written = 0;
    for (int s = 0; s < kSectors; ++s) {
        const uint8_t first = s * 4;
        // Авторизуемся найденным ключом (предпочтительно B, иначе A).
        bool auth = false;
        if (d.keyBok[s])
            auth = _nfc.mifareclassic_AuthenticateBlock(uid, uidLen, first, 1, (uint8_t*)d.keyB[s]);
        if (!auth && d.keyAok[s])
            auth = _nfc.mifareclassic_AuthenticateBlock(uid, uidLen, first, 0, (uint8_t*)d.keyA[s]);
        if (!auth) continue;

        for (int b = 0; b < 3; ++b) {            // только дата-блоки (не трейлер)
            int idx = first + b;
            if (idx == 0) continue;              // блок 0 — только на magic-карте
            if (!d.blockOk[idx]) continue;
            if (_nfc.mifareclassic_WriteDataBlock(idx, (uint8_t*)d.block[idx])) written++;
        }
    }
    LOGI(TAG, "clone: %d blocks written", written);
    return written;
}
