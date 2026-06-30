# VARSYS — Module catalog

🌐 **English** · [Русский](MODULES.ru.md)

Every module derives from `IModule` in `src/modules/<Name>Module/`. Accessed via
`XModule::instance()`. Registration and order are in `Core::registerModules()`.
Convention: **lifecycle** = `init/start/update/stop`; feature methods are named
otherwise (`activate/deactivate/scan/...`). Legend: 🟢 built-in · 🔌 external
(QWIIC) · ⚠️ caveat.

- **InputModule** 🟢 — encoder/buttons → input events. Half-step quadrature
  decoder, debounce. Emits `INPUT_ENCODER_CW/CCW`, `BTN_CLICK/LONG`, `BACK`.
- **PowerModule** 🟢 — BQ27220 (I2C 0x55), screen timeouts (off + CPU 80 MHz),
  wake on input. API: `batteryPercent/Mv/Ma`, `charging`, `powerOff`. ⚠️ compute
  idle with underflow guard.
- **RadioModule** 🟢 (Sub-GHz/CC1101) — `present`, `freqKhz/setFreqKhz`, `scan`,
  `sweep`, `recordRaw/replayLast`, `decodeLast`, `saveLast/loadSignal` (Flipper
  `.sub` format), `bruteforce/replayCandidates`. Helpers: `RfDecoder`, `RfBrute`.
  ⚠️ RMT channel 2; OOK config unverified on hardware.
- **IrModule** 🟢 — `capture/replayLast`, `sendNEC`, `sendTvOff`, `sendUniversal`.
  Encoders (in `hal::InfraRed`): NEC/NEC-ext/Samsung/Sony. RMT channel 3. No RC5 yet.
- **NfcModule** 🔌 — PN532 (I2C, IRQ 17, RST 45). `readTag`, `writeBlock`,
  `dumpClassic` (Mifare 1K with key dictionary), `saveDump/loadDump`, `cloneDump`.
- **WifiModule** 🟢 — `scan`, `aps`, `startDeauth/stopDeauth`, sniffer,
  `handshakeCount`, `radioOff`. ⚠️ radio is lazy (off at boot). deauth-tx may need
  an SDK patch.
- **BleModule** 🟢 — `scan`, `devices`, `ensureReady`, `radioOff` (NimBLE,
  controller brought up lazily).
- **BadUsbModule** 🟢 — USB-HID (TinyUSB). `runScript/runScriptFile` (`/ducky/`),
  `setLayout` (US/DE). Layouts via raw HID reports. Requires `ARDUINO_USB_MODE=0`.
- **NrfModule** 🔌 — NRF24 (QWIIC 43/44). `acquire/release` (lazy!), `scanPass`,
  `mjScan/mjPing/mjInject` (Mousejack, Logitech Unifying). ⚠️ promiscuous needs
  on-device tuning.
- **GpsModule** 🔌 — TinyGPS++ (UART QWIIC). `acquire/release` (lazy!), `hasFix`,
  `sats`, `lat/lng`, `charsRx`.
- **IButtonModule** 🔌 — OneWire (QWIIC 44). `readKey`, `lastKey`.
- **FmModule** 🔌 — Si4713 (I2C). `present`, `freqKhz10`, `cyclePreset`, `setTx`.
- **AudioModule** 🟢⚠️ — I2S NS4168. **Disabled**: `SPK_WCLK(40)==LCD_RST(40)`,
  I2S init breaks the display. Boot beep off.
- **LedModule** 🟢 — WS2812B ×8 (FastLED, pin 14). `setColor` (ambient = focused
  Home tile color), `flash`, `applySettings`. ⚠️ FastLED holds RMT channel 0.
- **StorageModule** 🟢 — SD (CS 13) + LittleFS. `fs`, `appendLine/writeFile/
  readFile/exists/listDir`, `saveSignal/loadSignal/listSignals`. All under SpiBusGuard.
- **WardriveModule** 🔌 — WiFi+GPS log to `/wardrive.csv` (WiGLE). `activate/
  deactivate`, BSSID dedup. Acquires GPS while logging.
- **WebUiModule** 🟢 — softAP + HTTP + OTA. `activate/deactivate` (NOT start/stop!).
- **EvilPortalModule** 🟢 — captive portal (Expert section). `activate(ssid)/
  deactivate`.
- **CliModule** 🟢 — Serial CLI: `help|hw|ver|freq|rssi|rec|replay|ir tvoff|
  wifi scan|ble scan|wifi hs|reboot`.

**Sub-GHz helpers:** `RfDecoder` (`rfDecode(pulses)` → protocol/bits/key),
`RfBrute.h` (bruteforce protocol table). Details — [ALGORITHMS](ALGORITHMS.md).
