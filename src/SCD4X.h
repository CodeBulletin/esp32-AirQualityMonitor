#ifndef SCD4X_H
#define SCD4X_H

#include <Wire.h>
#include "SensirionI2CScd4x.h"
#include "debug.h"

SensirionI2CScd4x scd4x;

struct SCD4xData {
    uint16_t co2;
    float temperature;
    float humidity;
};

DEBUG_ONLY(
    void printUint16Hex(uint16_t value) {
        Serial.print(value < 4096 ? "0" : "");
        Serial.print(value < 256 ? "0" : "");
        Serial.print(value < 16 ? "0" : "");
        Serial.print(value, HEX);
    }
)

DEBUG_ONLY(
    void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
        Serial.print("Serial: 0x");
        printUint16Hex(serial0);
        printUint16Hex(serial1);
        printUint16Hex(serial2);
        Serial.println();
    }
)

DEBUG_ONLY(
    void printData(SCD4xData& data) {
        Serial.print("CO2: ");
        Serial.print(data.co2);
        Serial.print(" | Temperature: ");
        Serial.print(data.temperature);
        Serial.print(" | Humidity: ");
        Serial.println(data.humidity);
    }
)

void setupSCD4x(SensirionI2CScd4x& scd4x, TwoWire& I2C) {
    scd4x.begin(I2C);
    DEBUG_ONLY(char errorMessage[256]);
    uint16_t error;

    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        DEBUG_PRINT("Error executing stopPeriodicMeasurement(): ");
        DEBUG_PRINTLN(error);

        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINT("Error message: ");
        DEBUG_PRINTLN(errorMessage);
        return;
    } else {
        DEBUG_PRINTLN("Stopped periodic measurement");
    }

    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (error) {
        DEBUG_PRINT("Error trying to execute getSerialNumber(): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
        return;
    } else {
        DEBUG_ONLY(printSerialNumber(serial0, serial1, serial2));
    }

    error = scd4x.setAutomaticSelfCalibration(0);
    if (error) {
        DEBUG_PRINT("Error trying to execute setAutomaticSelfCalibration(0): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
        return;
    } else {
        DEBUG_PRINTLN("Disabled automatic self calibration");
    }

    error = scd4x.startPeriodicMeasurement();
    if (error) {
        DEBUG_PRINT("Error executing startPeriodicMeasurement(): ");
        DEBUG_PRINTLN(error);

        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINT("Error message: ");
        DEBUG_PRINTLN(errorMessage);
        return;
    } else {
        DEBUG_PRINTLN("Started periodic measurement");
    }
}

bool isReady(SensirionI2CScd4x& scd4x) {
    bool isDataReady;
    char errorMessage[256];
    uint16_t error = scd4x.getDataReadyFlag(isDataReady);
    if (error) {
        DEBUG_PRINT("Error trying to execute getDataReadyFlag(): ");
        errorToString(error, errorMessage, 256);
        DEBUG_PRINTLN(errorMessage);
        return false;
    } else {
        return isDataReady;
    }
}

bool readSCD4x(SensirionI2CScd4x& scd4x, SCD4xData& data) {
    uint16_t co2;
    uint16_t temperature;
    uint16_t humidity;
    bool isDataReady;
    DEBUG_ONLY(char errorMessage[256]);

    uint16_t error = scd4x.readMeasurementTicks(co2, temperature, humidity);
    if (error) {
        DEBUG_PRINT("Error trying to execute readMeasurementTicks(): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
    } else {
        data.co2 = co2;
        data.temperature = -45.0 + (175.0 * temperature) / 65536.0;
        data.humidity = 100.0 * humidity / 65536.0;
        return true;
    }
}

void setPressureCompensation(SensirionI2CScd4x& scd4x, uint16_t pressure) {
    DEBUG_ONLY(char errorMessage[256]);
    uint16_t error = scd4x.setAmbientPressure(pressure);
    if (error) {
        DEBUG_PRINT("Error trying to execute setAmbientPressure(): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
        return;
    } else {
        DEBUG_PRINT("Set pressure compensation to ");
        DEBUG_PRINT(pressure);
        DEBUG_PRINTLN(" hPa");
    }
}

void setAltitudeCompensation(SensirionI2CScd4x& scd4x, uint16_t altitude) {
    DEBUG_ONLY(char errorMessage[256]);
    uint16_t error = scd4x.setSensorAltitude(altitude);
    if (error) {
        DEBUG_PRINT("Error trying to execute setSensorAltitude(): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
        return;
    } else {
        DEBUG_PRINT("Set altitude compensation to ");
        DEBUG_PRINT(altitude);
        DEBUG_PRINTLN(" meters");
    }
}

uint16_t forcedRecalibration(SensirionI2CScd4x& scd4x, uint16_t targetCo2Concentration, uint16_t& frc) {
    DEBUG_ONLY(char errorMessage[256]);

    uint16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        DEBUG_PRINT("Error executing stopPeriodicMeasurement(): ");
        DEBUG_PRINTLN(error);

        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINT("Error message: ");
        DEBUG_PRINTLN(errorMessage);
        return error;
    }

    error = scd4x.setAutomaticSelfCalibration(0);
    if (error) {
        DEBUG_PRINT("Error trying to execute setAutomaticSelfCalibration(0): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
        return error;
    }

    error = scd4x.performForcedRecalibration(targetCo2Concentration, frc);
    if (error) {
        DEBUG_PRINT("Error trying to execute performForcedRecalibration(): ");
        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINTLN(errorMessage);
        return error;
    } 

    error = scd4x.startPeriodicMeasurement();
    if (error) {
        DEBUG_PRINT("Error executing startPeriodicMeasurement(): ");
        DEBUG_PRINTLN(error);

        DEBUG_ONLY(errorToString(error, errorMessage, 256));
        DEBUG_PRINT("Error message: ");
        DEBUG_PRINTLN(errorMessage);
        return error;
    }

    return 0;
}

#endif // SCD4X_H