#include "InfraRed.h"
#include "board_pins.h"
#include "core/Logger.h"
#include "driver/rmt.h"
#include "freertos/ringbuf.h"

static const char* TAG = "IR";

static constexpr rmt_channel_t IR_CH  = RMT_CHANNEL_1;   // отдельно от CC1101 (CH0)
static constexpr uint8_t  IR_DIV      = 80;              // 1 тик = 1 мкс
static constexpr uint16_t IR_MAX_DUR  = 32767;

namespace hal {

void InfraRed::begin() {
    pinMode(PIN_IR_RX, INPUT);
    pinMode(PIN_IR_TX, OUTPUT);
    digitalWrite(PIN_IR_TX, LOW);
    _ready = true;
    LOGI(TAG, "IR ready (RX=%d TX=%d)", PIN_IR_RX, PIN_IR_TX);
}

size_t InfraRed::captureRaw(std::vector<uint16_t>& out, uint32_t timeoutMs,
                            size_t maxPulses) {
    out.clear();

    rmt_config_t cfg = {};
    cfg.rmt_mode      = RMT_MODE_RX;
    cfg.channel       = IR_CH;
    cfg.gpio_num      = (gpio_num_t)PIN_IR_RX;
    cfg.clk_div       = IR_DIV;
    cfg.mem_block_num = 4;
    cfg.rx_config.filter_en          = true;
    cfg.rx_config.filter_ticks_thresh = 100;
    cfg.rx_config.idle_threshold      = 10000;   // 10 мс тишины = конец кадра
    if (rmt_config(&cfg) != ESP_OK ||
        rmt_driver_install(IR_CH, 2048, 0) != ESP_OK) {
        LOGE(TAG, "RMT RX install failed");
        return 0;
    }

    RingbufHandle_t rb = nullptr;
    rmt_get_ringbuf_handle(IR_CH, &rb);
    rmt_rx_start(IR_CH, true);

    size_t len = 0;
    rmt_item32_t* items =
        (rmt_item32_t*)xRingbufferReceive(rb, &len, pdMS_TO_TICKS(timeoutMs));
    if (items) {
        size_t n = len / sizeof(rmt_item32_t);
        for (size_t i = 0; i < n && out.size() < maxPulses; ++i) {
            if (items[i].duration0 == 0) break;
            out.push_back(items[i].duration0);
            if (items[i].duration1 == 0) break;
            out.push_back(items[i].duration1);
        }
        vRingbufferReturnItem(rb, items);
    }

    rmt_rx_stop(IR_CH);
    rmt_driver_uninstall(IR_CH);
    return out.size();
}

void InfraRed::sendRaw(const std::vector<uint16_t>& pulses, uint32_t carrierHz) {
    if (pulses.empty()) return;

    rmt_config_t cfg = {};
    cfg.rmt_mode      = RMT_MODE_TX;
    cfg.channel       = IR_CH;
    cfg.gpio_num      = (gpio_num_t)PIN_IR_TX;
    cfg.clk_div       = IR_DIV;
    cfg.mem_block_num = 4;
    cfg.tx_config.carrier_en           = true;
    cfg.tx_config.carrier_freq_hz      = carrierHz;
    cfg.tx_config.carrier_duty_percent = 33;
    cfg.tx_config.carrier_level        = RMT_CARRIER_LEVEL_HIGH;
    cfg.tx_config.idle_output_en       = true;
    cfg.tx_config.idle_level           = RMT_IDLE_LEVEL_LOW;
    if (rmt_config(&cfg) != ESP_OK ||
        rmt_driver_install(IR_CH, 0, 0) != ESP_OK) {
        LOGE(TAG, "RMT TX install failed");
        return;
    }

    // Плоский список (mark-first), дробим длинные под 15 бит.
    struct P { uint8_t lvl; uint16_t dur; };
    std::vector<P> flat;
    bool mark = true;   // первый импульс — mark (несущая включена)
    for (uint16_t d : pulses) {
        uint32_t rem = d ? d : 1;
        while (rem > 0) {
            uint16_t c = (rem > IR_MAX_DUR) ? IR_MAX_DUR : (uint16_t)rem;
            flat.push_back({(uint8_t)(mark ? 1 : 0), c});
            rem -= c;
        }
        mark = !mark;
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

    rmt_write_items(IR_CH, items.data(), items.size(), true);
    rmt_driver_uninstall(IR_CH);
    digitalWrite(PIN_IR_TX, LOW);
}

void InfraRed::sendNEC(uint8_t addr, uint8_t cmd) {
    // NEC: 9000 mark, 4500 space, 32 бита (addr, ~addr, cmd, ~cmd), стоп-mark.
    std::vector<uint16_t> p;
    p.push_back(9000);
    p.push_back(4500);
    uint8_t bytes[4] = { addr, (uint8_t)~addr, cmd, (uint8_t)~cmd };
    for (uint8_t b : bytes) {
        for (int i = 0; i < 8; ++i) {           // LSB first
            p.push_back(560);                   // mark
            p.push_back((b & (1 << i)) ? 1690 : 560);  // space: 1=1690, 0=560
        }
    }
    p.push_back(560);                           // финальный mark
    sendRaw(p, 38000);
}

} // namespace hal
