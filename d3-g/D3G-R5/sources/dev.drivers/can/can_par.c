/*
***************************************************************************************************
*
*   FileName : can_par.c
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

/**************************************************************************************************
*                                           INCLUDE FILES
**************************************************************************************************/

#include "can_config.h"
#include "can_reg.h"
#include "can.h"
#include "can_par.h"


/**************************************************************************************************
*                                            DEFINITIONS
**************************************************************************************************/


/**************************************************************************************************
*                                          LOCAL VARIABLES
**************************************************************************************************/

static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ];
static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ];


/**************************************************************************************************
*
*                                      CAN Timing Information
*
**************************************************************************************************/

#if !defined(EDUCATION_CAN_DEMO_EN)
/* Arbitration Phase Timing Information */
CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 0, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 },  //channel 1, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 }   //channel 2, speed : 500Kbps / sample point : 80%
};
#else
/* Arbitration Phase Timing Information */
CANTimingParam_t ArbitrationPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    /*** 20Kbps ***/
    // { 250,   95,     16,     16,     16,     0,      0,      0 },  // channel 0, speed : 20Kbps / sample point : 80%
    // { 250,   95,     16,     16,     16,     0,      0,      0 },  // channel 1, speed : 20Kbps / sample point : 80%
    // { 250,   95,     16,     16,     16,     0,      0,      0 }   // channel 2, speed : 20Kbps / sample point : 80%
    /*** 100Kbps ***/
    // { 20,    1,      15,     3,      1,      0,      0,      0 },  // channel 0, speed: 100 Kbps, sample point: 약 80%
    // { 20,    1,      15,     3,      1,      0,      0,      0 },  // channel 1, speed: 100 Kbps, sample point: 약 80%
    // { 20,    1,      15,     3,      1,      0,      0,      0 }   // channel 2, speed: 100 Kbps, sample point: 약 80%
    /*** 200Kbps ***/
    // { 10,    1,      15,     3,      1,      0,      0,      0 },  // channel 0, speed: 200 Kbps, sample point: 약 80%
    // { 10,    1,      15,     3,      1,      0,      0,      0 },  // channel 1, speed: 200 Kbps, sample point: 약 80%
    // { 10,    1,      15,     3,      1,      0,      0,      0 }   // channel 2, speed: 200 Kbps, sample point: 약 80%
    /*** 250Kbps ***/
    // { 8,     1,      15,     3,      1,      0,      0,      0 },  // channel 0, speed: 250 Kbps, sample point: 약 80%
    // { 8,     1,      15,     3,      1,      0,      0,      0 },  // channel 1, speed: 250 Kbps, sample point: 약 80%
    // { 8,     1,      15,     3,      1,      0,      0,      0 }   // channel 2, speed: 250 Kbps, sample point: 약 80%
    /*** 500Kbps ***/
    { 1,     47,     16,     16,     16,     0,      0,      0 },  // channel 0, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 },  // channel 1, speed : 500Kbps / sample point : 80%
    { 1,     47,     16,     16,     16,     0,      0,      0 }   // channel 2, speed : 500Kbps / sample point : 80%
};
#endif

/* Data Phase Timing Information */
CANTimingParam_t DataPhaseTimingPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* BRP,   PROP,   TSEG1,  TSEG2,  SJW,    TDC,    TDCO,   TDCF */
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 0, speed : 2Mbps / sample point : 70%
    { 1,     0,      13,     6,      6,      1,      14,     0 },  //channel 1, speed : 2Mbps / sample point : 70%
    { 1,     0,      13,     6,      6,      1,      14,     0 }   //channel 2, speed : 2Mbps / sample point : 70%
};


/**************************************************************************************************
*
*                                      CAN Buffer Information
*
**************************************************************************************************/

/* TX Buffer */
CANTxBuffer_t TxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Buffer count,  FIFO,QUEUE count, FIFO,QUEUE mode,          Buffer size,                Interrupt Config */
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL },
    { 16,            16,               CAN_TX_BUFFER_MODE_FIFO,  CAN_BUFFER_DATA_FIELD_64,   0xFFFFFFFFUL }
};

/* RX Dedicated Buffer */
CANRxBuffer_t RxBufferInfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Don't care,    Data Field Size,                Don't care */
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING },
    { 0,             CAN_BUFFER_DATA_FIELD_64,       CAN_RX_OP_BLOCKING }
};

/* RX FIFO 0 */
CANRxBuffer_t RxBufferFIFO0InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
};

/* RX FIFO 1 */
CANRxBuffer_t RxBufferFIFO1InfoPar[ CAN_CONTROLLER_NUMBER ] =
{
   /* Buffer count,  Data Field Size,            Operation mode */
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE },
    { 64,            CAN_BUFFER_DATA_FIELD_64,   CAN_RX_OP_OVERWRITE }
};


/**************************************************************************************************
*
*                                      CAN Standard ID Filter
*
**************************************************************************************************/

static CANIdFilterList_t StandardIDFilterPar_CH0[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
#if defined(EDUCATION_CAN_DEMO_EN)
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x000,      0x7FF }
#else
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#endif
};

static CANIdFilterList_t StandardIDFilterPar_CH1[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
#if defined(EDUCATION_CAN_DEMO_EN)
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x000,      0x7FF }
#else
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#endif
};

static CANIdFilterList_t StandardIDFilterPar_CH2[ CAN_STANDARD_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x000,      0x100 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x101,      0x200 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x201,      0x300 },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x301,      0x400 },
#if defined(EDUCATION_CAN_DEMO_EN)
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x000,      0x7FF }
#else
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x401,      0x0   },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x402,      0x7FF }
#endif
};

CANIdFilter_t StandardIDFilterPar =
{
    { &StandardIDFilterPar_CH0[ 0 ], &StandardIDFilterPar_CH1[ 0 ], &StandardIDFilterPar_CH2[ 0 ] },
};


/**************************************************************************************************
*
*                                      CAN Extended ID Filter
*
**************************************************************************************************/

static CANIdFilterList_t ExtendedIDFilterPar_CH0[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF }
};

static CANIdFilterList_t ExtendedIDFilterPar_CH1[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF }
};

static CANIdFilterList_t ExtendedIDFilterPar_CH2[ CAN_EXTENDED_ID_FILTER_NUMBER ] =
{
   /* FilterType,            FilterElementConfiguration,     FilterID1,  FilterID2 */
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO0,         0x0000,     0x1000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXFIFO1,         0x1001,     0x2000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO0,   0x2001,     0x3000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_PRY_N_RXFIFO1,   0x3001,     0x4000     },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_RXBUFFER,        0x4001,     0x0        },
    { CAN_FILTER_TYPE_RANGE, CAN_FILTER_CFG_REJECT_ID,       0x4002,     0x1FFFFFFF }
};

CANIdFilter_t ExtendedIDFilterPar =
{
    { &ExtendedIDFilterPar_CH0[ 0 ], &ExtendedIDFilterPar_CH1[ 0 ], &ExtendedIDFilterPar_CH2[ 0 ] },
};


/**************************************************************************************************
*
*                                      CAN Call Back Function
*
**************************************************************************************************/

CANCallBackFunc_t CANCallbackFunctions =
{
    NULL_PTR,  //cbNotifyRxEvent
    NULL_PTR,  //cbNotifyTxEvent
    NULL_PTR,  //cbNotifyErrorEvent
};


/**************************************************************************************************
*                                        FUNCTION PROTOTYPES
**************************************************************************************************/

