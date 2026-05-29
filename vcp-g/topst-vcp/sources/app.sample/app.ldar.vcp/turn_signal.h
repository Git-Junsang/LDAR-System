// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_TURN_SIGNAL_H
#define LDAR_TURN_SIGNAL_H

typedef enum {
    TURN_OFF = 0,
    TURN_LEFT,
    TURN_RIGHT,
} TurnSignal_t;

void TurnSignal_Init(void);
TurnSignal_t TurnSignal_Read(void);

#endif
