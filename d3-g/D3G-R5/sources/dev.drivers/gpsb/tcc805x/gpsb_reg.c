/*
***************************************************************************************************
*
*   FileName : gpsb_reg.c
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

#include <gpsb_reg.h>

/*                                                                                                 */
/***************************************************************************************************/
/*                                             LOCAL VARIABLES                                     */
/***************************************************************************************************/


GPSBDev_t gpsb[GPSB_CH_NUM + 1UL] =
{
    {
        0,                              /* GPSB channel */
        GPSB0_BASE,                     /* GPSB base address */
        GPSB_PCFG_BASE,                 /* GPSB Port Configuration address */
        (uint32)CLOCK_PERI_GPSB0,       /* Peri. clock number */
        (sint32)CLOCK_IOBUS_GPSB0,      /* IOBUS number */
        TRUE,                           /* GPSB mode (Master or Slave) */
        NULL,                           /* SDO */
        NULL,                           /* SDI */
        NULL,                           /* SCLK */
        NULL,                           /* PORT */
        GIC_PRIORITY_NO_MEAN,           /* Interrupt priority */
        FALSE,                          /* support dma */
        {NULL, },
        {NULL, },
        NULL_PTR,                       /* async pio tx buffer address */
        NULL_PTR,                       /* async pio rx buffer address */
        NULL,                           /* async pio tx buffer size */
        NULL,                           /* async pio rx buffer size */
        NULL,
        {NULL_PTR, },
    },
    {
        1,                              /* GPSB channel */
        GPSB1_BASE,                     /* GPSB base address */
        GPSB_PCFG_BASE,                 /* GPSB Port Configuration address */
        (uint32)CLOCK_PERI_GPSB1,       /* Peri.clock nmumber */
        (sint32)CLOCK_IOBUS_GPSB1,      /* IOBUS number */
        TRUE,                           /* GPSB mode (Master or Slave) */
        NULL,                           /* SDO */
        NULL,                           /* SDI */
        NULL,                           /* SCLK */
        NULL,                           /* PORT */
        GIC_PRIORITY_NO_MEAN,           /* Interrupt priority */
        FALSE,
        {NULL, },
        {NULL, },
        NULL_PTR,                       /* async pio tx buffer address */
        NULL_PTR,                       /* async pio rx buffer address */
        NULL,                           /* async pio tx buffer size */
        NULL,                           /* async pio rx buffer size */
        NULL,
        {NULL_PTR, },
    },
    {
        2,
        GPSB2_BASE,                     /* GPSB channel */
        GPSB_PCFG_BASE,                 /* GPSB Port Configuration address */
        (uint32)CLOCK_PERI_GPSB2,       /* GPSB base address */
        (sint32)CLOCK_IOBUS_GPSB2,      /* Peri.clock number */
        TRUE,                           /* GPSB mode (Master or Slave) */
        NULL,                           /* SDO */
        NULL,                           /* SDI */
        NULL,                           /* SCLK */
        NULL,                           /* PORT */
        GIC_PRIORITY_NO_MEAN,           /* Interrupt priority */
        FALSE,
        {NULL, },
        {NULL, },
        NULL_PTR,                       /* async pio tx buffer address */
        NULL_PTR,                       /* async pio rx buffer address */
        NULL,                           /* async pio tx buffer size */
        NULL,                           /* async pio rx buffer size */
        NULL,
        {NULL_PTR, },
    },
    {
        3,
        GPSB3_BASE,                     /* GPSB channel */
        GPSB_PCFG_BASE,                 /* GPSB Port Configuration address */
        (uint32)CLOCK_PERI_GPSB3,       /* GPSB base address */
        (sint32)CLOCK_IOBUS_GPSB3,      /* Peri.clock number */
        TRUE,                           /* GPSB mode (Master or Slave) */
        NULL,                           /* SDO */
        NULL,                           /* SDI */
        NULL,                           /* SCLK */
        NULL,                           /* PORT */
        GIC_PRIORITY_NO_MEAN,           /* Interrupt priority */
        FALSE,
        {NULL, },
        {NULL, },
        NULL_PTR,                       /* async pio tx buffer address */
        NULL_PTR,                       /* async pio rx buffer address */
        NULL,                           /* async pio tx buffer size */
        NULL,                           /* async pio rx buffer size */
        NULL,
        {NULL_PTR, },
    },
    {
        4,
        GPSB4_BASE,                     /* GPSB channel */
        GPSB_PCFG_BASE,                 /* GPSB Port Configuration address */
        (uint32)CLOCK_PERI_GPSB4,       /* GPSB base address */
        (sint32)CLOCK_IOBUS_GPSB4,      /* Peri.clock number */
        TRUE,                           /* mode (Master or Slave) */
        NULL,                           /* SDO */
        NULL,                           /* SDI */
        NULL,                           /* SCLK */
        NULL,                           /* PORT */
        GIC_PRIORITY_NO_MEAN,           /* Interrupt priority */
        FALSE,
        {NULL, },
        {NULL, },
        NULL_PTR,                       /* async pio tx buffer address */
        NULL_PTR,                       /* async pio rx buffer address */
        NULL,                           /* async pio tx buffer size */
        NULL,                           /* async pio rx buffer size */
        NULL,
        {NULL_PTR, },
    },
    {
        5,
        GPSB5_BASE,                     /* GPSB channel */
        GPSB_PCFG_BASE,                 /* GPSB Port Configuration address */
        (uint32)CLOCK_PERI_GPSB5,       /* GPSB base address */
        (sint32)CLOCK_IOBUS_GPSB5,      /* Peri.clock number */
        TRUE,                           /* GPSB mode (Master or Slave) */
        NULL,                           /* SDO */
        NULL,                           /* SDI */
        NULL,                           /* SCLK */
        NULL,                           /* PORT */
        GIC_PRIORITY_NO_MEAN,           /* Interrupt priority */
        FALSE,
        {NULL, },
        {NULL, },
        NULL_PTR,                       /* async pio tx buffer address */
        NULL_PTR,                       /* async pio rx buffer address */
        NULL,                           /* async pio tx buffer size */
        NULL,                           /* async pio rx buffer size */
        NULL,
        {NULL_PTR, },
    },
};

const uint32 gpsbport[GPSB_PORT_NUM][4] =
{
    {NULL           , NULL           , NULL           , NULL},
    {NULL           , NULL           , NULL           , NULL},
    {NULL           , NULL           , NULL           , NULL},
    {NULL           , NULL           , NULL           , NULL},
    {NULL           , NULL           , NULL           , NULL},

    {GPIO_GPA(0UL)  , GPIO_GPA(1UL)  , GPIO_GPA(2UL)  , GPIO_GPA(3UL)},
    {GPIO_GPA(6UL)  , GPIO_GPA(7UL)  , GPIO_GPA(8UL)  , GPIO_GPA(9UL)},
    {GPIO_GPA(12UL) , GPIO_GPA(13UL) , GPIO_GPA(14UL) , GPIO_GPA(15UL)},
    {GPIO_GPA(18UL) , GPIO_GPA(19UL) , GPIO_GPA(20UL) , GPIO_GPA(21UL)},
    {GPIO_GPA(24UL) , GPIO_GPA(25UL) , GPIO_GPA(26UL) , GPIO_GPA(27UL)},

    {GPIO_GPB(0UL)  , GPIO_GPB(1UL)  , GPIO_GPB(2UL)  , GPIO_GPB(3UL)},
    {GPIO_GPB(6UL)  , GPIO_GPB(7UL)  , GPIO_GPB(8UL)  , GPIO_GPB(9UL)},
    {GPIO_GPB(12UL) , GPIO_GPB(13UL) , GPIO_GPB(14UL) , GPIO_GPB(15UL)},
    {GPIO_GPB(19UL) , GPIO_GPB(20UL) , GPIO_GPB(21UL) , GPIO_GPB(22UL)},
    {GPIO_GPB(25UL) , GPIO_GPB(26UL) , GPIO_GPB(27UL) , GPIO_GPB(28UL)},

    {GPIO_GPC(0UL)  , GPIO_GPC(1UL)  , GPIO_GPC(2UL)  , GPIO_GPC(3UL)},
    {GPIO_GPC(4UL)  , GPIO_GPC(5UL)  , GPIO_GPC(6UL)  , GPIO_GPC(7UL)},
    {GPIO_GPC(12UL) , GPIO_GPC(13UL) , GPIO_GPC(14UL) , GPIO_GPC(15UL)},
    {GPIO_GPC(16UL) , GPIO_GPC(17UL) , GPIO_GPC(18UL) , GPIO_GPC(19UL)},
    {GPIO_GPC(20UL) , GPIO_GPC(21UL) , GPIO_GPC(22UL) , GPIO_GPC(23UL)},

    {GPIO_GPC(26UL) , GPIO_GPC(27UL) , GPIO_GPC(28UL) , GPIO_GPC(29UL)},
    {GPIO_GPG(0UL)  , GPIO_GPG(1UL)  , GPIO_GPG(2UL)  , GPIO_GPG(3UL)},
    {GPIO_GPG(7UL)  , GPIO_GPG(8UL)  , GPIO_GPG(9UL)  , GPIO_GPG(10UL)},
    {GPIO_GPE(5UL)  , GPIO_GPE(6UL)  , GPIO_GPE(7UL)  , GPIO_GPE(8UL)},
    {GPIO_GPE(11UL) , GPIO_GPE(12UL) , GPIO_GPE(13UL) , GPIO_GPE(14UL)},

    {GPIO_GPE(16UL) , GPIO_GPE(17UL) , GPIO_GPE(18UL) , GPIO_GPE(19UL)},
    {GPIO_GPH(4UL)  , GPIO_GPH(5UL)  , GPIO_GPH(6UL)  , GPIO_GPH(7UL)},
    {GPIO_GPMA(0UL) , GPIO_GPMA(1UL) , GPIO_GPMA(2UL) , GPIO_GPMA(3UL)},
    {GPIO_GPMA(6UL) , GPIO_GPMA(7UL) , GPIO_GPMA(8UL) , GPIO_GPMA(9UL)},
    {GPIO_GPMA(12UL), GPIO_GPMA(13UL), GPIO_GPMA(14UL), GPIO_GPMA(15UL)},

    {GPIO_GPMA(18UL), GPIO_GPMA(19UL), GPIO_GPMA(20UL), GPIO_GPMA(21UL)},
    {GPIO_GPMA(24UL), GPIO_GPMA(25UL), GPIO_GPMA(26UL), GPIO_GPMA(27UL)},
    {GPIO_GPMB(0UL) , GPIO_GPMB(1UL) , GPIO_GPMB(2UL) , GPIO_GPMB(3UL)},
    {GPIO_GPMB(6UL) , GPIO_GPMB(7UL) , GPIO_GPMB(8UL) , GPIO_GPMB(9UL)},
    {GPIO_GPMB(12UL), GPIO_GPMB(13UL), GPIO_GPMB(14UL), GPIO_GPMB(15UL)},

    {GPIO_GPMB(18UL), GPIO_GPMB(19UL), GPIO_GPMB(20UL), GPIO_GPMB(21UL)},
    {GPIO_GPMB(24UL), GPIO_GPMB(25UL), GPIO_GPMB(26UL), GPIO_GPMB(27UL)},
    {GPIO_GPMC(0UL) , GPIO_GPMC(1UL) , GPIO_GPMC(2UL) , GPIO_GPMC(3UL)},
    {GPIO_GPMC(6UL) , GPIO_GPMC(7UL) , GPIO_GPMC(8UL) , GPIO_GPMC(9UL)},
    {GPIO_GPMC(12UL), GPIO_GPMC(13UL), GPIO_GPMC(14UL), GPIO_GPMC(15UL)},

    {GPIO_GPMC(18UL), GPIO_GPMC(19UL), GPIO_GPMC(20UL), GPIO_GPMC(21UL)},
    {GPIO_GPMD(0UL) , GPIO_GPMD(1UL) , GPIO_GPMD(2UL) , GPIO_GPMD(3UL)},
    {GPIO_GPMD(12UL), GPIO_GPMD(13UL), GPIO_GPMD(14UL), GPIO_GPMD(15UL)},
};

const GPSBSafetyBase_t gpsb_sm[GPSB_CH_NUM + 1UL] =
{
    {
        (GPSB_IO_MONITOR_BASE + GPSB0_IOMON),
    },
    {
        (GPSB_IO_MONITOR_BASE + GPSB1_IOMON),
    },
    {
        (GPSB_IO_MONITOR_BASE + GPSB2_IOMON),
    },
    {
        (GPSB_IO_MONITOR_BASE + GPSB3_IOMON),
    },
    {
        (GPSB_IO_MONITOR_BASE + GPSB4_IOMON),
    },
    {
        (GPSB_IO_MONITOR_BASE + GPSB5_IOMON),
    }
};

