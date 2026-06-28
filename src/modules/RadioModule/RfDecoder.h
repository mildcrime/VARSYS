// ============================================================================
//  RfDecoder.h — Декодер OOK-протоколов sub-GHz (как в Bruce)
//
//  По захваченным длительностям импульсов распознаёт распространённые
//  фиксированные протоколы (Princeton/PT2262/EV1527, CAME, Nice FLO, Holtek)
//  и извлекает количество бит и ключ. Чистая логика, без обращения к железу.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>

struct RfDecoded {
    bool        ok    = false;
    const char* proto = "RAW";
    int         bits  = 0;
    uint64_t    key   = 0;
    uint16_t    te    = 0;     // базовая длительность (мкс)
};

RfDecoded rfDecode(const std::vector<uint16_t>& pulses);
