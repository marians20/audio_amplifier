#ifndef __VUMETER_H__
#define __VUMETER_H__
#include <Arduino.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include "value.h"
#include "utils.h"

class Vumeter {
  public:
    Vumeter(uint16_t pixelsCount, int16_t pin,  uint16_t luminosity);
    ~Vumeter();
    void show(uint16_t data[]);
    void show(uint16_t left, uint16_t right);
    void clear();
  private:
    Adafruit_NeoPixel pixels;
    uint16_t pin;
    uint16_t pixelsCount;
    uint16_t luminosity;
    uint16_t ledsPerBar;
    Value value;
    int getLedAddress(uint16_t row, uint16_t column);
    uint32_t getColor(uint16_t r, uint16_t g, uint16_t b, uint16_t luminosity);
    uint32_t getLedColor(uint16_t column, uint16_t luminosity);
    uint32_t getPeakColor(uint16_t column);
    void setBarValue(int row, int value, int maxValue);
    void clearPixels();
    uint16_t scale(uint16_t value, uint16_t maxValue);
};

#endif