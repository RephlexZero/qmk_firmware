/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include_next <mcuconf.h>

#undef STM32_ADC_USE_ADC1
#define STM32_ADC_USE_ADC1          TRUE

#undef STM32_ADC_USE_ADC2
#define STM32_ADC_USE_ADC2          TRUE

// #undef STM32_ADC_USE_ADC3
// #define STM32_ADC_USE_ADC3          TRUE

#undef STM32_ADC_USE_ADC4
#define STM32_ADC_USE_ADC4          TRUE

#undef STM32_PWM_USE_TIM16
#define STM32_PWM_USE_TIM16          TRUE

// #define STM32_HSE_BYPASS
#undef STM32_HSECLK
#define STM32_HSECLK 16000000U
#undef  STM32_PREDIV_VALUE
#define STM32_PREDIV_VALUE          2
#undef  STM32_PPRE2
#define STM32_PPRE2                 STM32_PPRE2_DIV1
