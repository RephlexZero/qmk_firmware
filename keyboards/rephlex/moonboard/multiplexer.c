/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#include "quantum.h"
#include "multiplexer.h"

const pin_t mux_pins[MUXES] = MUX_PINS;
const pin_t mux_selector_pins[MUX_SELECTOR_BITS] = MUX_SELECTOR_PINS;

void multiplexer_init(void) {
    for (uint8_t i = 0; i < MUX_SELECTOR_BITS; i++) {
        pin_t pin = mux_selector_pins[i];
        setPinOutput(pin);
    }
}

bool select_mux(uint8_t channel) {
    if (channel > MUX_CHANNELS) return 0;
    for (uint8_t i = 0; i < MUX_SELECTOR_BITS; i++) {
        writePin(mux_selector_pins[i], channel & (1 << i));
    }
    current_channel = channel;
    return 1;
}

const mux_t NC = {255,255}; // A coord with a Null pin (from JSON)
const mux_t mux_index[MUXES][MUX_CHANNELS] = {
    {{2,1},{1,2},{NC},{0,1},{1,1},{0,0},{1,0},{2,0},{3,0},{4,0},{5,0},{5,1},{4,1},{3,1},{NC},{NC}},
    {{2,3},{NC},{0,3},{NC},{1,3},{NC},{0,2},{2,2},{NC},{3,2},{5,2},{4,2},{4,3},{NC},{3,3},{NC}},
    {{2,5},{1,6},{0,5},{2,6},{1,5},{1,4},{0,4},{2,4},{3,4},{4,4},{5,3},{4,5},{NC},{NC},{3,5},{NC}},
    {{2,8},{1,9},{0,8},{2,9},{1,8},{0,6},{0,7},{2,7},{3,7},{3,6},{NC},{1,7},{NC},{4,6},{4,7},{3,8}},
    {{2,11},{1,12},{0,11},{0,9},{0,10},{1,10},{1,11},{2,10},{3,10},{4,8},{4,9},{5,4},{3,9},{4,10},{5,5},{3,11}},
    {{2,14},{NC},{1,14},{0,13},{0,12},{1,13},{2,13},{2,12},{3,12},{4,11},{5,6},{5,7},{4,12},{5,8},{4,13},{3,13}}
};
