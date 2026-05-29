/*
***************************************************************************************************
*
*   FileName : i2c_reg.h
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

#include <i2c_reg.h>

/*                                                                                                 */
/***************************************************************************************************/
/*                                             LOCAL VARIABLES                                     */
/***************************************************************************************************/

I2CDev_t i2c[I2C_CH_NUM] =
{
    {
        400,                            /* I2C Clock                */
        I2C_CH0_BASE,                   /* Channel Base Address     */
        I2C_PCFG_BASE,                  /* I2C Port Configuration   */
        (uint32)CLOCK_PERI_I2C0,        /* Peri clock index         */
        (sint32)CLOCK_IOBUS_I2C0,       /* I/O Bus index            */
        NULL,                           /* SDA                      */
        NULL,                           /* SCL                      */
        NULL,                           /* Port number              */
        {0, },
        {0, },
        I2C_STATE_DISABLED              /* I2C State                */
    },
    {
        400,
        I2C_CH1_BASE,
        I2C_PCFG_BASE,
        (uint32)CLOCK_PERI_I2C1,
        (sint32)CLOCK_IOBUS_I2C1,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
    {
        400,
        I2C_CH2_BASE,
        I2C_PCFG_BASE,
        (uint32)CLOCK_PERI_I2C2,
        (sint32)CLOCK_IOBUS_I2C2,
        NULL,
        NULL,
        NULL,
        {0, },
        {0, },
        I2C_STATE_DISABLED
    },
};

const uint32 i2cport[I2C_PORT_NUM+1UL][3] =
{
    {NULL,               NULL,              NULL},
    {NULL,               NULL,              NULL},
    {NULL,               NULL,              NULL},
    {GPIO_GPA(4UL),      GPIO_GPA(5UL),     8UL},
    {GPIO_GPA(10UL),     GPIO_GPA(11UL),    8UL},
    {GPIO_GPA(16UL),     GPIO_GPA(17UL),    8UL},
    {GPIO_GPA(22UL),     GPIO_GPA(23UL),    8UL},
    {GPIO_GPA(28UL),     GPIO_GPA(29UL),    8UL},
    {GPIO_GPB(4UL),      GPIO_GPB(5UL),     8UL},
    {GPIO_GPB(10UL),     GPIO_GPB(11UL),    8UL},
    {GPIO_GPB(17UL),     GPIO_GPB(18UL),    8UL},
    {GPIO_GPB(19UL),     GPIO_GPB(20UL),    8UL},
    {GPIO_GPB(21UL),     GPIO_GPB(22UL),    8UL},
    {GPIO_GPB(23UL),     GPIO_GPB(24UL),    8UL},
    {GPIO_GPC(14UL),     GPIO_GPC(15UL),    8UL},
    {GPIO_GPC(20UL),     GPIO_GPC(21UL),    8UL},
    {GPIO_GPC(24UL),     GPIO_GPC(25UL),    8UL},
    {GPIO_GPC(28UL),     GPIO_GPC(29UL),    8UL},
    {GPIO_GPG(4UL),      GPIO_GPG(5UL),     8UL},
    {GPIO_GPE(3UL),      GPIO_GPE(4UL),     8UL},
    {GPIO_GPE(9UL),      GPIO_GPE(10UL),    8UL},
    {GPIO_GPE(14UL),     GPIO_GPE(15UL),    8UL},
    {GPIO_GPH(4UL),      GPIO_GPH(5UL),     9UL},
    {GPIO_GPH(6UL),      GPIO_GPH(7UL),     9UL},
    {GPIO_GPMA(4UL),     GPIO_GPMA(5UL),    8UL},
    {GPIO_GPMA(10UL),    GPIO_GPMA(11UL),   8UL},
    {GPIO_GPMA(16UL),    GPIO_GPMA(17UL),   8UL},
    {GPIO_GPMA(22UL),    GPIO_GPMA(23UL),   8UL},
    {GPIO_GPMA(28UL),    GPIO_GPMA(29UL),   8UL},
    {GPIO_GPMB(4UL),     GPIO_GPMB(5UL),    8UL},
    {GPIO_GPMB(10UL),    GPIO_GPMB(11UL),   8UL},    //30
    {GPIO_GPMB(16UL),    GPIO_GPMB(17UL),   8UL},
    {GPIO_GPMB(22UL),    GPIO_GPMB(23UL),   8UL},
    {GPIO_GPMB(26UL),    GPIO_GPMB(27UL),   8UL},
    {GPIO_GPMB(30UL),    GPIO_GPMB(31UL),   8UL},
    {GPIO_GPMC(4UL),     GPIO_GPMC(5UL),    8UL},    //35
    {GPIO_GPMC(10UL),    GPIO_GPMC(11UL),   8UL},
    {GPIO_GPMC(16UL),    GPIO_GPMC(17UL),   8UL},
    {GPIO_GPMC(22UL),    GPIO_GPMC(23UL),   8UL},
    {GPIO_GPMC(28UL),    GPIO_GPMC(29UL),   8UL}     //39
};

