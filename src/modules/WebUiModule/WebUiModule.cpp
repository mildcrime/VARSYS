#include "WebUiModule.h"
#include "core/Logger.h"
#include "varsys_config.h"
#include "modules/RadioModule/RadioModule.h"
#include "modules/PowerModule/PowerModule.h"
#include "modules/StorageModule/StorageModule.h"
#include <WiFi.h>
#include <FS.h>
#include <Update.h>

static const char* TAG = "WebUi";
static const char* AP_PASS = "varsys1234";

WebUiModule* WebUiModule::_self = nullptr;

bool WebUiModule::init() {
    _self = this;
    return true;   // сервер стартует по запросу с экрана
}

void WebUiModule::start() {
    if (_active) return;

    uint8_t mac[6];
    WiFi.macAddress(mac);
    char ssid[24];
    snprintf(ssid, sizeof(ssid), "VARSYS-%02X%02X", mac[4], mac[5]);
    _ssid = ssid;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(_ssid.c_str(), AP_PASS);
    _ip = WiFi.softAPIP().toString();

    routes();
    _server.begin();
    _active = true;
    LOGI(TAG, "WebUI up: %s  http://%s", _ssid.c_str(), _ip.c_str());
}

void WebUiModule::stop() {
    if (!_active) return;
    _server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);   // гасим радио после остановки (экономия)
    _active = false;
    LOGI(TAG, "WebUI down");
}

void WebUiModule::update(uint32_t) {
    if (_active) _server.handleClient();
}

void WebUiModule::routes() {
    _server.on("/", [this] { handleRoot(); });
    _server.on("/signals", [this] { handleSignals(); });
    _server.on("/dl", [this] { handleDownload(); });

    // OTA-обновление прошивки (Update.h, разделы app0/app1).
    _server.on("/update", HTTP_POST,
        [this] {
            _server.send(200, "text/html",
                         Update.hasError() ? "update failed" : "OK, rebooting");
            delay(400);
            ESP.restart();
        },
        [this] {
            HTTPUpload& up = _server.upload();
            if (up.status == UPLOAD_FILE_START) {
                Update.begin(UPDATE_SIZE_UNKNOWN);
            } else if (up.status == UPLOAD_FILE_WRITE) {
                Update.write(up.buf, up.currentSize);
            } else if (up.status == UPLOAD_FILE_END) {
                Update.end(true);
            }
        });
}

void WebUiModule::handleRoot() {
    String h = F("<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
                 "<style>body{font-family:-apple-system,sans-serif;background:#f2f2f7;margin:0;padding:16px;color:#1c1c1e}"
                 "h1{font-size:22px}.c{background:#fff;border-radius:12px;padding:12px 16px;margin:10px 0}"
                 "a{color:#007aff;text-decoration:none}</style>");
    h += "<h1>VARSYS</h1>";
    h += "<div class=c>Firmware: v" VARSYS_VERSION "</div>";
    h += "<div class=c>Battery: " + String(PowerModule::instance().batteryPercent()) + "%</div>";
    h += "<div class=c>Sub-GHz: " + String(RadioModule::instance().freqKhz() / 1000.0, 2) + " MHz</div>";
    h += "<div class=c><a href='/signals'>Signal library &rsaquo;</a></div>";
    h += "<div class=c><form method=POST action='/update' enctype='multipart/form-data'>"
         "OTA: <input type=file name=f> <input type=submit value='Flash'></form></div>";
    _server.send(200, "text/html", h);
}

void WebUiModule::handleSignals() {
    String h = F("<!doctype html><meta name=viewport content='width=device-width,initial-scale=1'>"
                 "<style>body{font-family:-apple-system,sans-serif;background:#f2f2f7;margin:0;padding:16px}"
                 "a{display:block;background:#fff;border-radius:10px;padding:10px 14px;margin:6px 0;color:#007aff;text-decoration:none}</style>");
    h += "<h2>Signals</h2>";
    auto names = StorageModule::instance().listSignals();
    if (names.empty()) h += "<p>empty</p>";
    for (auto& n : names)
        h += "<a href='/dl?f=" + n + "'>" + n + "</a>";
    h += "<p><a href='/'>&lsaquo; back</a></p>";
    _server.send(200, "text/html", h);
}

void WebUiModule::handleDownload() {
    String f = _server.arg("f");
    if (f.isEmpty()) { _server.send(400, "text/plain", "no file"); return; }
    fs::FS* fs = StorageModule::instance().fs();
    if (!fs) { _server.send(500, "text/plain", "no fs"); return; }

    String path = "/signals/" + f;
    File file = fs->open(path, FILE_READ);
    if (!file) { _server.send(404, "text/plain", "not found"); return; }
    _server.streamFile(file, "application/octet-stream");
    file.close();
}
