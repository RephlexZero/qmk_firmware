/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#include "matrix.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "quantum.h"
#include "custom_analog.h"
#include "lut.h"
#include "multiplexer.h"
#include "scanfunctions.h"
#include <ch.h>
#include <hal.h>
#include "gpio.h"

// External definitions
// Remove duplicate externs if already declared via other headers:
// extern const mux_t mux_index[MUXES][MUX_CHANNELS];
// extern ADCManager  adcManager;

analog_key_t    keys[MATRIX_ROWS][MATRIX_COLS] = {0};
static uint16_t pressedAdcValue                = 0;
static uint16_t restAdcValue                   = 0;

void matrix_init_custom(void) {
    gpio_set_pin_input_high(ENCODER_BUTTON_PIN);
    generate_lut();
    pressedAdcValue = distance_to_adc(255);
    restAdcValue    = distance_to_adc(0);
    multiplexer_init();
    initADCGroups();
    select_mux(0);
    adcStartAllConversions();
    wait_ms(100);
    get_sensor_offsets();
}

matrix_row_t previous_matrix[MATRIX_ROWS];

// Add the greycode conversion function
static inline uint8_t greycode(uint8_t channel) {
    return (channel >> 1) ^ channel;
}

// Modify process_adc_readings to accept a snapshot pointer.
static void process_adc_readings(matrix_row_t current_matrix[], uint8_t ch, const ADCManager *snapshot) {
    for (uint8_t mux = 0; mux < MUXES; ++mux) {
        const mux_t *mux_idx = &mux_index[mux][ch];
        if (mux_idx->row == 255 && mux_idx->col == 255) continue; // Skip unconnected mux pin.

        analog_key_t *key = &keys[mux_idx->row][mux_idx->col];
        key->raw          = getADCSample(snapshot, mux);
        key->value        = lut[key->raw + key->offset];

        switch (g_config.mode) {
            case dynamic_actuation:
                matrix_read_cols_dynamic_actuation(&current_matrix[mux_idx->row], mux_idx->col, key);
                break;
            case continuous_dynamic_actuation:
                matrix_read_cols_continuous_dynamic_actuation(&current_matrix[mux_idx->row], mux_idx->col, key);
                break;
            case static_actuation:
                matrix_read_cols_static_actuation(&current_matrix[mux_idx->row], mux_idx->col, key);
                break;
            case flashing:
            default:
                bootloader_jump();
                break;
        }
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    memcpy(previous_matrix, current_matrix, sizeof(previous_matrix));

    scanActive = true;
    // Start first ADC conversion and wait for its result.
    uint8_t current = greycode(0);
    waitForAdcConversion();
    ADCManager curr_snapshot = *getAdcManagerSnapshot();

    // Pipeline the ADC conversions.
    for (uint8_t ch = 1; ch < MUX_CHANNELS; ch++) {
        uint8_t next = greycode(ch);
        select_mux(next);
        process_adc_readings(current_matrix, current, &curr_snapshot);
        waitForAdcConversion();
        curr_snapshot = *getAdcManagerSnapshot();
        current       = next;
    }
    // Process the final conversion result.
    process_adc_readings(current_matrix, current, &curr_snapshot);
    select_mux(0);
#ifdef ENCODER_ENABLE
    bool encoder_button_pressed = gpio_read_pin(ENCODER_BUTTON_PIN);
    if (current_matrix[ENCODER_ROW] & (1 << ENCODER_COL)) {
        if (!encoder_button_pressed) {
            deregister_key(&current_matrix[ENCODER_ROW], ENCODER_COL);
        }
    } else {
        if (encoder_button_pressed) {
            register_key(&current_matrix[ENCODER_ROW], ENCODER_COL);
        }
    }
#endif
    scanActive = false;
    return memcmp(previous_matrix, current_matrix, sizeof(previous_matrix)) != 0;
}
