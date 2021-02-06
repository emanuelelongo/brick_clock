#include "weather.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "config.h"

static char buffer[1024];

void weather_update(float *temp, float *hum, char *weather) {
    if (strcmp(curConfig.owkey, "") == 0 || strcmp(curConfig.owcity, "") == 0 || strcmp(curConfig.owcountry, "") == 0) {
        return;
    }

    HTTPClient http;
    sprintf(buffer, "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&lang=en&units=metric&APPID=%s", curConfig.owcity, curConfig.owcountry, curConfig.owkey);
    http.begin(buffer);
    http.GET();
    String resp = http.getString();
    http.end();

    StaticJsonDocument<128> filter;
    filter["weather"][0]["description"] = true;
    filter["main"]["temp"] = true;
    filter["main"]["humidity"] = true;

    DynamicJsonDocument doc(300);
    DeserializationError error = deserializeJson(doc, resp, DeserializationOption::Filter(filter));
    if (error) {
        Serial.println(error.f_str());
        return;
    }
    *temp = doc["main"]["temp"];
    *hum = doc["main"]["humidity"];
    strcpy(weather, doc["weather"][0]["description"]);
}