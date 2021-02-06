#include "sdcard.h"

#include <Arduino.h>
#include <FS.h>
#include <SD.h>

#define SDCARD_CS_PIN 16

bool sdcard_setup() {
    if (!SD.begin(SDCARD_CS_PIN)) {
        return false;
    }

    if (SD.cardType() == CARD_NONE) {
        return false;
    }

    return true;
}