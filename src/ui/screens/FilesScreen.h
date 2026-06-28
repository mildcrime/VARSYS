// ============================================================================
//  FilesScreen.h — Браузер файлов хранилища (SD / LittleFS)
//
//  Показывает сохранённые файлы из основных каталогов (сигналы, перебор, NFC,
//  iButton, Ducky). Только просмотр; список перечитывается при показе.
// ============================================================================
#pragma once
#include <vector>
#include <Arduino.h>
#include "ui/Screen.h"

class FilesScreen : public Screen {
public:
    const char* name() const override { return "Files"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    void rebuild();
    void select(int idx, bool on);
    void moveSelection(int delta);

    lv_obj_t* _list  = nullptr;
    lv_obj_t* _empty = nullptr;
    std::vector<lv_obj_t*> _rows;
    int _count = 0;
    int _selected = 0;
};
