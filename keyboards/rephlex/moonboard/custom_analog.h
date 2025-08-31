/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef CUSTOM_ANALOG_H
#define CUSTOM_ANALOG_H

#include "hal.h"
#include "hal_adc_lld.h"

#define ADC_GROUPS 3

#define SAMPLE_BUFFER_SIZE 2

#define MUXES 6

// Type Definitions
typedef struct {
    adcsample_t sampleBuffer1[SAMPLE_BUFFER_SIZE];
    adcsample_t sampleBuffer2[SAMPLE_BUFFER_SIZE];
    adcsample_t sampleBuffer4[SAMPLE_BUFFER_SIZE];
    volatile int completedConversions;
    semaphore_t sem;
} ADCManager;

bool scanActive;

const ADCManager *getAdcManagerSnapshot(void);

void initADCGroups(void);
msg_t adcStartAllConversions(uint8_t channel);
void adcErrorCallback(ADCDriver *adcp, adcerror_t err);
static inline adcsample_t getADCSample(const ADCManager *m, uint8_t muxIndex) {
    if (muxIndex < 2)
        return m->sampleBuffer1[muxIndex];
    else if (muxIndex < 4)
        return m->sampleBuffer2[muxIndex - 2];
    else if (muxIndex < 6)
        return m->sampleBuffer4[1 - (muxIndex - 4)]; // swaps order for ADC4
    return 0;
}
bool waitForAdcConversion(void);

#endif
