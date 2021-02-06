#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "config.h"
#include "distance_sensor.h"
#include "internet_time.h"
#include "rgb_led.h"
#include "screen.h"
#include "sdcard.h"
#include "speaker.h"
#include "temp_sensor.h"
#include "weather.h"
#include "web.h"
#include "wifi_connection.h"
#include "data.h"
#include "common.h"

// Name of the WiFi Access Point that will be created for the configuration phase
#define AP_SSID "BrickClock"

// Application State
static tm now;                           // current time
static long nowMillis;                   // used to store milliseconds from boot
static float outTemp = .0F;              // outdoor temperature
static float indTemp = .0F;              // indoor temperature
static float outHum = .0F;               // outdoor humidity
static float indHum = .0F;               // indoor humidity
static float indPress = .0F;             // atmospheric pressure
static int pressureHistory[16];          // latest 16 pressure values
static char weather[32];                 // weather description
static bool altTemp = false;             // toggle between indoor and outdoor view
static long timers[] = {0l, 0l, 0l};     // timers used to check stuff periodically
static bool alarmStoppedByUser = false;  // true if user already stopped the current alarm

void setup() {
    Serial.begin(115200);

    sdcard_setup() || stop();
    config_load() || stop();
    temp_sensor_setup() || stop();
    distance_sensor_setup();
    rgb_led_setup();
    screen_setup();

    if (wifi_connection_start(AP_SSID)) {
        screen_renderIPInfo(WiFi.localIP());
        internet_time_setup();
        getLocalTime(&now);
        weather_update(&outTemp, &outHum, weather);
        temp_sensor_readData(&indTemp, &indPress, &indHum);
    } else {
        // if the connection to WiFi failed, an Access Point has been started.
        screen_showAPInfo(AP_SSID, WiFi.softAPIP());
        web_setup();
        stop();
    }

    web_setup();
    data_setup();
    screen_renderAlarmInfo();
    screen_renderTime(&now);
    screen_renderIndoorData(round(indTemp), round(indHum));
    screen_renderWeather(weather);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        return;
    }
    // get current millis (for timers)
    nowMillis = millis();

    // set LED color
    rgb_led_update(now.tm_hour);

    // refresh current time
    getLocalTime(&now);
    screen_renderTime(&now);

    // things done every 5 seconds
    if (nowMillis - timers[0] > 5000) {
        altTemp = !altTemp;
        timers[0] = nowMillis;
        if (altTemp) {
            screen_renderOutdoorData(outTemp, outHum);
        } else {
            screen_renderIndoorData(indTemp, indHum);
        }
    }

    // things done every 1 minutes
    if (nowMillis - timers[1] > 60000) {
        temp_sensor_readData(&indTemp, &indPress, &indHum);
        screen_renderPressure(outTemp, outHum, indPress);
        data_collect(now, indPress, outTemp, indTemp, outHum, indHum);
        screen_renderWeather(weather);
        screen_renderAlarmInfo();
        timers[1] = nowMillis;
    }

    // things done every 30 minutes
    if (nowMillis - timers[2] > 1800000) {
        internet_time_setup();
        weather_update(&outTemp, &  outHum, weather);
        timers[2] = nowMillis;
    }

    // check if alarm has to start playing
    if (curConfig.enabled) {
        if (now.tm_hour == curConfig.hour && now.tm_min == curConfig.minute) {
            if (!speaker_isPlaying()) {
                speaker_play(true);
            }
        } else {
            speaker_resetalarm();
        }
    }

    // check for user interaction:
    // detecting object in front of the clock will stop the alarm if it is playing
    // and will swtich clock pages otherwise
    if (distance_sensor_getDistance() <= 10) {
        if (speaker_isPlaying()) {
            speaker_stop();
        }
        else {
            screen_nextPage();
            // force re-render:
            timers[0] = -99999999;
            timers[1] = -99999999;
        }
    }
    delay(1000);
}

bool stop() {
    while(true) delay(1000);
}