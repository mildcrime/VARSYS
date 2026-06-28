// ============================================================================
//  WifiScreen.h — WiFi: список AP + деаутентификация (ландшафт 320×170)
//
//  При входе сканирует эфир. Энкодер выбирает AP, нажатие — старт/стоп deauth
//  по выбранной точке (для авторизованного тестирования).
// ============================================================================
#pragma once
#include <vector>
#include "ui/Screen.h"

class WifiScreen : public Screen {
public:
    const char* name() const override { return "Wifi"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onHide() override;
    void onEvent(const Event& e) override;

private:
    void rebuild();
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _list   = nullptr;
    lv_obj_t* _status = nullptr;
    std::vector<lv_obj_t*> _rows;
    int _selected = 0;
    uint32_t _task = 0;
};
