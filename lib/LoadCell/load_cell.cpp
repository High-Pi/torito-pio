#include "load_cell.h"

// Constructor: Initializes pins and calculates force scale factor
LoadCell::LoadCell(uint8_t dataPin, uint8_t clockPin, float rating) 
    : _dataPin(dataPin), _clockPin(clockPin), _loadCellRating(rating), _offset(0) {
    
    // Calculate force per step: (Rating in kg * 1000g) / max 24-bit resolution
    _forcePerStep = (_loadCellRating * 1000.0) / 4295241.0; //
}

void LoadCell::begin() {
    loadCell.begin(_dataPin, _clockPin);
    
    while (!loadCell.is_ready()) {
        Serial.println("HX711 not yet ready...");
        delay(100);
    }
    
    Serial.println("HX711 ready.");
    _offset = setZero();
    loadCell.set_gain(128);
}

uint32_t LoadCell::setZero() {
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += loadCell.read();
    }
    _offset = sum / 10;
    return _offset;
}

long LoadCell::readRaw() {
    return loadCell.read();
}

float LoadCell::getForceInGrams() {
    uint32_t currentRead = loadCell.read();
    // Prevent negative results if reading is slightly below offset
    if (currentRead < _offset) return 0.0; 
    
    uint32_t diff = currentRead - _offset;
    return (float)(_forcePerStep * diff);
}