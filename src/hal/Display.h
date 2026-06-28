// ============================================================================
//  Display.h — HAL дисплея ST7789 на базе TFT_eSPI
//
//  Используем TFT_eSPI (Arduino SPI), как в Bruce: его SPIClass (HSPI) делится
//  с CC1101 — один объект шины, разные CS. Подсветка управляется через LEDC
//  (TFT_eSPI её яркость не регулирует).
// ============================================================================
#pragma once
#include <TFT_eSPI.h>
#include <SPI.h>
#include "board_pins.h"

class Display {
public:
    bool begin();
    void setBrightness(uint8_t value);
    void setRotation(uint8_t r) { _tft.setRotation(r); }

    int width()  { return _tft.width(); }
    int height() { return _tft.height(); }

    TFT_eSPI&  tft() { return _tft; }
    // Общий объект шины SPI — передаётся драйверу CC1101 (см. RadioModule).
    SPIClass&  spi() { return _tft.getSPIinstance(); }

private:
    TFT_eSPI _tft;
};
