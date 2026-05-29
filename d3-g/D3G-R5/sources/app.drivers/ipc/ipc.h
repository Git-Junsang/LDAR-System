/*
***************************************************************************************************
*
*   FileName : ipc.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*
*   TCC Version 1.0
*
*   This source code contains confidential information of Telechips.
*
*   Any unauthorized use without a written permission of Telechips including not limited to
*   re-distribution in source or binary form is strictly prohibited.
*
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty of
*   merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
*   or other third party intellectual property right. No warranty is made, express or implied,
*   regarding the information's accuracy,completeness, or performance.
*
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
*   Telechips and Company.
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty
*   (of merchantability, fitness for a particular purpose or non-infringement of any patent,
*   copyright or other third party intellectual property right. No warranty is made, express or
*   implied, regarding the information's accuracy, completeness, or performance.
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
*   between Telechips and Company.
*
***************************************************************************************************
*/

#ifndef TCC_IPC_H
#define TCC_IPC_H

#define IPC_SUCCESS                     (0)              /* Success */
#define IPC_ERR_COMMON                  (-1)             /* common error*/
#define IPC_ERR_BUSY                    (-2)             /* IPC is busy. You got the return, After a while you should try.*/
#define IPC_ERR_NOTREADY                (-3)             /* IPC is not ready. Other processor is not ready yet.*/
#define IPC_ERR_TIMEOUT                 (-4)             /* Other process is not responding. */
#define IPC_ERR_WRITE                   (-5)
#define IPC_ERR_READ                    (-6)
#define IPC_ERR_BUFFER                  (-7)
#define IPC_ERR_ARGUMENT                (-8)             /* Invalid argument */
#define IPC_ERR_RECEIVER_NOT_SET        (-9)             /* Receiver processor not set */
#define IPC_ERR_RECEIVER_DOWN           (-10)            /* Mbox is not set */
#define IPC_ERR_RECV_NACK               (-11)            /* AP sent NACK */
#define IPC_ERR_INVALID_DL              (-800)
#define IPC_ERR_NOT_SUPPORTED           (-900)

#define IPC_FLAG_USENAMCK               (0x00000001U)
#define IPC_NACK_REASON_BUFFFUL         (0x00000001U)
#define IPC_NACK_REASON_BUFFERR         (0x00000002U)
/*
***********************************************************************************
*   IPC CMD ID - Key
***********************************************************************************
*/
#define TCC_IPC_CMD_KEY                 (0x02) // define for CMD1
#define TCC_IPC_CMD_KEY_EXT             (0x01) // define for CMD2
#define TCC_IPC_CMD_KEY_ROTARY          (0x02) // define for CMD2
/*
**********************************************************************************
*   IPC CMD ID - System Monitoring
***********************************************************************************
*/
#define TCC_IPC_CMD_SYS_MONITORING      (0x04U) // define for CMD1
#define IPC_SYS_MON_RECOG_REQ           (0x01U) // define for CMD2 : (MICOM => Peer Core)
#define IPC_SYS_MON_RECOG_RES           (0x01U) // define for CMD2 : (Peer Core => MICOM)


#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
#define TCC_IPC_CMD_CAN_MESSAGE_ONE      (0x111U) // define for CMD1
#define TCC_IPC_CMD_CAN_SPEED_GAUGE      (0x0U) // 0 ~ 180
#define TCC_IPC_CMD_CAN_RPM_GAUGE        (0x1U) // 0 ~ 3000
#define TCC_IPC_CMD_CAN_TORQUE_GAUGE     (0x3U) // 0 ~ 2
#define TCC_IPC_CMD_CAN_FEUL_GAUGE       (0x4U) // 0 ~ 100
#define TCC_IPC_CMD_CAN_BATTERY_GAUGE    (0x5U) // 0 ~ 100

#define TCC_IPC_CMD_CAN_MESSAGE_TWO      (0x222U) //  define for CMD1
#define TCC_IPC_CMD_CAN_BREAK_SIGNAL      (0x0U) // 0~1
#define TCC_IPC_CMD_CAN_OVERHEAT_SIGNAL   (0x1U) // 0~1
#define TCC_IPC_CMD_CAN_ENGINE_SIGNAL     (0x2U) // 0~1
#define TCC_IPC_CMD_CAN_SEATBELT_SIGNAL   (0x3U) // 0~1
#define TCC_IPC_CMD_CAN_FEUL_SIGNAL       (0x4U) // 0~1
#define TCC_IPC_CMD_CAN_TURN_L_SIGNAL     (0x5U) // 0~1
#define TCC_IPC_CMD_CAN_TUNR_R_SIGNAL     (0x6U) // 0~1
#define TCC_IPC_CMD_CAN_GEAR_SIGNAL       (0x7U) // 1(P), 2(R), 3(N), 8(D)
#endif

#if defined(EDUCATION_CAN_DEMO_EN)
#define TCC_IPC_CMD_CA72_EDUCATION_CAN_DEMO           (0x05U) // CMD1 : EDUCATION_CAN
#define IPC_IPC_CMD_CA72_EDUCATION_CAN_DEMO_START     (0x01U) // CMD2 : START : CA72 TO MICOM 
#define IPC_IPC_CMD_CA72_EDUCATION_CAN_DEMO_STOP      (0x02U) // CMD2 : STOP  : CA72 TO MICOM
#define IPC_IPC_CMD_EDUCATION_CAN_DEMO_CA72_START     (0x03U) // CMD2 : START : MICOM TO CA72
#define IPC_IPC_CMD_EDUCATION_CAN_DEMO_CA72_STOP      (0x04U) // CMD2 : STOP  : MICOM TO CA72
#endif

typedef void (*IPCCallback)
(
    uint16                              uiCmd,
    uint8 *                             pucData,
    uint16                              uiLength
);

typedef enum IPCSvcCh
{
#if defined(TCC805x)
    IPC_SVC_CH_CA53_SECURE,
    IPC_SVC_CH_CA53_NONSECURE,
    IPC_SVC_CH_CA72_SECURE,
    IPC_SVC_CH_CA72_NONSECURE,
#else
    #error no target device designated.
#endif
    IPC_SVC_CH_MAX,
} IPCSvcCh_t;

typedef enum IPCIoCmd
{
    IOCTL_IPC_WRITE                     = 0,
    IOCTL_IPC_READ                      = 1,
    IOCTL_IPC_SET_PARAM                 = 2,
    IOCTL_IPC_GET_PARAM                 = 3,
    IOCTL_IPC_PING_TEST                 = 4,
    IOCTL_IPC_FLUSH                     = 5,
    IOCTL_IPC_ISREADY                   = 6,
} IPCIoCmd_t;


void IPC_Create
(
    void
);

void IPC_RegisterCbFunc
(
    IPCSvcCh_t                          siCh,
    uint8                               ucId,
    IPCCallback                         pFunc,
    void *                              pArg1,
    void *                              pArg2
);

int32 IPC_SendPacket
(
    IPCSvcCh_t                          siCh,
    uint16                              uiCmd1,
    uint16                              uiCmd2,
    const uint8 *                       pucData,
    uint16                              uiLength
);

int32 IPC_IsReady
(
    uint32                              uiCh,
    uint32 *                            uiIpcReady
);
#endif

