#include "loadcell.h"

int16_t LoadCell::compress_raw_counts(long net_counts) {
    // Keep LoRa raw field compact by dropping lower 8 bits of the 24-bit HX711 value.
    long shifted = net_counts >> 8;
    if (shifted > 32767L) return 32767;
    if (shifted < -32768L) return -32768;
    return static_cast<int16_t>(shifted);
}

bool LoadCell::init() {
    load_cell.begin(HX711_DOUT_PIN, HX711_SCK_PIN, HX711_GAIN);

    if (!load_cell.wait_ready_timeout(1000, 1)) {
        return false;
    }

    load_cell.set_gain(HX711_GAIN);
    load_cell.set_scale(1.0f);

    return set_zero(default_tare_samples);
}

bool LoadCell::set_zero(uint8_t samples) {
    if (!load_cell.wait_ready_timeout(1000, 1)) {
        return false;
    }

    load_cell.tare(samples == 0 ? 1 : samples);
    offset_counts = load_cell.get_offset();
    return true;
}

bool LoadCell::read(const SensorDesc &sensor, int32_t &data, int16_t &raw_adc) {
    (void)sensor;

    if (!load_cell.is_ready()) {
        return false;
    }

    long raw_read = load_cell.read();
    long net_counts = raw_read - load_cell.get_offset();
    offset_counts = load_cell.get_offset();

    if (net_counts < 0) {
        net_counts = 0;
    }

    // Convert counts to grams with the same calibration model discussed earlier.
    // Update load_cell_rating_kg when your specific cell rating is known.
    constexpr long double load_cell_rating_kg = 1.0L;
    constexpr long double force_per_step = (load_cell_rating_kg * 1000.0L) / 4295241.0L;

    data = static_cast<int32_t>(force_per_step * static_cast<long double>(net_counts));
    raw_adc = compress_raw_counts(net_counts);
    
    return true;
}