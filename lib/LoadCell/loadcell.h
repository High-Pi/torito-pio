#ifndef LOAD_CELL_H
#define LOAD_CELL_H

#include <Arduino.h>
#include <HX711.h>
#include <hwconfig.h>
#include <SensorDesc.h>

class LoadCell {
private:
    HX711 load_cell;
    static constexpr uint8_t default_tare_samples = 5;
    long offset_counts = 0;

    static int16_t compress_raw_counts(long net_counts);

public:
    bool init();
    bool read(const SensorDesc &sensor, int32_t &data, int16_t &raw_adc);
    bool set_zero(uint8_t samples = default_tare_samples);
    long get_offset_counts() const { return offset_counts; }

};

#endif