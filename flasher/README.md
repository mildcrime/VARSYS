# VARSYS Web Flasher

Flash VARSYS onto a LILYGO T‑Embed CC1101 **straight from your browser** — no
toolchain, no drivers to configure, works on Windows / macOS / Linux.

## Easiest way

Open **https://mildcrime.github.io/VARSYS/flasher/** in **Chrome or Edge**
(desktop), connect the device by USB, click **Install**, pick the serial port.

Or double‑click the shortcut for your OS (they just open the page above):
- Windows — `VARSYS-Flasher.url`
- macOS — `VARSYS-Flasher.webloc`
- Linux — `VARSYS-Flasher.desktop`

## Requirements

- **Chrome or Edge** on a desktop (uses the Web Serial API; Firefox/Safari are
  not supported).
- A USB data cable.

## Drivers

The T‑Embed CC1101 (ESP32‑S3) usually uses the chip’s **native USB** and needs
**no driver** on Windows 10/11, macOS and Linux. Some boards/cables use a
USB‑serial bridge that needs one. The page has a **“Check device”** button that
reads the USB vendor id, identifies the chip (native / CP210x / CH340·CH9102 /
FTDI) and links the right driver for your OS. Note: a browser can’t install
drivers — download, install, then re‑plug the device.

Driver sources: [CP210x](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers),
[CH340/CH9102](https://www.wch-ic.com/downloads/CH343SER_EXE.html),
[FTDI](https://ftdichip.com/drivers/vcp-drivers/).

## Run it locally (offline)

Web Serial needs `https://` or `localhost`, so serve this folder over localhost:

```bash
cd flasher
python3 -m http.server 8000      # then open http://localhost:8000
```

## What gets flashed

`manifest.json` writes four images at their ESP32‑S3 offsets:
`bootloader.bin` (0x0), `partitions.bin` (0x8000), `boot_app0.bin` (0xe000),
`firmware.bin` (0x10000). Rebuild them with `pio run -e t-embed-cc1101` and copy
from `.pio/build/t-embed-cc1101/`.

> Authorized use only — see [../DISCLAIMER.md](../DISCLAIMER.md).
