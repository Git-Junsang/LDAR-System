// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_TURN_LED_H
#define LDAR_TURN_LED_H

#include <stdint.h>
#include "turn_signal.h"
#include "override.h"

/* 상태 LED 4개:
 *  - 녹색 L/R : 방향지시(turn) 활성 쪽을 1Hz 깜박임
 *  - 적색 L/R : 속도 오버라이드 — LIMIT=양쪽 1Hz 깜박임, STOP=양쪽 점등, NONE=소등 */
void TurnLed_Init(void);
void TurnLed_Update(TurnSignal_t turn, OverrideMode_t mode, uint32_t tick);

#endif
