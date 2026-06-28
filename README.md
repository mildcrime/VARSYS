# VARSYS

**English** · [Русский](README.ru.md)

Modular security‑research firmware for the **LILYGO T‑Embed CC1101 Plus (ESP32‑S3)** —
a Flipper‑class multitool with a clean, phone‑like UI.

> ⚠️ For **authorized testing and education only**. Read the
> [Disclaimer](DISCLAIMER.md) before use.

![status](https://img.shields.io/badge/build-passing-brightgreen)
![platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![license](https://img.shields.io/badge/license-MIT-green)

## Why VARSYS

It covers essentially the same toolset as Bruce, but rethought as a product:
a modular cooperative core, an LVGL UI in iOS‑17 style (focus carousel, dark
theme, RU/EN on the fly), runtime settings in NVS, structured power management,
and a unified RMT signal pipeline shared by Sub‑GHz and IR.

## Features

| Domain | Capabilities |
|---|---|
| **Sub‑GHz** (CC1101) | frequency presets, live RSSI, preset scan, **record/replay** (RMT), **protocol decoder** (Princeton/EV1527/CAME/Nice/Holtek), **bruteforce** (6 protocols + All, candidate auto‑save + auto‑replay to confirm a code), **signal library** (`.sub`), spectrum analyzer |
| **Infrared** | RMT capture/replay, NEC, universal "TV‑off" |
| **NFC/RFID** (PN532) | read UID/type, Mifare block write, save dumps |
| **WiFi** | AP scan, sniffer, deauth (authorized), **EAPOL handshake detection**, captive **evil portal** (gated) |
| **Bluetooth** | BLE device scan / recon (NimBLE) |
| **NRF24** | 2.4 GHz channel analyzer |
| **GPS** | satellites / fix / coordinates (TinyGPS++) |
| **iButton** | Dallas 1‑Wire ROM read |
| **FM** | Si4713 transmitter |
| **BadUSB** | native USB‑HID, Ducky‑style scripts |
| **Connectivity** | WebUI (AP + HTTP, signal library, **OTA**), Serial CLI |
| **System** | dark/light theme, RU/EN, battery (BQ27220), dimming/sleep, RGB feedback, audio beep, Dev tools (system info / I²C scan / CC1101 dump / factory reset) |

## Hardware

LILYGO **T‑Embed CC1101 Plus S3**: ESP32‑S3 (16 MB flash, OPI PSRAM),
ST7789 320×170 display, rotary encoder + buttons, CC1101 sub‑GHz, PN532 NFC,
WS2812 RGB ×8, NS4168 speaker, BQ27220 fuel gauge.

> Pinout follows the verified Bruce configuration — see
> [`src/hal/board_pins.h`](src/hal/board_pins.h). External modules
> (NRF24, GPS, FM, iButton) attach to the QWIIC/serial port.

## Flash from your browser (no install)

Open **<https://mildcrime.github.io/VARSYS/flasher/>** in **Chrome or Edge**,
connect the device via USB, click **Install**. Works on Windows / macOS / Linux.
Or double‑click a shortcut from [`flasher/`](flasher/). Details:
[flasher/README.md](flasher/README.md).

## Build & flash (from source)

Requires [PlatformIO](https://platformio.org/).

```bash
git clone <your-fork-url> VARSYS && cd VARSYS
pio run -e t-embed-cc1101                 # build
pio run -e t-embed-cc1101 -t upload -t monitor   # flash + serial monitor
```

First build downloads the toolchain and libraries (several minutes). The serial
monitor exposes the CLI — type `help`.

## Controls

- **Rotate encoder** — move selection / scroll the carousel.
- **Press encoder** — open / activate.
- **Back button** (side) or **long press** — go back / abort a running action.

## Usage

Full guides: **[USAGE (EN)](docs/USAGE_EN.md)** · [USAGE (RU)](docs/USAGE_RU.md).
Architecture: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

Quick examples:
- **Open a gate (bruteforce):** Sub‑GHz → set frequency → *Bruteforce* → pick
  protocol (or *All*) → *Start*. When the gate opens, press **back** — the recent
  codes are saved to `/brute/*.txt`. Run *Candidates* to auto‑replay and confirm
  the exact code (saved to `/brute/confirmed_*.sub`).
- **Capture & replay a remote:** Sub‑GHz → *Record* → *Replay* / *Save*.
- **Read an NFC tag:** NFC → *Read* → *Save*.

## Project structure

```
src/
  core/      Core, ModuleManager, EventBus, Scheduler, Settings, Clock, Logger
  hal/       board_pins, Display (TFT_eSPI), CC1101, InfraRed, SpiBus
  modules/   Radio, Ir, Nfc, Wifi, Ble, Nrf, Gps, IButton, Fm, BadUsb,
             Audio, Led, Power, Storage, WebUi, Cli, EvilPortal, Input
  ui/        UIManager, UITheme, i18n, Notify, Splash, StatusOverlay, screens/, fonts/
tools/fonts/ font generator (Inter + Tabler) — see its README
boards/      custom lilygo-t-embed-cc1101.json
```

## Expert section

Indiscriminate tools (broadband jammer, mass BLE/Apple spam) are intentionally
shipped as **non‑functional stubs** behind a gated "Expert" mode, because they
affect bystanders and their transmission is illegal in most jurisdictions. The
Expert section contains working engineering tools (Dev tools, captive portal).

## Legal

See **[DISCLAIMER.md](DISCLAIMER.md)**. Authorized use only. You are responsible
for compliance with all applicable laws, including radio regulations.

## Credits

Built on LVGL, TFT_eSPI, NimBLE, Adafruit/PN532·Si4713, RF24, TinyGPS++,
FastLED, OneWire. Fonts: Inter (OFL), Tabler Icons (MIT). Hardware pinout and
protocol references from the [Bruce](https://github.com/pr3y/Bruce) project.

## License

MIT — see [LICENSE](LICENSE).
