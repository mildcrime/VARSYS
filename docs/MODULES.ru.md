# VARSYS — Каталог модулей

🌐 [English](MODULES.md) · **Русский**

Каждый модуль — наследник `IModule` в `src/modules/<Name>Module/`. Доступ через
`XModule::instance()`. Регистрация и порядок — в `Core::registerModules()`.
Соглашение: **lifecycle** = `init/start/update/stop`; фича-методы именуются иначе
(`activate/deactivate/scan/...`). Легенда: 🟢 встроено · 🔌 внешнее (QWIIC) · ⚠️ нюанс.

- **InputModule** 🟢 — энкодер/кнопки → события ввода. Полушаговый квадратурный
  декодер, антидребезг. Выдаёт `INPUT_ENCODER_CW/CCW`, `BTN_CLICK/LONG`, `BACK`.
- **PowerModule** 🟢 — BQ27220 (I2C 0x55), таймауты экрана (гашение+CPU 80МГц),
  пробуждение по вводу. API: `batteryPercent/Mv/Ma`, `charging`, `powerOff`. ⚠️ idle
  с защитой от underflow.
- **RadioModule** 🟢 (Sub-GHz/CC1101) — `present`, `freqKhz/setFreqKhz`, `scan`,
  `sweep`, `recordRaw/replayLast`, `decodeLast`, `saveLast/loadSignal` (формат
  Flipper `.sub`), `bruteforce/replayCandidates`. Помощники: `RfDecoder`, `RfBrute`.
  ⚠️ RMT канал 2; OOK-конфиг не проверен на железе.
- **IrModule** 🟢 — `capture/replayLast`, `sendNEC`, `sendTvOff`, `sendUniversal`.
  Кодировщики (в `hal::InfraRed`): NEC/NEC-ext/Samsung/Sony. RMT канал 3. RC5 — нет.
- **NfcModule** 🔌 — PN532 (I2C, IRQ 17, RST 45). `readTag`, `writeBlock`,
  `dumpClassic` (Mifare 1K со словарём), `saveDump/loadDump`, `cloneDump`.
- **WifiModule** 🟢 — `scan`, `aps`, `startDeauth/stopDeauth`, снифер,
  `handshakeCount`, `radioOff`. ⚠️ радио ленивое (выкл при старте). deauth-tx может
  требовать патч SDK.
- **BleModule** 🟢 — `scan`, `devices`, `ensureReady`, `radioOff` (NimBLE,
  контроллер лениво).
- **BadUsbModule** 🟢 — USB-HID (TinyUSB). `runScript/runScriptFile` (`/ducky/`),
  `setLayout` (US/DE). Раскладки сырыми HID-репортами. Требует `ARDUINO_USB_MODE=0`.
- **NrfModule** 🔌 — NRF24 (QWIIC 43/44). `acquire/release` (ленивый!), `scanPass`,
  `mjScan/mjPing/mjInject` (Mousejack, Logitech Unifying). ⚠️ promiscuous требует
  подстройки на железе.
- **GpsModule** 🔌 — TinyGPS++ (UART QWIIC). `acquire/release` (ленивый!), `hasFix`,
  `sats`, `lat/lng`, `charsRx`.
- **IButtonModule** 🔌 — OneWire (QWIIC 44). `readKey`, `lastKey`.
- **FmModule** 🔌 — Si4713 (I2C). `present`, `freqKhz10`, `cyclePreset`, `setTx`.
- **AudioModule** 🟢⚠️ — I2S NS4168. **Отключён**: `SPK_WCLK(40)==LCD_RST(40)`,
  инициализация I2S ломает дисплей. Бип выключен.
- **LedModule** 🟢 — WS2812B ×8 (FastLED, пин 14). `setColor` (ambient = цвет
  плитки Home), `flash`, `applySettings`. ⚠️ FastLED держит RMT канал 0.
- **StorageModule** 🟢 — SD (CS 13) + LittleFS. `fs`, `appendLine/writeFile/
  readFile/exists/listDir`, `saveSignal/loadSignal/listSignals`. Всё под SpiBusGuard.
- **WardriveModule** 🔌 — лог WiFi+GPS в `/wardrive.csv` (WiGLE). `activate/
  deactivate`, дедуп BSSID. Занимает GPS на время лога.
- **WebUiModule** 🟢 — softAP + HTTP + OTA. `activate/deactivate` (НЕ start/stop!).
- **EvilPortalModule** 🟢 — captive-портал (раздел «Эксперт»). `activate(ssid)/
  deactivate`.
- **CliModule** 🟢 — Serial-CLI: `help|hw|ver|freq|rssi|rec|replay|ir tvoff|
  wifi scan|ble scan|wifi hs|reboot`.

**Помощники Sub-GHz:** `RfDecoder` (`rfDecode(pulses)` → протокол/бит/ключ),
`RfBrute.h` (таблица протоколов перебора). Детали — [ALGORITHMS](ALGORITHMS.ru.md).
