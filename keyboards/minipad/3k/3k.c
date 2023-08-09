/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdint.h>
#include "3k.h"
#include "quantum.h"
#include "analog.h"
#include "eeprom.h"
#include "config.h"

extern pin_t matrix_pins[MATRIX_ROWS][MATRIX_COLS];
void         bootmagic_lite(void) {
    if (analogReadPin(matrix_pins[BOOTMAGIC_LITE_ROW][BOOTMAGIC_LITE_COLUMN] < 1350)) {
        bootloader_jump();
    }
}

#ifdef DEBUG_MATRIX
static uint8_t i = 0;
void           housekeeping_task_user(void) {
    if (i == 0) {
        uprintf("Mode:%d Actuation Point %d Press/Release sensitivity:%d/%d\n", g_config.mode, g_config.actuation_point, g_config.press_hysteresis, g_config.release_hysteresis);
        for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
            uprintf("\n");
            for (uint8_t j = 0; j < MATRIX_COLS; j++) {
                uprintf("%d/%d", keys[i][j].value, analogReadPin(matrix_pins[i][j]));
            }
            uprintf("\n");
        }
    }
    i++;
}
#endif

analog_config g_config = {
    .mode                = 1,
    .actuation_point     = 32,
    .press_sensitivity   = 32,
    .release_sensitivity = 32,
    .press_hysteresis    = 5,
    .release_hysteresis  = 5,
};

#ifdef VIA_ENABLE
void values_load(void) {
    eeprom_read_block(&g_config, ((void *)VIA_EEPROM_CUSTOM_CONFIG_ADDR), sizeof(g_config));
}

void values_save(void) {
    eeprom_update_block(&g_config, ((void *)VIA_EEPROM_CUSTOM_CONFIG_ADDR), sizeof(g_config));
}

void via_init_kb(void) {
    /* If the EEPROM has the magic, the data is good.
    OK to load from EEPROM */
    if (via_eeprom_is_valid()) {
        values_load();
    } else {
        values_save();
        /* DO NOT set EEPROM valid here, let caller do this */
    }
}

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    /* data = [ command_id, channel_id, value_id, value_data ] */
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
            case id_custom_set_value: {
                via_config_set_value(value_id_and_data);
                break;
            }
            case id_custom_get_value: {
                via_config_get_value(value_id_and_data);
                break;
            }
            case id_custom_save: {
                values_save();
                break;
            }
            default: {
                /* Unhandled message */
                *command_id = id_unhandled;
                break;
            }
        }
        return;
    }

    /* Return the unhandled state */
    *command_id = id_unhandled;

    /* DO NOT call raw_hid_send(data,length) here, let caller do this */
}

enum via_dynamic_actuation {
    id_mode = 1,
    id_actuation_point,
    id_press_sensitivity,
    id_release_sensitivity,
    id_press_hysteresis,
    id_release_hysteresis,
};

void via_config_set_value(uint8_t *data) {
    /* data = [ value_id, value_data ] */
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_mode:
            g_config.mode = *value_data;
            break;
        case id_actuation_point:
            g_config.actuation_point = *value_data * 255 / 40;
            break;
        case id_press_sensitivity:
            g_config.press_sensitivity = *value_data * 255 / 40;
            break;
        case id_release_sensitivity:
            g_config.release_sensitivity = *value_data * 255 / 40;
            break;
        case id_press_hysteresis:
            g_config.press_hysteresis = *value_data;
            break;
        case id_release_hysteresis:
            g_config.release_hysteresis = *value_data;
            break;
    }
}

void via_config_get_value(uint8_t *data) {
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_mode:
            *value_data = g_config.mode;
            break;
        case id_actuation_point:
            *value_data = g_config.actuation_point * 40 / 255;
            break;
        case id_press_sensitivity:
            *value_data = g_config.press_sensitivity * 40 / 255;
            break;
        case id_release_sensitivity:
            *value_data = g_config.release_sensitivity * 40 / 255;
            break;
        case id_press_hysteresis:
            *value_data = g_config.press_hysteresis;
            break;
        case id_release_hysteresis:
            *value_data = g_config.release_hysteresis;
            break;
    }
}
#endif