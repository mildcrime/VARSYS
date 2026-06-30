# VARSYS — Архитектура

Прошивка для **LILYGO T-Embed CC1101 Plus S3** (ESP32-S3). Это карта того, как
устроена кодовая база, чтобы быстро войти и спокойно дорабатывать.

> Документация (этот комплект):
> - **ARCHITECTURE.md** — ядро, модель исполнения, потоки данных (этот файл)
> - [MODULES.md](MODULES.md) — каталог модулей и их публичные API
> - [UI.md](UI.md) — UI-фреймворк (LVGL), экраны, темы, локализация
> - [ALGORITHMS.md](ALGORITHMS.md) — ключевые алгоритмы (OOK, brute, RMT, NFC, энкодер…)
> - [HARDWARE.md](HARDWARE.md) — распиновка, общие ресурсы, сборка/прошивка, грабли
> - [EXTENDING.md](EXTENDING.md) — рецепты: добавить модуль/экран/плитку/строку/CLI
> - [../BACKLOG.md](../BACKLOG.md) — известные проблемы и задачи

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
Развязка «вбок» — через `EventBus` (модули не держат ссылок друг на друга),
а редкий прямой доступ — через синглтоны `Module::instance()`.

---

## 2. Ключевые сущности ядра (`src/core/`)

### Core (`Core.*`)
Синглтон-оркестратор. `main.cpp` лишь делегирует:
```cpp
void setup() { Core::instance().begin(); }
void loop()  { Core::instance().loop();  }
```
- `begin()`: Logger → `hal::spiBusInit()` → `Clock::begin()` → `Settings::begin()`
  → `registerModules()` → `ModuleManager::initAll()` → `startAll()` →
  publish `SYS_BOOT_DONE`.
- `loop()`: раз в `VARSYS_TICK_MS` (5 мс) — `EventBus::dispatchDeferred()` →
  `Scheduler::update()` → `ModuleManager::updateAll()`. В конце `delay(1)` —
  уступает CPU FreeRTOS-idle (энергосбережение, кормит WDT).

### IModule (`Module.h`)
Контракт любой подсистемы:
```cpp
const char* name();          // тег логов / поиск
bool init();                 // однократно, false = фатально
void start();                // после initAll() всех модулей
void update(uint32_t now);   // каждый тик (now = millis())
void stop();                 // корректное завершение
bool enabled;                // участвует ли в updateAll
```
⚠️ **Не называйте фича-методы `start()`/`stop()`** — они переопределят lifecycle
и `startAll()` их вызовет при загрузке (был баг: WebUI/Wardrive автозапускались).
Используйте `activate()/deactivate()` (см. [MODULES.md](MODULES.md)).

### ModuleManager (`ModuleManager.*`)
Хранит список модулей, гоняет lifecycle (`initAll/startAll/updateAll`),
`find(name)` для редкого прямого доступа. **Порядок регистрации = порядок
init/update** — критичен (см. §4).

### EventBus (`EventBus.*`)
Pub/sub. `EventType` — перечисление (системные, ввод, питание, настройки…).
- `publish(type, i32, ptr)` — **синхронно**, зовёт подписчиков немедленно.
- `publishDeferred(...)` — в очередь, доставляется в начале следующего тика
  (`dispatchDeferred`). Использовать, когда обработчик меняет UI/подписки
  (напр. `LANG_CHANGED`, `UI_REBUILD`).
- ⚠️ Не потокобезопасен (синхронная итерация вектора).

### Scheduler (`Scheduler.*`)
Кооперативные таймеры (`every`/`after`/`cancel`), колбэки крутятся в главном
цикле — **не блокируют, но и не вытесняют**. Возвращает `id` для отмены.

### Settings (`Settings.*`)
Синглтон рантайм-конфига на NVS (`Preferences`). Грузится в `Core::begin()` до
модулей. Каждый сеттер пишет в NVS и публикует `SETTINGS_CHANGED` (а смена
языка/темы — `LANG_CHANGED`/`UI_REBUILD`). Поля: яркость, ориентация, язык,
звук, вибро, тёмная тема, эксперт, частота Sub-GHz, таймаут экрана, раскладка
BadUSB, вкл/яркость LED.

### Clock / Logger / SpiBus
- **Clock** — системное время до NTP (статус-бар).
- **Logger** — `LOGI/LOGW/LOGE/LOGD(TAG, …)` в Serial. Уровень в `Core::begin`.
- **SpiBus** — арбитраж общей шины: `hal::SpiBusGuard` (RAII) на **рекурсивном
  мьютексе** (потокобезопасен — задел под многозадачность). Любой доступ к
  дисплею/CC1101/SD/NRF24 — под гардом.

---

## 3. Модель исполнения (важно понимать)

**Кооперативная, однопоточная.** Один главный цикл (`loop()` в FreeRTOS-задаче
`loopTask`) по очереди прокручивает все модули раз в 5 мс. Своих потоков мы не
заводим (WiFi/BLE-стеки ESP-IDF — исключение, у них свои задачи).

Следствия:
- **Блокирующая операция морозит весь UI.** WiFi-скан (~2с), запись CC1101
  (~3с), BLE-скан — на это время интерфейс замирает. Это осознанный компромисс
  (простота vs отзывчивость). Кандидат на вынос в FreeRTOS-задачи — см.
  [BACKLOG.md](../BACKLOG.md).
- **LVGL обслуживается** в `UIManager::update()` (`lv_timer_handler` раз в
  `VARSYS_LVGL_TICK_MS`); тик LVGL — `LV_TICK_CUSTOM` через `millis()`.

⚠️ **Грабли времени:** `now` в `update(now)` фиксируется один раз в начале тика
(`Core::loop`). Если обработчик события вызвал `millis()` позже и сохранил его
как «время активности», то `now - saved` уходит в **underflow** (был баг ложного
гашения экрана). Для вычитания времени всегда защищайтесь:
`idle = (now >= last) ? now - last : 0;`

---

## 4. Порядок инициализации (мина, на которой подрывались)

`registerModules()` задаёт порядок. `initAll()` идёт по нему, `startAll()` — после.

```
Input, UI, Storage, Power, Radio, Ir, Nfc, Wifi, Ble, Badusb,
Cli, WebUi, Led, Gps, Nrf, Audio, Portal, IButton, Fm, Wardrive   (20 модулей)
```

Правила:
- **UI инициализируется рано** (поднимает дисплей/SPI), но **экраны строятся
  не в `init()`, а в `UIManager::start()`** — их `onCreate()` обращается к
  другим модулям (RadioModule и т.п.), которые инициализируются ПОЗЖE UI.
  Строить экраны в init = null-дереф (был краш-цикл).
- Модуль, чей `init()` зовёт `OtherModule::instance()`, должен быть
  зарегистрирован ПОСЛЕ того модуля (Storage/Radio/Nrf используют
  `UIManager::instance().display().spi()` — поэтому после UI).
- Модули общего QWIIC-порта (Gps/Nrf/IButton) **не занимают пины в `init()`**
  (иначе затирают друг друга) — ленивый `acquire()/release()` (см. [HARDWARE.md](HARDWARE.md)).

---

## 5. Поток управления при старте

```
setup() → Core::begin()
  Logger::begin()
  hal::spiBusInit()                 // мьютекс шины
  Clock::begin(); Settings::begin() // NVS до модулей
  registerModules()
  ModuleManager::initAll()          // init() каждого по порядку
    └ UIManager::init(): Display.begin() + lv_init + draw buffers + flush_cb
  ModuleManager::startAll()
    └ UIManager::start(): buildScreens() → setScreen("Home") → Splash::play()
  publish(SYS_BOOT_DONE)            // LED-вспышка, статус-бар, аудио-бип(выкл)
loop() → Core::loop() каждые 5 мс
  dispatchDeferred() → Scheduler::update() → updateAll()
    InputModule.update(): опрос энкодера/кнопок → publish INPUT_*
    UIManager.update():   lv_timer_handler() → active screen onUpdate/flush
    PowerModule.update():  таймауты экрана, опрос батареи
    …остальные модули
  delay(1)
```

---

## 6. Поток событий ввода

```
Энкодер/кнопки (GPIO)
  → InputModule::update() (квадратурный декодер, антидребезг)
  → EventBus::publish(INPUT_ENCODER_CW/CCW | BTN_CLICK | BTN_LONG | BACK)
      ├→ PowerModule (noteActivity: сброс сна / пробуждение)
      └→ UIManager (forward → активный Screen::onEvent)
            └→ Screen реагирует (moveSelection / activate / push)
INPUT_BACK / BTN_LONG → UIManager::popScreen() (централизованно)
```

---

## 7. Дисплей и общая шина SPI

Дисплей (ST7789), CC1101 и SD **на одной SPI-шине** (MOSI 9 / MISO 10 / SCK 11),
различаются только CS. Модель — как в Bruce: один `SPIClass` от TFT_eSPI,
остальные используют `Display::spi()` (= `tft.getSPIinstance()`). Каждая
транзакция — под `hal::SpiBusGuard`. LVGL flush тоже под гардом
(`UIManager::flushCb`). Подробности и RMT/QWIIC — в [HARDWARE.md](HARDWARE.md).

---

## 8. Где что лежит

```
src/
  main.cpp                 setup/loop → Core
  core/                    Core, ModuleManager, EventBus, Scheduler,
                           Settings, Clock, Logger, Module.h, SpiBus
  hal/                     Display(TFT_eSPI), CC1101, InfraRed, board_pins.h
  modules/<Name>Module/    одна подсистема = один модуль (IModule)
  ui/
    UIManager, UITheme, Screen.h, Splash, StatusOverlay, Notify, i18n
    fonts/                 varsys_* (Inter+кириллица+иконки Tabler, 4bpp сжатые)
    screens/               ~30 экранов (Screen)
include/
  varsys_config.h          тайминги/константы
  lv_conf.h                конфиг LVGL (LV_USE_FONT_COMPRESSED=1 обязателен!)
boards/                    кастомный board JSON (16MB / OPI PSRAM)
flasher/                   веб-прошивальщик (ESP Web Tools) + бинарники
docs/                      эта документация
```

---

## 9. Принятые архитектурные решения

- **Модульность вместо монолита** (в отличие от Bruce) — каждая подсистема
  изолирована, добавление локально.
- **Кооперативное ядро** — простота ценой отзывчивости при блокирующих операциях.
- **Один `SPIClass` (TFT_eSPI) на всю шину** — как в Bruce (после отказа от
  LovyanGFX), ради совместимости CC1101/SD/дисплея.
- **Граница ассистента**: широкополосная глушилка и массовый BLE/Apple-spam —
  намеренно заглушки (раздел «Эксперт»), реализуется только для лаборатории.
