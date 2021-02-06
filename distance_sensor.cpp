#include "distance_sensor.h"

#include <Arduino.h>
#include <Wire.h>

#define DISTANCE_SENSOR_TRIGGER_PIN 33
#define DISTANCE_SENSOR_ECHO_PIN 32

void distance_sensor_setup() {
    pinMode(DISTANCE_SENSOR_TRIGGER_PIN, OUTPUT);
    pinMode(DISTANCE_SENSOR_ECHO_PIN, INPUT);
}

int distance_sensor_getDistance() {
    digitalWrite(DISTANCE_SENSOR_TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(DISTANCE_SENSOR_TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(DISTANCE_SENSOR_TRIGGER_PIN, LOW);
    long duration = pulseIn(DISTANCE_SENSOR_ECHO_PIN, HIGH);
    int distance = duration * 0.034 / 2;
    return distance;
}