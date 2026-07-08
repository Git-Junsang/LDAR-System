// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_TURN_SIGNAL_H
#define LDAR_TURN_SIGNAL_H

typedef enum {
    TURN_OFF = 0,
    TURN_LEFT,
    TURN_RIGHT,
} TurnSignal_t;

void TurnSignal_Init(void);

/* 버튼 L/R 상승엣지(누름)로 토글. L·R 동시 점등 불가 — 새로 누른 쪽이 덮어씀.
 * 매 루프 1회 호출하고, 현재 래치된 방향지시 상태를 반환. */
TurnSignal_t TurnSignal_Update(void);

#endif
