#include "wifi_connection.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "common.h"

bool wifi_connection_start(const char *ap_ssid) {
    wl_status_t status = WL_DISCONNECTED;
    if (strcmp(curConfig.ssid, "") != 0) {
        DEBUG_PRINTF("Connecting to %s \n", curConfig.ssid);
        WiFi.begin(curConfig.ssid, curConfig.password);
        for (int retry = 0; retry < 20; retry++) {
            DEBUG_PRINT(".");
            status = WiFi.status();
            if (status == WL_CONNECTED || status == WL_CONNECT_FAILED)
                break;
            delay(500);
        }
    }
    delay(100);
    if (status == WL_CONNECTED) {
        DEBUG_PRINTLN(" CONNECTED");
        return true;
    }
    WiFi.softAP(ap_ssid);
    return false;
}