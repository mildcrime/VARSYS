# VARSYS — UI-фреймворк

UI на **LVGL 8.3**, ландшафт **320×170** (ST7789, rotation 3). Стиль — iOS-подобный
(спрингборд из плиток). Шрифты собственные (Inter + кириллица + иконки Tabler).

---

## 1. Screen (`ui/Screen.h`)
Базовый класс экрана. Каждый экран владеет своим корневым `lv_obj_t* _root`.
```cpp
const char* name();              // имя для навигации (setScreen/pushScreen)
void onCreate(lv_obj_t* parent); // построить виджеты (root уже создан UIManager)
void onShow();                   // стал активным
void onHide();                   // ушёл в фон
void onUpdate(uint32_t now);     // периодика (если нужна)
void onEvent(const Event& e);    // ввод (INPUT_ENCODER_*, BTN_CLICK…)
```
⚠️ `onCreate` вызывается при **сборке** и при **пересборке** (`retranslateAll`).
Лениво созданные дочерние объекты (напр. `_hwPanel`) обнуляйте в начале
`onCreate` — старый объект уже удалён `lv_obj_clean`, иначе висячий указатель.

## 2. UIManager (`ui/UIManager.*`)
Модуль (`IModule`), он же владелец LVGL.
- `init()`: `Display.begin()` → `lv_init()` → draw buffers (`heap_caps_malloc`,
  `kBufLines` строк) → `lv_disp_drv` с `flushCb` → статус-бар/Splash. **Экраны не
  строит** (см. ARCHITECTURE §4).
- `start()`: `buildScreens()` (все `addScreen(new XScreen())`) → `setScreen("Home")`
  → `Splash::play(2000)` + подписки на ввод/назад/`LANG_CHANGED`/`UI_REBUILD`.
- `update()`: `lv_timer_handler()` раз в `VARSYS_LVGL_TICK_MS` + `active->onUpdate`.
- **Навигация**: `setScreen(name)` (верхний уровень, сброс стека),
  `pushScreen(name)` (в стек, для «назад»), `popScreen()`. Кнопка «назад» и
  долгое нажатие → `popScreen()` централизованно.
- `retranslateAll()`: при смене языка/темы — `lv_obj_clean(root)+onCreate` всем,
  затем перезагрузка активного.
- `flushCb`: пушит буфер LVGL в TFT под `hal::SpiBusGuard` (`setAddrWindow` +
  `pushColors(swap=true)`).

## 3. UITheme (`ui/UITheme.*`)
Палитра и фабрики. Цвета — функции, ветвящиеся по `ui::darkMode`
(устанавливается из Settings):
```cpp
cBg() cCard() cText() cText2() cSep() cBlue() cGreen() cOrange() cRed()
cGray() cTint()
styleScreen(root)  header(parent,title)  statusBar(...)  card(parent)
hwMissingPanel(parent, "PN532 NFC", "I2C / QWIIC")  // «нет железа»
```
Тёмная тема — по умолчанию. Светлый фон ≈ белый (0xF2F2F7) — на нём UI выглядит
«пустовато», что путало при отладке.

## 4. Локализация (`ui/i18n.*`)
```cpp
enum StrId { STR_MENU, STR_FILES, … , STR_COUNT };  // i18n.h
const char* tr(StrId);                               // текущий язык
```
- Две позиционные таблицы `RU[]` / `EN[]` в `i18n.cpp` — **порядок строго = enum**.
- Смена языка → `LANG_CHANGED` → `UIManager::retranslateAll()`.
- По умолчанию **English**. Имена собственные (Sub-GHz, Bluetooth) не переводятся.

## 5. Шрифты (`ui/fonts/`)
`varsys_12/14/16/22` (Inter + кириллица 0x400-0x45F + иконки Tabler 0xEA…0xF0).
- Объявлены `LV_FONT_DECLARE` в `varsys_fonts.h`; иконки — макросы
  `ICON_ANTENNA`, `ICON_WIFI`, … (UTF-8 кодпоинты Tabler).
- ⚠️ Шрифты **сжатые** (`bitmap_format=1`) → в `include/lv_conf.h` обязателен
  `LV_USE_FONT_COMPRESSED 1`, иначе текст/иконки не рисуются вообще.
- Генератор: `tools/fonts/generate_fonts.py` (lv_font_conv).

## 6. Оверлеи и уведомления
- **Splash** (`ui/Splash.*`) — анимированный логотип VARSYS на `lv_layer_top`;
  `Splash::create()` (в UI init), `Splash::play(ms)` (boot). На `lv_layer_top`,
  поэтому всегда поверх (фикс бага «логотип под иконками»).
- **StatusOverlay** (`ui/StatusOverlay.*`) — статус-бар (время + заряд) на
  `lv_layer_top`, обновляется раз в секунду из UIManager.
- **Notify** (`ui/Notify.*`) — тосты: `Notify::toast(text, Info|Success|Warn|Error)`,
  авто-скрытие через Scheduler, на `lv_layer_top`.

---

## 7. Каталог экранов (`ui/screens/`)

Регистрируются в `UIManager::buildScreens()`. Открываются по `name()`.

| Экран | name | Назначение |
|---|---|---|
| HomeScreen | Home | Карусель плиток (спрингборд), подсветка LED цветом плитки |
| SubGhzScreen | SubGhz | CC1101: частота, RSSI, скан, запись/повтор, сохранение, спектр, brute |
| SpectrumScreen | Spectrum | Водопад/спектр RSSI (lv_chart) |
| SavedSignalsScreen | SavedSignals | Библиотека `.sub`: выбрать → загрузить/воспроизвести |
| BruteScreen | Brute | Перебор фикс-кодов (протокол/All, автопрогон кандидатов) |
| IrScreen | Ir | ИК: захват/повтор/ТВ-выкл/Universal |
| IrRemoteScreen | IrRemote | Универсальный пульт (функции → sweep кодов) |
| NfcScreen | Nfc | NFC: Read/Dump/Clone/Write |
| WifiScreen | Wifi | Список AP, deauth, счётчик handshakes |
| BleScreen | Ble | Список BLE-устройств |
| BadUsbScreen | Badusb | Раскладка US/DE, демо, запуск `/ducky/*.txt` |
| MousejackScreen | Mousejack | NRF24: скан адресов → инъекция |
| NrfScreen | Nrf | NRF24 анализатор каналов 2.4 ГГц |
| GpsScreen | Gps | Фикс/спутники/координаты |
| WardriveScreen | Wardrive | Старт/стоп лог WiFi+GPS |
| IButtonScreen | IButton | Чтение ROM iButton |
| FmScreen | Fm | FM-передатчик |
| WebUiScreen | WebUi | Старт/стоп веб-интерфейса |
| FilesScreen | Files | Браузер файлов SD/LittleFS |
| BatteryScreen | Battery | Заряд/напряжение/ток |
| SettingsScreen | Settings | Звук/вибро/тема/LED/эксперт/яркость/таймаут/язык/батарея |
| ExpertScreen | Expert | Гейт неизбирательных инструментов (глушилка/spam — заглушки) |
| PortalScreen | Portal | Captive-портал |
| DevToolsScreen | DevTools | System/I2C-скан/CC1101 diag/reset |

**HomeScreen** — горизонтальная карусель: плитки 72px, фокус в центре,
прокрутка мгновенная (`LV_ANIM_OFF` — без тиринга), focus через прозрачность
(не `transform_zoom` — он не рисовал центральную плитку). Каждая плитка хранит
цвет → `LedModule::setColor` при смене фокуса.

См. рецепты в [EXTENDING.md](EXTENDING.md).
