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

const ADCManager *getAdcManagerSnapshot(void);

void initADCGroups(void);
msg_t adcStartAllConversions(uint8_t channel);
void adcErrorCallback(ADCDriver *adcp, adcerror_t err);
adcsample_t getADCSample(const ADCManager *adcManager, uint8_t muxIndex);
bool waitForAdcConversion(void);

#endif
