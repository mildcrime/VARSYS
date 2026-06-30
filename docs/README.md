# VARSYS — Documentation / Документация

Each developer doc exists in **English** and **Русский** (language switcher at the
top of every page). Каждый документ есть на двух языках.

## Developer docs / Для разработчика
| Topic / Тема | EN | RU |
|---|---|---|
| Architecture / Архитектура | [ARCHITECTURE.md](ARCHITECTURE.md) | [ARCHITECTURE.ru.md](ARCHITECTURE.ru.md) |
| Modules / Модули | [MODULES.md](MODULES.md) | [MODULES.ru.md](MODULES.ru.md) |
| UI framework / UI-фреймворк | [UI.md](UI.md) | [UI.ru.md](UI.ru.md) |
| Algorithms / Алгоритмы | [ALGORITHMS.md](ALGORITHMS.md) | [ALGORITHMS.ru.md](ALGORITHMS.ru.md) |
| Hardware / Железо | [HARDWARE.md](HARDWARE.md) | [HARDWARE.ru.md](HARDWARE.ru.md) |
| Extending / Доработки | [EXTENDING.md](EXTENDING.md) | [EXTENDING.ru.md](EXTENDING.ru.md) |

**Start here / Начни отсюда:** Architecture → Hardware → Extending.

## User docs / Для пользователя
[USAGE_EN.md](USAGE_EN.md) · [USAGE_RU.md](USAGE_RU.md)

## Other / Прочее
[../BACKLOG.md](../BACKLOG.md) · [../README.md](../README.md) ·
[../README.ru.md](../README.ru.md) · [../DISCLAIMER.md](../DISCLAIMER.md)

## Quick start (dev) / Быстрый старт
1. Read Architecture + Hardware (execution model, shared resources, gotchas).
2. Build: `pio run -e t-embed-cc1101`.
3. Flash: download mode = **press the encoder wheel + RST**, then `-t upload`.
4. Extend: see Extending recipes.

UI mockups / макеты экранов: [UI.md](UI.md#screens-mockups).
