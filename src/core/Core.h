// ============================================================================
//  Core.h — Ядро прошивки VARSYS
//
//  Точка сборки системы: инициализирует логгер, шину событий, регистрирует
//  модули и крутит главный цикл. Реализован как синглтон.
// ============================================================================
#pragma once
#include <Arduino.h>
#include "ModuleManager.h"

class Core {
public:
    static Core& instance();

    // Инициализация ядра и всех модулей. Вызывается из setup().
    void begin();

    // Один шаг главного цикла. Вызывается из loop().
    void loop();

    ModuleManager& modules() { return _modules; }

private:
    Core() = default;
    Core(const Core&) = delete;
    Core& operator=(const Core&) = delete;

    // Регистрация всех модулей системы (определена в Core.cpp).
    void registerModules();

    ModuleManager _modules;
    uint32_t      _lastTick = 0;
    bool          _booted   = false;
};
