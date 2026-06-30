# VARSYS — Рецепты доработок

🌐 [English](EXTENDING.md) · **Русский**

Пошагово для частых изменений. Пути от корня репо.

## Новый модуль
1. `src/modules/FooModule/FooModule.h` (наследник `IModule`): `name()="Foo"`,
   `init()`, при нужде `update()`, `static instance()`, фича-методы (**не start/stop**).
2. `.cpp`: `FooModule* FooModule::_self=nullptr;`, в `init()` — `_self=this;`.
3. В `src/core/Core.cpp`: `#include`, `static FooModule s_foo;`, `_modules.add(&s_foo);`
   — **учесть порядок** (после модулей, что зовёт в `init()`).
- ⚠️ Тяжёлое железо — лениво (`acquire()` из экрана), не в `init()`.

## Новый экран
1. `src/ui/screens/FooScreen.h` (наследник `Screen`): `name()`, `onCreate`,
   при нужде `onShow/onHide/onEvent`. Лениво созданные объекты обнулять в `onCreate`.
2. `.cpp`: `styleScreen(_root); header(_root,"Foo");` + виджеты (`card()`/LVGL).
3. В `UIManager.cpp`: `#include`, `addScreen(new FooScreen());`.
4. Открыть: `UIManager::instance().pushScreen("Foo");`.

## Плитка на главном экране
В `HomeScreen.cpp::onCreate`: `addTile(ICON_X, lv_color_hex(0x...), "Foo", "Foo");`
(иконка, цвет=цвет LED, подпись, name экрана).

## Строка локализации
`i18n.h`: `STR_FOO` перед `STR_COUNT`. `i18n.cpp`: строка в **обе** таблицы
`RU[]/EN[]` в той же позиции. Использовать `tr(STR_FOO)`.

## Настройка
`Settings.h/.cpp`: поле, геттер/сеттер (NVS + `SETTINGS_CHANGED`), ключ (≤15),
загрузка с дефолтом в `begin()`. Показать в `SettingsScreen.cpp`.

## CLI-команда
`CliModule.cpp::exec()`: `else if (cmd=="foo"){...}` + строка в `help`.

## Иконка (Tabler)
Кодпоинт → диапазон в `tools/fonts/generate_fonts.py` → перегенерить шрифты →
макрос `ICON_FOO "\xEE\xAB\xCD"` в `varsys_fonts.h`.

## Таймер / события
`Scheduler::instance().every(ms,cb)/after(ms,cb)/cancel(id)` (не блокировать).
`EventBus::subscribe(type,cb)`, `publish(...)`, `publishDeferred(...)` (если меняете UI).

## SPI/SD
`{ hal::SpiBusGuard g; ... }`. Файлы — через `StorageModule` (уже под гардом).

## QWIIC-модуль (43/44)
Не занимать пины в `init()`. `acquire()` (begin) / `release()` (end +
`pinMode(43/44,INPUT)`). В экране: `onShow→acquire`, `onHide→release`. Помнить про
взаимоисключаемость GPS/NRF24/iButton.

## Sub-GHz протокол / ИК-бренд
Декод — ветка в `RfDecoder.cpp`; перебор — таблица `RfBrute.h`; ИК — таблицы `U_*`
в `IrModule.cpp` (+ кодировщик в `InfraRed.cpp` для нового протокола).

## Рабочий цикл на железе
1. Правка → `pio run -e t-embed-cc1101`.
2. Download mode (**колесо+RST**) → `-t upload`.
3. Логи: pyserial `dtr=True`; временный `LOGI`, потом убрать.
4. Релиз: тег `vX.Y.Z` → CI. Версия в `platformio.ini`+`flasher/manifest.json`+
   `flasher/index.html`.

Перед изменением — сверься: [ARCHITECTURE](ARCHITECTURE.ru.md) (порядок init/время),
[HARDWARE](HARDWARE.ru.md) (общие ресурсы), [BACKLOG](../BACKLOG.md).
