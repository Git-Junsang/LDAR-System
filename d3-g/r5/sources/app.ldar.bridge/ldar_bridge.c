// SPDX-License-Identifier: Apache-2.0
/*
 * D3-G R5 CAN→IPC bridge.
 *
 * Listens for CAN frames on the bus shared with VCP-G; when ID 0x120
 * (Driver Input) arrives, forwards (canId, dataLen, data[8]) to A72 over
 * the inter-processor mailbox (Telechips IPC, /dev/tcc_ipc_micom on the
 * Linux side).
 *
 * Build context: this file is meant to live at
 *   ~/d3-g-r5-bsp/sources/app.sample/app.ldar.bridge/ldar_bridge.c
 * inside the R5 BSP tree (TCC8050 R5 build env, see D02-T01). Drop the
 * matching rules.mk into the same dir, include it from app.sample/rules.mk,
 * and call LdarBridge_Init() from app.base/main.c's AppTaskCreate().
 *
 * NOTE — IPC send API placeholder. Replace `Tcc_Ipc_Send` with the actual
 * symbol exposed by the R5 BSP (e.g. IPC_Send / tcc_ipc_write / MBOX_*).
 * Reference: ~/Education/d3-g app/IPC_Example.py shows the A72-side usage
 * of channel=1 — match that channel here in LDAR_IPC_CHANNEL.
 */

#include "ldar_bridge.h"
#include "../../../../shared/ldar_ipc_proto.h"

#include <sal_api.h>
#include <debug.h>
#include <can_config.h>
#include <can_reg.h>
#include <can.h>

#define LDAR_BRIDGE_CAN_CH      (0U)         /* matches VCP-G LDAR_CAN_CH */
#define LDAR_BRIDGE_STK_SIZE    (1024U)
#define LDAR_BRIDGE_PRIO        (15U)        /* mid priority */

/* --- IPC API placeholder ----------------------------------------------- */
/* TODO(user): replace prototype + body with R5 BSP's real IPC send call. */
extern int Tcc_Ipc_Send(uint32_t channel, const void *data, uint32_t len);

static int IpcForward(const LdarIpcUpstream_t *pkt)
{
    return Tcc_Ipc_Send(LDAR_IPC_CHANNEL, pkt, (uint32_t)sizeof(*pkt));
}

/* --- bridge state ------------------------------------------------------ */

static volatile uint32_t s_rxPending = 0U;   /* set in ISR, cleared by task */
static uint32_t          s_taskId    = 0U;
static uint32_t          s_taskStk[LDAR_BRIDGE_STK_SIZE];

static void LdarBridge_RxCb(uint8 ucCh,
                            uint32 uiRxIndex,
                            CANMessageBufferType_t uiRxBufferType,
                            CANErrorType_t uiError)
{
    (void)uiRxBufferType;
    if ((uiError == CAN_ERROR_NONE) && (ucCh == LDAR_BRIDGE_CAN_CH)) {
        s_rxPending = uiRxIndex + 1UL;
    }
}

static void LdarBridge_Task(void *pArg)
{
    (void)pArg;
    CANMessage_t        rx;
    LdarIpcUpstream_t   pkt;

    while (1) {
        if (s_rxPending == 0U) {
            (void)SAL_TaskSleep(5);
            continue;
        }
        s_rxPending = 0U;

        while (CAN_CheckNewRxMessage(LDAR_BRIDGE_CAN_CH) != 0UL) {
            if (CAN_GetNewRxMessage(LDAR_BRIDGE_CAN_CH, &rx) != CAN_ERROR_NONE) {
                break;
            }
            if (rx.mId != LDAR_CAN_ID_DRIVER_INPUT) {
                continue;   /* not ours — ignore */
            }

            pkt.canId   = rx.mId;
            pkt.dataLen = (rx.mDataLength > LDAR_IPC_DATA_MAX)
                          ? LDAR_IPC_DATA_MAX : rx.mDataLength;
            for (uint8_t i = 0U; i < LDAR_IPC_DATA_MAX; i++) {
                pkt.data[i] = (i < pkt.dataLen) ? rx.mData[i] : 0U;
            }

            int ipcRet = IpcForward(&pkt);
            if (ipcRet != 0) {
                mcu_printf("\n[LDAR-BR] IPC send fail ret=%d", ipcRet);
            } else {
                mcu_printf("\n[LDAR-BR] CAN 0x%X -> IPC ch%d data[0]=0x%02X",
                           (int)pkt.canId, (int)LDAR_IPC_CHANNEL, (int)pkt.data[0]);
            }
        }
    }
}

void LdarBridge_Init(void)
{
    (void)CAN_RegisterCallbackFunctionRx(&LdarBridge_RxCb);

    SALRetCode_t err = (SALRetCode_t)SAL_TaskCreate(
        &s_taskId,
        (const uint8 *)"LdarBridge",
        (SALTaskFunc)&LdarBridge_Task,
        &s_taskStk[0],
        LDAR_BRIDGE_STK_SIZE,
        LDAR_BRIDGE_PRIO,
        NULL);

    if (err != SAL_RET_SUCCESS) {
        mcu_printf("\n[LDAR-BR] task create fail err=%d", (int)err);
    } else {
        mcu_printf("\n[LDAR-BR] bridge up (CAN ch=%d, IPC ch=%d, ID=0x%X)",
                   (int)LDAR_BRIDGE_CAN_CH, (int)LDAR_IPC_CHANNEL,
                   (int)LDAR_CAN_ID_DRIVER_INPUT);
    }
}
