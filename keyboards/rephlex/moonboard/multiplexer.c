/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#include "quantum.h"
#include "multiplexer.h"

const pin_t mux_pins[MUXES] = MUX_PINS;
const pin_t mux_selector_pins[MUX_SELECTOR_BITS] = MUX_SELECTOR_PINS;

// Compile-time sanity checks
_Static_assert(MUX_CHANNELS == (1U << MUX_SELECTOR_BITS), "MUX_CHANNELS must equal (1 << MUX_SELECTOR_BITS)");
_Static_assert((sizeof mux_index / sizeof mux_index[0]) == MUXES, "mux_index first dimension must equal MUXES");
_Static_assert((sizeof mux_index[0] / sizeof mux_index[0][0]) == MUX_CHANNELS, "mux_index second dimension must equal MUX_CHANNELS");

// Atomic mux selector: pre-computed GPIOB BSRR values for all 16 channels.
// All 4 selector pins (B11, B10, B1, B2) are on GPIOB, so a single BSRR
// write updates them simultaneously with no intermediate glitch states.
// Bit mapping (matches MUX_SELECTOR_PINS order {B11, B10, B1, B2}):
//   channel bit 0 → PB11 (0x0800)
//   channel bit 1 → PB10 (0x0400)
//   channel bit 2 → PB1  (0x0002)
//   channel bit 3 → PB2  (0x0004)
// BSRR format: bits[15:0] = set HIGH, bits[31:16] = set LOW (write 0 = no effect)
static const uint16_t mux_pin_bits[MUX_SELECTOR_BITS] = {
    1U << 11, // PB11 (channel bit 0)
    1U << 10, // PB10 (channel bit 1)
    1U << 1,  // PB1  (channel bit 2)
    1U << 2,  // PB2  (channel bit 3)
};
static const uint16_t MUX_PORT_MASK = (1U << 11) | (1U << 10) | (1U << 1) | (1U << 2);
static uint32_t mux_bsrr[MUX_CHANNELS];

static void build_mux_bsrr_table(void) {
    for (uint8_t ch = 0; ch < MUX_CHANNELS; ch++) {
        uint16_t set_mask = 0;
        for (uint8_t i = 0; i < MUX_SELECTOR_BITS; i++) {
            if (ch & (1U << i)) set_mask |= mux_pin_bits[i];
        }
        mux_bsrr[ch] = (uint32_t)set_mask | ((uint32_t)(MUX_PORT_MASK & ~set_mask) << 16);
    }
}

void multiplexer_init(void) {
    // Initialize selector pins to output
    for (uint8_t i = 0; i < MUX_SELECTOR_BITS; i++) {
        setPinOutput(mux_selector_pins[i]);
    }

    build_mux_bsrr_table();

    // Drive all selector pins LOW atomically (channel 0)
    GPIOB->BSRR.W = mux_bsrr[0];
    current_channel = 0;

    // Small delay to ensure multiplexer settles
    wait_us(10);
}

bool select_mux(uint8_t channel) {
    if (channel >= MUX_CHANNELS) return false;
    // Atomically update all 4 selector pins in one BSRR write —
    // no intermediate mux states, no glitch sampling.
    GPIOB->BSRR.W = mux_bsrr[channel];
    current_channel = channel;
    return true;
}

const mux_t NC = {255,255}; // A coord with a Null pin (from JSON)
const mux_t mux_index[MUXES][MUX_CHANNELS] = {
    {{2,1},NC,{0,1},NC,{1,1},{1,0},{0,0},{2,0},{3,0},{4,0},NC,{5,0},NC,{4,1},{3,1},{5,1}},
    {{2,3},NC,{0,3},NC,{1,3},{1,2},{0,2},{2,2},NC,{3,2},NC,{4,2},NC,{5,2},{3,3},{4,3}},
    {{2,5},{1,6},{0,5},{2,6},{1,5},{1,4},{0,4},{2,4},{3,4},{4,4},NC,{4,5},NC,{5,3},{3,5},{4,6}},
    {{2,8},{1,9},{0,8},{2,9},{0,7},{0,6},{1,8},{1,7},{3,6},{3,7},{2,7},{4,7},NC,{4,8},{3,8},{4,9}},
    {{2,11},{1,12},{0,11},{0,9},{0,10},{1,10},{1,11},{2,10},{3,10},{5,4},{4,10},{5,5},{3,9},{4,11},{3,11},{5,6}},
    {{2,14},{3,13},NC,NC,{0,12},{2,12},{1,13},{2,13},{3,12},{5,7},{4,12},{4,13},NC,{5,8},{4,14},{5,9}}
};
