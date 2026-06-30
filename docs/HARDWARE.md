# VARSYS — Hardware, build, flashing

🌐 **English** · [Русский](HARDWARE.ru.md)

Board: **LILYGO T-Embed CC1101 Plus S3** (ESP32-S3, 16MB, OPI PSRAM, native
USB-C, ST7789 170×320, encoder, CC1101, RGB ×8, BQ27220, SD, QWIIC). Pinout —
`src/hal/board_pins.h`.

## 1. GPIO map
| GPIO | Function | Note |
|---|---|---|
| 15 | POWER_ON | power latch; LOW = power off |
| 9/10/11 | SPI MOSI/MISO/SCLK | **shared bus** display+CC1101+SD+NRF24 |
| 8/18 | I2C SDA/SCL | BQ27220, PN532, Si4713 |
| 21 | LCD backlight (PWM) | |
| 16 | LCD DC | |
| 41 | LCD CS | |
| 40 | LCD RST | ⚠️ = SPK_WCLK (audio conflict) |
| 4/5 | Encoder A/B | |
| 0 | Encoder press | ⚠️ BOOT strap (hold at power-on → download) |
| 6 | Back button | LOW |
| 12 | CC1101 CS | |
| 3/38 | CC1101 GDO0/GDO2 | |
| 48/47 | CC1101 SW0/SW1 | antenna switch |
| 13 | SD CS | |
| 2/1 | IR TX/RX | |
| 17/45 | PN532 IRQ/RST | ⚠️ 45 = VDD_SPI strap |
| 14 | RGB WS2812B ×8 | FastLED (RMT ch0) |
| 43/44 | **QWIIC** | ⚠️ GPS UART / NRF24 CE,CSN / iButton — shared! |
| 39/42 | Mic CLK/DATA | ⚠️ 39 = SPK_MCLK |
| 46/40/7/39 | Speaker I2S | audio (conflicts) |

## 2. Shared resources (source of bugs)
- **SPI (9/10/11)**: one `SPIClass` (TFT_eSPI), access under `hal::SpiBusGuard`.
- **RMT (4 channels)**: ch0=FastLED (held), ch2=CC1101, ch3=IR (per op). New RMT
  user — avoid ch0.
- **QWIIC (43/44)** — mutually exclusive: GPS/NRF24/iButton one at a time, lazy
  `acquire/release`; do not configure pins in `init()`.
- **I2C (8/18)**: `Wire.begin(8,18)` once in `PowerModule::init()`.
- **Audio disabled**: `SPK_WCLK(40)==LCD_RST(40)`, I2S breaks the display.

## 3. Display (TFT_eSPI flags in `platformio.ini`)
`USER_SETUP_LOADED, ST7789_DRIVER, CGRAM_OFFSET(!), TFT_WIDTH=170, TFT_HEIGHT=320,
MOSI=9/MISO=10/SCLK=11/CS=41/DC=16/RST=40/BL=21, INVERSION_ON, SPI_FREQUENCY=40MHz(!)`.
USB: `ARDUINO_USB_MODE=0` (TinyUSB for BadUSB) + `CDC_ON_BOOT=1`.
⚠️ `CGRAM_OFFSET` mandatory (else image shifts off-screen); SPI 40 not 80 (CC1101=0x00).

## 4. Build
`pio run -e t-embed-cc1101`. Board: `boards/lilygo-t-embed-cc1101.json`.
LVGL: `include/lv_conf.h` (⚠️ `LV_USE_FONT_COMPRESSED 1`).

## 5. Flashing
**Download mode:** BOOT = **encoder press** (not the side button): hold the
encoder wheel in → press RST → release the wheel (LEDs go dark). Then
`pio run -e t-embed-cc1101 -t upload --upload-port /dev/cu.usbmodemXXXX`.
ROM port `usbmodem1101`, app port `usbmodem<serial>`. Web flasher: `flasher/`.

## 6. Serial logs
Open pyserial with **`dtr=True`** (else TinyUSB CDC stays silent). Toggle DTR/RTS
carefully (can drop into download). Level — `Logger::setLevel` in `Core::begin`.

## 7. Bring-up gotchas (so we don't repeat them)
1) screens in init → crash (build in start). 2) USB_MODE=1 vs TinyUSB → port
vanished (→ 0). 3) GPIO40 LCD_RST=SPK_WCLK → white screen (audio off). 4)
start/stop collision → auto-start (→ activate/deactivate). 5) missing
`LV_USE_FONT_COMPRESSED` → no text. 6) `transform_zoom` → empty carousel center.
7) full-step encoder → CW every other, no CCW (→ half-step). 8) tearing (→ instant
scroll, buffer 1/6, SPI 40). 9) `now-_lastActivity` underflow → false screen-off
(→ guard). 10) 80 MHz → CC1101 0x00 (→ 40). 11) RMT FastLED ch0 ↔ CC1101 (→ ch2/3).
12) QWIIC pin clobber (→ lazy acquire/release).
