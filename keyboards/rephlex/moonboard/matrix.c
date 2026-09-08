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
    wait_ms(100);
    get_sensor_offsets();
}

matrix_row_t previous_matrix[MATRIX_ROWS];

typedef void (*scan_fn_t)(matrix_row_t *, uint8_t, analog_key_t *);

static void process_adc_readings(matrix_row_t current_matrix[], uint8_t ch, const ADCManager *snapshot) {
    // Resolve the scan function once before the loop — g_config.mode is
    // constant for the duration of a scan, so the switch runs once per
    // channel call (16×) instead of once per key (16×6=96×).
    scan_fn_t scan_fn;
    switch (g_config.mode) {
        case dynamic_actuation:            scan_fn = matrix_read_cols_dynamic_actuation;            break;
        case continuous_dynamic_actuation: scan_fn = matrix_read_cols_continuous_dynamic_actuation; break;
        case static_actuation:             scan_fn = matrix_read_cols_static_actuation;             break;
        case flashing:
        default:
            bootloader_jump();
            return;
    }

    for (uint8_t mux = 0; mux < MUXES; ++mux) {
        const mux_t *mux_idx = &mux_index[mux][ch];
        if (mux_idx->row == 255 && mux_idx->col == 255) continue; // Skip unconnected mux pin.

        analog_key_t *key = &keys[mux_idx->row][mux_idx->col];
        key->raw = getADCSample(snapshot, mux);

        // Clamp index before LUT access: raw (uint16) + offset (int16) can
        // exceed [0, ADC_RESOLUTION_MAX-1] when a key is at travel extremes.
        int32_t idx = (int32_t)key->raw + key->offset;
        key->value  = lut[MAX(MIN(idx, ADC_RESOLUTION_MAX - 1), 0)];

        scan_fn(&current_matrix[mux_idx->row], mux_idx->col, key);
    }
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    memcpy(previous_matrix, current_matrix, sizeof(previous_matrix));

    scanActive = true;

    // Kick off the first conversion on channel 0 (binary order).
    uint8_t     current       = 0;
    adcStartAllConversions(current);

    // Iterate remaining channels, pipelining conversions: wait -> snapshot -> process -> start next
    for (uint8_t ch = 1; ch < MUX_CHANNELS; ch++) {
        // Wait for the current conversion to finish
        waitForAdcConversion();

        // Snapshot results and process the current channel
        ADCManager curr_snapshot = *getAdcManagerSnapshot();
        process_adc_readings(current_matrix, current, &curr_snapshot);

        // Start the next channel conversion
        current = ch; // binary order
        adcStartAllConversions(current);
    }

    // Final channel: wait, snapshot, process
    waitForAdcConversion();
    ADCManager final_snapshot = *getAdcManagerSnapshot();
    process_adc_readings(current_matrix, current, &final_snapshot);

    // Return MUX to channel 0 (idle)
    select_mux(0);
#ifdef ENCODER_ENABLE
    bool encoder_button_pressed = !gpio_read_pin(ENCODER_BUTTON_PIN);
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
