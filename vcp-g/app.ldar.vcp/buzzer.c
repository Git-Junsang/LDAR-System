// SPDX-License-Identifier: Apache-2.0

#include "buzzer.h"
#include "ldar_pins.h"

#include <gpio.h>

static uint32_t s_last = 0xFFU;     /* sentinel — force first apply */

void Buzzer_Init(void)
{
    GPIO_Config(LDAR_PIN_BUZZER, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Set(LDAR_PIN_BUZZER, 0);
    s_last = 0U;
}

void Buzzer_Set(uint32_t on)
{
    uint32_t v = (on != 0U) ? 1U : 0U;
    if (v == s_last) {
        return;
    }
    GPIO_Set(LDAR_PIN_BUZZER, v);
    s_last = v;
}
