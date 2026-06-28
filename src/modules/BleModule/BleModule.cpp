#include "BleModule.h"
#include "core/Logger.h"
#include <NimBLEDevice.h>

static const char* TAG = "Ble";

BleModule* BleModule::_self = nullptr;

bool BleModule::init() {
    _self = this;
    NimBLEDevice::init("");
    _ready = true;
    LOGI(TAG, "BLE ready");
    return true;
}

int BleModule::scan(uint32_t seconds) {
    if (!_ready) return 0;
    _devs.clear();

    NimBLEScan* scan = NimBLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setInterval(100);
    scan->setWindow(99);
    NimBLEScanResults res = scan->start(seconds, false);

    for (int i = 0; i < res.getCount(); ++i) {
        NimBLEAdvertisedDevice d = res.getDevice(i);
        BleDev dev;
        dev.name = d.getName().c_str();
        if (dev.name.isEmpty()) dev.name = "<no name>";
        dev.addr = d.getAddress().toString().c_str();
        dev.rssi = d.getRSSI();
        _devs.push_back(dev);
    }
    scan->clearResults();
    LOGI(TAG, "scan: %d devices", (int)_devs.size());
    return (int)_devs.size();
}
