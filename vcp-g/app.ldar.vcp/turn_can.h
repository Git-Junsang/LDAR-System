// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_TURN_CAN_H
#define LDAR_TURN_CAN_H

#include "turn_signal.h"

/* Sends CAN ID 0x120 (Driver Input — turn signal intent) when state changes.
 * Byte [0]: 0x00=off, 0x01=L, 0x02=R. CAN channel inited by CAN_DemoInitialize() in cmain. */
void TurnCan_Init(void);
void TurnCan_SendIfChanged(TurnSignal_t ts);

#endif
