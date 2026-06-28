#include "RfDecoder.h"

static inline bool nearv(uint16_t d, uint32_t ref, int tolPct = 45) {
    uint32_t tol = ref * tolPct / 100;
    return (d + tol >= ref) && (d <= ref + tol);
}

RfDecoded rfDecode(const std::vector<uint16_t>& p) {
    RfDecoded r;
    const int n = (int)p.size();
    if (n < 16) return r;

    // 1) Базовая длительность te — минимальный значимый импульс.
    uint16_t te = 5000;
    for (uint16_t d : p) if (d > 80 && d < te) te = d;
    if (te < 80 || te > 1500) return r;
    r.te = te;

    // 2) Синхро — самый длинный импульс (длинная пауза между кадрами).
    int sync = 0; uint16_t mx = 0;
    for (int i = 0; i < n; ++i) if (p[i] > mx) { mx = p[i]; sync = i; }

    // 3) Декод PWM-битов после синхро: пара (te,3te)=0, (3te,te)=1.
    uint64_t val = 0;
    int bits = 0;
    for (int i = sync + 1; i + 1 < n && bits < 64; i += 2) {
        uint16_t a = p[i], b = p[i + 1];
        int bit;
        if (nearv(a, te) && nearv(b, te * 3))      bit = 0;
        else if (nearv(a, te * 3) && nearv(b, te)) bit = 1;
        else break;                                  // конец кадра
        val = (val << 1) | bit;
        bits++;
    }
    if (bits < 8) {
        // Фикс-PWM не распознан — пробуем rolling-code (KeeLoq/HCS и аналоги)
        // по характерной длинной однородной преамбуле из равных коротких
        // импульсов, заканчивающейся длинной паузой. Ключ не извлекаем
        // (rolling-код меняется), но идентифицируем семейство.
        int lead = 0;
        for (int i = 0; i < n; ++i) {
            if (p[i] > (uint32_t)te * 4) break;      // длинный gap = конец преамбулы
            if (nearv(p[i], te)) lead++; else break;
        }
        if (lead >= 20 && te >= 200 && te <= 800) {
            r.ok = true; r.proto = "KeeLoq/rolling"; r.bits = 0; r.key = 0;
        }
        return r;
    }

    // 4) Классификация по числу бит и te.
    r.ok = true; r.bits = bits; r.key = val;
    if (bits >= 23 && bits <= 25)                 r.proto = "Princeton/EV1527";
    else if (bits == 12 && te >= 550)             r.proto = "Nice FLO";
    else if (bits == 12)                          r.proto = "CAME";
    else if (bits >= 8 && bits <= 10)             r.proto = "Holtek";
    else                                          r.proto = "OOK";
    return r;
}
