# Сборка шрифтов VARSYS

Генерирует LVGL-шрифты `src/ui/fonts/varsys_{12,14,16,22}.c` и заголовок
`varsys_fonts.h` (объявления шрифтов + UTF-8 макросы иконок `ICON_*`).

Состав: **Inter** (OFL) — латиница + кириллица + пунктуация, **Tabler Icons**
(MIT) — иконки интерфейса. Размер 12/16 — Inter Regular, 22 — SemiBold, bpp 4.

## Когда пересобирать
- добавили/сменили иконку (правка словаря `ICONS` в `generate_fonts.py`);
- изменили набор размеров или диапазоны символов (`SIZES`, `TEXT_RANGES`);
- сменили исходный шрифт.

## Восстановление инструментов и сборка

Артефакты (`node_modules/`, `package/`, `inter/`) не хранятся в репозитории —
восстановите их и запустите генератор:

```bash
cd tools/fonts

# 1. конвертер LVGL
npm install lv_font_conv@^1.5.3

# 2. иконочный шрифт Tabler
npm pack @tabler/icons-webfont && tar -xzf tabler-icons-webfont-*.tgz

# 3. шрифт Inter (статические TTF)
url=$(curl -s https://api.github.com/repos/rsms/inter/releases/latest \
  | python3 -c "import sys,json;print([a['browser_download_url'] for a in json.load(sys.stdin)['assets'] if a['name'].endswith('.zip')][0])")
curl -sL "$url" -o inter.zip
unzip -o -j inter.zip "*/ttf/Inter-Regular.ttf" "*/ttf/Inter-SemiBold.ttf" -d ./inter

# 4. генерация
python3 generate_fonts.py
```

Сгенерированные `.c`/`.h` в `src/ui/fonts/` **коммитятся** в репозиторий —
для обычной сборки прошивки инструменты из этой папки не нужны.

## Лицензии
- Inter — SIL Open Font License 1.1
- Tabler Icons — MIT
