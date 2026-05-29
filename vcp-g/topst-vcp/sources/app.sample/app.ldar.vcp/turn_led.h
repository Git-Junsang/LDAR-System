// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_TURN_LED_H
#define LDAR_TURN_LED_H

#include <stdint.h>
#include "turn_signal.h"

/* 방향지시 LED 2개. ts == LEFT/RIGHT 인 동안 해당 LED를 1Hz로 깜박임. */
void TurnLed_Init(void);
void TurnLed_Update(TurnSignal_t ts, uint32_t tick);

#endif
