# VARSYS — Architecture

🌐 **English** · [Русский](ARCHITECTURE.ru.md)

Firmware for the **LILYGO T-Embed CC1101 Plus S3** (ESP32-S3). This is the map of
how the codebase is structured, so you can onboard fast and extend it safely.

> Docs: **ARCHITECTURE** · [MODULES](MODULES.md) · [UI](UI.md) ·
> [ALGORITHMS](ALGORITHMS.md) · [HARDWARE](HARDWARE.md) ·
> [EXTENDING](EXTENDING.md) · [BACKLOG](../BACKLOG.md)

---

## 1. Layers

```
┌──────────────────────────────────────────────────────────────┐
│  Screens (Screen)        HomeScreen, SubGhzScreen, NfcScreen…  │  UI
├──────────────────────────────────────────────────────────────┤
│  UIManager + UITheme + i18n + Notify + Splash + StatusOverlay  │  UI framework
├──────────────────────────────────────────────────────────────┤
│  Modules (IModule)       Radio, Ir, Nfc, Wifi, Ble, Power…     │  business logic
├──────────────────────────────────────────────────────────────┤
│  Core  Core · ModuleManager · EventBus · Scheduler · Settings  │  orchestration
│        Clock · Logger · SpiBus                                  │
├──────────────────────────────────────────────────────────────┤
│  HAL   Display(TFT_eSPI) · CC1101 · InfraRed · board_pins      │  hardware
├──────────────────────────────────────────────────────────────┤
│  Arduino-ESP32 · LVGL 8.3 · TFT_eSPI · NimBLE · RF24 · …       │  platform
└──────────────────────────────────────────────────────────────┘
```

Dependencies point **down**: screens call modules, modules call core/HAL.
Sideways decoupling is via `EventBus`; rare direct access via `Module::instance()`.

## 2. Core entities (`src/core/`)

**Core** — singleton orchestrator. `begin()`: Logger → `hal::spiBusInit()` →
`Clock::begin()` → `Settings::begin()` → `registerModules()` → `initAll()` →
`startAll()` → publish `SYS_BOOT_DONE`. `loop()` every `VARSYS_TICK_MS` (5 ms):
`dispatchDeferred()` → `Scheduler::update()` → `updateAll()`, then `delay(1)`
(yields CPU to FreeRTOS idle — power saving, feeds the WDT).

**IModule** (`Module.h`) — subsystem contract: `name/init/start/update/stop` +
`enabled`. ⚠️ **Do not name feature methods `start()/stop()`** — those are the
lifecycle and `startAll()` calls them at boot (caused an auto-start bug). Use
`activate()/deactivate()`.

**ModuleManager** — module list, lifecycle, `find(name)`. **Registration order =
init/update order** (see §4).

**EventBus** — pub/sub. `publish` is synchronous; `publishDeferred` queues and
delivers at the start of the next tick — use it when a handler mutates the
UI/subscriptions. ⚠️ Not thread-safe.

**Scheduler** — cooperative timers `every/after/cancel` (callbacks run on the main
loop — must not block).

**Settings** — NVS runtime config, loaded before modules; setters write NVS +
publish `SETTINGS_CHANGED` (language/theme → `LANG_CHANGED`/`UI_REBUILD`).

**Clock / Logger / SpiBus** — time; `LOGI/W/E/D`; shared-SPI arbitration
(`hal::SpiBusGuard`, recursive mutex — thread-safe, ready for future threads).

## 3. Execution model

**Cooperative, single-threaded.** One loop drives every module every 5 ms.
- **A blocking op freezes the whole UI** (WiFi scan ~2s, CC1101 record ~3s). A
  deliberate trade-off; candidate for FreeRTOS worker tasks — see BACKLOG.
- LVGL is serviced in `UIManager::update()` (`lv_timer_handler`); tick is
  `LV_TICK_CUSTOM` via `millis()`.

⚠️ **Timing pitfall:** `now` is captured once at the start of the tick. If a
handler later stored `millis()` as a timestamp, `now - saved` underflows on
unsigned (caused a false screen-off bug). Always: `idle = (now>=last)?now-last:0;`

## 4. Init order (the mine we stepped on)

```
Input, UI, Storage, Power, Radio, Ir, Nfc, Wifi, Ble, Badusb,
Cli, WebUi, Led, Gps, Nrf, Audio, Portal, IButton, Fm, Wardrive   (20)
```
- **Screens are built in `UIManager::start()`, not `init()`** — their `onCreate()`
  touches other modules initialized after UI (else null-deref crash loop).
- A module whose `init()` calls `OtherModule::instance()` must be registered AFTER it.
- QWIIC modules (Gps/Nrf/IButton) **don't claim pins in `init()`** — lazy
  `acquire/release` (see [HARDWARE](HARDWARE.md)).

## 5. Startup control flow

```
setup() → Core::begin()
  Logger / spiBusInit / Clock / Settings / registerModules
  initAll()  → UIManager::init(): Display + LVGL (buffers, flush_cb)
  startAll() → UIManager::start(): buildScreens → setScreen("Home") → Splash
  publish(SYS_BOOT_DONE)
loop() every 5 ms: dispatchDeferred → Scheduler → updateAll → delay(1)
```

## 6. Input event flow

```
Encoder/buttons → InputModule (decoder + debounce) → publish INPUT_*
   ├→ PowerModule (noteActivity: reset sleep / wake)
   └→ UIManager → active Screen::onEvent
INPUT_BACK / BTN_LONG → UIManager::popScreen()
```

## 7. Display & shared SPI bus
Display (ST7789) + CC1101 + SD share one SPI bus (9/10/11), differ only in CS.
One `SPIClass` (TFT_eSPI); others use `Display::spi()`. Every transaction and the
LVGL flush run under `hal::SpiBusGuard`. RMT/QWIIC details — [HARDWARE](HARDWARE.md).

## 8. Structure

```
src/main.cpp · core/ · hal/ · modules/<Name>Module/ · ui/(UIManager,screens,fonts,i18n)
include/varsys_config.h · include/lv_conf.h (LV_USE_FONT_COMPRESSED=1!)
boards/ · flasher/ · docs/
```

## 9. Architectural decisions
- Modularity over monolith (unlike Bruce).
- Cooperative core — simplicity at the cost of responsiveness.
- One `SPIClass` (TFT_eSPI) for the whole bus.
- Boundary: broadband jammer / mass spam are deliberate stubs (Expert section).
