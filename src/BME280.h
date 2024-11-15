#ifndef BME280_H
#define BME280_H

#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

struct BME280Data {
    float temperature;
    float humidity;
    float pressure;
    float altitude;
};

float readBME280Temperature() {
    return bme.readTemperature();
}

float readBME280Humidity() {
    return bme.readHumidity();
}

float readBME280Pressure() {
    return bme.readPressure();
}

float readBME280Altitude(float seaLevelPressure) {
    return bme.readAltitude(seaLevelPressure);
}

void setupBME280(TwoWire& I2CWire) {
    bool status = bme.begin(0x76, &I2CWire);  
    if (!status) {
        DEBUG_PRINTLN("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    } else {
        DEBUG_PRINTLN("BME280 sensor found");
    }
}

bool readBME280(BME280Data& data, float seaLevelPressure) {
    data.temperature = readBME280Temperature();
    data.humidity = readBME280Humidity();
    data.pressure = readBME280Pressure();
    data.altitude = readBME280Altitude(seaLevelPressure);
    return true;
}

#endif // BME280_H