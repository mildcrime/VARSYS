# VARSYS — Каталог модулей

Каждый модуль — наследник `IModule` в `src/modules/<Name>Module/`. Доступ через
`XModule::instance()`. Регистрация и порядок — в `Core::registerModules()`.

Соглашение: **lifecycle** = `init/start/update/stop` (зовёт ядро); **фича-методы**
именуются иначе (`activate/deactivate/scan/...`) — чтобы не переопределить
lifecycle случайно.

Легенда железа: 🟢 встроено · 🔌 внешнее (QWIIC/Grove) · ⚠️ нюанс.

---

## InputModule 🟢 `InputModule/`
Опрос энкодера и кнопок, публикация событий ввода.
- **HW**: энкодер A=4 B=5, нажатие=0 (BOOT), боковая «назад»=6.
- **Алгоритм**: полушаговый квадратурный декодер Буxтона (детенты на 00 и 11);
  антидребезг кнопок по устойчивому уровню; long-press энкодера.
- **Выдаёт**: `INPUT_ENCODER_CW/CCW`, `INPUT_BTN_CLICK/LONG`, `INPUT_BACK`.
- ⚠️ Полношаговая таблица давала «через раз» — см. [ALGORITHMS.md](ALGORITHMS.md).

## PowerModule 🟢 `PowerModule/`
Батарея, энергосбережение, выключение.
- **HW**: топливомер BQ27220 по I2C (addr 0x55); линия питания `PIN_POWER_ON=15`.
- **API**: `batteryPercent()`, `batteryMv()`, `batteryMa()`, `charging()`,
  `powerOff()`.
- **Логика**: при простое > `screenTimeoutSec` гасит экран (яркость 0) + роняет
  CPU 240→80 МГц; затемнение на 2/3 таймаута; пробуждение по любому вводу
  (`noteActivity`→`wake`). Опрос батареи раз в `VARSYS_BATTERY_POLL_MS`.
- ⚠️ `idle` считать с защитой от underflow (см. ARCHITECTURE §3).

## RadioModule (Sub-GHz, CC1101) 🟢 `RadioModule/`
Высокоуровневая обёртка над `hal::CC1101`.
- **HW**: CC1101 на общей SPI (CS 12, GDO0 3, GDO2 38, антенный SW0 48/SW1 47).
- **API**: `present()`, `freqKhz()/setFreqKhz()`, `cycleFreqPreset()`, `rssi()`,
  `scan()` (свип пресетов → лучшая частота), `sweep()` (для спектра),
  `recordRaw()/replayLast()`, `decodeLast()` (→ [RfDecoder]), `saveLast()/loadSignal()`
  (→ Storage, формат Flipper `.sub`), `bruteforce()/replayCandidates()` (→ [RfBrute]).
- **Файлы-помощники**: `RfDecoder.*` (OOK-декод), `RfBrute.h` (таблица протоколов
  для перебора). См. [ALGORITHMS.md](ALGORITHMS.md).
- ⚠️ Запись/воспроизведение через RMT (канал 2 — FastLED держит 0). Модемный
  OOK-конфиг CC1101 НЕ проверен на железе.

## IrModule 🟢 `IrModule/`
Инфракрасный приёмопередатчик.
- **HW**: IR TX=2, RX=1 (через `hal::InfraRed`, RMT канал 3).
- **API**: `capture()/replayLast()`, `sendNEC()`, `sendTvOff()` (sweep power-кодов),
  `sendUniversal(UniFn)` (универсальный пульт: Power/Vol/Mute/Ch/Source ×
  Samsung/LG/Sony), `uniName()`.
- **Кодировщики** (в `hal::InfraRed`): NEC, NEC-ext, Samsung, Sony SIRC. RC5 пока
  нет (Manchester несовместим с mark-first sendRaw).

## NfcModule 🔌 `NfcModule/`
NFC/RFID 13.56 МГц на PN532 (I2C).
- **HW**: PN532 по I2C (SDA 8/SCL 18), IRQ 17, RST 45. Adafruit_PN532.
- **API**: `present()`, `readTag(uid,type)`, `writeBlock()`, `dumpClassic()`
  (полный дамп Mifare 1K со словарём ключей), `saveDump()/loadDump()`
  (`/nfc/<uid>.dump`), `cloneDump()` (запись дата-блоков назад).
- ⚠️ Словарь вскроет только карты со стандартными ключами (nested/MFKey — нет).

## WifiModule 🟢 `WifiModule/`
WiFi-сюита (для авторизованного тестирования).
- **API**: `scan()` (блокирующий), `aps()`, `startDeauth()/stopDeauth()`,
  `startSniffer()/stopSniffer()`, `handshakeCount()` (EAPOL-детект при деауте),
  `radioOff()` (экономия).
- ⚠️ Радио НЕ включается при загрузке (`WIFI_OFF`), поднимается лениво в `scan()`,
  гасится при выходе с экрана. deauth-tx на части IDF требует патча SDK (как Bruce).

## BleModule 🟢 `BleModule/`
BLE-разведка (NimBLE).
- **API**: `scan(seconds)`, `devices()`, `ensureReady()`, `radioOff()`.
- ⚠️ Контроллер NimBLE поднимается лениво в `scan()`, освобождается
  (`NimBLEDevice::deinit`) при выходе с экрана. HID/spam — отложено/заглушка.

## BadUsbModule 🟢 `BadUsbModule/`
USB-HID клавиатура (TinyUSB), Ducky-скрипты.
- **HW**: нативный USB-C (требует `ARDUINO_USB_MODE=0`).
- **API**: `runScript(text)`, `runScriptFile(name)` (из `/ducky/`), `runDemo()`,
  `setLayout()/layout()` (US/DE). Раскладки — сырыми HID-репортами (`sendReport`).
- ⚠️ DE-таблица символов — проверять на хосте.

## NrfModule 🔌 `NrfModule/`
NRF24L01: анализатор 2.4 ГГц + Mousejack.
- **HW**: NRF24 на общей SPI, CE/CSN на QWIIC 43/44 (RF24 lib).
- **API**: `present()`, `acquire()/release()` (ленивый QWIIC!), `scanPass()`
  (несущая по 126 каналам), `mjScan()` (promiscuous-поиск адресов), `mjPing()`,
  `mjInject()` (Logitech Unifying unencrypted).
- ⚠️ Promiscuous-приём ESB требует подстройки на железе. `acquire` обязателен
  перед использованием (иначе пины не подняты).

## GpsModule 🔌 `GpsModule/`
GPS (TinyGPS++, UART).
- **HW**: UART на QWIIC 43/44.
- **API**: `acquire()/release()` (ленивый QWIIC!), `hasFix()`, `sats()`,
  `lat()/lng()`, `charsRx()`.
- ⚠️ UART поднимается только в `acquire()` (экран GPS / wardrive).

## IButtonModule 🔌 `IButtonModule/`
iButton / Dallas 1-Wire.
- **HW**: OneWire data на QWIIC 44.
- **API**: `readKey(out)` (ROM 8 байт + CRC), `lastKey()`.
- OneWire сам управляет пином по месту (явный acquire не нужен).

## FmModule 🔌 `FmModule/`
FM-передатчик Si4713 (I2C).
- **API**: `present()`, `freqKhz10()`, `cyclePreset()`, `setTx()/txOn()`.

## AudioModule 🟢⚠️ `AudioModule/`
Звук (I2S, NS4168).
- ⚠️ **Отключён**: `PIN_SPK_WCLK(40) == PIN_LCD_RST(40)` — инициализация I2S
  ломает дисплей (белый экран). Бип на старте выключен. Включать только после
  выяснения корректных I2S-пинов платы.

## LedModule 🟢 `LedModule/`
Адресный RGB WS2812B ×8 (FastLED, пин 14).
- **API**: `setColor(r,g,b)` (постоянный ambient-цвет), `flash()`, `off()`,
  `applySettings()`.
- **Логика**: цвет = цвет выбранной плитки в Home; вспышка зелёным на boot,
  красным при низком заряде; учитывает вкл/яркость из Settings.
- ⚠️ FastLED занимает **RMT канал 0** на первой `show()` — поэтому CC1101=ch2, IR=ch3.

## StorageModule 🟢 `StorageModule/`
Файловое хранилище (SD + LittleFS) и библиотека сигналов.
- **HW**: SD на общей SPI (CS 13); LittleFS во flash (fallback).
- **API**: `fs()`, `sdMounted()/fsReady()`, `appendLine()/writeFile()/readFile()/
  exists()/listDir()`, `saveSignal()/loadSignal()/listSignals()` (формат Flipper `.sub`).
- ⚠️ Весь SD-доступ — под `hal::SpiBusGuard`.

## WardriveModule 🔌 `WardriveModule/`
Лог WiFi-точек с GPS в CSV (формат WiGLE).
- **API**: `activate()/deactivate()`, `active()`, `apCount()`, `path()`.
- **Логика**: периодический WiFi-скан (Scheduler 4с) + текущая позиция GPS →
  `/wardrive.csv`, дедуп по BSSID. `activate` занимает GPS (`acquire`),
  `deactivate` — освобождает.
- ⚠️ Блокирующий скан морозит UI — останавливается при выходе с экрана.

## WebUiModule 🟢 `WebUiModule/`
Веб-интерфейс (softAP + HTTP) и OTA.
- **API**: `activate()/deactivate()` (НЕ start/stop!), `active()`, `ssid()/ip()`.
- AP `VARSYS-XXXX` / pass `varsys1234`; статус, библиотека сигналов, OTA (`/update`).
- ⚠️ После остановки `WIFI_OFF` (экономия).

## EvilPortalModule 🟢 `EvilPortalModule/`
Captive-портал (AP + DNS + форма) — раздел «Эксперт».
- **API**: `activate(ssid)/deactivate()`, `active()`, креды в `/portal_creds.txt`.

## CliModule 🟢 `CliModule/`
Serial-CLI по USB-CDC.
- Команды: `help | hw | ver | freq <kHz> | rssi | rec | replay | ir tvoff |
  wifi scan | ble scan | wifi hs | reboot`. `hw` печатает карту железо→статус.

---

## Помощники Sub-GHz

### RfDecoder (`RadioModule/RfDecoder.*`)
`rfDecode(pulses)` → `{ok, proto, bits, key, te}`. Распознаёт PWM-протоколы
(Princeton/EV1527, CAME, Nice FLO, Holtek) и rolling-code семейства (KeeLoq/HCS)
по преамбуле. Подробно — [ALGORITHMS.md](ALGORITHMS.md).

### RfBrute (`RadioModule/RfBrute.h`)
Таблица протоколов перебора фикс-кодов (Came/Nice/Ansonic/Holtek/Linear/
Chamberlain) — тайминги бит для генерации кадров.
