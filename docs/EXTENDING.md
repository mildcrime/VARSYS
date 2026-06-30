# VARSYS — Extending (recipes)

🌐 **English** · [Русский](EXTENDING.ru.md)

Step-by-step for common changes. Paths from repo root.

## New module
1. `src/modules/FooModule/FooModule.h` (derives `IModule`): `name()="Foo"`,
   `init()`, `update()` if needed, `static instance()`, feature methods (**not
   start/stop**).
2. `.cpp`: `FooModule* FooModule::_self=nullptr;`, set `_self=this;` in `init()`.
3. In `src/core/Core.cpp`: `#include`, `static FooModule s_foo;`,
   `_modules.add(&s_foo);` — **mind the order** (after modules it calls in `init()`).
- ⚠️ Heavy hardware — go lazy (`acquire()` from a screen), not in `init()`.

## New screen
1. `src/ui/screens/FooScreen.h` (derives `Screen`): `name()`, `onCreate`, and
   `onShow/onHide/onEvent` if needed. Reset lazy children in `onCreate`.
2. `.cpp`: `styleScreen(_root); header(_root,"Foo");` + widgets (`card()`/LVGL).
3. In `UIManager.cpp`: `#include`, `addScreen(new FooScreen());`.
4. Open: `UIManager::instance().pushScreen("Foo");`.

## Home tile
In `HomeScreen.cpp::onCreate`: `addTile(ICON_X, lv_color_hex(0x...), "Foo", "Foo");`
(icon, color = LED color, label, screen name).

## i18n string
`i18n.h`: `STR_FOO` before `STR_COUNT`. `i18n.cpp`: add to **both** `RU[]/EN[]`
tables at the same index. Use `tr(STR_FOO)`.

## Setting
`Settings.h/.cpp`: field, getter/setter (NVS + `SETTINGS_CHANGED`), key (≤15),
load with default in `begin()`. Surface it in `SettingsScreen.cpp`.

## CLI command
`CliModule.cpp::exec()`: `else if (cmd=="foo"){...}` + a line in `help`.

## Icon (Tabler)
Codepoint → range in `tools/fonts/generate_fonts.py` → regenerate fonts → macro
`ICON_FOO "\xEE\xAB\xCD"` in `varsys_fonts.h`.

## Timers / events
`Scheduler::instance().every(ms,cb)/after(ms,cb)/cancel(id)` (don't block).
`EventBus::subscribe(type,cb)`, `publish(...)`, `publishDeferred(...)` (when
mutating UI).

## SPI/SD
`{ hal::SpiBusGuard g; ... }`. Files — via `StorageModule` (already guarded).

## QWIIC module (43/44)
Don't claim pins in `init()`. `acquire()` (begin) / `release()` (end +
`pinMode(43/44,INPUT)`). In the screen: `onShow→acquire`, `onHide→release`.
Mind GPS/NRF24/iButton mutual exclusivity.

## Sub-GHz protocol / IR brand
Decode — a branch in `RfDecoder.cpp`; bruteforce — `RfBrute.h` table; IR — `U_*`
tables in `IrModule.cpp` (+ an encoder in `InfraRed.cpp` for a new protocol).

## On-device workflow
1. Edit → `pio run -e t-embed-cc1101`.
2. Download mode (**encoder wheel + RST**) → `-t upload`.
3. Logs: pyserial `dtr=True`; temporary `LOGI`, then remove.
4. Release: tag `vX.Y.Z` → CI. Version in `platformio.ini` + `flasher/manifest.json`
   + `flasher/index.html`.

Before changing — check: [ARCHITECTURE](ARCHITECTURE.md) (init order / timing),
[HARDWARE](HARDWARE.md) (shared resources), [BACKLOG](../BACKLOG.md).
