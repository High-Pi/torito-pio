#include "solenoidreceive.h"

bool SolenoidReceive::init(uint8_t i2c_address) {
    if (i2c_address == 0) return false; // 0 is not a valid 7-bit I2C address here
    i2c_address_ = i2c_address;
    // Do NOT touch/select the I2C mux here — DAQ will select the channel before reads.
    return true;
}

bool SolenoidReceive::read(uint16_t &out_state) {
    if (i2c_address_ == 0) return false;

    const int MAX_ATTEMPTS = 3;
    size_t got = 0;
    for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt) {
        got = Wire.requestFrom((int)i2c_address_, (int)2, (int)true);
        if (got == 2) {
            Serial.print("SolenoidReceive: I2C partial read addr=0x");
        if (i2c_address_ < 16) Serial.print("0");
        Serial.print(i2c_address_, HEX);
        Serial.print(" -> got "); Serial.print(got); Serial.print(" bytes: ");

            // Consume and print available bytes in hex
        for (size_t i = 0; i < got; ++i) {
            int b = Wire.read();
            if (b < 0) break; // no more data
            if ((uint8_t)b < 16) Serial.print("0");
            Serial.print((uint8_t)b, HEX);
            if (i + 1 < got) Serial.print(" ");
            break;
        };
        delay(5); // short backoff for slave to prepare data
    }
   if (got > 2) {
        Serial.print("SolenoidReceive: I2C partial read addr=0x");
        if (i2c_address_ < 16) Serial.print("0");
        Serial.print(i2c_address_, HEX);
        Serial.print(" -> got "); Serial.print(got); Serial.print(" bytes: ");

            // Consume and print available bytes in hex
        for (size_t i = 0; i < got; ++i) {
            int b = Wire.read();
            if (b < 0) break; // no more data
            if ((uint8_t)b < 16) Serial.print("0");
            Serial.print((uint8_t)b, HEX);
            if (i + 1 < got) Serial.print(" ");
        }
        Serial.println();
    }
    return false;
}

    uint8_t hi = (uint8_t)Wire.read();
    uint8_t lo = (uint8_t)Wire.read();

    // MSB-first: hi = high byte, lo = low byte
    out_state = (uint16_t(hi) << 8) | uint16_t(lo);
    cached_state = out_state;

    Serial.print("SolenoidReceive: I2C read success -> 0x");
    Serial.println(out_state, HEX);

    return true;
} 
