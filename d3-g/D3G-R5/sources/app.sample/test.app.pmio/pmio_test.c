/*
***************************************************************************************************
*
*   FileName : pmio_test.c
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

#include <reg_phys.h>
#include <errno.h>
#include <stdlib.h>
#include <debug.h>
#include <pmio.h>
#include "pmio_test.h"


/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/**************************************************************************************************/
/*                                             LOCAL VARIABLES                                    */
/**************************************************************************************************/
static PmioTestRegStruct_t PmioTestRegNameTable[PMIO_TEST_VA_REG_NUM] = {
    {    "BACKUP_RAM",  10U, (uint32)PMIO_TEST_VA_BACKUP_RAM    },
    {    "PMGPIO_DAT",  10U, (uint32)PMIO_TEST_VA_PMGPIO_DAT    },
    {    "PMGPIO_EN",   9,   (uint32)PMIO_TEST_VA_PMGPIO_EN     },
    {    "PMGPIO_FS",   9,   (uint32)PMIO_TEST_VA_PMGPIO_FS     },
    {    "PMGPIO_IEN",  10,  (uint32)PMIO_TEST_VA_PMGPIO_IEN    },
    {    "PMGPIO_PE",   9,   (uint32)PMIO_TEST_VA_PMGPIO_PE     },
    {    "PMGPIO_PS",   9,   (uint32)PMIO_TEST_VA_PMGPIO_PS     },
    {    "PMGPIO_CD0",  10,  (uint32)PMIO_TEST_VA_PMGPIO_CD0    },
    {    "PMGPIO_CD1",  10,  (uint32)PMIO_TEST_VA_PMGPIO_CD1    },
    {    "PMGPIO_EE0",  10,  (uint32)PMIO_TEST_VA_PMGPIO_EE0    },
    {    "PMGPIO_EE1",  10,  (uint32)PMIO_TEST_VA_PMGPIO_EE1    },
    {    "PMGPIO_CTL",  10,  (uint32)PMIO_TEST_VA_PMGPIO_CTL    },
    {    "PMGPIO_DI ",  9,   (uint32)PMIO_TEST_VA_PMGPIO_DI     },
    {    "PMGPIO_STR",  10,  (uint32)PMIO_TEST_VA_PMGPIO_STR    },
    {    "PMGPIO_STF",  10,  (uint32)PMIO_TEST_VA_PMGPIO_STF    },
    {    "PMGPIO_POL",  10,  (uint32)PMIO_TEST_VA_PMGPIO_POL    },
    {    "PMGPIO_PROT", 11,  (uint32)PMIO_TEST_VA_PMGPIO_PROT   },
    {    "PCTLIO_DAT",  10,  (uint32)PMIO_TEST_VA_PCTLIO_DAT    },
    {    "PCTLIO_EN",   9,   (uint32)PMIO_TEST_VA_PCTLIO_EN     },
    {    "PCTLIO_FS",   9,   (uint32)PMIO_TEST_VA_PCTLIO_FS     },
    {    "PCTLIO_IEN",  10,  (uint32)PMIO_TEST_VA_PCTLIO_IEN    },
    {    "PCTLIO_PE",   9,   (uint32)PMIO_TEST_VA_PCTLIO_PE     },
    {    "PCTLIO_PS",   9,   (uint32)PMIO_TEST_VA_PCTLIO_PS     },
    {    "PCTLIO_CD0",  10,  (uint32)PMIO_TEST_VA_PCTLIO_CD0    },
    {    "PCTLIO_CD1",  10,  (uint32)PMIO_TEST_VA_PCTLIO_CD1    },
    {    "PMGPIO_APB",  10,  (uint32)PMIO_TEST_VA_PMGPIO_APB    },
    {    "GP_K_FDAT",   9,   (uint32)PMIO_TEST_VA_GP_K_FDAT     },
    {    "GP_K_IRQST",  10,  (uint32)PMIO_TEST_VA_GP_K_IRQST    },
    {    "GP_K_IRQEN",  10,  (uint32)PMIO_TEST_VA_GP_K_IRQEN    },
    {    "GP_K_IRQPOL", 11,  (uint32)PMIO_TEST_VA_GP_K_IRQPOL   },
    {    "GP_K_IRQTM0", 11,  (uint32)PMIO_TEST_VA_GP_K_IRQTM0   },
    {    "GP_K_IRQTM1", 11,  (uint32)PMIO_TEST_VA_GP_K_IRQTM1   },
    {    "GP_K_FCK",    8,   (uint32)PMIO_TEST_VA_GP_K_FCK      },
    {    "GP_K_FBP",    8,   (uint32)PMIO_TEST_VA_GP_K_FBP      },
    {    "GP_K_CTL",    8,   (uint32)PMIO_TEST_VA_GP_K_CTL      }
};

static const PmioTestRegStruct_t PmioTestRegOffSetTable[PMIO_TEST_VA_REG_NUM] = {
    {    "000",  3,  (uint32)PMIO_TEST_VA_BACKUP_RAM    },
    {    "800",  3,  (uint32)PMIO_TEST_VA_PMGPIO_DAT    },
    {    "804",  3,  (uint32)PMIO_TEST_VA_PMGPIO_EN     },
    {    "808",  3,  (uint32)PMIO_TEST_VA_PMGPIO_FS     },
    {    "80c",  3,  (uint32)PMIO_TEST_VA_PMGPIO_IEN    },
    {    "810",  3,  (uint32)PMIO_TEST_VA_PMGPIO_PE     },
    {    "814",  3,  (uint32)PMIO_TEST_VA_PMGPIO_PS     },
    {    "818",  3,  (uint32)PMIO_TEST_VA_PMGPIO_CD0    },
    {    "81c",  3,  (uint32)PMIO_TEST_VA_PMGPIO_CD1    },
    {    "820",  3,  (uint32)PMIO_TEST_VA_PMGPIO_EE0    },
    {    "824",  3,  (uint32)PMIO_TEST_VA_PMGPIO_EE1    },
    {    "828",  3,  (uint32)PMIO_TEST_VA_PMGPIO_CTL    },
    {    "82c",  3,  (uint32)PMIO_TEST_VA_PMGPIO_DI     },
    {    "830",  3,  (uint32)PMIO_TEST_VA_PMGPIO_STR    },
    {    "834",  3,  (uint32)PMIO_TEST_VA_PMGPIO_STF    },
    {    "838",  3,  (uint32)PMIO_TEST_VA_PMGPIO_POL    },
    {    "83c",  3,  (uint32)PMIO_TEST_VA_PMGPIO_PROT   },
    {    "840",  3,  (uint32)PMIO_TEST_VA_PCTLIO_DAT    },
    {    "844",  3,  (uint32)PMIO_TEST_VA_PCTLIO_EN     },
    {    "848",  3,  (uint32)PMIO_TEST_VA_PCTLIO_FS     },
    {    "84c",  3,  (uint32)PMIO_TEST_VA_PCTLIO_IEN    },
    {    "850",  3,  (uint32)PMIO_TEST_VA_PCTLIO_PE     },
    {    "854",  3,  (uint32)PMIO_TEST_VA_PCTLIO_PS     },
    {    "858",  3,  (uint32)PMIO_TEST_VA_PCTLIO_CD0    },
    {    "85c",  3,  (uint32)PMIO_TEST_VA_PCTLIO_CD1    },
    {    "900",  3,  (uint32)PMIO_TEST_VA_PMGPIO_APB    },
    {    "a00",  3,  (uint32)PMIO_TEST_VA_GP_K_FDAT     },
    {    "a10",  3,  (uint32)PMIO_TEST_VA_GP_K_IRQST    },
    {    "a14",  3,  (uint32)PMIO_TEST_VA_GP_K_IRQEN    },
    {    "a18",  3,  (uint32)PMIO_TEST_VA_GP_K_IRQPOL   },
    {    "a1c",  3,  (uint32)PMIO_TEST_VA_GP_K_IRQTM0   },
    {    "a20",  3,  (uint32)PMIO_TEST_VA_GP_K_IRQTM1   },
    {    "a24",  3,  (uint32)PMIO_TEST_VA_GP_K_FCK      },
    {    "a28",  3,  (uint32)PMIO_TEST_VA_GP_K_FBP      },
    {    "a2c",  3,  (uint32)PMIO_TEST_VA_GP_K_CTL      }
#if 0
    {    (const uint8 *)"0x000",  5,  PMIO_TEST_VA_BACKUP_RAM    },
        {    (const uint8 *)"0x800",  5,  PMIO_TEST_VA_PMGPIO_DAT    },
        {    (const uint8 *)"0x804",  5,  PMIO_TEST_VA_PMGPIO_EN     },
        {    (const uint8 *)"0x808",  5,  PMIO_TEST_VA_PMGPIO_FS     },
        {    (const uint8 *)"0x80C",  5,  PMIO_TEST_VA_PMGPIO_IEN    },
        {    (const uint8 *)"0x810",  5,  PMIO_TEST_VA_PMGPIO_PE     },
        {    (const uint8 *)"0x814",  5,  PMIO_TEST_VA_PMGPIO_PS     },
        {    (const uint8 *)"0x818",  5,  PMIO_TEST_VA_PMGPIO_CD0    },
        {    (const uint8 *)"0x81C",  5,  PMIO_TEST_VA_PMGPIO_CD1    },
        {    (const uint8 *)"0x820",  5,  PMIO_TEST_VA_PMGPIO_EE0    },
        {    (const uint8 *)"0x824",  5,  PMIO_TEST_VA_PMGPIO_EE1    },
        {    (const uint8 *)"0x828",  5,  PMIO_TEST_VA_PMGPIO_CTL    },
        {    (const uint8 *)"0x82C",  5,  PMIO_TEST_VA_PMGPIO_DI     },
        {    (const uint8 *)"0x830",  5,  PMIO_TEST_VA_PMGPIO_STR    },
        {    (const uint8 *)"0x834",  5,  PMIO_TEST_VA_PMGPIO_STF    },
        {    (const uint8 *)"0x838",  5,  PMIO_TEST_VA_PMGPIO_POL    },
        {    (const uint8 *)"0x83C",  5,  PMIO_TEST_VA_PMGPIO_PROT   },
        {    (const uint8 *)"0x840",  5,  PMIO_TEST_VA_PCTLIO_DAT    },
        {    (const uint8 *)"0x844",  5,  PMIO_TEST_VA_PCTLIO_EN     },
        {    (const uint8 *)"0x848",  5,  PMIO_TEST_VA_PCTLIO_FS     },
        {    (const uint8 *)"0x84C",  5,  PMIO_TEST_VA_PCTLIO_IEN    },
        {    (const uint8 *)"0x850",  5,  PMIO_TEST_VA_PCTLIO_PE     },
        {    (const uint8 *)"0x854",  5,  PMIO_TEST_VA_PCTLIO_PS     },
        {    (const uint8 *)"0x858",  5,  PMIO_TEST_VA_PCTLIO_CD0    },
        {    (const uint8 *)"0x85C",  5,  PMIO_TEST_VA_PCTLIO_CD1    },
        {    (const uint8 *)"0x900",  5,  PMIO_TEST_VA_PMGPIO_APB    },
        {    (const uint8 *)"0xA00",  5,  PMIO_TEST_VA_GP_K_FDAT     },
        {    (const uint8 *)"0xA10",  5,  PMIO_TEST_VA_GP_K_IRQST    },
        {    (const uint8 *)"0xA14",  5,  PMIO_TEST_VA_GP_K_IRQEN    },
        {    (const uint8 *)"0xA18",  5,  PMIO_TEST_VA_GP_K_IRQPOL   },
        {    (const uint8 *)"0xA1C",  5,  PMIO_TEST_VA_GP_K_IRQTM0   },
        {    (const uint8 *)"0xA20",  5,  PMIO_TEST_VA_GP_K_IRQTM1   },
        {    (const uint8 *)"0xA24",  5,  PMIO_TEST_VA_GP_K_FCK      },
        {    (const uint8 *)"0xA28",  5,  PMIO_TEST_VA_GP_K_FBP      },
        {    (const uint8 *)"0xA2C",  5,  PMIO_TEST_VA_GP_K_CTL      }
#endif
};
/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/
static sint8 PMIO_TEST_Get
(
    uint32 uiAd,
    uint32 *uiVa
);
static sint8 PMIO_TEST_Set
(
    uint32 uiAd,
    uint32 uiVa
);
static uint32 PMIO_TEST_FindAddr
(
    uint8* pucRegName
);

/*
***************************************************************************************************
*                                         IMPLEMENT
***************************************************************************************************
*/

static sint8 PMIO_TEST_Get(uint32 uiAd, uint32 *uiVa)
{
    sint8 cRet;
    uint32 uiAddr;

    uiAddr = uiAd;

    if(uiAddr < PMIO_TEST_VA_BASE_ADDR)
    {
        uiAddr |= PMIO_TEST_VA_BASE_ADDR;
    }

    if((uiAddr & PMIO_TEST_VA_BASE_ADDR) == PMIO_TEST_VA_BASE_ADDR)
    {
        if(uiVa != NULL_PTR)
        {
            *uiVa = SAL_ReadReg(uiAddr);
            cRet = 0;
        }
        else
        {
            cRet = -1;
        }
    }
    else
    {
        cRet = -1;
    }
    return cRet;
}

static sint8 PMIO_TEST_Set(uint32 uiAd, uint32 uiVa)
{
    sint8 cRet;
    uint32 uiAddr;

    uiAddr = uiAd;

    if(uiAddr < PMIO_VA_BASE_ADDR)
    {
        uiAddr |= PMIO_VA_BASE_ADDR;
    }

    if((uiAddr & PMIO_VA_BASE_ADDR) == PMIO_VA_BASE_ADDR)
    {
        SAL_WriteReg(uiVa, uiAddr);
        cRet = 0;
    }
    else
    {
        cRet = -1;
    }
    return cRet;
}

static uint32 PMIO_TEST_FindAddr(uint8* pucRegName)
{
    uint32 uiAddr;
    uint32 uiIdx;
    sint32 ret;

    uiAddr = 0;

    for(uiIdx = 0UL; uiIdx < (uint32)PMIO_TEST_VA_REG_NUM; uiIdx++)
    {
        if ((SAL_StrNCmp(pucRegName, \
                         PmioTestRegNameTable[uiIdx].prtName,
                         PmioTestRegNameTable[uiIdx].prtSize,
                         &ret) == SAL_RET_SUCCESS)
            && (ret == 0))
        {
            uiAddr = PmioTestRegNameTable[uiIdx].prtAddr;
        }
    }

    if(uiAddr == 0UL)
    {
        for(uiIdx = 0UL; uiIdx < (uint32)PMIO_TEST_VA_REG_NUM; uiIdx++)
        {
            if ((SAL_StrNCmp(pucRegName, \
                             PmioTestRegOffSetTable[uiIdx].prtName, PmioTestRegOffSetTable[uiIdx].prtSize,
                             &ret) == SAL_RET_SUCCESS)
                && (ret == 0))
            {
                uiAddr = PmioTestRegOffSetTable[uiIdx].prtAddr;
            }
        }
    }
    return uiAddr;
}

uint32 PMIO_TEST_GetByAddress(uint32 uiAddr)
{
    uint32 uiVa;

    uiVa = 0;

    if( PMIO_TEST_Get(uiAddr, &uiVa) == 0)
    {
        mcu_printf("PMIO_TEST: read [x%08X] x%08X.\n", uiAddr, uiVa);
    }
    else
    {
        mcu_printf("PMIO_TEST: read [x%08X] WRONG address.\n", uiAddr);
    }
    return uiVa;
}

uint32 PMIO_TEST_GetByName(uint8* pucName)
{
    uint32 uiAddr;
    uint32 uiVa;

    uiAddr  = PMIO_TEST_FindAddr(pucName);
    uiVa    = 0;

    if(uiAddr != 0UL)
    {
        if( PMIO_TEST_Get(uiAddr, &uiVa) == 0)
        {
            mcu_printf("PMIO_TEST: read [x%08X] x%08X.\n", uiAddr, uiVa);
        }
        else
        {
            mcu_printf("PMIO_TEST: read [x%08X] WRONG address.\n", uiAddr);
        }
    }
    else
    {
        mcu_printf("PMIO_TEST: read [%s] WRONG name.\n", pucName);
    }

    return uiVa;
}

void PMIO_TEST_SetByAddress(uint32 uiAddr, uint32 uiVa)
{
    uint32 uiRva;

    uiRva = 0;

    if( PMIO_TEST_Set(uiAddr, uiVa) == 0 )
    {
        mcu_printf("PMIO_TEST: write [x%08X] done. \n", uiAddr);

        if( PMIO_TEST_Get(uiAddr, &uiRva) == 0)
        {
            mcu_printf("PMIO_TEST: write [x%08X] check: %s\n", uiAddr, \
                    ((uiRva==uiVa)?"Pass":"Fail") );
        }
    }
    else
    {
        mcu_printf("PMIO_TEST: write [x%08X] WRONG address.\n", uiAddr);
    }
}

void PMIO_TEST_SetByName(uint8* pucName, uint32 uiRegVa)
{
    uint32 uiAddr;
    uint32 uiVa;
    uint32 uiRva;

    uiAddr  = PMIO_TEST_FindAddr(pucName);
    uiVa    = uiRegVa;
    uiRva   = 0;

    if(uiAddr != 0UL)
    {
        if( PMIO_TEST_Set(uiAddr, uiVa) == 0 )
        {
            mcu_printf("PMIO_TEST: write [x%08X] x%08x. done. \n", uiAddr, uiVa);

            if( PMIO_TEST_Get(uiAddr, &uiRva) == 0)
            {
                mcu_printf("PMIO_TEST: write [x%08X] check: %s\n", uiAddr, \
                        ((uiRva==uiVa)?"Pass":"Fail") );
            }
        }
        else
        {
            mcu_printf("PMIO_TEST: write [x%08X] WRONG address.\n", uiAddr);
        }
    }
    else
    {
        mcu_printf("PMIO_TEST: write [%s] WRONG name.\n", pucName);
    }
}

void PMIO_TEST_PrintStatus(void)
{
    mcu_printf("PMIO_TEST: debug print power status\n");
    PMIO_DebugPrintPowerStatus();
}

void PMIO_TEST_Run(uint32 uiNum)
{
    switch(uiNum)
    {
        case 1:
        {
            PMIO_ForceSTRModeTest();
            break;
        }
        default:
        {
            mcu_printf("  pmio test 1 : PMIO_ForceSTRModeTest\n");
            break;
        }
    }
}

