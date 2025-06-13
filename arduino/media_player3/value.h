#include <stdint.h>
#include <stdlib.h>
#ifndef __VALUE_H__
#define __VALUE_H__

#include "utils.h"

class Value {
public:
  Value();
  Value(uint16_t data[]);
  void Parse(uint16_t data[]);
  void Parse(uint16_t left, uint16_t right);
  uint16_t getLeft();
  uint16_t getRight();
  uint16_t getMax();
private:
  unsigned long sampleTime = 0;
  unsigned long deltaT = 0;
  double _left;
  double _right;
  double _oldLeft = 0;
  double _oldRight = 0;
  double _absoluteMaxValue = 0;
  double _maxValue = 0;
  double valueFallSpeed = -0.5;
};

#endif