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
