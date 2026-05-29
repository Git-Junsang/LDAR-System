// SPDX-License-Identifier: Apache-2.0

#include "joystick_sw.h"
#include "ldar_pins.h"
#include <gpio.h>

void JoystickSW_Init(void)
{
    GPIO_Config(LDAR_PIN_JOY_SW,
                GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_PULLDN);
}

uint32_t JoystickSW_Read(void)
{
    return (uint32_t)GPIO_Get(LDAR_PIN_JOY_SW);
}
