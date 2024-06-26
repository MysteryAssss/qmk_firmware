// add license
// he_switch_matrix.c
#include "he_switch_matrix.h"
#include "analog.h"
#include "print.h"
#include "wait.h"
#include "eeprom.h"
#include "math.h"
#include "timer.h" // Debugging

// map = row, col, sensor_id, mux_id, mux_channel
const sensor_to_matrix_map_t sensor_to_matrix_map[] = {
    {0,0,0,0,4},  {0,1,1,0,3},
    {1,0,2,0,12}, {1,1,3,0,13}
};

matrix_row_t matrix_get_row(uint8_t row) {
    if (row < MATRIX_ROWS) {
        return matrix[row];
    } else {
        return 0;
    }
}

static key_debounce_t debounce_matrix[MATRIX_ROWS][MATRIX_COLS] = {{{0, 0}}};
const uint32_t mux_en_pins[] = MUX_EN_PINS;
const uint32_t mux_sel_pins[] = MUX_SEL_PINS;
static adc_mux adcMux;
he_config_t he_config;

//move to he_init?
static void init_mux_sel(void) {
    int array_size = sizeof(mux_sel_pins) / sizeof(mux_sel_pins[0]);
    for (int i = 0; i < array_size; i++) {
        setPinOutput(mux_sel_pins[i]);
        writePinLow(mux_sel_pins[i]);
    }
}

int he_init(he_config_t const* const he_config) {
    palSetLineMode(ANALOG_PORT, PAL_MODE_INPUT_ANALOG);
    adcMux = pinToMux(ANALOG_PORT);
    adc_read(adcMux);

    init_mux_sel();

    return 0;
}

// Sets EN and SEL pins on the multiplexer
// write all low on kb init, then write low during scanning after en_pin is set high and read
static inline void select_mux(uint8_t sensor_id) {
    uint8_t mux_id = sensor_to_matrix_map[sensor_id].mux_id;
    uint8_t mux_channel = sensor_to_matrix_map[sensor_id].mux_channel;

    for (int i = 0; i < sizeof(mux_en_pins)/ sizeof(mux_en_pins[0]); i++) {
        writePinLow(mux_en_pins[i]);
    }

    writePinHigh(mux_en_pins[mux_id]);

    for (int j = 0; j < 4; j++) {
        if (mux_channel & (1 << j)) {
            writePinHigh(mux_sel_pins[j]);
        } else {
            writePinLow(mux_sel_pins[j]);
        }
    }
}

// TODO scale and map to calibrated implementation
uint16_t he_readkey_raw(uint8_t sensorIndex) {
    uint16_t sensor_value = 0;
    select_mux(sensorIndex);
    sensor_value = adc_read(adcMux);
    return sensor_value;
}

bool he_update_key(matrix_row_t* current_matrix, uint8_t row, uint8_t col, uint16_t sensor_value) {
    key_debounce_t *key_info = &debounce_matrix[row][col];
    bool previously_pressed = key_info->debounced_state;
    bool currently_pressed = sensor_value > he_config.he_actuation_threshold;
    bool should_release = sensor_value < he_config.he_release_threshold;

    if (currently_pressed && !previously_pressed) {
        if (++key_info->debounce_counter >= DEBOUNCE_THRESHOLD) {
            key_info->debounced_state = true; // Key is pressed
            current_matrix[row] |= (1UL << col);
            key_info->debounce_counter = 0;
            return true;
        }
    } else if (should_release && previously_pressed) {
        // Key release logic
        key_info->debounced_state = false; // Key is released
        current_matrix[row] &= ~(1UL << col);
        key_info->debounce_counter = 0;
        return true;
    } else {
        // Reset debounce counter if the state is stable
        key_info->debounce_counter = 0;
    }

    return false; // No change in stable state
}

// Optimize scan plis
bool he_matrix_scan(void) {
    bool updated = false;

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        uint8_t sensor_id = sensor_to_matrix_map[i].sensor_id;
        uint8_t row = sensor_to_matrix_map[i].row;
        uint8_t col = sensor_to_matrix_map[i].col;

        select_mux(sensor_id);
        uint16_t sensor_value = he_readkey_raw(sensor_id);

        if (he_update_key(matrix, row, col, sensor_value)) {
            updated = true;
        }
    }

    return updated;
}

// Debug stuff
sensor_data_t sensor_data[SENSOR_COUNT];

void add_sensor_sample(uint8_t sensor_id, uint16_t value) {
    sensor_data[sensor_id].samples[sensor_data[sensor_id].index % SAMPLE_COUNT] = value;
    sensor_data[sensor_id].index++;
}

double calculate_std_dev(uint8_t sensor_id) {
    double mean = 0.0;
    double std_dev = 0.0;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        mean += sensor_data[sensor_id].samples[i];
    }
    mean /= SAMPLE_COUNT;

    for (int i = 0; i < SAMPLE_COUNT; i++) {
        std_dev += pow(sensor_data[sensor_id].samples[i] - mean, 2);
    }
    std_dev = sqrt(std_dev / SAMPLE_COUNT);
    return std_dev;
}

double calculate_mean(uint8_t sensor_id) {
    double mean = 0.0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        mean += sensor_data[sensor_id].samples[i];
    }
    mean /= SAMPLE_COUNT;
    return mean;
}
void he_matrix_print(void) {
    print("+----------------------------------------------------------------------------+\n");
    print("| Sensor Matrix                                                              |\n");
    print("+----------------------------------------------------------------------------+\n");

    char buffer[256]; // Adjusted buffer size for additional content

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        uint8_t sensor_id = sensor_to_matrix_map[i].sensor_id;
        uint16_t sensor_value = he_readkey_raw(sensor_id);

        // Add current sensor value to samples
        add_sensor_sample(sensor_id, sensor_value);

        // Calculate mean and noise as standard deviation
        double mean = calculate_mean(sensor_id);
        double noise = calculate_std_dev(sensor_id);
        int noise_int = (int)(noise * 100); // Convert to integer representation for printing

        int mean_fixed = (int)(mean * 100); // Convert to fixed-point representation

        snprintf(buffer, sizeof(buffer),
                "| Sensor %d (%d,%d): Value: %-5u Act: %-5d Rel: %-5d Mean: %d.%02d Noise (std dev): %d.%02d |\n",
                sensor_id, sensor_to_matrix_map[i].row, sensor_to_matrix_map[i].col,
                sensor_value, he_config.he_actuation_threshold, he_config.he_release_threshold,
                mean_fixed / 100, mean_fixed % 100, // Display fixed-point mean as a floating-point value
                noise_int / 100, abs(noise_int) % 100); // Use abs() to ensure a positive value for the noise fractional part

        print(buffer);
    }

    print("+----------------------------------------------------------------------------+\n");
}
/*
void he_matrix_print(void) {
    const uint32_t num_scans = 100;
    uint32_t total_duration_ms = 0;

    // Perform the scanning process num_scans times
    for (uint32_t scan_count = 0; scan_count < num_scans; scan_count++) {
        uint32_t start_time = timer_read32(); // Start time for each scan

        // Scan logic
        for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
            uint8_t sensor_id = sensor_to_matrix_map[i].sensor_id;
            uint8_t row = sensor_to_matrix_map[i].row;
            uint8_t col = sensor_to_matrix_map[i].col;

            select_mux(sensor_id);
            uint16_t sensor_value = he_readkey_raw(sensor_id);
            // Update key state based on sensor_value
            he_update_key(matrix, row, col, sensor_value);
        }

        uint32_t end_time = timer_read32(); // End time for each scan
        total_duration_ms += (end_time - start_time); // Accumulate total scan duration
    }

    // Calculate the average scan duration in milliseconds
    float average_scan_time_ms = (float)total_duration_ms / num_scans;

    // Print the average scan duration
    char scan_buffer[128];
    snprintf(scan_buffer, sizeof(scan_buffer), "Average scan duration for %lu scans: %.2f ms\n", num_scans, average_scan_time_ms);
    print(scan_buffer);

    // Sensor data printing logic
    print("+----------------------------------------------------------------------------+\n");
    print("| Sensor Matrix                                                              |\n");
    print("+----------------------------------------------------------------------------+\n");

    char buffer[192]; // Buffer for printing sensor data

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        uint8_t sensor_id = sensor_to_matrix_map[i].sensor_id;
        uint16_t sensor_value = he_readkey_raw(sensor_id);

        // Add current sensor value to samples for noise calculation
        add_sensor_sample(sensor_id, sensor_value);

        // Calculate noise as the standard deviation
        double noise = calculate_std_dev(sensor_id);
        int noise_int = (int)(noise * 100); // Convert noise to an integer representation for printing

        // Format and print sensor data along with noise information
        snprintf(buffer, sizeof(buffer),
                 "| Sensor %d (%d,%d): Value: %-5u Act: %-5d Rel: %-5d Noise (std dev): %d.%02d |\n",
                 sensor_id, sensor_to_matrix_map[i].row, sensor_to_matrix_map[i].col,
                 sensor_value, he_config.he_actuation_threshold, he_config.he_release_threshold,
                 noise_int / 100, noise_int % 100);

        print(buffer);
    }

    print("+----------------------------------------------------------------------------+\n");
}

*/
