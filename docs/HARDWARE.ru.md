# VARSYS — Железо, сборка, прошивка

🌐 [English](HARDWARE.md) · **Русский**

Плата: **LILYGO T-Embed CC1101 Plus S3** (ESP32-S3, 16MB, OPI PSRAM, нативный
USB-C, ST7789 170×320, энкодер, CC1101, RGB ×8, BQ27220, SD, QWIIC). Распиновка —
`src/hal/board_pins.h`.

## 1. Карта GPIO
| GPIO | Назначение | Примечание |
|---|---|---|
| 15 | POWER_ON | удержание питания; LOW = выкл |
| 9/10/11 | SPI MOSI/MISO/SCLK | **общая шина** дисплей+CC1101+SD+NRF24 |
| 8/18 | I2C SDA/SCL | BQ27220, PN532, Si4713 |
| 21 | LCD подсветка (PWM) | |
| 16 | LCD DC | |
| 41 | LCD CS | |
| 40 | LCD RST | ⚠️ = SPK_WCLK (конфликт аудио) |
| 4/5 | Энкодер A/B | |
| 0 | Нажатие энкодера | ⚠️ BOOT-strap (удержание при вкл → download) |
| 6 | Кнопка «назад» | LOW |
| 12 | CC1101 CS | |
| 3/38 | CC1101 GDO0/GDO2 | |
| 48/47 | CC1101 SW0/SW1 | антенный переключатель |
| 13 | SD CS | |
| 2/1 | IR TX/RX | |
| 17/45 | PN532 IRQ/RST | ⚠️ 45 = strap VDD_SPI |
| 14 | RGB WS2812B ×8 | FastLED (RMT ch0) |
| 43/44 | **QWIIC** | ⚠️ GPS UART / NRF24 CE,CSN / iButton — общие! |
| 39/42 | Микрофон CLK/DATA | ⚠️ 39 = SPK_MCLK |
| 46/40/7/39 | Динамик I2S | аудио (конфликты) |

## 2. Общие ресурсы (источник багов)
- **SPI (9/10/11)**: один `SPIClass` (TFT_eSPI), доступ под `hal::SpiBusGuard`.
- **RMT (4 канала)**: ch0=FastLED (держит постоянно), ch2=CC1101, ch3=IR (по
  запросу). Новый RMT-потребитель — не ch0.
- **QWIIC (43/44)** — взаимоисключающий: GPS/NRF24/iButton по одному за раз,
  ленивый `acquire/release`; пины не configurировать в `init()`.
- **I2C (8/18)**: `Wire.begin(8,18)` один раз в `PowerModule::init()`.
- **Аудио отключено**: `SPK_WCLK(40)==LCD_RST(40)`, I2S ломает дисплей.

## 3. Дисплей (TFT_eSPI, флаги в `platformio.ini`)
`USER_SETUP_LOADED, ST7789_DRIVER, CGRAM_OFFSET(!), TFT_WIDTH=170, TFT_HEIGHT=320,
MOSI=9/MISO=10/SCLK=11/CS=41/DC=16/RST=40/BL=21, INVERSION_ON, SPI_FREQUENCY=40MHz(!)`.
USB: `ARDUINO_USB_MODE=0` (TinyUSB для BadUSB) + `CDC_ON_BOOT=1`.
⚠️ `CGRAM_OFFSET` обязателен (иначе изображение уезжает); SPI 40, не 80 (CC1101=0x00).

## 4. Сборка
`pio run -e t-embed-cc1101`. Board: `boards/lilygo-t-embed-cc1101.json`.
LVGL: `include/lv_conf.h` (⚠️ `LV_USE_FONT_COMPRESSED 1`).

## 5. Прошивка
**Download mode:** BOOT = **нажатие колеса-энкодера** (не боковая кнопка):
вдавить колесо → нажать RST → отпустить колесо (LED погаснут). Затем
`pio run -e t-embed-cc1101 -t upload --upload-port /dev/cu.usbmodemXXXX`.
ROM-порт `usbmodem1101`, порт приложения — `usbmodem<serial>`. Веб-флешер: `flasher/`.

## 6. Логи Serial
Открывать pyserial с **`dtr=True`** (иначе TinyUSB CDC молчит). DTR/RTS дёргать
осторожно (уводит в download). Уровень — `Logger::setLevel` в `Core::begin`.

## 7. Грабли bring-up (чтобы не повторять)
1) экраны в init → краш (строить в start). 2) USB_MODE=1 vs TinyUSB → пропадал
порт (стало 0). 3) GPIO40 LCD_RST=SPK_WCLK → белый экран (аудио off). 4) коллизия
start/stop → автозапуск (→ activate/deactivate). 5) нет `LV_USE_FONT_COMPRESSED` →
нет текста. 6) `transform_zoom` → пустой центр карусели. 7) полношаговый энкодер →
CW через раз, CCW нет (→ полушаговый). 8) тиринг (→ мгновенный скролл, буфер 1/6,
SPI 40). 9) underflow `now-_lastActivity` → ложное гашение (→ защита). 10) 80МГц →
CC1101 0x00 (→ 40). 11) RMT FastLED ch0 ↔ CC1101 (→ ch2/ch3). 12) QWIIC затирание
(→ ленивый acquire/release).
