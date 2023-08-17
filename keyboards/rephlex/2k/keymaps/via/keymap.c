/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┐
     * │ Z │ X │
     * └───┴───┘
     */
    [0] = LAYOUT_ortho_2x3(KC_Z, KC_X, KC_C, KC_GRV, KC_K, KC_ESC)
};
