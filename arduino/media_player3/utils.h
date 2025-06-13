#ifndef __UTILS_H__
#define __UTILS_H__
#include <Arduino.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef void (*OnDataAvailableFunction)(uint16_t* values, uint16_t valuesCount);

class Utils {
  public:
    static void parseCommaSeparatedValues(const char* input, uint16_t maxSize, uint16_t& size, uint16_t* output);
    template <typename T1, typename T2, typename T3> static T1 scale(T1 value, T2 maxInputValue, T3 maxOutputValue);
    template <typename T1, typename T2> static T1 limit(T1 value, T2 maxValue);
    static double limitFallingSpeed(double value, double initialValue, unsigned long deltaT, double maxFallingSpeed);
};

template <typename T1, typename T2, typename T3> T1 Utils::scale(T1 value, T2 maxInputValue, T3 maxOutputValue) {
    if(value <= 0) {
        return value;
    }

    return maxInputValue == 0 ? 0 : Utils::limit((double)value * maxOutputValue / maxInputValue, maxOutputValue);
}

template <typename T1, typename T2> T1 Utils::limit(T1 value, T2 maxValue) {
    return value <= maxValue ? value : maxValue;
}

#endif