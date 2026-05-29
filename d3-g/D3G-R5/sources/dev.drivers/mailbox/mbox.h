/*
***************************************************************************************************
*
*   FileName : mbox.h
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

#ifndef MBOX_CFG_H
#define MBOX_CFG_H

#define MBOX_CMD_FIFO_SIZE              (8UL)
#define MBOX_DATA_FIFO_SIZE             (128UL)

#define MBOX_MAX_QUEUE_SIZE             (4)
#define MBOX_MAX_MULTI_OPEN             (10UL)
#define MBOX_MAX_KEY_SIZE               (6)

#define MBOX_IO_CMD_INVALID             (-1)
#define MBOX_IO_CMD_SET_CTRL_REG        (0)
#define MBOX_IO_CMD_CLR_CTRL_REG        (1)
#define MBOX_IO_CMD_GET_ILVL_BIT        (2)
#define MBOX_IO_CMD_GET_INEN_BIT        (3)
#define MBOX_IO_CMD_GET_WOEN_BIT        (4)
#define MBOX_IO_CMD_GET_CFLS_BIT        (5)
#define MBOX_IO_CMD_GET_DFLS_BIT        (6)
#define MBOX_IO_CMD_GET_RCNT_CMD        (7)
#define MBOX_IO_CMD_GET_RFUL_CMD        (8)
#define MBOX_IO_CMD_GET_REMT_CMD        (9)
#define MBOX_IO_CMD_GET_WCNT_CMD        (10)
#define MBOX_IO_CMD_GET_WFUL_CMD        (11)
#define MBOX_IO_CMD_GET_WEMT_CMD        (12)
#define MBOX_IO_CMD_GET_RCNT_DAT        (13)
#define MBOX_IO_CMD_GET_RFUL_DAT        (14)
#define MBOX_IO_CMD_GET_REMT_DAT        (15)
#define MBOX_IO_CMD_GET_WCNT_DAT        (16)
#define MBOX_IO_CMD_GET_WFUL_DAT        (17)
#define MBOX_IO_CMD_GET_WEMT_DAT        (18)
#define MBOX_IO_CMD_GET_OWN_STS         (19)
#define MBOX_IO_CMD_GET_OPP_STS         (20)
#define MBOX_IO_CMD_MAX                 (21)

/*******************************************************************************
*    MailBox Error Define
********************************************************************************/
#define MBOX_SUCCESS                    ( 0)
#define MBOX_ERR_COMMON                 (-1)
#define MBOX_ERR_BUSY                   (-2)
#define MBOX_ERR_NOT_INITIALIZE         (-3)
#define MBOX_ERR_TIMEOUT                (-4)
#define MBOX_ERR_WRITE                  (-5)
#define MBOX_ERR_READ                   (-6)
#define MBOX_ERR_ARGUMENT               (-7)
#define MBOX_ERR_AGAIN                  (-8)
#define MBOX_ERR_QUEUE_FULL             (-9)
#define MBOX_ERR_QUEUE_EMPTY            (-10)
#define MBOX_ERR_NO_SUCH_DEV            (-11)
#define MBOX_ERR_FIFO_FULL              (-12)
#define MBOX_ERR_FIFO_EMPTY             (-13)
#define MBOX_ERR_FIFO_SIZE              (-14)
#define MBOX_ERR_MULTI_OPEN             (-15)
#define MBOX_ERR_DEV_NOT_OPENED         (-16)
#define MBOX_ERR_DEV_NOT_FOUND          (-17)
#define MBOX_ERR_OS_INIT                (-18)
#define MBOX_ERR_SIG_INIT               (-19)
#define MBOX_ERR_OPP_INIT               (-20)
#define MBOX_ERR_IOCTL                  (-21)

#define MBOX_DEV_ERR_NONE               (0)
#define MBOX_DEV_ERR_FULL               (-1)
#define MBOX_DEV_ERR_EXIST              (-2)
#define MBOX_DEV_ERR_INVALID_CH         (-3)
#define MBOX_DEV_ERR_HASH_MAP_FULL      (-4)
#define MBOX_DEV_ERR_INVALID_NAME       (-5)
#define MBOX_DEV_ERR_KEY_NOTINUSED      (-6)
#define MBOX_DEV_ERR_DEV_NULL           (-7)
#define MBOX_DEV_GET_AGAIN              (0xFFFFFFFFUL)


typedef enum MBOXCh
{
    MBOX_CH_INVAL                       = -1,
#if defined (MCU_BSP_BOARD_VER_TCC8050_EVM_0_1) || defined (MCU_BSP_BOARD_VER_TCC8059_EVM_0_1)
    MBOX_CH_CA53MP_SECURE,              // CR5 <-> CA53 With Secure Channel.
    MBOX_CH_CA53MP_NONSECURE,           // CR5 <-> CA53 With Non-Secure Channel.
    MBOX_CH_CA72MP_SECURE,              // CR5 <-> CA72 With Secure Channel.
    MBOX_CH_CA72MP_NONSECURE,           // CR5 <-> CA72 With Non-Secure Channel.
#if defined (MCU_BSP_SOC_REV_TCC805x_ES)
    MBOX_CH_CM4_NONSECURE,              // CR5 <-> CM4 With Non-Secure Channel.
#endif
    MBOX_CH_HSM_NONSECURE,              // CR5 <-> HSM With Non-Secure Channel.
#else
    #error board version not defined.
#endif
    MBOX_CH_MAX,
} MBOXCh_t;

typedef int32 (*MBOXInitFunc)
(
    uint32 *                            id
);

typedef int32 (*MBOXCallFunc)
(
    uint32                              id
);

typedef struct MBOXCallback
{
    MBOXInitFunc                        osInit;
    MBOXInitFunc                        rsInit;
    MBOXInitFunc                        wsInit;
    MBOXCallFunc                        wrLock;
    MBOXCallFunc                        wrUnlk;
    MBOXCallFunc                        rdWake;
    MBOXCallFunc                        rdWait;
    MBOXCallFunc                        wrWake;
    MBOXCallFunc                        wrWait;
    MBOXCallFunc                        wrWclr;
} MBOXCallback_t;

typedef struct MBOXMsage
{
    uint32                              mmDlen;
    uint32                              mmCmnd[MBOX_CMD_FIFO_SIZE];
    uint32                              mmData[MBOX_DATA_FIFO_SIZE];
} MBOXMsage_t;


typedef struct MBOXQueue
{
    int32                               qiFrnt;
    int32                               qiRear;
    MBOXMsage_t                         qiData[MBOX_MAX_QUEUE_SIZE];
} MBOXQueue_t;

typedef struct MBOXDvice
{
    MBOXCh_t                            dvChnl;
    int32                               dvIden;
    uint8                               dvFlag;
    uint32                              dvSiid;
    uint8                               dvName[MBOX_MAX_KEY_SIZE + 1];
    MBOXQueue_t *                       dvQueu;
} MBOXDvice_t;

typedef struct MBOXSpecialDev
{
    MBOXDvice_t*                        mboxDev;
} MBOXSpecialDev_t;

extern MBOXSpecialDev_t                 spMboxDev[];
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

void MBOX_Init
(
    void
);

int32 MBOX_DevInit
(
    MBOXCh_t                            siCh,
    uint32                              uiIlv,
    uint32                              ucOpn,
    MBOXCallback_t *                    pFunc
);

MBOXDvice_t* MBOX_DevOpen
(
    MBOXCh_t                            siCh,
    int8 *                              pcDevName,
    uint8                               ucFlag
);

int32 MBOX_DevSend
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData,
    uint32                              uiLen
);

int32 MBOX_DevSendSss
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData,
    uint32                              uiLen
);

int32 MBOX_DevSendCmd
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd
);

int32 MBOX_DevSendData
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiData,
    uint32                              uiLen
);

int32 MBOX_DevRecv
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData
);

int32 MBOX_DevRecvSss
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd,
    uint32 *                            puiData
);

int32 MBOX_DevRecvCmd
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiCmd
);

int32 MBOX_DevRecvData
(
    MBOXDvice_t *                       pDev,
    uint32 *                            puiData
);

int32 MBOX_DevIoctl
(
    MBOXDvice_t *                       pDev,
    uint32                              uiCmd,
    uint32 *                            puiValue
);
#endif // MBOX_CFG_H

