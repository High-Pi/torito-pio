#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include <Arduino.h>
#include "HX711.h"

class LoadCell {
private:
    HX711 loadCell;
    uint8_t _dataPin;
    uint8_t _clockPin;
    uint32_t _offset;
    float _loadCellRating;
    long double _forcePerStep;

public:
    // Constructor
    LoadCell(uint8_t dataPin, uint8_t clockPin, float rating);

    // Methods
    void begin();
    uint32_t setZero();
    float getForceInGrams();
    long readRaw();
};

#endif