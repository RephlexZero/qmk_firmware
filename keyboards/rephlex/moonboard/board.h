/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include_next <board.h>

#define STM32_HSE_BYPASS

#undef STM32F303xC
#define STM32F303xE
