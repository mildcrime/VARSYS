# VARSYS — Железо, сборка, прошивка

Плата: **LILYGO T-Embed CC1101 Plus S3** (ESP32-S3, 16MB flash, OPI PSRAM,
нативный USB-C, дисплей ST7789 170×320, энкодер, CC1101, RGB ×8, BQ27220,
SD, QWIIC/Grove). Распиновка — `src/hal/board_pins.h` (сверена с Bruce).

---

## 1. Карта GPIO

| GPIO | Назначение | Примечание |
|---|---|---|
| 15 | PIN_POWER_ON | удержание питания (LCD+батарея); LOW = выключить |
| 9 / 10 / 11 | SPI MOSI / MISO / SCLK | **общая шина**: дисплей+CC1101+SD+NRF24 |
| 8 / 18 | I2C SDA / SCL | BQ27220, PN532, Si4713 |
| 21 | LCD подсветка (LEDC PWM) | |
| 16 | LCD DC | |
| 41 | LCD CS | |
| 40 | LCD RST | ⚠️ **= SPK_WCLK** (аудио) — конфликт |
| 4 / 5 | Энкодер A / B | |
| 0 | Энкодер нажатие | ⚠️ **BOOT-strap**: удержание при включении → download |
| 6 | Кнопка «назад» | активный LOW |
| 12 | CC1101 CS | |
| 3 | CC1101 GDO0 | RMT (strap JTAG, ок после boot) |
| 38 | CC1101 GDO2 | |
| 48 / 47 | CC1101 антенный SW0 / SW1 | обязательны для TX/RX |
| 13 | SD CS | |
| 2 / 1 | IR TX / RX | |
| 17 | PN532 IRQ | |
| 45 | PN532 RST | ⚠️ strap VDD_SPI (низкий риск) |
| 14 | RGB WS2812B ×8 | FastLED (RMT ch0) |
| 43 / 44 | **QWIIC** | ⚠️ GPS UART / NRF24 CE,CSN / iButton 1-Wire — общие! |
| 39 / 42 | Микрофон CLK / DATA | ⚠️ 39 = SPK_MCLK |
| 46 / 40 / 7 / 39 | Динамик BCLK/WCLK/DOUT/MCLK | аудио (см. конфликты) |

---

## 2. Общие ресурсы (источник большинства багов)

### SPI-шина (9/10/11)
Дисплей, CC1101, SD, NRF24 — на одной шине, различаются CS. Один `SPIClass`
(TFT_eSPI), остальные через `Display::spi()`. **Каждый доступ — под
`hal::SpiBusGuard`** (рекурсивный мьютекс). LVGL flush тоже.

### RMT-каналы (4 на S3)
- **ch0** — FastLED (RGB), занимает на первой `show()`, держит постоянно.
- **ch2** — CC1101 (по запросу install/uninstall).
- **ch3** — IR (по запросу).
- ⚠️ Если добавляете RMT-потребителя — не берите ch0; CC1101/IR падали бы, если
  их канал занят FastLED (поэтому они на 2/3, а не 0/1).

### QWIIC-порт (43/44) — взаимоисключающий
GPS(UART) / NRF24(CE,CSN) / iButton(1-Wire) делят одни пины. Работает **один за
раз**. Реализовано **лениво**: `GpsModule`/`NrfModule` поднимают пины в
`acquire()` (на входе в свой экран), освобождают в `release()` (→ INPUT) на
выходе. iButton управляет пином через OneWire по месту. Wardrive занимает GPS на
время лога. ⚠️ Не configurировать эти пины в `init()` — затрут друг друга.

### I2C-шина (8/18)
`Wire.begin(8,18)` один раз в `PowerModule::init()` (до Nfc/Fm). BQ27220 (0x55),
PN532, Si4713 — на ней же.

### Аудио (отключено)
`SPK_WCLK(40) == LCD_RST(40)` и `MIC_CLK(39) == SPK_MCLK(39)`. Любая
инициализация I2S дёргает сброс дисплея → белый экран. `AudioModule` немой,
boot-бип выключен. Включать только после выяснения корректных I2S-пинов.

---

## 3. Конфигурация дисплея (TFT_eSPI)
Задаётся флагами в `platformio.ini` (НЕ User_Setup.h):
```
-DUSER_SETUP_LOADED=1 -DST7789_DRIVER=1 -DCGRAM_OFFSET=1
-DTFT_WIDTH=170 -DTFT_HEIGHT=320
-DTFT_MOSI=9 -DTFT_MISO=10 -DTFT_SCLK=11 -DTFT_CS=41 -DTFT_DC=16
-DTFT_RST=40 -DTFT_BL=21 -DTFT_BACKLIGHT_ON=1 -DTFT_INVERSION_ON=1
-DSPI_FREQUENCY=40000000 -DUSE_HSPI_PORT=1
```
- ⚠️ **`-DCGRAM_OFFSET=1` обязателен** для 170-панели — иначе TFT_eSPI не
  применяет смещение colstart/rowstart=35 и изображение уезжает за экран.
- ⚠️ **SPI 40 МГц, не 80** — на 80 МГц CC1101 на общей шине читался как 0x00.

USB: `-DARDUINO_USB_MODE=0` (TinyUSB — нужен для BadUSB) + `-DARDUINO_USB_CDC_ON_BOOT=1`.

---

## 4. Сборка
```
pio run -e t-embed-cc1101                 # сборка (в репо .piovenv/bin/platformio)
```
Кастомный board: `boards/lilygo-t-embed-cc1101.json` (16MB, OPI PSRAM, qio_opi).
LVGL-конфиг: `include/lv_conf.h` (⚠️ `LV_USE_FONT_COMPRESSED 1`).

## 5. Прошивка
**Download mode (ВАЖНО):** на T-Embed BOOT = **нажатие колеса-энкодера** (GPIO0),
не боковая кнопка:
1. Вдавить колесо-энкодер и держать.
2. Коротко нажать RST, отпустить RST.
3. Отпустить колесо. (LED погаснут = режим загрузки.)

Затем:
```
pio run -e t-embed-cc1101 -t upload --upload-port /dev/cu.usbmodemXXXX
```
- ROM download-порт — короткий `usbmodem1101`; порт приложения (TinyUSB CDC) —
  серийный `usbmodem<serial>`.
- Веб-прошивальщик: `flasher/` (ESP Web Tools, Chrome/Edge), обновляется CI по
  тегу `vX.Y.Z`.

## 6. Чтение логов по Serial
- Открывать pyserial с **`dtr=True`** — иначе TinyUSB CDC не отдаёт вывод.
- `dtr/rts` дёргать осторожно: неверная комбинация уводит чип в download.
- Уровень логов — `Logger::setLevel(LogLevel::DEBUG)` в `Core::begin`.

---

## 7. Грабли bring-up (history, чтобы не наступить снова)
Найдены при первом подъёме на железе (см. git-историю «on-device bringup»):
1. Экраны в `init()` → краш (строить в `start()`).
2. `ARDUINO_USB_MODE=1` несовместим с TinyUSB → порт пропадал (стало `=0`).
3. GPIO40 (LCD_RST=SPK_WCLK): boot-бип ломал дисплей (аудио отключено).
4. Коллизия `start()/stop()` с lifecycle → автозапуск WebUI/Wardrive (→ activate/deactivate).
5. `LV_USE_FONT_COMPRESSED` не задан → не рисовался текст/иконки.
6. `transform_zoom` → пустой центр карусели (→ прозрачность).
7. Энкодер: полношаговая таблица → CW через раз, CCW нет (→ полушаговая).
8. Тиринг прокрутки (→ мгновенный скролл + буфер 1/6 + SPI 40).
9. Underflow `now - _lastActivity` → ложное гашение на каждый ввод (→ защита).
10. 80 МГц SPI → CC1101 0x00 (→ 40 МГц).
11. RMT: FastLED ch0 ↔ CC1101 (→ CC1101 ch2, IR ch3).
12. QWIIC 43/44: затирание пинов в init (→ ленивый acquire/release).
