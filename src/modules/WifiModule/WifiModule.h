// ============================================================================
//  WifiModule.h — WiFi-сюита VARSYS (для авторизованного тестирования)
//
//  Скан точек доступа, снифер (promiscuous, счётчики), деаутентификация
//  выбранной AP (esp_wifi_80211_tx). На некоторых версиях ESP-IDF передача
//  deauth требует патча SDK (как в Bruce) — см. примечание в .cpp.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <vector>
#include "core/Module.h"

struct ApInfo {
    String   ssid;
    uint8_t  bssid[6];
    int32_t  rssi;
    uint8_t  channel;
    bool     locked;
};

class WifiModule : public IModule {
public:
    const char* name() const override { return "Wifi"; }
    bool init() override;
    void update(uint32_t now) override;

    static WifiModule& instance() { return *_self; }

    int scan();                              // блокирующий, заполняет _aps
    const std::vector<ApInfo>& aps() const { return _aps; }

    void startDeauth(int apIndex);
    void stopDeauth();
    bool deauthing() const { return _deauthActive; }

    // Снифер 802.11 (promiscuous): счётчик кадров.
    void startSniffer();
    void stopSniffer();
    bool sniffing() const { return _sniffActive; }
    uint32_t sniffCount() const;

    // Захват рукопожатий: при деауте детектируются кадры EAPOL (4-way / PMKID).
    uint32_t handshakeCount() const;

private:
    static WifiModule* _self;
    std::vector<ApInfo> _aps;

    bool     _deauthActive = false;
    int      _deauthIdx    = -1;
    uint32_t _lastTx       = 0;

    bool     _sniffActive  = false;
};
