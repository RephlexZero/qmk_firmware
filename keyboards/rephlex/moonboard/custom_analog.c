/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */

#include "custom_analog.h"
#include "print.h"
#include "multiplexer.h"
#include "stm32f303xc.h"

// Define the global ADC manager instance
ADCManager adcManager;

static void adcCompleteCallback(ADCDriver *adcp) {
    (void)adcp; // Unused parameter
    // Count completions across ADC1/2/4; signal once all have completed.
    // Safe: DMA ISRs at equal priority are serialized by the Cortex-M4 NVIC
    // (no preemption at same priority), and the reset to 0 in
    // adcStartAllConversions is done under chSysLock. Use == not >= to
    // avoid a double-signal if an ADC fires spuriously after the count hits ADC_GROUPS.
    adcManager.completedConversions++;
    if (adcManager.completedConversions == ADC_GROUPS) {
        chSemSignalI(&adcManager.sem);
    }
}

bool waitForAdcConversion(void) {
    // Wait until all ADC groups have completed the current conversion set.
    // Use a timeout to prevent hanging (e.g. 2ms, which is plenty for ADC)
    msg_t result = chSemWaitTimeout(&adcManager.sem, TIME_MS2I(2));
    if (result == MSG_TIMEOUT) {
        return false;
    }
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
    // Signal semaphore to prevent main loop hang
    adcManager.completedConversions++;
    if (adcManager.completedConversions == ADC_GROUPS) {
        chSemSignalI(&adcManager.sem);
    }
}

static const ADCConversionGroup adcConversionGroup = {.circular     = false, // Single-shot conversions per request
                                                      .num_channels = 2U,
                                                      .end_cb       = adcCompleteCallback,
                                                      .error_cb     = adcErrorCallback,
                                                      .cfgr         = ADC_RESOLUTION | ADC_CFGR_DMAEN, // Enable DMA
                                                      .tr1          = ADC_TR_DISABLED,
                                                      .tr2          = ADC_TR_DISABLED,
                                                      .tr3          = ADC_TR_DISABLED,
                                                      .awd2cr       = 0U,
                                                      .awd3cr       = 0U,
                                                      .smpr =
                                                          {
                                                              ADC_SMPR1_SMP_AN3(ADC_SAMPLING_TIME) | ADC_SMPR1_SMP_AN4(ADC_SAMPLING_TIME),
                                                          },
                                                      .sqr = {
                                                          ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN4),
                                                      }};

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
    chSysLock();
    // Prepare for a new conversion set
    adcManager.completedConversions = 0;
    select_mux(channel);

    // Start conversions on multiple ADCs using interrupt-driven starts.
    adcStartConversionI(&ADCD1, &adcConversionGroup, adcManager.sampleBuffer1, 1);
    adcStartConversionI(&ADCD2, &adcConversionGroup, adcManager.sampleBuffer2, 1);
    adcStartConversionI(&ADCD4, &adcConversionGroup, adcManager.sampleBuffer4, 1);
    chSysUnlock();

    return MSG_OK;
}

// Snapshot accessor.
const ADCManager *getAdcManagerSnapshot(void) {
    return &adcManager;
}
