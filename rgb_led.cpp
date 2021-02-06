#include "rgb_led.h"

#include <esp32-hal-matrix.h>

#include "config.h"

// PIN
#define RGB_LED_RED_PIN 26
#define RGB_LED_BLUE_PIN 27 // 27
#define RGB_LED_GREEN_PIN 12

// PWM
#define RGB_LED_FREQ 5000
#define RGB_LED_RES 8
#define RGB_LED_RED_CHANNEL 1
#define RGB_LED_BLUE_CHANNEL 2
#define RGB_LED_GREEN_CHANNEL 3

void rgb_led_setup() {
    ledcSetup(RGB_LED_RED_CHANNEL, RGB_LED_FREQ, RGB_LED_RES);
    ledcAttachPin(RGB_LED_RED_PIN, RGB_LED_RED_CHANNEL);
    ledcSetup(RGB_LED_GREEN_CHANNEL, RGB_LED_FREQ, RGB_LED_RES);
    ledcAttachPin(RGB_LED_GREEN_PIN, RGB_LED_GREEN_CHANNEL);
    ledcSetup(RGB_LED_BLUE_CHANNEL, RGB_LED_FREQ, RGB_LED_RES);
    ledcAttachPin(RGB_LED_BLUE_PIN, RGB_LED_BLUE_CHANNEL);
}

void rgb_led_update(int hour) {
    char *color = curConfig.led[hour];
    long number = (long)strtol(&color[1], NULL, 16);
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

    ledcWrite(RGB_LED_RED_CHANNEL, r);
    ledcWrite(RGB_LED_GREEN_CHANNEL, g);
    ledcWrite(RGB_LED_BLUE_CHANNEL, b);
}