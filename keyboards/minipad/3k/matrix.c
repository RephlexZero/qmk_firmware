/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "print.h"
#include "quantum.h"
#include "analog.h"
#include "3k.h"
#include "lut.h"
#include "scanfunctions.h"

pin_t matrix_pins[MATRIX_ROWS][MATRIX_COLS] = MATRIX_PINS;
key_t keys[MATRIX_ROWS][MATRIX_COLS]        = {0};

void matrix_init_custom(void) {
    uint16_t rest_adc_value = ADC_RESOLUTION >> 1;

    generate_lut();
    rest_adc_value = distance_to_adc(0) + 1;

    get_sensor_offsets(rest_adc_value);
    wait_ms(100); // Let ADC reach steady state
    get_sensor_offsets(rest_adc_value);
}

enum analog_key_modes { dynamic_actuation = 0, continuous_dynamic_actuation, static_actuation, flashing };
matrix_row_t previous_matrix[MATRIX_ROWS];

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    memcpy(previous_matrix, current_matrix, sizeof(previous_matrix));

    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        for (uint8_t current_col = 0; current_col < MATRIX_COLS; current_col++) {
            key_t *key = &keys[current_row][current_col];
            key->value = lut[analogReadPin(matrix_pins[current_row][current_col]) + key->offset];
            key->value = min(key->value * CALIBRATION_RANGE / lut[1100 + key->offset], 255);

            switch (g_config.mode) {
                case dynamic_actuation:
                    matrix_read_cols_dynamic_actuation(&current_matrix[current_row], current_col, key);
                    break;
                case continuous_dynamic_actuation:
                    matrix_read_cols_continuous_dynamic_actuation(&current_matrix[current_row], current_col, key);
                    break;
                case static_actuation:
                    matrix_read_cols_static_actuation(&current_matrix[current_row], current_col, key);
                    break;
                case flashing:
                default:
                    bootloader_jump();
                    break;
            }
        }
    }
    return memcmp(previous_matrix, current_matrix, sizeof(previous_matrix)) != 0;
}