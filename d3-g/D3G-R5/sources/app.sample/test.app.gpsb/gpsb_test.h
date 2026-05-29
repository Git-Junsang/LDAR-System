/*
***************************************************************************************************
*
*   FileName : gpsb_test.h
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

#if !defined(GPSB_TEST_HEADER)

#define GPSB_TEST_HEADER

//#define PORT_TEST

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <gpsb_reg.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

#define GPSB_TEST_NUM                   (8UL)
#define DMA_BUF_SIZE                    (512UL)
#define GPSB_TEST_BUF_SIZE              (8UL)
#define SDM_GPSB_MAX_WRITE_SIZE         (128UL)


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static SALRetCode_t GPSB_SafetyTestProtocolMonitor
(
    uint8                               ucCh,
    uint8                               ucBpw
);

static SALRetCode_t GPSB_SafetyTestStuckMonitor
(
    uint8                               ucMstCh
);

static SALRetCode_t GPSB_SafetyTestLoopback
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh
);

static SALRetCode_t GPSB_SafetyTestWriteLock
(
    void
);

static SALRetCode_t GPSB_TestIpVerification
(
    void
);

static SALRetCode_t GPSB_TestSdm
(
    void
);

static SALRetCode_t GPSB_SdmExtTest
(
    void
);

static SALRetCode_t GPSB_SdmIntTest
(
    uint32                              uiSclk
);

static SALRetCode_t GPSB_InternalLoopbackTest
(
    uint8                               ucCh,
    uint8                               ucBpw,
    uint8                               ucXferMode,
    uint32                              uiSpeed
);

#ifdef PORT_TEST
SALRetCode_t GPSB_ExterrnalLoopbackTest
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh,
    uint32                              uiMode,
    uint8                               ucMPortIndex,
    uint8                               ucSPortIndex
);

#else
static SALRetCode_t GPSB_ExterrnalLoopbackTest
(
    uint8                               ucMstCh,
    uint8                               ucSlvCh,
    uint32                              uiMode
);
#endif
static SALRetCode_t GPSB_ExtDevTest
(
    void
);

void GPSB_Test
(
    void
);

#endif
