// SPDX-License-Identifier: Apache-2.0

#include "override_can.h"
#include "override.h"
#include "ldar_pins.h"

#include <can_config.h>
#include <can.h>
#include <debug.h>
#include <stdint.h>

/* 0x110 Speed Override — data[0] mode 코드 */
#define SPDOVR_MODE_RELEASE  (0x00U)
#define SPDOVR_MODE_LIMIT    (0x01U)
#define SPDOVR_MODE_STOP     (0x02U)

void OverrideCan_Init(void)
{
    /* CAN 초기화는 CAN_DemoInitialize()에서 완료 — 여기서 추가 작업 없음.
     * (필요 시 향후 LDAR 전용 RX 콜백/필터를 분리할 자리.) */
}

static void OverrideCan_Apply(const CANMessage_t *msg)
{
    if (msg->mId != (uint32_t)LDAR_CAN_ID_SPEED_OVERRIDE) {
        return;   /* 다른 ID는 무시(드레인만) */
    }
    if (msg->mDataLength < 1U) {
        return;
    }

    uint8_t mode  = msg->mData[0];
    uint8_t limit = (msg->mDataLength >= 2U) ? msg->mData[1] : 0U;

    switch (mode) {
        case SPDOVR_MODE_LIMIT:
            /* 표지판 한계속도(km/h)를 듀티% 상한으로 1:1 적용 (30→30%, 60→60%) */
            Override_SetLimit(limit);
            mcu_printf("\n[OVR] RX 0x%X LIMIT %d%%", (int)msg->mId, (int)limit);
            break;

        case SPDOVR_MODE_STOP:
            Override_SetStop();
            mcu_printf("\n[OVR] RX 0x%X STOP", (int)msg->mId);
            break;

        case SPDOVR_MODE_RELEASE:
        default:
            Override_Release();
            mcu_printf("\n[OVR] RX 0x%X RELEASE", (int)msg->mId);
            break;
    }
}

void OverrideCan_Poll(void)
{
    CANMessage_t rx;

    /* 링버퍼에 쌓인 수신 메시지를 모두 드레인 (없으면 즉시 반환). */
    while (CAN_CheckNewRxMessage(LDAR_CAN_CH) > 0U) {
        if (CAN_GetNewRxMessage(LDAR_CAN_CH, &rx) != CAN_ERROR_NONE) {
            break;
        }
        OverrideCan_Apply(&rx);
    }
}
