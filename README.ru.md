# VARSYS

[English](README.md) · **Русский**

Модульная прошивка для исследований безопасности на **LILYGO T‑Embed CC1101 Plus
(ESP32‑S3)** — мультитул класса Flipper с удобным «телефонным» интерфейсом.

> ⚠️ Только для **авторизованного тестирования и обучения**. Перед использованием
> прочитайте [правовую оговорку](DISCLAIMER.md).

![status](https://img.shields.io/badge/build-passing-brightgreen)
![platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![license](https://img.shields.io/badge/license-MIT-green)

## Зачем VARSYS

Покрывает практически тот же арсенал, что и Bruce, но переосмыслен как продукт:
модульное кооперативное ядро, интерфейс LVGL в стиле iOS 17 (focus‑карусель,
тёмная тема, переключение RU/EN на лету), рантайм‑настройки в NVS, структурное
управление питанием и единый аппаратный конвейер сигналов (RMT) для Sub‑GHz и ИК.

## Возможности

| Домен | Что умеет |
|---|---|
| **Sub‑GHz** (CC1101) | пресеты частот, живой RSSI, скан, **запись/повтор** (RMT), **декодер протоколов** (Princeton/EV1527/CAME/Nice/Holtek), **перебор** (6 протоколов + All, автосохранение кандидатов + автопрогон до подтверждённого кода), **библиотека сигналов** (`.sub`), спектр |
| **Инфракрасный** | захват/повтор (RMT), NEC, универсальное «выключить ТВ» |
| **NFC/RFID** (PN532) | чтение UID/типа, запись блока Mifare, сохранение дампов |
| **WiFi** | скан AP, снифер, deauth (авторизованно), **детект handshake (EAPOL)**, captive **evil portal** (за гейтом) |
| **Bluetooth** | скан/разведка BLE‑устройств (NimBLE) |
| **NRF24** | анализатор каналов 2.4 ГГц |
| **GPS** | спутники / фикс / координаты (TinyGPS++) |
| **iButton** | чтение ROM Dallas 1‑Wire |
| **FM** | передатчик Si4713 |
| **BadUSB** | встроенный USB‑HID, скрипты в стиле Ducky |
| **Связность** | WebUI (точка доступа + HTTP, библиотека сигналов, **OTA**), Serial CLI |
| **Система** | тёмная/светлая тема, RU/EN, батарея (BQ27220), димминг/сон, RGB‑индикация, бип, Dev tools (инфо/I²C‑скан/дамп CC1101/сброс) |

## Железо

LILYGO **T‑Embed CC1101 Plus S3**: ESP32‑S3 (16 МБ flash, OPI PSRAM),
дисплей ST7789 320×170, энкодер + кнопки, CC1101 sub‑GHz, NFC PN532,
RGB WS2812 ×8, динамик NS4168, топливомер BQ27220.

> Распиновка повторяет проверенную конфигурацию Bruce — см.
> [`src/hal/board_pins.h`](src/hal/board_pins.h). Внешние модули
> (NRF24, GPS, FM, iButton) подключаются к порту QWIIC/UART.

## Прошивка прямо из браузера (без установки)

Откройте **<https://mildcrime.github.io/VARSYS/flasher/>** в **Chrome или Edge**,
подключите устройство по USB, нажмите **Install**. Работает на Windows / macOS /
Linux. Или двойной клик по ярлыку из [`flasher/`](flasher/). Подробности:
[flasher/README.md](flasher/README.md).

## Сборка и прошивка (из исходников)

Нужен [PlatformIO](https://platformio.org/).

```bash
git clone <url-вашего-форка> VARSYS && cd VARSYS
pio run -e t-embed-cc1101                 # сборка
pio run -e t-embed-cc1101 -t upload -t monitor   # прошивка + монитор
```

Первая сборка скачивает тулчейн и библиотеки (несколько минут). В мониторе
доступен CLI — введите `help`.

## Управление

- **Вращение энкодера** — перемещение / прокрутка карусели.
- **Нажатие энкодера** — открыть / активировать.
- **Кнопка «назад»** (сбоку) или **долгое нажатие** — назад / прервать действие.

## Использование

Полные руководства: **[USAGE (RU)](docs/USAGE_RU.md)** · [USAGE (EN)](docs/USAGE_EN.md).
Архитектура: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

Быстрые примеры:
- **Открыть шлагбаум (перебор):** Sub‑GHz → задать частоту → *Перебор* →
  выбрать протокол (или *All*) → *Старт*. Когда открылся — нажать **«назад»**:
  последние коды сохранятся в `/brute/*.txt`. Запустить *Candidates* для
  автопрогона и подтверждения точного кода (→ `/brute/confirmed_*.sub`).
- **Записать и повторить пульт:** Sub‑GHz → *Запись* → *Воспроизвести* / *Сохранить*.
- **Прочитать NFC‑метку:** NFC → *Читать* → *Сохранить*.

## Структура проекта

```
src/
  core/      Core, ModuleManager, EventBus, Scheduler, Settings, Clock, Logger
  hal/       board_pins, Display (TFT_eSPI), CC1101, InfraRed, SpiBus
  modules/   Radio, Ir, Nfc, Wifi, Ble, Nrf, Gps, IButton, Fm, BadUsb,
             Audio, Led, Power, Storage, WebUi, Cli, EvilPortal, Input
  ui/        UIManager, UITheme, i18n, Notify, Splash, StatusOverlay, screens/, fonts/
tools/fonts/ генератор шрифтов (Inter + Tabler) — см. его README
boards/      кастомный lilygo-t-embed-cc1101.json
```

## Раздел «Эксперт»

Неизбирательные инструменты (широкополосная глушилка, массовый BLE/Apple‑spam)
намеренно поставляются как **нерабочие заглушки** за гейтнутым режимом
«Эксперт» — они бьют по окружающим, а их передача незаконна в большинстве
юрисдикций. В разделе есть рабочие инженерные инструменты (Dev tools, портал).

## Правовое

См. **[DISCLAIMER.md](DISCLAIMER.md)**. Только авторизованное использование. Вы
отвечаете за соблюдение всех законов, включая радиочастотное регулирование.

## Благодарности

Построено на LVGL, TFT_eSPI, NimBLE, Adafruit/PN532·Si4713, RF24, TinyGPS++,
FastLED, OneWire. Шрифты: Inter (OFL), Tabler Icons (MIT). Распиновка и
референсы протоколов — из проекта [Bruce](https://github.com/pr3y/Bruce).

## Лицензия

MIT — см. [LICENSE](LICENSE).
