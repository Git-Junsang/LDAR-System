// SPDX-License-Identifier: Apache-2.0

#include "joystick_sw.h"
#include "ldar_pins.h"
#include <gpio.h>

void JoystickSW_Init(void)
{
    /* 아날로그 조이스틱 모듈 SW: 누르면 GND로 단락(active-low) → 풀업 필요. */
    GPIO_Config(LDAR_PIN_JOY_SW,
                GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_PULLUP);
}

uint32_t JoystickSW_Read(void)
{
    /* active-low: 눌림=0. 상위 로직은 1=눌림을 가정하므로 반전해 반환. */
    return (GPIO_Get(LDAR_PIN_JOY_SW) == 0) ? 1U : 0U;
}
