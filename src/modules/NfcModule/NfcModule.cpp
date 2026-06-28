#include "NfcModule.h"
#include "core/Logger.h"

static const char* TAG = "Nfc";

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
