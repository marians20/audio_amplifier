#include "Arduino.h"
#include "value.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const int MAX_INPUT_LENGTH = 10;

Value::Value() {
  _left = 0;
  _right = 0;
}

Value::Value(uint16_t data[]) : _left(data[0]), _right(data[1]), sampleTime(millis()) {
  _maxValue = MAX(data[0], data[1]);
}

void Value::Parse(uint16_t data[]) {
  Parse(data[0], data[1]);
}

void Value::Parse(uint16_t left, uint16_t right) {
  deltaT = millis() - sampleTime;
  _left = Utils::limitFallingSpeed(left, _oldLeft, deltaT, valueFallSpeed);
  _right = Utils::limitFallingSpeed(right, _oldRight, deltaT, valueFallSpeed);
  _oldLeft = _left;
  _oldRight = _right;
  _absoluteMaxValue = MAX(_absoluteMaxValue, MAX(_left, _right));
  _maxValue = Utils::limit((_maxValue * 0.95) + (MAX(_left, _right) * 0.05), _absoluteMaxValue);
  sampleTime = millis();
  // Serial.printf("Left: %d / %f, Right: %d / %f, MaxValue: %f\n", left, _left, right, _right, _maxValue);
}

uint16_t Value::getLeft() {
  return _left;
}

uint16_t Value::getRight() {
  return _right;
}

uint16_t Value::getMax() {
  return ceil(_maxValue);
}