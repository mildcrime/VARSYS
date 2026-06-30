# VARSYS — Рецепты доработок

Пошагово, как делать частые изменения. Все пути от корня репо.

---

## Добавить новый модуль (подсистему)
1. Создать `src/modules/FooModule/FooModule.h` (наследник `IModule`):
```cpp
#pragma once
#include "core/Module.h"
class FooModule : public IModule {
public:
    const char* name() const override { return "Foo"; }
    bool init() override;
    void update(uint32_t now) override;   // если нужна периодика
    static FooModule& instance() { return *_self; }
    void doThing();                       // фича-метод (НЕ start/stop!)
private:
    static FooModule* _self;
};
```
2. `FooModule.cpp`: `FooModule* FooModule::_self=nullptr;` + реализация. В `init()`
   присвоить `_self = this;`.
3. Зарегистрировать в `src/core/Core.cpp`:
   - `#include "modules/FooModule/FooModule.h"`
   - `static FooModule s_foo;`
   - `_modules.add(&s_foo);` в `registerModules()` — **учесть порядок**: если
     `init()` зовёт другой модуль (`UIManager`/`Storage`), ставить ПОСЛЕ него.
- ⚠️ Тяжёлое/блокирующее железо инициализировать **лениво** (метод `acquire()`,
  вызываемый из экрана), а не в `init()`.

## Добавить экран
1. `src/ui/screens/FooScreen.h` (наследник `Screen`): `name()`, `onCreate`,
   при необходимости `onShow/onHide/onEvent`. Лениво созданные дочерние объекты
   обнулять в начале `onCreate`.
2. `FooScreen.cpp`: в `onCreate` — `styleScreen(_root); header(_root, "Foo");` +
   виджеты через `card()`/LVGL.
3. Зарегистрировать в `src/ui/UIManager.cpp`:
   - `#include "ui/screens/FooScreen.h"`
   - `addScreen(new FooScreen());` в `buildScreens()`.
4. Открывать: `UIManager::instance().pushScreen("Foo");`.

## Добавить плитку на главный экран
В `src/ui/screens/HomeScreen.cpp`, функция `onCreate`, рядом с другими `addTile`:
```cpp
addTile(ICON_ANTENNA, lv_color_hex(0xFF9F0A), "Foo", "Foo");
//       иконка        цвет (=цвет LED)        подпись  name экрана
```
Цвет плитки автоматически становится цветом RGB-подсветки при фокусе.

## Добавить строку локализации
В `src/ui/i18n.h` — новый `STR_FOO` **перед** `STR_COUNT`. В `src/ui/i18n.cpp` —
строку в **обе** таблицы `RU[]` и `EN[]` в **той же позиции**. Использовать `tr(STR_FOO)`.

## Добавить настройку
В `src/core/Settings.h/.cpp`: поле, геттер/сеттер (сеттер пишет в NVS + публикует
`SETTINGS_CHANGED`), ключ NVS (≤15 симв.), загрузку с дефолтом в `begin()`.
Показать в `SettingsScreen.cpp` (новая строка + `Action`).

## Добавить CLI-команду
В `src/modules/CliModule/CliModule.cpp`, функция `exec()`: новая ветка
`else if (cmd == "foo") { ... }` + строку в `help`.

## Добавить иконку (глиф Tabler)
1. Найти кодпоинт иконки Tabler (напр. U+EAxx).
2. Добавить диапазон в `tools/fonts/generate_fonts.py` (`-r 0xEAxx`), перегенерить
   шрифты `varsys_*`.
3. Объявить макрос в `src/ui/fonts/varsys_fonts.h`:
   `#define ICON_FOO "\xEE\xAB\xCD"` (UTF-8 кодпоинта).

## Периодическая задача
```cpp
uint32_t id = Scheduler::instance().every(1000, []{ /* раз в секунду */ });
Scheduler::instance().after(500, []{ /* однократно */ });
Scheduler::instance().cancel(id);
```
Колбэк крутится в главном цикле — **не блокировать**.

## События
```cpp
EventBus::subscribe(EventType::POWER_CHANGED, [](const Event& e){ /* e.i32 */ });
EventBus::publish(EventType::SETTINGS_CHANGED);
EventBus::publishDeferred(EventType::UI_REBUILD);   // если меняете UI/подписки
```

## Доступ к SPI/SD (общая шина)
```cpp
{ hal::SpiBusGuard g; /* транзакции CC1101/SD/NRF24 */ }   // RAII
```
SD-файлы — через `StorageModule` (`writeFile/readFile/appendLine/listDir`),
он уже под гардом.

## Модуль на порту QWIIC (43/44)
Не занимать пины в `init()`. Паттерн:
```cpp
void FooModule::acquire() { if(_active) return; /* begin UART/SPI/OneWire */ _active=true; }
void FooModule::release() { if(!_active) return; /* end + pinMode(43/44,INPUT) */ _active=false; }
```
В экране: `onShow → acquire()`, `onHide → release()`. Помнить про
взаимоисключаемость с GPS/NRF24/iButton.

## Добавить Sub-GHz протокол / ИК-бренд
- Декод: новая ветка классификации в `RadioModule/RfDecoder.cpp` (или отдельный
  декодер для не-PWM).
- Перебор: запись в таблицу `RadioModule/RfBrute.h`.
- ИК-пульт: дополнить таблицы `U_*` в `IrModule.cpp`; новый протокол — кодировщик
  в `hal/InfraRed.cpp`.

---

## Рабочий цикл (на железе)
1. Правка → `pio run -e t-embed-cc1101`.
2. Download mode (**колесо+RST**, см. [HARDWARE.md](HARDWARE.md)) → `-t upload`.
3. Логи: pyserial `dtr=True`; временный `LOGI` для диагностики, потом убрать.
4. Релиз: тег `vX.Y.Z` → CI собирает, обновляет веб-прошивальщик. Версию
   синхронизировать в `platformio.ini`, `flasher/manifest.json`, `flasher/index.html`.

## Перед изменением — свериться
- [ARCHITECTURE.md](ARCHITECTURE.md) — порядок init, модель времени.
- [HARDWARE.md](HARDWARE.md) — общие ресурсы (SPI/RMT/QWIIC), грабли.
- [BACKLOG.md](../BACKLOG.md) — известные проблемы (не чинить дважды).
