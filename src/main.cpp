#include "WifiController.h"
#include "TimeController.h"
#include <PubSubClient.h>
#include "driver/temp_sensor.h"
#include "MessageBuilder.h"
#include "SCD4X.h"
#include "BME280.h"
#include "string.h"

#include <WebServer.h>
#include <ElegantOTA.h>

#define I2C_SDA 16
#define I2C_SCL 15

TwoWire I2CWire = TwoWire(0);

float currentSeaLevelPressure = 1013.25;
uint16_t co2_outside = 420;

WifiController wifiController;
WiFiClient m_client;
PubSubClient mqttClient(m_client);
bool blinkLed = false;
int64_t lastTime = 0;
int64_t lastTimeSinceSync = 0;
int hasTime = 0;


void initCPUTempSensor(){
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
}

float CPUTemp() {
    float result = 0;
    temp_sensor_read_celsius(&result);
    return result;
}

unsigned long ota_progress_millis = 0;

void onOTAStart() {
    showPurple(wifiController.m_pixel);
}

void onOTAProgress(size_t current, size_t final) {
    if (millis() - ota_progress_millis > 1000) {
        ota_progress_millis = millis();
        DEBUG_ONLY(Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final));
        blinkBlue(&blinkLed, wifiController.m_pixel);
    }
}

void onOTAEnd(bool success) {
    if (success) {
        DEBUG_ONLY(Serial.println("OTA update finished successfully!"));
    } else {
        DEBUG_ONLY(Serial.println("There was an error during OTA update!"));
    }
    DEBUG_PRINTLN("INFO: Resetting device...");
    showPurple(wifiController.m_pixel);
    delay(1000);
    esp_restart();
}


bool reconnect() {
    if (!wifiController.isConnected()) {
        wifiController.reconnect();
    }
    if (!mqttClient.connected()) {
        DEBUG_PRINTLN("INFO: Attempting MQTT connection...");
        if (mqttClient.connect("ESP32Client", MQTT_USER, MQTT_PASS)) {
            DEBUG_PRINTLN("INFO: Connected to MQTT server");
            mqttClient.subscribe("esp32/AQM/output");
            mqttClient.subscribe("esp32/AQM/universal");
            showGreen(wifiController.m_pixel);
            return true;
        } else {
            DEBUG_PRINT("ERROR: Failed to connect to MQTT server, rc=");
            DEBUG_PRINTLN(mqttClient.state());
            DEBUG_PRINTLN("INFO: Trying again in 1 seconds...");
            blinkYellow(&blinkLed, wifiController.m_pixel);
            return false;
        }
    } else {
        return true;
    }
}


void OTASetup() {
    // define the server

    WebServer server(80);


    ElegantOTA.begin(&server, OTA_USER, OTA_PASS);
    ElegantOTA.setAutoReboot(true);
    // set the callback functions
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    ElegantOTA.onStart(onOTAStart);

    server.begin();

    ShowOrange(wifiController.m_pixel);

    while (true) {
        if (!mqttClient.connected()) {
            if(!reconnect()) {
                return;
            }
        } 
        mqttClient.loop();
        server.handleClient();
        delay(1);
    }
}



void callback(char* topic, byte* payload, unsigned int length) {
    // handle message arrived if topic is esp32/output
    char message[length + 1];
    for (int i = 0; i < length; i++) {
        message[i] = (char)payload[i];
    }
    message[length] = '\0';

    DEBUG_PRINT("INFO: Message arrived [");
    DEBUG_PRINT(topic);
    DEBUG_PRINT("] ");
    DEBUG_PRINTLN(message);

    // if topic is esp32/AQM/universal
    if (strcmp(topic, "esp32/AQM/universal") == 0) {

        // if message is "ota" then perform OTA update
        if (strcmp(message, "ota") == 0) {
            DEBUG_PRINTLN("INFO: OTA update requested");
            MessageBuilder builder = MessageBuilder(readTime(), DEVICE_ID, 1).addMessage("OTAUpdate");
            mqttClient.publish("esp32/input", builder.c_str());
            OTASetup();
        }

        // if message is "sea_level_pressure/xxx" then set sea level pressure to xxx
        else if (strncmp(message, "sea_level_pressure/", 19) == 0) {
            currentSeaLevelPressure = atof(message + 19);
            DEBUG_PRINT("INFO: Set sea level pressure to ");
            DEBUG_PRINTLN(currentSeaLevelPressure);
        } 

        // if message is "reset" then restart the device
        else if (strcmp(message, "restart") == 0) {
            DEBUG_PRINTLN("INFO: Resetting device...");
            MessageBuilder builder = MessageBuilder(readTime(), DEVICE_ID, 1).addMessage("Reset");
            mqttClient.publish("esp32/input", builder.c_str());
            esp_restart();
        }
    }

    // if topic is esp32/AQM/output
    else if (strcmp(topic, "esp32/AQM/output") == 0) {

        // OTA update
        if (strncmp(message, "ota/", 4) == 0) {
            int deviceId = atoi(message + 4);
            if (deviceId == DEVICE_ID) {
                DEBUG_PRINTLN("INFO: OTA update requested");
                MessageBuilder builder = MessageBuilder(readTime(), DEVICE_ID, 1).addMessage("OTAUpdate");
                mqttClient.publish("esp32/input", builder.c_str());
                OTASetup();
            }
        }

        // if message is "reset" then reset the device
        else if (strncmp(message, "reset/", 6) == 0) {
            int deviceId = atoi(message + 6);
            if (deviceId == DEVICE_ID) {
                DEBUG_PRINTLN("INFO: Resetting device...");
                MessageBuilder builder = MessageBuilder(readTime(), DEVICE_ID, 1).addMessage("Reset");
                mqttClient.publish("esp32/input", builder.c_str());
                esp_restart();
            }
        }

        // if message is "forced_recalibration/device_id:xxx" check if device_id = DEVICE_ID then perform forced recalibration with xxx
        else if (strncmp(message, "forced_recalibration/", 21) == 0) {
            int colonIndex = 0;
            for (int i = 21; i < length; i++) {
                if (message[i] == ':') {
                    colonIndex = i;
                    break;
                }
            }

            int deviceId = atoi(message + 21);


            if (deviceId == DEVICE_ID) {
                uint16_t frc = 0;
                co2_outside = atoi(message + colonIndex + 1);
                char errorMessage[256];
                uint16_t err = forcedRecalibration(scd4x, atoi(message + colonIndex + 1), frc);
                errorToString(err, errorMessage, 256);
                DEBUG_PRINT("INFO: Forced recalibration performed with ");
                DEBUG_PRINTLN(frc);

                MessageBuilder builder = MessageBuilder(readTime(), DEVICE_ID, 1).addMessage("ForcedRecalibration");
                
                if (err != 0) {
                    builder.addMessage("NACK").addMessage(errorMessage);

                    mqttClient.publish("esp32/input", builder.c_str());

                    return;
                }

                if (frc !=  0xFFFF) {
                    builder.addMessage("ACK").addMessage("co2:" + String(frc - 0x8000));
                } else {
                    builder.addMessage("NACK");
                }

                mqttClient.publish("esp32/input", builder.c_str());
            }
        }
    }
}

void setup() {
    DEBUG_ONLY(Serial.begin(115200));
    DEBUG_PRINTLN("INFO: Starting...");
    wifiController.startConnection();
    DEBUG_PRINTLN("INFO: Setting up MQTT client...");
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callback);

    while (!wifiController.checkWifi()) {
        delay(1000);
    }
    lastTime = getCurrentTime();
    lastTimeSinceSync = getCurrentTime();
    while (hasTime != 2) {
        hasTime = syncTimeToServer(true);
    }
    DEBUG_ONLY(printLocalTime());
    initCPUTempSensor();
    DEBUG_PRINTF("INFO: I2C SDA: %d, SCL: %d\n", I2C_SDA, I2C_SCL);

    I2CWire.begin(I2C_SDA, I2C_SCL, 100000);

    setupSCD4x(scd4x, I2CWire);
    setupBME280(I2CWire);
}

void main_loop() {
    if (!mqttClient.connected()) {
        if(!reconnect()) {
            return;
        }
    } 
    mqttClient.loop();

    if (getCurrentTime() < lastTimeSinceSync) {
        lastTimeSinceSync = getCurrentTime();
    }

    if (getCurrentTime() < lastTime) {
        lastTime = getCurrentTime();
    } 

    if (getCurrenTimeDiff(lastTimeSinceSync) >= 60 * 1000) {
        lastTimeSinceSync = getCurrentTime();
        int id = syncTimeToServer(true);
        if (id != 0) {
            DEBUG_ONLY(printLocalTime());
        }
    }

    if (getCurrenTimeDiff(lastTime) >= 30 * 1000 && hasTime != 0) {
        lastTime = getCurrentTime();
        MessageBuilder builder = MessageBuilder(readTime(), DEVICE_ID);

        float temp = CPUTemp();
        builder.addSensor(CPU_SENSOR_ID)
            .addAttribute("temperature", temp);

        
        BME280Data bmeData;
        if (readBME280(bmeData, currentSeaLevelPressure)) {
            builder.addSensor(BME280_SENSOR_ID)
                .addAttribute("temperature", bmeData.temperature)
                .addAttribute("humidity", bmeData.humidity)
                .addAttribute("pressure", bmeData.pressure)
                .addAttribute("altitude", bmeData.altitude);
        }

        SCD4xData data;
        if (isReady(scd4x)) {
            if (readSCD4x(scd4x, data)) {
            builder.addSensor(SCD41_SENSOR_ID)
                .addAttribute("co2", data.co2)
                .addAttribute("temperature", data.temperature)
                .addAttribute("humidity", data.humidity);
            }
        }

        mqttClient.publish("esp32/input", builder.end().c_str());
        DEBUG_PRINT("INFO: Published message: ");
        DEBUG_PRINTLN(builder.c_str());
    }
}

void loop() {
    main_loop();
    delay(100);
}