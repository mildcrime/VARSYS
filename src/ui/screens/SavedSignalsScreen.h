// ============================================================================
//  SavedSignalsScreen.h — Библиотека записанных сигналов (из Storage)
//
//  Список файлов сигналов; выбор энкодером, нажатие — загрузить и
//  воспроизвести. Список перечитывается при каждом показе.
// ============================================================================
#pragma once
#include <vector>
#include <Arduino.h>
#include "ui/Screen.h"

class SavedSignalsScreen : public Screen {
public:
    const char* name() const override { return "Saved"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    void rebuild();
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _list  = nullptr;
    lv_obj_t* _empty = nullptr;
    std::vector<lv_obj_t*> _rows;
    std::vector<String>    _names;
    int _selected = 0;
};
