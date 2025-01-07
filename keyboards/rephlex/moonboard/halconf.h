/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#define HAL_USE_ADC TRUE
#define ADC_USE_WAIT TRUE

#ifdef OLED_ENABLE
#define HAL_USE_I2C TRUE
#endif

#ifdef RGB_MATRIX_ENABLE
#define HAL_USE_PWM TRUE
#endif

#ifdef AUDIO_ENABLE
#define HAL_USE_DAC TRUE
#define HAL_USE_GPT TRUE
#endif

#include_next <halconf.h>
