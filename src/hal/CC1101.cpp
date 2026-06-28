#include "CC1101.h"
#include "SpiBus.h"
#include "board_pins.h"
#include "core/Logger.h"
#include "driver/rmt.h"
#include "freertos/ringbuf.h"

static const char* TAG = "CC1101";

// --- Строб-команды ---
static constexpr uint8_t SRES  = 0x30;
static constexpr uint8_t SCAL  = 0x33;
static constexpr uint8_t SRX   = 0x34;
static constexpr uint8_t STX   = 0x35;
static constexpr uint8_t SIDLE = 0x36;

// --- Регистры ---
static constexpr uint8_t REG_IOCFG2   = 0x00;
static constexpr uint8_t REG_IOCFG0   = 0x02;
static constexpr uint8_t REG_PKTCTRL0 = 0x08;
static constexpr uint8_t REG_FREQ2    = 0x0D;
static constexpr uint8_t REG_FREQ1    = 0x0E;
static constexpr uint8_t REG_FREQ0    = 0x0F;
static constexpr uint8_t REG_MDMCFG4  = 0x10;
static constexpr uint8_t REG_MDMCFG3  = 0x11;
static constexpr uint8_t REG_MDMCFG2  = 0x12;
static constexpr uint8_t REG_FREND0   = 0x22;
static constexpr uint8_t REG_VERSION  = 0x31;   // статусный (burst)
static constexpr uint8_t REG_RSSI     = 0x34;   // статусный (burst)
static constexpr uint8_t REG_PATABLE  = 0x3E;

static constexpr uint8_t READ_SINGLE  = 0x80;
static constexpr uint8_t READ_BURST   = 0xC0;

namespace hal {

// CC1101 поддерживает ~6.5 МГц на header; берём 4 МГц с запасом.
static const SPISettings kCcSpi(4000000, MSBFIRST, SPI_MODE0);

// RAII доступа к чипу: общий мьютекс шины + транзакция SPIClass + CS LOW.
// Объект шины тот же, что у дисплея (TFT_eSPI), поэтому beginTransaction
// корректно переконфигурирует её под CC1101, а endTransaction отпускает.
namespace {
struct Access {
    SPIClass* spi;
    SpiBusGuard guard;                  // сериализация с отрисовкой дисплея
    explicit Access(SPIClass* s) : spi(s) {
        spi->beginTransaction(kCcSpi);
        digitalWrite(PIN_CC1101_CS, LOW);
        // Ждём готовности чипа (MISO уходит в LOW). Ограничено по времени.
        uint32_t t0 = micros();
        while (digitalRead(PIN_CC1101_MISO) == HIGH && (micros() - t0) < 1000) {}
    }
    ~Access() {
        digitalWrite(PIN_CC1101_CS, HIGH);
        spi->endTransaction();
    }
};
} // namespace

void CC1101::strobe(uint8_t cmd) {
    Access a(_spi);
    _spi->transfer(cmd);
}

void CC1101::writeReg(uint8_t addr, uint8_t val) {
    Access a(_spi);
    _spi->transfer(addr);
    _spi->transfer(val);
}

uint8_t CC1101::readReg(uint8_t addr) {
    Access a(_spi);
    _spi->transfer(addr | READ_SINGLE);
    return _spi->transfer(0x00);
}

uint8_t CC1101::readStatusReg(uint8_t addr) {
    Access a(_spi);
    _spi->transfer(addr | READ_BURST);
    return _spi->transfer(0x00);
}

void CC1101::reset() {
    // Ручная последовательность сброса по даташиту.
    digitalWrite(PIN_CC1101_CS, LOW);
    delayMicroseconds(10);
    digitalWrite(PIN_CC1101_CS, HIGH);
    delayMicroseconds(40);
    strobe(SRES);
    delay(1);
}

bool CC1101::begin(SPIClass* spi) {
    _spi = spi;
    if (!_spi) { LOGE(TAG, "no SPI instance"); return false; }

    pinMode(PIN_CC1101_CS, OUTPUT);
    digitalWrite(PIN_CC1101_CS, HIGH);
    pinMode(PIN_CC1101_GDO0, INPUT);
    pinMode(PIN_CC1101_GDO2, INPUT);
    pinMode(PIN_CC1101_SW0, OUTPUT);
    pinMode(PIN_CC1101_SW1, OUTPUT);

    // Шина — тот же объект SPIClass, что и у дисплея (общая, как в Bruce).
    reset();

    uint8_t ver = readStatusReg(REG_VERSION);
    _present = (ver != 0x00 && ver != 0xFF);
    if (!_present) {
        LOGW(TAG, "CC1101 not detected (VERSION=0x%02X)", ver);
        return false;
    }
    LOGI(TAG, "CC1101 detected (VERSION=0x%02X)", ver);

    baseConfig();
    setFrequencyKhz(_khz);
    enterIdle();
    return true;
}

void CC1101::baseConfig() {
    // Базовая конфигурация ASK/OOK, асинхронный последовательный режим
    // (демодулированные данные на GDO0). Значения — рабочая отправная точка.
    writeReg(REG_IOCFG2,   0x0E);   // GDO2 = carrier sense
    writeReg(REG_IOCFG0,   0x0D);   // GDO0 = асинхронный serial data
    writeReg(REG_PKTCTRL0, 0x32);   // async serial, бесконечная длина пакета
    writeReg(REG_MDMCFG2,  0x30);   // ASK/OOK, без синхрослова
    writeReg(REG_MDMCFG3,  0x93);   // baud
    writeReg(REG_MDMCFG4,  0xF7);   // полоса/baud
    writeReg(REG_FREND0,   0x11);   // PATABLE индекс для OOK
    writeReg(REG_PATABLE,  0xC0);   // макс. мощность
}

void CC1101::applyAntenna(uint32_t khz) {
    // Карта SW1/SW0 по диапазону (из прошивки Bruce для T-Embed CC1101):
    //   SW1:1 SW0:0 — ≤350 МГц (315)
    //   SW1:1 SW0:1 — 350..468 МГц (434)
    //   SW1:0 SW0:1 — >778 МГц (868/915)
    int band;
    if (khz <= 350000) band = 0;
    else if (khz < 468000) band = 1;
    else band = 2;
    if (band == _antenna) return;
    _antenna = band;

    switch (band) {
        case 0: digitalWrite(PIN_CC1101_SW1, HIGH); digitalWrite(PIN_CC1101_SW0, LOW);  break;
        case 1: digitalWrite(PIN_CC1101_SW1, HIGH); digitalWrite(PIN_CC1101_SW0, HIGH); break;
        default:digitalWrite(PIN_CC1101_SW1, LOW);  digitalWrite(PIN_CC1101_SW0, HIGH); break;
    }
    delay(10);   // время установления антенного тракта
}

void CC1101::setFrequencyKhz(uint32_t khz) {
    if (khz < 280000 || khz > 928000) khz = 433920;
    _khz = khz;
    applyAntenna(khz);

    // FREQ = f_hz * 2^16 / Fxtal,  Fxtal = 26 МГц.
    uint64_t fhz  = (uint64_t)khz * 1000ULL;
    uint32_t word = (uint32_t)((fhz << 16) / 26000000ULL);
    enterIdle();
    writeReg(REG_FREQ2, (word >> 16) & 0xFF);
    writeReg(REG_FREQ1, (word >> 8) & 0xFF);
    writeReg(REG_FREQ0, word & 0xFF);
    strobe(SCAL);
    delay(1);
}

int CC1101::rssiDbm() {
    uint8_t raw = readStatusReg(REG_RSSI);
    int rssi = (raw >= 128) ? ((int)raw - 256) / 2 - 74 : (int)raw / 2 - 74;
    return rssi;
}

String CC1101::diag() {
    if (!_present) return "CC1101: n/a";
    uint8_t ver  = readStatusReg(0x31);   // VERSION
    uint8_t part = readStatusReg(0x30);   // PARTNUM
    uint8_t marc = readStatusReg(0x35);   // MARCSTATE
    char buf[64];
    snprintf(buf, sizeof(buf), "CC1101 v%02X part%02X\nmarc %02X  %lu kHz",
             ver, part, marc & 0x1F, (unsigned long)_khz);
    return String(buf);
}

void CC1101::enterRx() {
    pinMode(PIN_CC1101_GDO0, INPUT);
    strobe(SRX);
}

void CC1101::enterIdle() {
    strobe(SIDLE);
}

// Канал RMT для GDO0 (один на приём и передачу, переконфигурируется).
// ВАЖНО: FastLED (RGB) захватывает RMT_CHANNEL_0 на первой show() (вспышка при
// загрузке) и держит постоянно -> CC1101 на канале 0 не смог бы поставить
// драйвер (rmt_driver_install падает) и запись/воспроизведение Sub-GHz не
// работали бы. Поэтому канал 2 (FastLED=0, IR=3).
static constexpr rmt_channel_t RMT_CH = RMT_CHANNEL_2;
static constexpr uint8_t  RMT_DIV     = 80;     // 80 МГц / 80 = 1 тик = 1 мкс
static constexpr uint16_t RMT_MAX_DUR = 32767;  // 15-битное поле длительности

size_t CC1101::captureRaw(std::vector<uint16_t>& out, bool& startHigh,
                          uint32_t timeoutMs, size_t maxPulses) {
    out.clear();
    startHigh = true;

    enterRx();          // CC1101: async-режим, демод-данные на GDO0
    delay(1);

    rmt_config_t cfg = {};
    cfg.rmt_mode       = RMT_MODE_RX;
    cfg.channel        = RMT_CH;
    cfg.gpio_num       = (gpio_num_t)PIN_CC1101_GDO0;
    cfg.clk_div        = RMT_DIV;
    cfg.mem_block_num  = 4;
    cfg.rx_config.filter_en          = true;
    cfg.rx_config.filter_ticks_thresh = 100;    // подавление глитчей
    cfg.rx_config.idle_threshold      = 12000;  // 12 мс тишины = конец фрейма
    if (rmt_config(&cfg) != ESP_OK ||
        rmt_driver_install(RMT_CH, 4096, 0) != ESP_OK) {
        LOGE(TAG, "RMT RX install failed");
        enterIdle();
        return 0;
    }

    RingbufHandle_t rb = nullptr;
    rmt_get_ringbuf_handle(RMT_CH, &rb);
    rmt_rx_start(RMT_CH, true);

    size_t len = 0;
    rmt_item32_t* items =
        (rmt_item32_t*)xRingbufferReceive(rb, &len, pdMS_TO_TICKS(timeoutMs));
    if (items) {
        size_t n = len / sizeof(rmt_item32_t);
        startHigh = items[0].level0;
        for (size_t i = 0; i < n && out.size() < maxPulses; ++i) {
            if (items[i].duration0 == 0) break;
            out.push_back(items[i].duration0);
            if (items[i].duration1 == 0) break;
            out.push_back(items[i].duration1);
        }
        vRingbufferReturnItem(rb, items);
    }

    rmt_rx_stop(RMT_CH);
    rmt_driver_uninstall(RMT_CH);
    pinMode(PIN_CC1101_GDO0, INPUT);
    enterIdle();
    return out.size();
}

void CC1101::txBegin() {
    pinMode(PIN_CC1101_GDO0, OUTPUT);
    digitalWrite(PIN_CC1101_GDO0, LOW);
    strobe(STX);
    delay(1);
}

void CC1101::txPulse(int durUs) {
    digitalWrite(PIN_CC1101_GDO0, durUs > 0 ? HIGH : LOW);
    delayMicroseconds(durUs > 0 ? durUs : -durUs);
}

void CC1101::txEnd() {
    digitalWrite(PIN_CC1101_GDO0, LOW);
    enterIdle();
    pinMode(PIN_CC1101_GDO0, INPUT);
}

void CC1101::transmitRaw(const std::vector<uint16_t>& pulses, bool startHigh) {
    if (pulses.empty()) return;

    // Плоский список импульсов с дроблением длинных под 15-битное поле RMT.
    struct P { uint8_t lvl; uint16_t dur; };
    std::vector<P> flat;
    bool lvl = startHigh;
    for (uint16_t d : pulses) {
        uint32_t rem = d ? d : 1;
        while (rem > 0) {
            uint16_t c = (rem > RMT_MAX_DUR) ? RMT_MAX_DUR : (uint16_t)rem;
            flat.push_back({(uint8_t)(lvl ? 1 : 0), c});
            rem -= c;
        }
        lvl = !lvl;
    }

    std::vector<rmt_item32_t> items;
    items.reserve(flat.size() / 2 + 1);
    for (size_t i = 0; i < flat.size(); i += 2) {
        rmt_item32_t it = {};
        it.level0 = flat[i].lvl;
        it.duration0 = flat[i].dur;
        if (i + 1 < flat.size()) {
            it.level1 = flat[i + 1].lvl;
            it.duration1 = flat[i + 1].dur;
        }
        items.push_back(it);
    }

    rmt_config_t cfg = {};
    cfg.rmt_mode      = RMT_MODE_TX;
    cfg.channel       = RMT_CH;
    cfg.gpio_num      = (gpio_num_t)PIN_CC1101_GDO0;
    cfg.clk_div       = RMT_DIV;
    cfg.mem_block_num = 4;
    cfg.tx_config.carrier_en     = false;
    cfg.tx_config.idle_output_en = true;
    cfg.tx_config.idle_level     = RMT_IDLE_LEVEL_LOW;
    if (rmt_config(&cfg) != ESP_OK ||
        rmt_driver_install(RMT_CH, 0, 0) != ESP_OK) {
        LOGE(TAG, "RMT TX install failed");
        return;
    }

    strobe(STX);        // CC1101 в передачу (данные с GDO0 — от RMT)
    delay(1);
    rmt_write_items(RMT_CH, items.data(), items.size(), true);   // ждём конца
    rmt_driver_uninstall(RMT_CH);

    pinMode(PIN_CC1101_GDO0, INPUT);
    enterIdle();
}

} // namespace hal
