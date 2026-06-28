// ============================================================================
//  SubGhzScreen.h — Экран Sub-GHz (CC1101), ландшафт 320×170
//
//  Слева — частота, живой RSSI и статус. Справа — список действий (частота,
//  скан, запись, воспроизведение), управляемый энкодером. RSSI обновляется
//  по таймеру, пока экран активен.
// ============================================================================
#pragma once
#include "ui/Screen.h"

class SubGhzScreen : public Screen {
public:
    const char* name() const override { return "SubGhz"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 8;     // +Спектр, +Перебор

    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();
    void refreshFreq();
    void refreshRssi();

    lv_obj_t* _freq   = nullptr;
    lv_obj_t* _bar    = nullptr;
    lv_obj_t* _dbm    = nullptr;
    lv_obj_t* _status = nullptr;
    lv_obj_t* _rows[kRows] = {};
    int       _selected = 0;
    uint32_t  _rssiTask = 0;
};
