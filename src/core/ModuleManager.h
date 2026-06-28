// ============================================================================
//  ModuleManager.h — Реестр и планировщик модулей VARSYS
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>
#include "Module.h"

class ModuleManager {
public:
    // Регистрация модуля. Владение остаётся за вызывающим (обычно Core).
    void add(IModule* module);

    // Инициализация всех модулей по порядку регистрации.
    bool initAll();

    // Уведомление о завершении инициализации (вызов start()).
    void startAll();

    // Обновление всех включённых модулей.
    void updateAll(uint32_t now);

    // Завершение работы всех модулей в обратном порядке.
    void stopAll();

    // Поиск модуля по имени (для межмодульного доступа).
    IModule* find(const char* name);

    size_t count() const { return _modules.size(); }

private:
    std::vector<IModule*> _modules;
};
