// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_BRIDGE_H
#define LDAR_BRIDGE_H

/* Register CAN RX callback and spawn the bridge task.
 * Call from R5 AppTaskCreate() after CAN_Init() completes. */
void LdarBridge_Init(void);

#endif
