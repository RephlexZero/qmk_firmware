/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef CUSTOM_ANALOG_H
#define CUSTOM_ANALOG_H

#include "hal.h"
#include "hal_adc_lld.h"

// Constants
#define SAMPLE_BUFFER_SIZE 2  // Adjust as necessary
#define MUXES 6  // Number of multiplexer lines, adjust as necessary

// Type Definitions
typedef struct {
    adcsample_t sampleBuffer1[SAMPLE_BUFFER_SIZE];
    adcsample_t sampleBuffer2[SAMPLE_BUFFER_SIZE];
    adcsample_t sampleBuffer4[SAMPLE_BUFFER_SIZE];
    volatile int completedConversions;
    adcsample_t processingBuffer1[SAMPLE_BUFFER_SIZE];
    adcsample_t processingBuffer2[SAMPLE_BUFFER_SIZE];
    adcsample_t processingBuffer4[SAMPLE_BUFFER_SIZE];
    semaphore_t sem;
} ADCManager;

// Function Prototypes
void initADCGroups(void);
msg_t adcStartAllConversions(uint8_t channel);
void adcErrorCallback(ADCDriver *adcp, adcerror_t err);
adcsample_t getADCSample(uint8_t muxIndex);
bool waitForAdcConversion(void);

#endif // CUSTOM_ANALOG_H
