/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include_next <mcuconf.h>

#undef STM32_ADC_USE_ADC1
#define STM32_ADC_USE_ADC1 TRUE

#undef STM32_ADC_USE_ADC2
#define STM32_ADC_USE_ADC2 TRUE

#undef STM32_ADC_USE_ADC3
#define STM32_ADC_USE_ADC3 TRUE

#undef STM32_ADC_USE_ADC4
#define STM32_ADC_USE_ADC4 TRUE

#undef STM32_I2C_USE_I2C1
#define STM32_I2C_USE_I2C1 TRUE

#undef STM32_PWM_USE_ADVANCED
#define STM32_PWM_USE_ADVANCED TRUE

#undef STM32_PWM_USE_TIM1
#define STM32_PWM_USE_TIM1 TRUE

#undef STM32_DAC_USE_DAC1_CH1
#define STM32_DAC_USE_DAC1_CH1              TRUE
#undef STM32_DAC_USE_DAC1_CH2
#define STM32_DAC_USE_DAC1_CH2              TRUE
#undef STM32_GPT_USE_TIM6
#define STM32_GPT_USE_TIM6                  TRUE
