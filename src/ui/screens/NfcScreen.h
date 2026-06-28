// ============================================================================
//  NfcScreen.h — Экран NFC/RFID (PN532), ландшафт 320×170
//
//  Слева — UID/тип последней метки. Справа — действия: Читать, Сохранить.
// ============================================================================
#pragma once
#include "ui/Screen.h"

class NfcScreen : public Screen {
public:
    const char* name() const override { return "Nfc"; }
    void onCreate(lv_obj_t* parent) override;
    void onShow() override;
    void onEvent(const Event& e) override;

private:
    static constexpr int kRows = 3;     // Читать, Сохранить, Записать

    lv_obj_t* makeAction(lv_obj_t* parent, int idx, const char* sym,
                         lv_color_t color, const char* label, bool sep);
    void select(int idx, bool on);
    void moveSelection(int delta);
    void activateSelected();

    lv_obj_t* _uid    = nullptr;
    lv_obj_t* _type   = nullptr;
    lv_obj_t* _status = nullptr;
    lv_obj_t* _rows[kRows] = {};
    int       _selected = 0;
};
