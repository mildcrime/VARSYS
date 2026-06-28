// ============================================================================
//  ExpertScreen.h — Раздел «Эксперт» (ландшафт 320×170)
//
//  Содержит неизбирательные инструменты (широкополосная глушилка, массовый
//  BLE-spam). Виден только при включённом режиме эксперта (Settings::expert).
//  Каркас, предупреждение и гейтинг реализованы; сама атакующая нагрузка —
//  намеренная точка расширения (стаб), заполняется командой под лабораторные
//  условия (Фарадей, подтверждение оператора, региональные ограничения).
// ============================================================================
#pragma once
#include "ui/Screen.h"

class ExpertScreen : public Screen {
public:
    const char* name() const override { return "Expert"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 4;     // Глушилка, BLE-spam, Captive-портал, Dev tools

    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _rows[kRows] = {};
    lv_obj_t* _status = nullptr;
    int       _selected = 0;
};
