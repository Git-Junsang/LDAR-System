// SPDX-License-Identifier: Apache-2.0

#include "turn_signal.h"
#include "ldar_pins.h"
#include <gpio.h>
#include <stdint.h>

TurnSignal_t TurnSignal_Read(void)
{
    uint32_t l = (uint32_t)GPIO_Get(LDAR_PIN_TURN_L);
    uint32_t r = (uint32_t)GPIO_Get(LDAR_PIN_TURN_R);
    if ((l != 0U) && (r == 0U)) {
        return TURN_LEFT;
    }
    if ((r != 0U) && (l == 0U)) {
        return TURN_RIGHT;
    }
    return TURN_OFF;
}

void TurnSignal_Init(void)
{
    GPIO_Config(LDAR_PIN_TURN_L,
                GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_PULLDN);
    GPIO_Config(LDAR_PIN_TURN_R,
                GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_PULLDN);
}
