// ============================================================================
//  BootScreen.h — Загрузочный экран (сплеш) VARSYS
// ============================================================================
#pragma once
#include "ui/Screen.h"

class BootScreen : public Screen {
public:
    const char* name() const override { return "Boot"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onUpdate(uint32_t now) override;

private:
    lv_obj_t* _bar = nullptr;
    uint32_t  _shownAt = 0;
};
