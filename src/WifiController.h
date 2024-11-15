#ifndef WIFICONTROLLER_H
#define WIFICONTROLLER_H

#include <WiFi.h>
#include "led.h"
#include "secrets.h"
#include "debug.h"

class WifiController {
    private: 
        int m_connected;
        bool m_ledstate;

    public:
        Adafruit_NeoPixel m_pixel;
        WifiController(): m_connected(0), m_ledstate(0), m_pixel(Adafruit_NeoPixel(1, RGBLED, NEO_GRB + NEO_KHZ800)) {}
        void startConnection();
        bool checkWifi();
        bool isConnected() {
            return WiFi.status() == WL_CONNECTED;
        }
        void reconnect();
};

void WifiController::startConnection() {
    m_pixel.begin();
    m_pixel.setBrightness(10);
    DEBUG_PRINTLN("INFO: Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

bool WifiController::checkWifi() {
    if (WiFi.status() == WL_CONNECTED && !m_connected) {
        DEBUG_PRINTLN("INFO: Connected to WiFi");
        m_connected = 1;
        showBlue(m_pixel);
        return true;
    } else if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("INFO: Disconnected from WiFi");
        m_connected = 0;
        blinkRed(&m_ledstate, m_pixel);
        return false;
    }
    return true;
}

void WifiController::reconnect() {
    while (!WiFi.isConnected())
    {
        WiFi.reconnect();
    }
}

#endif