/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include_next <mcuconf.h>

#undef STM32_ADC_USE_ADC1
#define STM32_ADC_USE_ADC1          TRUE

#undef STM32_ADC_USE_ADC2
#define STM32_ADC_USE_ADC2          TRUE

#undef STM32_ADC_USE_ADC4
#define STM32_ADC_USE_ADC4          TRUE

#undef STM32_ADC_ADC1_DMA_STREAM
#define STM32_ADC_ADC1_DMA_STREAM STM32_DMA_STREAM_ID(1, 1)
#undef STM32_ADC_ADC2_DMA_STREAM
#define STM32_ADC_ADC2_DMA_STREAM STM32_DMA_STREAM_ID(2, 1)
#undef STM32_ADC_ADC4_DMA_STREAM
#define STM32_ADC_ADC4_DMA_STREAM STM32_DMA_STREAM_ID(2, 2)

#ifdef OLED_ENABLE
#undef STM32_I2C_USE_I2C1
#define STM32_I2C_USE_I2C1          TRUE
#endif

#ifdef RGB_MATRIX_ENABLE
#undef STM32_PWM_USE_TIM16
#define STM32_PWM_USE_TIM16          TRUE
#endif

#ifdef AUDIO_ENABLE
#undef STM32_DAC_USE_DAC1_CH1
#define STM32_DAC_USE_DAC1_CH1      TRUE
#undef STM32_DAC_USE_DAC1_CH2
#define STM32_DAC_USE_DAC1_CH2      TRUE
#undef  STM32_GPT_USE_TIM6
#define STM32_GPT_USE_TIM6                  TRUE
#undef  STM32_GPT_USE_TIM7
#define STM32_GPT_USE_TIM7                  TRUE
#undef  STM32_GPT_USE_TIM8
#define STM32_GPT_USE_TIM8                  TRUE
#endif

// 8 MHz HSE
// #undef STM32_HSECLK
// #define STM32_HSECLK 8000000U
// #undef  STM32_PREDIV_VALUE
// #define STM32_PREDIV_VALUE          1
// #undef  STM32_PPRE2
// #define STM32_PPRE2                 STM32_PPRE2_DIV1

// 16 MHz HSE
#undef STM32_HSECLK
#define STM32_HSECLK 16000000U
#undef  STM32_PREDIV_VALUE
#define STM32_PREDIV_VALUE          2
#undef  STM32_PPRE2
#define STM32_PPRE2                 STM32_PPRE2_DIV1
