# VARSYS — Key algorithms

🌐 **English** · [Русский](ALGORITHMS.ru.md)

Non-trivial logic explained so you can change it deliberately.

## 1. Encoder: Buxton half-step decoder (`InputModule.cpp`)
Quadrature FSM: `pins=(A<<1)|B`, transition `kTable[state][pins]`, emit on
`state&0x30`. **Half-step** (not full-step) because the T-Embed encoder has
detents on both 00 and 11 (one half-cycle per detent); the full-step table fired
CW every other detent and never CCW. Reverse — `VARSYS_ENCODER_REVERSED`.

## 2. OOK Sub-GHz decoder (`RfDecoder.cpp`)
1) `te` = min pulse (80…1500). 2) sync = longest pulse. 3) PWM decode:
`(te,3te)=0`, `(3te,te)=1`, ±45% tol. 4) classify by bits/te (Princeton/EV1527,
Nice FLO, CAME, Holtek). 5) rolling-code: long uniform preamble → "KeeLoq/rolling"
(key not extracted).

## 3. Fixed-code bruteforce (`RfBrute.h`)
Full-frame sweep over a protocol table; `protoIdx==count` = "All". Blocking,
aborted by Back → auto-saves the candidate window (`/brute/found_*.txt`). Auto-
replay (`replayCandidates`) → confirmed code (`/brute/confirmed_*.sub`).

## 4. RMT capture/replay (`CC1101.cpp`, `InfraRed.cpp`)
RMT (1 tick = 1 µs). Capture — RX via ringbuffer, "mark-first", end = idle gap.
Replay — RMT TX, split >32767 µs, carrier 38 kHz (IR) / 40 kHz (Sony). Channels:
CC1101=2, IR=3 (FastLED=0); driver installed/uninstalled per operation.

## 5. IR encoders (`InfraRed.cpp`)
NEC (9000/4500, 32 bits, 0=560/560, 1=560/1690), NEC-ext (16-bit addr), Samsung
(4500/4500), Sony SIRC (2400/600, 40 kHz, ×3). No RC5 (its Manchester starts with
a space). Universal remote — `U_*` tables in `IrModule.cpp`.

## 6. NFC: dictionary dump (`NfcModule.cpp`)
Mifare 1K: UID → per sector try `DICT[]` keys A/B → read blocks → save
`/nfc/<uid>.dump`. Clone — write data blocks back (block 0 needs a magic card).
Dictionary keys only.

## 7. Mousejack (`NrfModule.cpp`)
`mjScan` — promiscuous (addr width 2, preamble 0x00AA, first 5 bytes = address;
ESB bit-alignment needs on-device tuning). `mjPing` — active channel via ACK.
`mjInject` — Logitech Unifying unencrypted keyboard frame (10 bytes
`00 C1 mod usage*6 cksum` + release).

## 8. Wardrive (`WardriveModule.cpp`)
`activate` → GPS + `/wardrive.csv` (WiGLE header). 4 s timer: WiFi scan, new
BSSIDs (deduped) + GPS → CSV line. `deactivate` → stop + `WIFI_OFF` + release GPS.

## 9. Home carousel (`HomeScreen.cpp`)
`applyFocus`: distance to center → opacity, pick nearest, border, label, LED color.
Instant scroll (`LV_ANIM_OFF`). ⚠️ `transform_zoom` is NOT used (it failed to draw
the center tile).

## 10. Power saving (`PowerModule.cpp`)
`sleepMs=screenTimeoutSec*1000`, `dimMs=2/3`. Off → brightness 0 + CPU 80 MHz;
wake → 240 + brightness. ⚠️ `idle=(now>=last)?now-last:0` (underflow guard).

## `.sub` format (Flipper)
`saveSignal` writes `Filetype: Flipper SubGhz RAW File / Frequency(Hz) / Preset
Ook650Async / RAW_Data (≤512/line, sign = level)`. `loadSignal` is tolerant
(multi-line RAW_Data, start level from first sign). Files are Flipper-compatible.
