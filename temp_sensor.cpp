#include "temp_sensor.h"

#include <Adafruit_BME280.h>
#include <Arduino.h>

// default values defined by the library
#define TEMP_SENSOR_SDA_PIN 21
#define TEMP_SENSOR_SCL_PIN 22
// default value defined by the producer
#define TEMP_SENSOR_I2C_ADDRESS 0x76

static Adafruit_BME280 bme;

bool temp_sensor_setup() {
    if (!bme.begin(TEMP_SENSOR_I2C_ADDRESS)) {
        return false;
    }
    return true;
}

void temp_sensor_readData(float *temp, float *press, float *hum) {
    *temp = bme.readTemperature();
    *press = bme.readPressure() / 100.0F;  // hPa
    *hum = bme.readHumidity();
}