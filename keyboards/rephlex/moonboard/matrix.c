/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
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
extern ADCManager  adcManager;
extern const mux_t mux_index[MUXES][MUX_CHANNELS];

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

// Add the greycode conversion function
static inline uint8_t greycode(uint8_t channel) {
    return (channel >> 1) ^ channel;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    memcpy(previous_matrix, current_matrix, sizeof(previous_matrix));
    static bool first_iteration = true;
    uint8_t prev_ch = 0;
    
    // Iterate over each multiplexer channel.
    for (uint8_t ch = 0; ch < MUX_CHANNELS; ++ch) {
        uint8_t grey_ch = greycode(ch);
        adcStartAllConversions(grey_ch);
        
        if (first_iteration) {
            // On the first iteration, wait for the conversion and store the channel.
            waitForAdcConversion();
            first_iteration = false;
            prev_ch = grey_ch;
            continue;
        }
        
        // Process ADC values from the previous conversion for each multiplexer.
        uint8_t sequence[MUXES] = {0, 2, 5, 1, 3, 4};
        for (uint8_t i = 0; i < MUXES; ++i) {
            uint8_t mux = sequence[i];
            const mux_t *mux_idx = &mux_index[mux][prev_ch];
            if (mux_idx->row == 255 && mux_idx->col == 255) {
            continue; // Skip non-connected mux pin.
            }
            
            analog_key_t *key = &keys[mux_idx->row][mux_idx->col];
            key->raw = getADCSample(mux);
            key->value = lut[key->raw + key->offset];
            
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
        
        // Wait for conversion before starting the next channel scan.
        waitForAdcConversion();
        prev_ch = grey_ch;
    }

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
    return memcmp(previous_matrix, current_matrix, sizeof(previous_matrix)) != 0;
}
