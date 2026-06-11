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
    GPIO_Config(LDAR_PIN_LED_OVR_L,  GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Config(LDAR_PIN_LED_OVR_R,  GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Set(LDAR_PIN_LED_TURN_L, 0);
    GPIO_Set(LDAR_PIN_LED_TURN_R, 0);
    GPIO_Set(LDAR_PIN_LED_OVR_L,  0);
    GPIO_Set(LDAR_PIN_LED_OVR_R,  0);
}

void TurnLed_Update(TurnSignal_t turn, OverrideDir_t ovr, uint32_t tick)
{
    uint32_t phase = ((tick / BLINK_TOGGLE_TICKS) & 1U);

    /* 녹색 = 방향지시: 활성 쪽만 깜박임 (L·R 동시 점등은 상태머신이 이미 배제) */
    GPIO_Set(LDAR_PIN_LED_TURN_L, ((turn == TURN_LEFT)  && (phase != 0U)) ? 1U : 0U);
    GPIO_Set(LDAR_PIN_LED_TURN_R, ((turn == TURN_RIGHT) && (phase != 0U)) ? 1U : 0U);

    /* 적색 = 오버라이드 보상 방향: 점등(고정) */
    GPIO_Set(LDAR_PIN_LED_OVR_L, (ovr == OVR_LEFT)  ? 1U : 0U);
    GPIO_Set(LDAR_PIN_LED_OVR_R, (ovr == OVR_RIGHT) ? 1U : 0U);
}
