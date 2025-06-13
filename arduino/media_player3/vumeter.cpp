#include "vumeter.h"

Vumeter::Vumeter(uint16_t pixelsCount, int16_t pin, uint16_t luminosity)
  : pixelsCount(pixelsCount),
    pin(pin),
    luminosity(luminosity),
    ledsPerBar(pixelsCount / 2),
    value(Value()),
    pixels(Adafruit_NeoPixel(pixelsCount, pin, NEO_GRB + NEO_KHZ800)) {
}

void Vumeter::clear() {
  clearPixels();
  pixels.show();
}

int Vumeter::getLedAddress(uint16_t row, uint16_t column) {
  if (row == 0) {
    return column;
  }

  if (row == 1) {
    return this->pixelsCount - column - 1;
  }

  return -1;
}

uint32_t Vumeter::getColor(uint16_t r, uint16_t g, uint16_t b, uint16_t luminosity) {
  return this->pixels.Color((r * luminosity / 255), (g * luminosity / 255), (b * luminosity / 255));
}

uint32_t Vumeter::getLedColor(uint16_t column, uint16_t luminosity) {
  if (column < ledsPerBar / 2) {
    return this->getColor(0, 255, 128, luminosity);
  }

  if (column < 3 * ledsPerBar / 4) {
    return this->getColor(255, 200, 0, luminosity);
  }

  return this->getColor(255, 0, 10, luminosity);
}

uint32_t Vumeter::getPeakColor(uint16_t column) {
  return getColor(64, 0, 128, luminosity);
}

void Vumeter::setBarValue(int row, int value, int maxValue) {
  if (value < 0) {
    value = 0;
  }

  uint16_t scaledValue = scale(value, maxValue);
  for (auto i = 0; i < scaledValue; i++) {
    pixels.setPixelColor(getLedAddress(row, i), getLedColor(i, luminosity));
  }
  
  for (auto i = scaledValue; i < ledsPerBar; i++) {
    this->pixels.setPixelColor(getLedAddress(row, i), this->pixels.Color(0, 0, 0));
  }
}

void Vumeter::clearPixels() {
  for (auto i = 0; i < pixelsCount; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
}

uint16_t Vumeter::scale(uint16_t value, uint16_t maxValue) {
  return Utils::scale(value, maxValue, ledsPerBar);
}

void Vumeter::show(uint16_t data[]) {
  show(data[0], data[1]);
  setBarValue(0, value.getLeft(), value.getMax());
  setBarValue(1, value.getRight(), value.getMax());
  pixels.show();
}

void Vumeter::show(uint16_t left, uint16_t right) {
  value.Parse(left, right);
  setBarValue(0, value.getLeft(), value.getMax());
  setBarValue(1, value.getRight(), value.getMax());
  pixels.show();
}