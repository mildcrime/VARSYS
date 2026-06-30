# VARSYS — Ключевые алгоритмы

🌐 [English](ALGORITHMS.md) · **Русский**

Разбор нетривиальной логики, чтобы менять её осознанно.

## 1. Энкодер: полушаговый декодер Буxтона (`InputModule.cpp`)
Квадратурный КА: `pins=(A<<1)|B`, переход `kTable[state][pins]`, эмит при
`state&0x30`. **Полушаговая** (не полношаговая), т.к. у T-Embed детенты на 00 и 11
(полуцикл на детент); полношаговая давала CW «через раз» и не давала CCW. Реверс —
`VARSYS_ENCODER_REVERSED`.

## 2. OOK-декодер Sub-GHz (`RfDecoder.cpp`)
1) `te` = мин. импульс (80…1500). 2) синхро = самый длинный импульс. 3) PWM-декод:
`(te,3te)=0`, `(3te,te)=1`, tol ±45%. 4) классификация по бит/te (Princeton/EV1527,
Nice FLO, CAME, Holtek). 5) rolling-code: длинная однородная преамбула → «KeeLoq/
rolling» (ключ не извлекается).

## 3. Перебор фикс-кодов (`RfBrute.h`)
Полнокадровый перебор по таблице протоколов; `protoIdx==count` = «All». Блокирующий,
прерывается «назад» → автосейв окна кандидатов (`/brute/found_*.txt`). Автопрогон
(`replayCandidates`) → подтверждённый код (`/brute/confirmed_*.sub`).

## 4. RMT захват/воспроизведение (`CC1101.cpp`, `InfraRed.cpp`)
RMT (1 тик=1 мкс). Захват — RX через ringbuffer, «mark-first», конец = idle-пауза.
Воспроизведение — RMT TX, дробление >32767 мкс, несущая 38кГц(ИК)/40кГц(Sony).
Каналы: CC1101=2, IR=3 (FastLED=0); драйвер ставится/снимается на операцию.

## 5. ИК-кодировщики (`InfraRed.cpp`)
NEC (9000/4500, 32 бита, 0=560/560,1=560/1690), NEC-ext (16-бит addr), Samsung
(4500/4500), Sony SIRC (2400/600, 40кГц, ×3). RC5 нет (Manchester со старта-space).
Унив. пульт — таблицы `U_*` в `IrModule.cpp`.

## 6. NFC: дамп со словарём (`NfcModule.cpp`)
Mifare 1K: UID → по секторам перебор `DICT[]` ключей A/B → чтение блоков → сохранение
`/nfc/<uid>.dump`. Клон — запись дата-блоков назад (блок 0 — magic). Только
словарные ключи.

## 7. Mousejack (`NrfModule.cpp`)
`mjScan` — promiscuous (addr width 2, преамбула 0x00AA, первые 5 байт=адрес;
подстройка ESB на железе). `mjPing` — активный канал по ACK. `mjInject` — кадр
Logitech Unifying unencrypted (10 байт `00 C1 mod usage*6 cksum` + отпускание).

## 8. Wardrive (`WardriveModule.cpp`)
`activate` → GPS + `/wardrive.csv` (WiGLE-заголовок). Таймер 4с: WiFi-скан, новые
BSSID (дедуп) + GPS → строка CSV. `deactivate` → стоп + `WIFI_OFF` + release GPS.

## 9. Карусель Home (`HomeScreen.cpp`)
`applyFocus`: расстояние до центра → прозрачность, выбор ближайшей, рамка, подпись,
LED-цвет. Прокрутка мгновенная (`LV_ANIM_OFF`). ⚠️ `transform_zoom` не используется
(не рисовал центр).

## 10. Энергосбережение (`PowerModule.cpp`)
`sleepMs=screenTimeoutSec*1000`, `dimMs=2/3`. Гашение → яркость 0 + CPU 80МГц;
пробуждение → 240 + яркость. ⚠️ `idle=(now>=last)?now-last:0` (защита от underflow).

## Формат `.sub` (Flipper)
`saveSignal` пишет `Filetype: Flipper SubGhz RAW File / Frequency(Hz) / Preset
Ook650Async / RAW_Data (≤512/строка, знак=уровень)`. `loadSignal` терпим
(многострочный RAW_Data, стартовый уровень по знаку). Файлы совместимы с Flipper.
