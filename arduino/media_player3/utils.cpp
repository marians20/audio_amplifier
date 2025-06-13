#include "utils.h"

void Utils::parseCommaSeparatedValues(const char* input, uint16_t maxSize, uint16_t& size, uint16_t* output) {
    auto strCopy = strdup(input); // Duplicate the input to avoid modifying the original

    size = 0;
    for (auto i = 0; input[i] != '\0'; ++i) {
        if (input[i] == ',') size++;
    }
    size++;
    if(size > maxSize) {
      size = maxSize;
    }

    int i = 0;
    auto token = strtok(strCopy, ","); // Tokenize the string using ',' as the delimiter
    while (token != NULL && i < size) {
        uint16_t number = atoi(token); // Convert the token to an integer
        output[i++] = number;
        token = strtok(NULL, ","); // Move to the next token
    }

    free(strCopy); // Free the duplicated string
}

// double Utils::limit(double value, double maxValue) {
//     return value <= maxValue ? value : maxValue;
// }

double Utils::limitFallingSpeed(double value, double initialValue, unsigned long deltaT, double maxFallingSpeed) {
    if(value >= initialValue) {
        return value;
    }

    if(deltaT == 0) {
        return initialValue;
    }

    double actualSpeed = (value - initialValue) / deltaT;
    if(actualSpeed >= maxFallingSpeed) {
        return value;
    }

    double newValue = initialValue + maxFallingSpeed * (double)deltaT;
    return newValue < 0 ? 0 : newValue;
}

