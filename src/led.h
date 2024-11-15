#ifndef LED_H
#define LED_H

#include <Adafruit_NeoPixel.h>

void blinkRed(bool *blinkVar, Adafruit_NeoPixel &pixels) {
    pixels.clear();
    if (*blinkVar) {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(255, 0, 0));
    } else {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    pixels.show();
    *blinkVar = !*blinkVar;
}

void blinkYellow(bool *blinkVar, Adafruit_NeoPixel &pixels) {
    pixels.clear();
    if (*blinkVar) {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(255, 255, 0));
    } else {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    pixels.show();
    *blinkVar = !*blinkVar;
}

void blinkBlue(bool *blinkVar, Adafruit_NeoPixel &pixels) {
    pixels.clear();
    if (*blinkVar) {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 255));
    } else {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    pixels.show();
    *blinkVar = !*blinkVar;
}

void showBlue(Adafruit_NeoPixel &pixels) {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 255));
    pixels.show();
}

void showGreen(Adafruit_NeoPixel &pixels) {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 255, 0));
    pixels.show();
}

void showPurple(Adafruit_NeoPixel &pixels) {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(255, 0, 255));
    pixels.show();
}

void ShowOrange(Adafruit_NeoPixel &pixels) {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(255, 165, 0));
    pixels.show();
}

#endif