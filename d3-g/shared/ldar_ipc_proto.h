// SPDX-License-Identifier: Apache-2.0
/*
 * LDAR IPC packet format — R5 (CAN bridge) → A72 (listener).
 * Used by both d3-g/r5/sources/app.ldar.bridge/ and d3-g/a72/ldar_listener.c.
 *
 * Stream over /dev/tcc_ipc_micom (Linux side) / equivalent IPC channel (R5 side).
 * One packet per CAN frame received by R5. Fixed 13 bytes, packed.
 */

#ifndef LDAR_IPC_PROTO_H
#define LDAR_IPC_PROTO_H

#include <stdint.h>

#define LDAR_IPC_CHANNEL        (1U)
#define LDAR_IPC_DATA_MAX       (8U)

#define LDAR_CAN_ID_DRIVER_INPUT  (0x120U)   /* upstream from VCP-G */

typedef struct __attribute__((packed)) {
    uint32_t canId;                       /* CAN message ID */
    uint8_t  dataLen;                     /* 0 .. LDAR_IPC_DATA_MAX */
    uint8_t  data[LDAR_IPC_DATA_MAX];
} LdarIpcUpstream_t;

#endif
