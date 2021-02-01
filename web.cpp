#include "web.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SD.h>

#include "config.h"
#include "speaker.h"
#include "screen.h"
#include "data.h"
#include "common.h"

static char buffer[4096];
static AsyncWebServer server(80);

void handle_webpage(AsyncWebServerRequest *request) {
    request->send(SD, "/public/index.html", "text/html");
}

void handle_getConfig(AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(1024);
    doc["enabled"] = curConfig.enabled;
    doc["h"] = curConfig.hour;
    doc["m"] = curConfig.minute;
    doc["mon"] = curConfig.mon;
    doc["tue"] = curConfig.tue;
    doc["wed"] = curConfig.wed;
    doc["thu"] = curConfig.thu;
    doc["fri"] = curConfig.fri;
    doc["sat"] = curConfig.sat;
    doc["sun"] = curConfig.sun;
    doc["ssid"] = curConfig.ssid;
    doc["password"] = curConfig.password;
    doc["owkey"] = curConfig.owkey;
    doc["owcity"] = curConfig.owcity;
    doc["owcountry"] = curConfig.owcountry;

    JsonArray led = doc.createNestedArray("led");
    for (int i = 0; i < 24; i++) {
        led.add(curConfig.led[i]);
    }
    serializeJson(doc, buffer);
    request->send(200, "application/json", buffer);
}

void handle_saveConfig(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
        Serial.print(error.f_str());
        return;
    }

    curConfig.enabled = (bool)doc["enabled"];
    curConfig.hour = (uint8_t)doc["h"];
    curConfig.minute = (uint8_t)doc["m"];
    curConfig.mon = (bool)doc["mon"];
    curConfig.tue = (bool)doc["tue"];
    curConfig.wed = (bool)doc["wed"];
    curConfig.thu = (bool)doc["thu"];
    curConfig.fri = (bool)doc["fri"];
    curConfig.sat = (bool)doc["sat"];
    curConfig.sun = (bool)doc["sun"];
    // ToDo: make all these strcpy safe
    strcpy(curConfig.ssid, doc["ssid"]);
    strcpy(curConfig.password, doc["password"]);
    strcpy(curConfig.owkey, doc["owkey"]);
    strcpy(curConfig.owcity, doc["owcity"]);
    strcpy(curConfig.owcountry, doc["owcountry"]);
    JsonArray led = doc["led"];
    for (int i = 0; i < 24; i++) {
        strcpy(curConfig.led[i], led[i]);
    }
    config_save();
    screen_renderAlarmInfo();

    request->send(200);
}

void handle_play(AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{}");
    speaker_play(false);
}

void handle_stop(AsyncWebServerRequest *request) {
    speaker_stop();
    request->send(200, "application/json", "{}");
}

void handle_getData(AsyncWebServerRequest *request) {
    // TODO: try to reduce the payload size
    DynamicJsonDocument doc(3072);
    int pageIndex = 0;
    if(request->params() == 1) {
        AsyncWebParameter* p = request->getParam(0);
        if(strcmp(p->name().c_str(), "n") == 0) {
            pageIndex = atoi(p->value().c_str()); // the zero on error is acceptable
        }
    }
    DEBUG_PRINTF("Requested data (page: %d)\n", pageIndex);

    Sample* page = data_getPage(pageIndex);

    if(page == NULL) {
        request->send(404, "application/json");
        return;
    }

    for(int i=0; i<16; i++) {
        if(page[i].indTempMax == INT8_MIN) {
            continue;
        }
        JsonObject item = doc.createNestedObject();
        item["timestamp"] = mktime(&page[i].time);
        item["pressureMin"] = page[i].pressureMin;
        item["pressureMax"] = page[i].pressureMax;
        item["outTempMin"] = page[i].outTempMin;
        item["outTempMax"] = page[i].outTempMax;
        item["indTempMin"] = page[i].indTempMin;
        item["indTempMax"] = page[i].indTempMax;
        item["outHumMin"] = page[i].outHumMin;
        item["outHumMax"] = page[i].outHumMax;
        item["indHumMin"] = page[i].indHumMin;
        item["indHumMax"] = page[i].indHumMax;
    }

    serializeJson(doc, buffer);
    request->send(200, "application/json", buffer);
}

void handle_eraseData(AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{}");
    data_erase();
    esp_restart();
}

void handle_factoryReset(AsyncWebServerRequest *request) {
    data_erase();
    config_erase();
    request->send(200, "application/json", "{}");
    esp_restart();
}

void handle_restart(AsyncWebServerRequest *request) {
    request->send(200);
    esp_restart();
}

void web_setup() {
    DEBUG_PRINTLN("web_setup")
    server.serveStatic("/", SD, "/public");
    server.on("/config", HTTP_GET, handle_getConfig);
    server.on(
        "/config",
        HTTP_POST, [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {},
        handle_saveConfig);
    server.on("/play", HTTP_POST, handle_play);
    server.on("/stop", HTTP_POST, handle_stop);
    server.on("/data", HTTP_GET, handle_getData);
    server.on("/erase", HTTP_POST, handle_eraseData);
    server.on("/restart", HTTP_POST, handle_restart);
    server.on("/factoryreset", HTTP_POST, handle_factoryReset);

    server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });
    server.begin();
    DEBUG_PRINTLN("web_setup complete")
}