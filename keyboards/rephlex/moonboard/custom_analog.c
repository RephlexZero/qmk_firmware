/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */

#include "custom_analog.h"
#include "print.h"
#include "multiplexer.h"

// Define the global ADC manager instance
ADCManager adcManager;

static void adcCompleteCallback(ADCDriver *adcp) {
    (void)adcp; // Unused parameter
    chSemSignalI(&adcManager.sem);
}

bool waitForAdcConversion(void) {
    chSemWait(&adcManager.sem); // Wait for the semaphore to be signalled
    return true;
}

void adcErrorCallback(ADCDriver *adcp, adcerror_t err) {
    (void)adcp; // Unused parameter
    switch (err) {
        case ADC_ERR_DMAFAILURE:
            uprintf("ADC ERROR: DMA failure.\n");
            break;
        case ADC_ERR_OVERFLOW:
            uprintf("ADC ERROR: Overflow.\n");
            break;
        case ADC_ERR_AWD1:
            uprintf("ADC ERROR: Watchdog 1 triggered.\n");
            break;
        case ADC_ERR_AWD2:
            uprintf("ADC ERROR: Watchdog 2 triggered.\n");
            break;
        case ADC_ERR_AWD3:
            uprintf("ADC ERROR: Watchdog 3 triggered.\n");
            break;
        default:
            uprintf("ADC ERROR: Unknown error.\n");
            break;
    }
}

static const ADCConversionGroup adcConversionGroup = {
    .circular     = true,  // Enable circular mode for continuous DMA
    .num_channels = 2U,
    .end_cb       = adcCompleteCallback,
    .error_cb     = adcErrorCallback,
    .cfgr         = ADC_RESOLUTION | ADC_CFGR_DMAEN, // Enable DMA
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {
        ADC_SMPR1_SMP_AN3(ADC_SAMPLING_TIME) | ADC_SMPR1_SMP_AN4(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN4),
    }
};

void initADCGroups() {
    adcManager.completedConversions = 0;
    chSemObjectInit(&adcManager.sem, 0); // Initialize semaphore with a count of 0
    for (uint8_t i = 0; i < MUXES; i++) {
        palSetLineMode(mux_pins[i], PAL_MODE_INPUT_ANALOG);
    }
    adcStart(&ADCD1, NULL); // Start ADC1
    adcStart(&ADCD2, NULL); // Start ADC2
    adcStart(&ADCD4, NULL); // Start ADC4
}

msg_t adcStartAllConversions(uint8_t channel) {
    adcManager.completedConversions = 0;
    select_mux(channel);

    // Start conversions on multiple ADCs
    adcStartConversionI(&ADCD1, &adcConversionGroup, adcManager.sampleBuffer1, 1);
    adcStartConversionI(&ADCD2, &adcConversionGroup, adcManager.sampleBuffer2, 1);
    adcStartConversionI(&ADCD4, &adcConversionGroup, adcManager.sampleBuffer4, 1);

    return MSG_OK;
}

// Snapshot accessor.
const ADCManager *getAdcManagerSnapshot(void) {
    return &adcManager;
}
