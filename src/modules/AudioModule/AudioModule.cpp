#include "AudioModule.h"
#include "core/Logger.h"
#include "core/EventBus.h"
#include "core/Settings.h"
#include "hal/board_pins.h"
#include "driver/i2s.h"
#include <math.h>

static const char* TAG = "Audio";
static constexpr int SAMPLE_RATE = 16000;

AudioModule* AudioModule::_self = nullptr;

bool AudioModule::init() {
    _self = this;
    // ВНИМАНИЕ: PIN_SPK_WCLK (I2S WS) == 40 == PIN_LCD_RST (сброс дисплея).
    // Любая инициализация I2S дёргает линию сброса панели -> белый экран.
    // Поэтому стартовый бип ОТКЛЮЧЁН и аудио не трогает пин 40, пока не
    // выяснены корректные I2S-пины амперага T-Embed CC1101.
    LOGI(TAG, "Audio ready (muted: I2S WS clashes with LCD_RST=40)");
    return true;
}

void AudioModule::tone(uint32_t freqHz, uint32_t ms) {
    i2s_config_t cfg = {};
    cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
    cfg.sample_rate = SAMPLE_RATE;
    cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    cfg.intr_alloc_flags = 0;
    cfg.dma_buf_count = 4;
    cfg.dma_buf_len = 256;
    cfg.use_apll = false;

    i2s_pin_config_t pins = {};
    pins.bck_io_num   = PIN_SPK_BCLK;
    pins.ws_io_num    = PIN_SPK_WCLK;
    pins.data_out_num = PIN_SPK_DOUT;
    pins.data_in_num  = I2S_PIN_NO_CHANGE;

    if (i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr) != ESP_OK) return;
    i2s_set_pin(I2S_NUM_0, &pins);

    const int samples = SAMPLE_RATE * ms / 1000;
    const float w = 2.0f * (float)M_PI * freqHz / SAMPLE_RATE;
    int16_t buf[256];
    int written = 0;
    while (written < samples) {
        int n = (samples - written) > 256 ? 256 : (samples - written);
        for (int i = 0; i < n; ++i)
            buf[i] = (int16_t)(sinf(w * (written + i)) * 8000);
        size_t bw = 0;
        i2s_write(I2S_NUM_0, buf, n * sizeof(int16_t), &bw, portMAX_DELAY);
        written += n;
    }
    i2s_driver_uninstall(I2S_NUM_0);   // освобождаем пины (WS=40=TFT_RST)
}

void AudioModule::beep() {
    tone(2200, 70);
}
