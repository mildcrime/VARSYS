// ============================================================================
//  IrRemoteScreen.h — Универсальный ИК-пульт: функции -> sweep кодов (320×170)
// ============================================================================
#pragma once
#include <vector>
#include "ui/Screen.h"

class IrRemoteScreen : public Screen {
public:
    const char* name() const override { return "IrRemote"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _list   = nullptr;
    lv_obj_t* _status = nullptr;
    std::vector<lv_obj_t*> _rows;
    int _selected = 0;
};
