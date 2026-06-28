#include "ModuleManager.h"
#include "Logger.h"
#include <string.h>

static const char* TAG = "ModuleMgr";

void ModuleManager::add(IModule* module) {
    if (!module) return;
    _modules.push_back(module);
    LOGI(TAG, "Registered module: %s", module->name());
}

bool ModuleManager::initAll() {
    bool ok = true;
    for (auto* m : _modules) {
        LOGD(TAG, "Init %s ...", m->name());
        if (!m->init()) {
            LOGE(TAG, "Module '%s' failed to init", m->name());
            m->enabled = false;
            ok = false;
        }
    }
    return ok;
}

void ModuleManager::startAll() {
    for (auto* m : _modules) {
        if (m->enabled) m->start();
    }
}

void ModuleManager::updateAll(uint32_t now) {
    for (auto* m : _modules) {
        if (m->enabled) m->update(now);
    }
}

void ModuleManager::stopAll() {
    for (auto it = _modules.rbegin(); it != _modules.rend(); ++it) {
        (*it)->stop();
    }
}

IModule* ModuleManager::find(const char* name) {
    for (auto* m : _modules) {
        if (strcmp(m->name(), name) == 0) return m;
    }
    return nullptr;
}
