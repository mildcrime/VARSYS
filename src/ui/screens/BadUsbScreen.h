// ============================================================================
//  BadUsbScreen.h — BadUSB (USB-HID): раскладка, демо, скрипты /ducky (320×170)
// ============================================================================
#pragma once
#include <vector>
#include "ui/Screen.h"

class BadUsbScreen : public Screen {
public:
    const char* name() const override { return "Badusb"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    enum Kind { K_LAYOUT, K_DEMO, K_SCRIPT };
    struct Item { Kind kind; String name; lv_obj_t* row; lv_obj_t* val; };

    void rebuild();
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _list   = nullptr;
    lv_obj_t* _status = nullptr;
    std::vector<Item> _items;
    int _selected = 0;
};
