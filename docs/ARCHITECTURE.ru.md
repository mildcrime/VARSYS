# VARSYS — Архитектура

🌐 [English](ARCHITECTURE.md) · **Русский**

Прошивка для **LILYGO T-Embed CC1101 Plus S3** (ESP32-S3). Это карта того, как
устроена кодовая база, чтобы быстро войти и спокойно дорабатывать.

> Документация: **ARCHITECTURE** · [MODULES](MODULES.ru.md) · [UI](UI.ru.md) ·
> [ALGORITHMS](ALGORITHMS.ru.md) · [HARDWARE](HARDWARE.ru.md) ·
> [EXTENDING](EXTENDING.ru.md) · [BACKLOG](../BACKLOG.md)

---

## 1. Слои

```
┌──────────────────────────────────────────────────────────────┐
│  Экраны (Screen)         HomeScreen, SubGhzScreen, NfcScreen…  │  UI
├──────────────────────────────────────────────────────────────┤
│  UIManager + UITheme + i18n + Notify + Splash + StatusOverlay  │  UI-фреймворк
├──────────────────────────────────────────────────────────────┤
│  Модули (IModule)        Radio, Ir, Nfc, Wifi, Ble, Power…     │  бизнес-логика
├──────────────────────────────────────────────────────────────┤
│  Ядро  Core · ModuleManager · EventBus · Scheduler · Settings  │  оркестрация
│        Clock · Logger · SpiBus                                  │
├──────────────────────────────────────────────────────────────┤
│  HAL   Display(TFT_eSPI) · CC1101 · InfraRed · board_pins      │  железо
├──────────────────────────────────────────────────────────────┤
│  Arduino-ESP32 · LVGL 8.3 · TFT_eSPI · NimBLE · RF24 · …       │  платформа
└──────────────────────────────────────────────────────────────┘
```

Зависимости направлены **вниз**: экраны зовут модули, модули — ядро/HAL.
Развязка «вбок» — через `EventBus`, а редкий прямой доступ — через синглтоны
`Module::instance()`.

## 2. Ключевые сущности ядра (`src/core/`)

**Core** — синглтон-оркестратор. `begin()`: Logger → `hal::spiBusInit()` →
`Clock::begin()` → `Settings::begin()` → `registerModules()` → `initAll()` →
`startAll()` → publish `SYS_BOOT_DONE`. `loop()` раз в `VARSYS_TICK_MS` (5 мс):
`dispatchDeferred()` → `Scheduler::update()` → `updateAll()`, затем `delay(1)`.

**IModule** (`Module.h`) — контракт подсистемы: `name/init/start/update/stop` +
флаг `enabled`. ⚠️ **Не называйте фича-методы `start()/stop()`** — это lifecycle,
`startAll()` их вызовет при загрузке (был баг автозапуска). Используйте
`activate()/deactivate()`.

**ModuleManager** — список модулей, lifecycle, `find(name)`. **Порядок
регистрации = порядок init/update** (см. §4).

**EventBus** — pub/sub. `publish` синхронно; `publishDeferred` в очередь
(доставка в начале следующего тика) — для случаев, когда обработчик меняет
UI/подписки. ⚠️ Не потокобезопасен.

**Scheduler** — кооперативные таймеры `every/after/cancel` (колбэки в главном
цикле, не блокировать).

**Settings** — рантайм-конфиг на NVS, грузится до модулей; сеттеры пишут в NVS +
публикуют `SETTINGS_CHANGED` (язык/тема → `LANG_CHANGED`/`UI_REBUILD`).

**Clock / Logger / SpiBus** — время; `LOGI/W/E/D`; арбитраж общей SPI-шины
(`hal::SpiBusGuard`, рекурсивный мьютекс — потокобезопасен).

## 3. Модель исполнения

**Кооперативная, однопоточная.** Один цикл прокручивает все модули раз в 5 мс.
- **Блокирующая операция морозит весь UI** (WiFi-скан ~2с, запись CC1101 ~3с).
  Осознанный компромисс; кандидат на FreeRTOS-задачи — см. BACKLOG.
- LVGL обслуживается в `UIManager::update()` (`lv_timer_handler`); тик —
  `LV_TICK_CUSTOM` через `millis()`.

⚠️ **Грабли времени:** `now` фиксируется в начале тика. Если обработчик сохранил
`millis()` позже как «время активности», `now - saved` уходит в **underflow**
(был баг ложного гашения). Всегда: `idle = (now>=last)?now-last:0;`

## 4. Порядок инициализации (мина)

```
Input, UI, Storage, Power, Radio, Ir, Nfc, Wifi, Ble, Badusb,
Cli, WebUi, Led, Gps, Nrf, Audio, Portal, IButton, Fm, Wardrive   (20)
```
- **Экраны строятся в `UIManager::start()`, не в `init()`** — их `onCreate()`
  трогает другие модули, инициализируемые позже UI (иначе краш).
- Модуль, чей `init()` зовёт `OtherModule::instance()`, регистрировать ПОСЛЕ него.
- QWIIC-модули (Gps/Nrf/IButton) **не занимают пины в `init()`** — ленивый
  `acquire/release` (см. [HARDWARE](HARDWARE.ru.md)).

## 5. Поток управления при старте

```
setup() → Core::begin()
  Logger / spiBusInit / Clock / Settings / registerModules
  initAll()  → UIManager::init(): Display + LVGL (буферы, flush_cb)
  startAll() → UIManager::start(): buildScreens → setScreen("Home") → Splash
  publish(SYS_BOOT_DONE)
loop() каждые 5 мс: dispatchDeferred → Scheduler → updateAll → delay(1)
```

## 6. Поток событий ввода

```
Энкодер/кнопки → InputModule (декодер+антидребезг) → publish INPUT_*
   ├→ PowerModule (noteActivity: сброс сна/пробуждение)
   └→ UIManager → активный Screen::onEvent
INPUT_BACK / BTN_LONG → UIManager::popScreen()
```

## 7. Дисплей и общая шина SPI
Дисплей(ST7789)+CC1101+SD на одной SPI (9/10/11), различаются CS. Один `SPIClass`
(TFT_eSPI), прочие — `Display::spi()`. Каждая транзакция и LVGL flush — под
`hal::SpiBusGuard`. Подробности RMT/QWIIC — [HARDWARE](HARDWARE.ru.md).

## 8. Структура

```
src/main.cpp · core/ · hal/ · modules/<Name>Module/ · ui/(UIManager,screens,fonts,i18n)
include/varsys_config.h · include/lv_conf.h (LV_USE_FONT_COMPRESSED=1!)
boards/ · flasher/ · docs/
```

## 9. Архитектурные решения
- Модульность вместо монолита (в отличие от Bruce).
- Кооперативное ядро — простота ценой отзывчивости.
- Один `SPIClass` (TFT_eSPI) на всю шину.
- Граница: глушилка / массовый spam — намеренные заглушки (раздел «Эксперт»).
