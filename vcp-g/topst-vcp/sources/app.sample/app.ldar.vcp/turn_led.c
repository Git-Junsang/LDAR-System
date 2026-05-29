// SPDX-License-Identifier: Apache-2.0

#include "turn_led.h"
#include "ldar_pins.h"

#include <gpio.h>

/* 50 Hz 폴링(LDAR_Run의 20ms sleep) 기준 25 tick = 500ms → 1 Hz 깜박임 */
#define BLINK_TOGGLE_TICKS  (25U)

void TurnLed_Init(void)
{
    GPIO_Config(LDAR_PIN_LED_TURN_L, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Config(LDAR_PIN_LED_TURN_R, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Set(LDAR_PIN_LED_TURN_L, 0);
    GPIO_Set(LDAR_PIN_LED_TURN_R, 0);
}

void TurnLed_Update(TurnSignal_t ts, uint32_t tick)
{
    uint32_t phase = ((tick / BLINK_TOGGLE_TICKS) & 1U);
    uint32_t lOn   = ((ts == TURN_LEFT)  && (phase != 0U)) ? 1U : 0U;
    uint32_t rOn   = ((ts == TURN_RIGHT) && (phase != 0U)) ? 1U : 0U;
    GPIO_Set(LDAR_PIN_LED_TURN_L, lOn);
    GPIO_Set(LDAR_PIN_LED_TURN_R, rOn);
}
