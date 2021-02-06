#include "config.h"

#include <Arduino.h>
#include <FS.h>
#include <SD.h>

config curConfig;

bool config_load() {

    if (!SD.exists("/config.dat")) {
        Serial.println("config.dat not found. Loading default values");
        for (int i = 0; i < 24; i++) {
            strcpy(curConfig.led[i], "#0000000");
        }
        return true;
    }

    File file = SD.open("/config.dat", FILE_READ);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
    }
    file.read((byte *)&curConfig, sizeof(config));
    file.close();
    return true;
}

bool config_save() {
    File file = SD.open("/config.dat", FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    file.write((byte *)&curConfig, sizeof(config));
    file.close();
    return true;
}

void config_erase() {
    SD.remove("/config.dat");
}