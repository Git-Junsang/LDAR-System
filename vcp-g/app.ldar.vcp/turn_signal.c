// SPDX-License-Identifier: Apache-2.0

#include "turn_signal.h"
#include "ldar_pins.h"
#include <gpio.h>
#include <debug.h>
#include <stdint.h>

static TurnSignal_t s_active = TURN_OFF;
static uint32_t     s_prevL  = 0U;
static uint32_t     s_prevR  = 0U;

void TurnSignal_Init(void)
{
    /* 버튼: 핀→버튼→GND (active-low) → 내부 풀업, 눌림=0. */
    GPIO_Config(LDAR_PIN_TURN_L,
                GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_PULLUP);
    GPIO_Config(LDAR_PIN_TURN_R,
                GPIO_FUNC(0) | GPIO_INPUT | GPIO_INPUTBUF_EN | GPIO_PULLUP);
    s_active = TURN_OFF;
    s_prevL  = 0U;
    s_prevR  = 0U;
}

TurnSignal_t TurnSignal_Update(void)
{
    /* active-low → 눌림=1로 정규화 */
    uint32_t     l    = (GPIO_Get(LDAR_PIN_TURN_L) == 0) ? 1U : 0U;
    uint32_t     r    = (GPIO_Get(LDAR_PIN_TURN_R) == 0) ? 1U : 0U;
    TurnSignal_t prev = s_active;

    /* 상승엣지에서만 토글 — 같은 쪽 다시 누르면 OFF, 반대쪽 누르면 덮어씀 */
    if ((l != 0U) && (s_prevL == 0U)) {
        s_active = (s_active == TURN_LEFT) ? TURN_OFF : TURN_LEFT;
    }
    if ((r != 0U) && (s_prevR == 0U)) {
        s_active = (s_active == TURN_RIGHT) ? TURN_OFF : TURN_RIGHT;
    }
    s_prevL = l;
    s_prevR = r;

    if (s_active != prev) {
        mcu_printf("\n[TURN] L=%d R=%d -> %s", (int)l, (int)r,
                   (s_active == TURN_LEFT) ? "LEFT" :
                   (s_active == TURN_RIGHT) ? "RIGHT" : "OFF");
    }

    return s_active;
}
