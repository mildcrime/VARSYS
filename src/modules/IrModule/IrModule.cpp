#include "IrModule.h"
#include "core/Logger.h"

static const char* TAG = "Ir";

IrModule* IrModule::_self = nullptr;

// Небольшой набор power-кодов NEC (адрес, команда) для разных ТВ — заготовка
// универсального пульта. Полную базу TV-B-Gone подключим как ресурс позже.
struct TvCode { uint8_t addr; uint8_t cmd; };
static const TvCode TV_OFF_CODES[] = {
    {0x00, 0x02},   // Samsung-подобный (пример)
    {0x04, 0x08},
    {0x40, 0x0C},
    {0x10, 0x12},
};
static const size_t TV_OFF_COUNT = sizeof(TV_OFF_CODES) / sizeof(TV_OFF_CODES[0]);

bool IrModule::init() {
    _self = this;
    _ir.begin();
    LOGI(TAG, "IR module ready");
    return true;
}

size_t IrModule::capture() {
    size_t n = _ir.captureRaw(_last, 4000, 512);   // до 4 с ожидания кадра
    LOGI(TAG, "captured %u pulses", (unsigned)n);
    return n;
}

bool IrModule::replayLast() {
    if (_last.empty()) return false;
    _ir.sendRaw(_last);
    LOGI(TAG, "replayed %u pulses", (unsigned)_last.size());
    return true;
}

void IrModule::sendTvOff() {
    for (size_t i = 0; i < TV_OFF_COUNT; ++i) {
        _ir.sendNEC(TV_OFF_CODES[i].addr, TV_OFF_CODES[i].cmd);
        delay(40);
    }
    LOGI(TAG, "TV-off sweep: %u codes", (unsigned)TV_OFF_COUNT);
}

// ===========================================================================
//  Универсальный пульт — база кодов топ-брендов ТВ по функциям.
//  Коды по широко публикуемым значениям; проверять на конкретном ТВ.
//  Протоколы: NEC (8-бит addr), NECEXT (16-бит addr), SAMSUNG, SONY12.
// ===========================================================================
enum IrProto : uint8_t { P_NEC, P_NECEXT, P_SAMSUNG, P_SONY12 };
struct UCode { uint8_t proto; uint16_t addr; uint8_t cmd; };

// Для каждой функции — несколько брендов (Samsung / LG / Sony).
static const UCode U_POWER[] = {
    {P_SAMSUNG, 0x07, 0x02}, {P_NEC, 0x04, 0x08}, {P_SONY12, 1, 21},
};
static const UCode U_VOLUP[] = {
    {P_SAMSUNG, 0x07, 0x07}, {P_NEC, 0x04, 0x02}, {P_SONY12, 1, 18},
};
static const UCode U_VOLDN[] = {
    {P_SAMSUNG, 0x07, 0x0B}, {P_NEC, 0x04, 0x03}, {P_SONY12, 1, 19},
};
static const UCode U_MUTE[] = {
    {P_SAMSUNG, 0x07, 0x0F}, {P_NEC, 0x04, 0x09}, {P_SONY12, 1, 20},
};
static const UCode U_CHUP[] = {
    {P_SAMSUNG, 0x07, 0x12}, {P_NEC, 0x04, 0x00}, {P_SONY12, 1, 16},
};
static const UCode U_CHDN[] = {
    {P_SAMSUNG, 0x07, 0x10}, {P_NEC, 0x04, 0x01}, {P_SONY12, 1, 17},
};
static const UCode U_SOURCE[] = {
    {P_SAMSUNG, 0x07, 0x01}, {P_NEC, 0x04, 0x0B}, {P_SONY12, 1, 37},
};

struct UTable { const UCode* codes; uint8_t count; const char* name; };
static const UTable U_TABLE[IrModule::UNI_COUNT] = {
    { U_POWER,  3, "Power"  }, { U_VOLUP, 3, "Vol +" }, { U_VOLDN, 3, "Vol -" },
    { U_MUTE,   3, "Mute"   }, { U_CHUP,  3, "Ch +"  }, { U_CHDN,  3, "Ch -"  },
    { U_SOURCE, 3, "Source" },
};

const char* IrModule::uniName(UniFn fn) {
    if (fn >= UNI_COUNT) return "";
    return U_TABLE[fn].name;
}

int IrModule::sendUniversal(UniFn fn) {
    if (fn >= UNI_COUNT) return 0;
    const UTable& t = U_TABLE[fn];
    for (uint8_t i = 0; i < t.count; ++i) {
        const UCode& u = t.codes[i];
        switch (u.proto) {
            case P_NEC:     _ir.sendNEC((uint8_t)u.addr, u.cmd);      break;
            case P_NECEXT:  _ir.sendNECext(u.addr, u.cmd);           break;
            case P_SAMSUNG: _ir.sendSamsung((uint8_t)u.addr, u.cmd); break;
            case P_SONY12:  _ir.sendSony(u.cmd, u.addr, 12);         break;
        }
        delay(60);
    }
    LOGI(TAG, "universal %s: %u codes", t.name, t.count);
    return t.count;
}
