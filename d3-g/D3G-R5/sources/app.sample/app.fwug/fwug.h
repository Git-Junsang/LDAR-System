/*
***************************************************************************************************
*
*   FileName : fwug.h
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

#ifndef FWUG_HEADER
#define FWUG_HEADER

#include "mbox.h"

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#if (DEBUG_ENABLE)
#define FWUG_D(fmt, args...)            {LOGD(DBG_TAG_FWUG, fmt, ## args)}
#define FWUG_E(fmt, args...)            {LOGE(DBG_TAG_FWUG, fmt, ## args)}
#else
#define FWUG_D(fmt, args...)
#define FWUG_E(fmt, args...)
#endif


#define FWUG_CMD_UPDATE_START           (0x01UL)

#define FWUG_TASK_STK_SIZE              (2048)

#define FWUG_SNOR_BASEADDR              (0x08000000UL)
#define FWUG_SRAM1_BASEADDR             (0x07000000UL)

#ifdef __GNU_C__
#define FWUG_JUMP_TO_ADDR               {__asm__ ("ldr pc, =0x07000200");}
#else
#define FWUG_JUMP_TO_ADDR               {asm("ldr pc, =0x07000200");}
#endif


typedef struct FWUGDev
{
    MBOXDvice_t *                       dDev;
} FWUGDev_t;

typedef struct FWUGMsg
{
    uint32                              mCmd[MBOX_CMD_FIFO_SIZE];
    uint32                              mData[MBOX_DATA_FIFO_SIZE];
} FWUGMsg_t;

/*
***************************************************************************************************
*                                             LOCAL VARIABLES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

extern void FWUG_CreateApp
(
    void
);

extern void FWUG_Start
(
    uint32                              uiCmd
);

#endif  /* FWUG_HEADER */

