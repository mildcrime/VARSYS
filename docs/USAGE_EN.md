# VARSYS — User Guide (English)

[Русская версия](USAGE_RU.md)

> Authorized testing and education only. See [DISCLAIMER](../DISCLAIMER.md).

## 1. Basics

- **Boot:** the animated VARSYS logo appears, then the home carousel.
- **Navigate:** rotate the encoder to move between icons; the centered icon is
  the selection. **Press** the encoder to open it. **Back button** (side) or a
  **long press** returns to the previous screen / aborts a running action.
- **Status bar** (top‑right): time and battery.
- **Settings:** Home → *Settings* — language (RU/EN), dark theme, sound, vibro,
  brightness, **Expert mode**, About.

## 2. Sub‑GHz (CC1101)

Home → **Sub‑GHz**. Left card shows the frequency and live RSSI; the right list
is the actions (scroll with the encoder):

- **Frequency** — cycle presets (315 / 390 / 433.92 / 434.42 / 868.35 / 915 MHz).
- **Scan** — sweep presets, jump to the strongest.
- **Record** — capture a raw OOK signal; the decoder shows protocol + key.
- **Replay** — re‑transmit the last capture.
- **Save** / **Library** — store the capture (`.sub`) / browse & replay saved ones.
- **Spectrum** — live RSSI sweep around the current frequency.
- **Bruteforce** — opens the brute screen (see below).

### Bruteforce (gates / fixed‑code barriers)

1. Set the gate frequency in Sub‑GHz first (usually 433.92 or 315 MHz).
2. Sub‑GHz → **Bruteforce**. Choose **Protocol** (Came / Nice / Ansonic /
   Holtek / Linear / Chamberlain, or **All**) and **Repeats** (2–3).
3. **Start.** It sends full frames per code. When the gate opens, press **back** —
   the last ~48 codes are saved as candidates to `/brute/found_*.txt`.
4. **Candidates** — auto‑replays the saved window one by one with a pause and a
   live index. When the gate opens again, press **back**: the exact confirmed
   code is saved to `/brute/confirmed_*.sub`.

## 3. Infrared

Home → **Infrared**: **Capture** a remote, **Replay** it, or **TV‑off**
(sends a sweep of common power codes).

## 4. NFC / RFID (PN532)

Home → **NFC**: **Read** a tag (shows UID/type), **Save** the UID, **Write** a
Mifare Classic block (default key A). Needs a PN532 module and a tag.

## 5. WiFi

Home → **WiFi** scans nearby APs. Select an AP and press to toggle **deauth**;
while deauthing, EAPOL **handshakes are detected** and counted (also `wifi hs`
in the CLI). The **evil portal** (captive AP that collects credentials for
authorized phishing simulations) lives in the gated **Expert** section.

## 6. Bluetooth / NRF24 / GPS / iButton / FM

- **Bluetooth** — scan/recon of BLE devices; press an entry to see its address.
- **NRF24** — 2.4 GHz channel‑activity analyzer (needs an NRF24 module).
- **GPS** — satellites, fix and coordinates (needs a GPS module on UART).
- **iButton** — read a Dallas key ROM (needs a 1‑Wire probe), save it.
- **FM** — pick a frequency and toggle low‑power transmission (Si4713).

## 7. BadUSB

Home → **BadUSB**: plug the device into a host via USB; press to run the demo
Ducky script. Full scripts can be placed on storage (`/ducky/`).

## 8. WebUI + OTA

Home → **WebUI** → **Start**. Connect to the `VARSYS‑XXXX` access point
(password `varsys1234`), open `http://192.168.4.1`:
- device status, **signal library** (view / download `.sub`),
- **OTA** — upload a new `firmware.bin` to update over the air.

## 9. Serial CLI

Over USB serial (115200): `help`, `ver`, `freq <kHz>`, `rssi`, `rec`, `replay`,
`ir tvoff`, `wifi scan`, `wifi hs`, `ble scan`, `reboot`.

## 10. Expert section

Visible only when **Expert mode** is enabled in Settings. Contains:
- **Dev tools** — system info (heap/PSRAM/uptime/temperature), I²C scanner,
  CC1101 register dump, factory reset.
- **Captive portal** — authorized phishing simulation.
- **Jammer / BLE spam** — **non‑functional stubs** (see Disclaimer).

## Files on storage

- `/signals/*.sub` — saved Sub‑GHz captures
- `/brute/*.txt`, `/brute/confirmed_*.sub` — bruteforce candidates / confirmed
- `/nfc/*.txt`, `/ibutton/*.txt` — saved IDs
- `/portal_creds.txt` — captive‑portal captures

Storage uses the SD card if present, otherwise internal LittleFS.
