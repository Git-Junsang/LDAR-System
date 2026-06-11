// SPDX-License-Identifier: Apache-2.0
/*
 * D3-G R5 IPC→CAN 하향 브리지 구현.
 *
 * A72가 a72/Library/IPC_Library.py 의 IPC_SendPacketWithIPCHeader 로 보낸 교육용 패킷을
 * 파싱해 (canID, data)를 CAN으로 송신한다. 패킷 포맷(빅엔디안):
 *
 *   off  필드
 *   ---  --------------------------------------------------
 *    0   SYNC   0xFF
 *    1   START1 0x55
 *    2   START2 0xAA
 *   3-4  CMD1   0x0005 (TCC_IPC_CMD_CA72_EDUCATION_CAN_DEMO)
 *   5-6  CMD2   0x0001
 *   7-8  LENGTH = dataLen + 4
 *    9   channel_bitmask  (0x01 = CAN ch0)
 *   10   tx_only_bitmask  (0x00)
 *  11-12 canID (hi,lo)
 *  13..  data[dataLen]
 *   ...  CRC16 (hi,lo)  — SYNC부터 data 끝까지(=13+dataLen 바이트)에 대해 계산
 *
 *  예) 0x110 LIMIT 60 → FF 55 AA 00 05 00 01 00 06 01 00 01 10 01 3C <CRChi CRClo>
 */

#include "ldar_downstream.h"

#include <sal_api.h>
#include <debug.h>
#include <can_config.h>
#include <can_reg.h>
#include <can.h>

#define LDAR_DS_CAN_CH       (0U)      /* VCP-G LDAR_CAN_CH 와 동일 */
#define LDAR_DS_IPC_CHANNEL  (1U)      /* TODO: R5 BSP micom IPC 채널에 맞춰 교체 */
#define LDAR_DS_STK_SIZE     (1024U)
#define LDAR_DS_PRIO         (15U)
#define LDAR_DS_RXBUF        (128U)

/* 교육용 패킷 상수 (IPC_Library.py 와 일치) */
#define IPC_SYNC             (0xFFU)
#define IPC_START1           (0x55U)
#define IPC_START2           (0xAAU)
#define IPC_CMD1_CAN_DEMO    (0x0005U)
#define IPC_PREPARE          (9U)      /* SYNC..LENGTH = 9 바이트 */
#define IPC_CMD3_SIZE        (4U)      /* channel, tx_only, canID_hi, canID_lo */

/* --- IPC 수신 API 플레이스홀더 -------------------------------------------- */
/* TODO(user): R5 BSP의 실제 IPC 수신 호출로 교체. IPC 패킷 1개(SYNC..CRC)를
 * buf에 담아 바이트 수를 반환(없으면 0/음수). 교육용 CAN 데모를 켜면 이 경로 불필요. */
extern int Tcc_Ipc_Recv(uint32_t channel, void *buf, uint32_t maxLen);

/* CRC16-CCITT 테이블 (IPC_Library.py IPC_CalcCrc16 와 동일) */
static const uint16_t s_crc16Tab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static uint16_t LdarDs_Crc16(const uint8_t *p, uint32_t n)
{
    uint16_t crc = 0U;
    uint32_t i;
    for (i = 0U; i < n; i++) {
        uint8_t idx = (uint8_t)((crc >> 8) ^ p[i]);
        crc = (uint16_t)(s_crc16Tab[idx] ^ (uint16_t)(crc << 8));
    }
    return crc;
}

void LdarDownstream_OnIpcPacket(const uint8_t *buf, uint32_t len)
{
    uint16_t       cmd1;
    uint16_t       length;     /* = dataLen + 4 */
    uint32_t       pktSize;    /* SYNC..data 끝 (CRC 직전) */
    uint16_t       rxCrc;
    uint32_t       canId;
    uint8_t        dataLen;
    const uint8_t *data;
    CANMessage_t   tx;
    uint8_t        txIdx;
    CANErrorType_t r;
    uint8_t        i;

    if ((buf == NULL_PTR) || (len < (IPC_PREPARE + IPC_CMD3_SIZE + 2U))) {
        return;
    }
    if ((buf[0] != IPC_SYNC) || (buf[1] != IPC_START1) || (buf[2] != IPC_START2)) {
        return;
    }
    cmd1 = (uint16_t)(((uint16_t)buf[3] << 8) | buf[4]);
    if (cmd1 != IPC_CMD1_CAN_DEMO) {
        return;   /* CAN 데모 패킷 아님 — 무시 */
    }
    length = (uint16_t)(((uint16_t)buf[7] << 8) | buf[8]);
    if (length < IPC_CMD3_SIZE) {
        return;
    }
    pktSize = IPC_PREPARE + (uint32_t)length;          /* = 13 + dataLen */
    if ((pktSize + 2U) > len) {
        return;   /* CRC까지 다 안 들어옴 */
    }
    rxCrc = (uint16_t)(((uint16_t)buf[pktSize] << 8) | buf[pktSize + 1U]);
    if (LdarDs_Crc16(buf, pktSize) != rxCrc) {
        mcu_printf("\n[LDAR-DS] CRC mismatch (drop)");
        return;
    }

    /* uiCmd3 = [channel, tx_only, canID_hi, canID_lo], 그 뒤가 data */
    canId   = (uint32_t)(((uint32_t)buf[IPC_PREPARE + 2U] << 8) | buf[IPC_PREPARE + 3U]);
    dataLen = (uint8_t)(length - IPC_CMD3_SIZE);
    if (dataLen > 8U) {
        dataLen = 8U;
    }
    data = &buf[IPC_PREPARE + IPC_CMD3_SIZE];

    /* CAN 프레임 구성 (필드셋은 VCP-G turn_can.c 와 동일 — 같은 Telechips CAN 드라이버) */
    tx.mBufferType            = CAN_TX_BUFFER_TYPE_DBUFFER;
    tx.mBufferIndex           = 0U;
    tx.mErrorStateIndicator   = 0U;
    tx.mExtendedId            = 0U;
    tx.mRemoteTransmitRequest = 0U;
    tx.mId                    = canId;
    tx.mFDFormat              = 0U;
    tx.mBitRateSwitching      = 0U;
    tx.mMessageMarker         = 0U;
    tx.mEventFIFOControl      = 0U;
    tx.mDataLength            = dataLen;
    for (i = 0U; i < dataLen; i++) {
        tx.mData[i] = data[i];
    }

    txIdx = 0U;
    r = CAN_SendMessage(LDAR_DS_CAN_CH, &tx, &txIdx);
    if (r == CAN_ERROR_NONE) {
        mcu_printf("\n[LDAR-DS] IPC -> CAN 0x%X len=%d data[0]=0x%02X data[1]=0x%02X",
                   (int)canId, (int)dataLen,
                   (int)tx.mData[0], (int)((dataLen > 1U) ? tx.mData[1] : 0U));
    } else {
        mcu_printf("\n[LDAR-DS] CAN TX fail err=%d", (int)r);
    }
}

static void LdarDownstream_Task(void *pArg)
{
    static uint8_t buf[LDAR_DS_RXBUF];
    int            n;

    (void)pArg;

    while (1) {
        n = Tcc_Ipc_Recv(LDAR_DS_IPC_CHANNEL, buf, (uint32_t)LDAR_DS_RXBUF);
        if (n > 0) {
            LdarDownstream_OnIpcPacket(buf, (uint32_t)n);
        } else {
            (void)SAL_TaskSleep(5);
        }
    }
}

void LdarDownstream_Init(void)
{
    static uint32_t s_taskId = 0U;
    static uint32_t s_taskStk[LDAR_DS_STK_SIZE];

    SALRetCode_t err = (SALRetCode_t)SAL_TaskCreate(
        &s_taskId,
        (const uint8 *)"LdarDownstream",
        (SALTaskFunc)&LdarDownstream_Task,
        &s_taskStk[0],
        LDAR_DS_STK_SIZE,
        LDAR_DS_PRIO,
        NULL);

    if (err != SAL_RET_SUCCESS) {
        mcu_printf("\n[LDAR-DS] task create fail err=%d", (int)err);
    } else {
        mcu_printf("\n[LDAR-DS] downstream up (IPC ch=%d -> CAN ch=%d)",
                   (int)LDAR_DS_IPC_CHANNEL, (int)LDAR_DS_CAN_CH);
    }
}
