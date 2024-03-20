#include "custom_analog.h"
#include "gpio.h"

#define ADC_SAMPLING_RATE ADC_SMPR_SMP_1P5

static ADCConfig   adcCfg = {};
static adcsample_t sampleBuffer[1];

static void adc_callback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
    (void)adcp;
    for(size_t i = 0; i < n; i++) {
        adcSample[i] = buffer[i];
    }
    conversion_done = true;
}

static ADCConversionGroup adcConversionGroup = {
    .circular     = FALSE,
    .num_channels = (uint16_t)(1),
    .cfgr = ADC_CFGR_CONT | ADC_CFGR_RES_12BITS,
    .smpr = {ADC_SMPR1_SMP_AN0(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN1(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN2(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN3(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN4(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN5(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN6(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN7(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN8(ADC_SAMPLING_RATE) | ADC_SMPR1_SMP_AN9(ADC_SAMPLING_RATE), ADC_SMPR2_SMP_AN10(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN11(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN12(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN13(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN14(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN15(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN16(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN17(ADC_SAMPLING_RATE) | ADC_SMPR2_SMP_AN18(ADC_SAMPLING_RATE)},
    .sqr = {ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3)},
    .end_cb = adc_callback,
    .error_cb = NULL,
};

void adc_init() {
    palSetLineMode(A2, PAL_MODE_INPUT_ANALOG);
    adcStart(&ADCD1, &adcCfg);
}

void adc_start() {
    conversion_done = false;
    adcStartConversionI(&ADCD1, &adcConversionGroup, sampleBuffer, 1);
}
