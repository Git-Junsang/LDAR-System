// SPDX-License-Identifier: Apache-2.0

#include "turn_can.h"
#include "ldar_pins.h"

#include <can_config.h>
#include <can_reg.h>
#include <can.h>
#include <debug.h>
#include <stdint.h>

#define DRIVER_INPUT_OFF    (0x00U)
#define DRIVER_INPUT_LEFT   (0x01U)
#define DRIVER_INPUT_RIGHT  (0x02U)

static CANMessage_t s_msg;
static TurnSignal_t s_lastTs    = TURN_OFF;
static uint8_t      s_initSent  = 0U;

static uint8_t CodeFor(TurnSignal_t ts)
{
    switch (ts) {
        case TURN_LEFT:  return DRIVER_INPUT_LEFT;
        case TURN_RIGHT: return DRIVER_INPUT_RIGHT;
        case TURN_OFF:
        default:         return DRIVER_INPUT_OFF;
    }
}

void TurnCan_Init(void)
{
    s_msg.mBufferType            = CAN_TX_BUFFER_TYPE_DBUFFER;
    s_msg.mBufferIndex           = 0U;
    s_msg.mErrorStateIndicator   = 0U;
    s_msg.mExtendedId            = 0U;
    s_msg.mRemoteTransmitRequest = 0U;
    s_msg.mId                    = LDAR_CAN_ID_DRIVER_INPUT;
    s_msg.mFDFormat              = 0U;
    s_msg.mBitRateSwitching      = 0U;
    s_msg.mMessageMarker         = 0U;
    s_msg.mEventFIFOControl      = 0U;
    s_msg.mDataLength            = 1U;
    s_msg.mData[0]               = DRIVER_INPUT_OFF;
}

void TurnCan_SendIfChanged(TurnSignal_t ts)
{
    if ((s_initSent != 0U) && (ts == s_lastTs)) {
        return;
    }

    s_msg.mData[0] = CodeFor(ts);

    uint8_t        txIdx = 0U;
    CANErrorType_t r     = CAN_SendMessage(LDAR_CAN_CH, &s_msg, &txIdx);

    if (r == CAN_ERROR_NONE) {
        mcu_printf("\n[CAN] TX 0x%X data=%d", (int)LDAR_CAN_ID_DRIVER_INPUT, (int)s_msg.mData[0]);
    } else {
        mcu_printf("\n[CAN] TX fail err=%d", (int)r);
    }

    s_lastTs   = ts;
    s_initSent = 1U;
}
