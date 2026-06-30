# VARSYS — UI framework

🌐 **English** · [Русский](UI.ru.md)

UI on **LVGL 8.3**, landscape **320×170** (ST7789, rotation 3), iOS-style
springboard of tiles. Custom fonts (Inter + Cyrillic + Tabler icons).

## Screens (mockups)
> ⚠️ Below are **schematic layout mockups**, not device captures (real
> screen-grab is a hardware task, see [BACKLOG](../BACKLOG.md)).

| Home | Feature screen |
|---|---|
| ![Home](img/home.svg) | ![SubGhz](img/subghz.svg) |
| Settings | Hardware missing |
| ![Settings](img/settings.svg) | ![Missing](img/hw_missing.svg) |

---

## 1. Screen (`ui/Screen.h`)
Base class. Each screen owns its `lv_obj_t* _root`. Methods: `name`,
`onCreate(parent)`, `onShow/onHide`, `onUpdate(now)`, `onEvent(e)`.
⚠️ `onCreate` runs on build AND rebuild (`retranslateAll`). Reset lazily-created
children (e.g. `_hwPanel`) at the start of `onCreate` (the old one was freed by
`lv_obj_clean`).

## 2. UIManager (`ui/UIManager.*`)
The module and LVGL owner.
- `init()`: Display + `lv_init` + buffers (`heap_caps_malloc`, `kBufLines`) +
  `flushCb`. **Does not build screens.**
- `start()`: `buildScreens()` → `setScreen("Home")` → `Splash::play` + subscriptions.
- `update()`: `lv_timer_handler` + `active->onUpdate`.
- Navigation: `setScreen(name)` (top level), `pushScreen(name)` (stack),
  `popScreen()`. Back / long-press → `popScreen()`.
- `retranslateAll()`: language/theme change → `lv_obj_clean+onCreate` for all.
- `flushCb`: pushes the buffer to TFT under `hal::SpiBusGuard`.

## 3. UITheme (`ui/UITheme.*`)
Color functions branching on `ui::darkMode`: `cBg/cCard/cText/cText2/cSep/cBlue/
cGreen/cOrange/cRed/cGray/cTint`. Factories: `styleScreen`, `header`, `card`,
`hwMissingPanel(parent,"PN532 NFC","I2C / QWIIC")`. Dark theme by default.

## 4. Localization (`ui/i18n.*`)
`enum StrId` + `tr(StrId)`. Two positional tables `RU[]/EN[]` (order = enum).
Language change → `LANG_CHANGED` → `retranslateAll`. English by default.

## 5. Fonts (`ui/fonts/`)
`varsys_12/14/16/22` (Inter + Cyrillic + Tabler icons). `ICON_*` macros.
⚠️ Fonts are **compressed** → `LV_USE_FONT_COMPRESSED 1` is mandatory in
`include/lv_conf.h`. Generator: `tools/fonts/generate_fonts.py`.

## 6. Overlays
- **Splash** — animated logo on `lv_layer_top` (`create/play`).
- **StatusOverlay** — status bar (time + battery) on `lv_layer_top`.
- **Notify** — toasts `Notify::toast(text, Info|Success|Warn|Error)`.

## 7. Screens (`ui/screens/`)
Registered in `buildScreens()`, opened by `name()`:
Home, SubGhz, Spectrum, SavedSignals, Brute, Ir, IrRemote, Nfc, Wifi, Ble,
Badusb, Mousejack, Nrf, Gps, Wardrive, IButton, Fm, WebUi, Files, Battery,
Settings, Expert, Portal, DevTools.

**HomeScreen** — horizontal carousel: 72px tiles, center focus, instant scroll
(`LV_ANIM_OFF` — tear-free), focus via opacity (not `transform_zoom`). Each tile
stores its color → `LedModule::setColor` on focus change.

Recipes — [EXTENDING](EXTENDING.md).
