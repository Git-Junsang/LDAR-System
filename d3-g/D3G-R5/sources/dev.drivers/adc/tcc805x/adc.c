/*
***************************************************************************************************
*
*   FileName : adc.c
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

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/

#include <bsp.h>
#include <main.h>
#include "reg_phys.h"
#include "clock.h"
#include "clock_dev.h"
#include "adc.h"
#include "gic.h"

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/

/* Flags */
//#define ADC_DBG_ENABLED
//#define ADC_DMA_ENABLED
//#define ADC_TEST_ENABLED
//jbhyun: for tost
//#define ADC_AUTO_SCAN_ENABLED

/* Defines */
#define ADC_BASE_ADDR                   (0x1B700000UL)

#define ADC_REG                         ((SALReg32 *)(ADC_BASE_ADDR))
#define ADC_SWRESET_REG                 (0x1B930010UL) //24th bit

#define ADC_DMA_BAS_ADDR                (0x1B600000UL)
#define ADC_DMA_SREQ_REG                (ADC_DMA_BAS_ADDR + 0x24UL)
#define ADC_DMA_CONFIG_REG              (ADC_DMA_BAS_ADDR + 0x30UL)
#define ADC_DMA_SYNC_REG                (ADC_DMA_BAS_ADDR + 0x34UL)

#define ADC_DMA0_SRC_ADDR_REG           (ADC_DMA_BAS_ADDR + 0x100UL)
#define ADC_DMA0_DEST_ADDR_REG          (ADC_DMA_BAS_ADDR + 0x104UL)
#define ADC_DMA0_LLI_REG                (ADC_DMA_BAS_ADDR + 0x108UL)
#define ADC_DMA0_CONTROL_REG            (ADC_DMA_BAS_ADDR + 0x10CUL)
#define ADC_DMA0_CONFIG_REG             (ADC_DMA_BAS_ADDR + 0x110UL)

#define ADC_DMA_HIDDEN_REG              (0x1B93001CUL)

#define ADC_PERI_CLOCK                  (12000000UL)
#define ADC_CONV_CYCLE                  (30UL)

#define ADC_SMP_CMD_STS_REG             (0x00)
#define ADC_IRQ_DREQ_STS_REG            (0x01)
#define ADC_CTRL_CFG_REG                (0x02)
#define ADC_SDOUT0_REG                  (0x20)
#define ADC_SDOUT1_REG                  (0x21)
#define ADC_SDOUT2_REG                  (0x22)
#define ADC_SDOUT3_REG                  (0x23)
#define ADC_SDOUT4_REG                  (0x24)
#define ADC_SDOUT5_REG                  (0x25)
#define ADC_SDOUT6_REG                  (0x26)
#define ADC_SDOUT7_REG                  (0x27)
#define ADC_SDOUT8_REG                  (0x28)
#define ADC_SDOUT9_REG                  (0x29)
#define ADC_SDOUT10_REG                 (0x2A)
#define ADC_SDOUT11_REG                 (0x2B)

#define ADC_IRQ_CLR                     (1 << 0)
#define ADC_IRQ_EN                      (1 << 8)
#define ADC_DMA_EN                      (1 << 9)

#ifdef ADC_TEST_ENABLED
    /* Key definition*/
#define ADC_KEY_NULL                    (0x00)
#define ADC_KEY_1                       (0x01)
#define ADC_KEY_2                       (0x02)
#define ADC_KEY_3                       (0x03)
#define ADC_KEY_4                       (0x04)
#define ADC_KEY_5                       (0x05)
#define ADC_KEY_6                       (0x06)

#define ADC_KEY_ENTER                   (0x11)
#define ADC_KEY_SCAN                    (0x12)
#define ADC_KEY_SETUP                   (0x13)
#define ADC_KEY_MENU                    (0x14)
#define ADC_KEY_DISP                    (0x15)
#define ADC_KEY_TRACKDOWN               (0x16)
#define ADC_KEY_TRACKUP                 (0x17)
#define ADC_KEY_FOLDERDOWN              (0x18)
#define ADC_KEY_FOLDERUP                (0x19)
#define ADC_KEY_POWER                   (0x1a)
#define ADC_KEY_RADIO                   (0x1b)
#define ADC_KEY_MEDIA                   (0x1c)
#define ADC_KEY_PHONE                   (0x1d) //KEY_HANDSFREE
#define ADC_KEY_VOLUMEUP                (0x1e)
#define ADC_KEY_VOLUMEDOWN              (0x1f)

typedef struct ADCButton
{
    uint32                              btLower;
    uint32                              btUpper;
    uint16                              btVkcode;
    uint8                               btName[15];
} ADCButton_t;

typedef struct ADCButtonMap
{
    sint32                              bmSize;
    ADCButton_t *                       bmButton;
} ADCButtonMap_t;

/* Variables */
//uint32 ADCCompltedFlag = 0;   // removed for codesonar warning
#ifdef ADC_DMA_ENABLED
    uint32                              uiDMAData[100];
#endif

static struct ADCButton_t adc_btn_map_00[5] =
{
    /* ADC_CHANNEL_0 */
    {
        0x00,
        0x83,
        ADC_KEY_ENTER,
        "ENTER      "
    },
    {
        0x266,
        0x2EE,
        ADC_KEY_SCAN,
        "SCAN       "
    },
    {
        0x732,
        0x8CC,
        ADC_KEY_SETUP,
        "SETUP      "
    },
    {
        0x9E5,
        0xC18,
        ADC_KEY_MENU,
        "MENU       "
    },
    {
        0xD4A,
        0xFA0,
        ADC_KEY_DISP,
        "DISP       "
    },
};

static struct ADCButton_t adc_btn_map_01[4] =
{
    /* ADC_CHANNEL_1 */
    {
        0x6B,
        0x83,
        ADC_KEY_TRACKDOWN,
        "TRACK DOWN "
    },
    {
        0x732,
        0x8CC,
        ADC_KEY_TRACKUP,
        "TRACK UP   "
    },
    {
        0x9E5,
        0xC18,
        ADC_KEY_FOLDERDOWN,
        "FOLDER DOWN"
    },
    {
        0xD4A,
        0xFA0,
        ADC_KEY_FOLDERUP,
        "FOLDER UP  "
    },
};

static struct ADCButton_t adc_btn_map_02[4] =
{
    /* ADC_CHANNEL_2 */
    {
        0x00,
        0x83,
        ADC_KEY_POWER,
        "POWER      "
    },
    {
        0x732,
        0x8CC,
        ADC_KEY_RADIO,
        "RADIO      "
    },
    {
        0x9E5,
        0xC18,
        ADC_KEY_MEDIA,
        "MEDIA      "
    },
    {
        0xD4A,
        0xFA0,
        ADC_KEY_PHONE,
        "PHONE      "
    },
};

static struct ADCButton_t adc_btn_map_10[5] =
{
    /* ADC_CHANNEL_10 */
    {
        0x6B,
        0x83,
        ADC_KEY_1,
        "KEY1       "
    },
    {
        0x266,
        0x2EE,
        ADC_KEY_2,
        "KEY2       "
    },
    {
        0x52B,
        0x651,
        ADC_KEY_3,
        "KEY3       "
    },
    {
        0x732,
        0x8CC,
        ADC_KEY_4,
        "KEY4       "
    },
    {
        0x9E5,
        0xC18,
        ADC_KEY_5,
        "KEY5       "
    },
    {
        0xD4A,
        0xFA0,
        ADC_KEY_6,
        "KEY6       "
    },
};


struct ADCButtonMap_t adc_btn_map_ptr[12] =
{
    {
        SAL_ArraySize(adc_btn_map_00),
        adc_btn_map_00
    },
    {
        SAL_ArraySize(adc_btn_map_01),
        adc_btn_map_01
    },
    {
        SAL_ArraySize(adc_btn_map_02),
        adc_btn_map_02
    },
    {
        0,
        NULL
    },  // 03
    {
        0,
        NULL
    },  // 04
    {
        0,
        NULL
    },  // 05
    {
        0,
        NULL
    },  // 06
    {
        0,
        NULL
    },  // 07
    {
        0,
        NULL
    },  // 08
    {
        0,
        NULL
    },  // 09
    {
        SAL_ArraySize(adc_btn_map_10),
        adc_btn_map_10
    },
    {
        0,
        NULL
    }   // 11
};
#endif /* ADC_TEST_ENABLED */

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void ADC_Delay
(
    sint32                              iDelay
);

static void ADC_Isr
(
    void *                              pArg
);

static void ADC_SwresetSet
(
    void
);

static void ADC_SwresetClear
(
    void
);

static sint32 ADC_ClockEnable
(
    void
);

static sint32 ADC_ClockDisable
(
    void
);

static void ADC_StartMultiChannel
(
    uint32                              iMultiChannel,
    ADCMode_t                           iType
);

static uint32 ADC_CheckDoneBit
(
    void
);

static uint32 ADC_GetCVTCount
(
    void
);

#ifdef ADC_DMA_ENABLED
    static void ADC_SetDma
    (
        void
    );
#endif


/*
***************************************************************************************************
*                                          ADC_Delay
*
* @param    iDelay [in]
*
* Notes
*
***************************************************************************************************
*/

static void ADC_Delay
(
    sint32                              iDelay
)
{
    sint32 i;
    sint32 modi_delay;

    /* wait 1 cycle */
    modi_delay = iDelay * 50;   // 600/12 (CPU_CLOCK/ADC_PERI_CLOCK)

    for (i = 0 ; i < modi_delay ; i++)
    {
        BSP_NOP_DELAY();
    }
}

/*
***************************************************************************************************
*                                          ADC_Isr
*
* @param    pArg [in]
*
* Notes
*
***************************************************************************************************
*/

static void ADC_Isr
(
    void *                              pArg
)
{
    uint32 IRQStatus;

    (void)pArg;
    IRQStatus   = SAL_ReadReg(ADC_REG + ADC_IRQ_DREQ_STS_REG);
    IRQStatus   |= (uint32)0x01;
    SAL_WriteReg(IRQStatus, ADC_REG + ADC_IRQ_DREQ_STS_REG);
}

/*
***************************************************************************************************
*                                          ADC_ClockEnable
*
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/

static sint32 ADC_ClockEnable
(
    void
)
{
    sint32 ret;

#if 0
    ret = CLOCK_SetPeriRate((sint32)CLOCK_PERI_ADC, (uint32)ADC_PERI_CLOCK); //set clock
    if (ret != 0)
    {
        ADC_E("failed to enable adc clock\n");
    }
    else
    {
        ret = CLOCK_EnablePeri((sint32)CLOCK_PERI_ADC);
        if (ret != 0)
        {
            ADC_E("failed to enable adc clock\n");
        }
    }
#endif

    ret = CLOCK_SetPeriRate((sint32)CLOCK_PERI_ADC, (uint32)ADC_PERI_CLOCK); //set clock

    if (ret == 0)
    {
        (void)CLOCK_EnablePeri((sint32)CLOCK_PERI_ADC);
    }
    else
    {
        ADC_E("failed to enable adc clock\n");
    }

    return ret;
}

/*
***************************************************************************************************
*                                          ADC_ClockDisable
*
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/

static sint32 ADC_ClockDisable
(
    void
)
{
    sint32 ret;

    ret = CLOCK_DisablePeri((sint32)CLOCK_PERI_ADC);

    return ret;
}


/*
***************************************************************************************************
*                                          ADC_CheckDoneBit
*
*
*
* Notes
*
***************************************************************************************************
*/

static uint32 ADC_CheckDoneBit
(
    void
)
{
    uint32 ret;

    ret = ((SAL_ReadReg(ADC_REG + ADC_SMP_CMD_STS_REG) & (uint32)(0x80000000U)) >> (uint32)31);

    return ret;
}



#ifdef ADC_DMA_ENABLED

/*
***************************************************************************************************
*                                          ADC_GetCVTCount
*
* @return
*
* Notes
*
***************************************************************************************************
*/

static uint32 ADC_GetCVTCount
(
    void
)
{
    uint32 ret;

    ret = (SAL_ReadReg(ADC_REG + ADC_SMP_CMD_STS_REG) & (uint32)(0x1f00000));//CVT_CNT

    return ret;
}


/*
***************************************************************************************************
*                                          ADC_StartMultiChannel
*
*
* @param    iMultiChannel [in]
* @param    iType [in]
*
* Notes
*
***************************************************************************************************
*/

static void ADC_StartMultiChannel
(
    uint32                              iMultiChannel,
    ADCMode_t                           iType
)
{
    if(iMultiChannel != (uint32)0)
    {
        SAL_WriteReg(((iMultiChannel) & (uint32)0x1FF), ADC_REG + ADC_SMP_CMD_STS_REG);

        if(iType == ADC_MODE_IRQ)
        {
            //HwMC_ADC->uADC_CFG.bReg.IRQ_EN = 1;
            SAL_WriteReg((SAL_ReadReg(ADC_REG + ADC_CTRL_CFG_REG) | (uint32)(0x100)), (ADC_REG + ADC_CTRL_CFG_REG));//IRQ_EN
        }

#ifdef ADC_DMA_ENABLED
        if(iType == ADC_MODE_DMA)
        {
            ADC_SetDma();
            //HwMC_ADC->uADC_CFG.bReg.DREQ_EN= 1;
            SAL_WriteReg((SAL_ReadReg(ADC_REG + ADC_CTRL_CFG_REG) | (uint32)(0x200)), (ADC_REG + ADC_CTRL_CFG_REG));//DREQ_EN
        }
#endif
    }
}


/*
***************************************************************************************************
*                                          ADC_SetDma
*
* @return
*
* Notes
*
***************************************************************************************************
*/

void ADC_SetDma
(
    void
)
{
    SAL_WriteReg((SAL_ReadReg(DMA_CONFIG_REG) |= (1 << 0)), DMA_CONFIG_REG);  //DMA Enable
    //  DMA_SREQ_REG |= (1 << 12);
    //  DMA_SYNC_REG |= (1 << 12);
    SAL_WriteReg(0xFFFFFFFF, DMA_HIDDEN_REG);

    //DMA 0 channel
    SAL_WriteReg(0x1B700080, DMA0_SRC_ADDR_REG);    //ADC OUTPUT(0 Channl)
    SAL_WriteReg((uint32)&uiDMAData[0], DMA0_DEST_ADDR_REG);
    SAL_WriteReg((uint32)&uiDMAData[0], DMA0_LLI_REG );
    SAL_WriteReg(((2 << 21) | (2 << 18) | (0 << 15) | (0 << 12) | (100 << 0)), DMA0_CONTROL_REG);           //
    SAL_WriteReg((SAL_ReadReg(DMA0_CONFIG_REG) |= (2 << 11) | (12 << 1) | (1 << 0)), DMA0_CONFIG_REG);      //0 channel of DMA Enable
}
#endif

/*
***************************************************************************************************
*                                          ADC_Init
*
*
* @param    ADCMode_t [in]
*
* Notes
*
***************************************************************************************************
*/

void ADC_Init
(
    uint8                               Type
)
{
    sint32 ret;

    ret = ADC_ClockEnable();

    if (ret != 0)
    {
        ADC_E("ADC_Init failed\n");
    }
    else
    {
        ADC_D("ADC_Init\n");
    }

    if (Type == (uint8)ADC_MODE_IRQ)
    {
        (void)GIC_IntVectSet(GIC_ADC, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&ADC_Isr, (void *) 0);//BSP_NULL);
        (void)GIC_IntSrcEn(GIC_ADC);
    }
#ifdef ADC_DMA_ENABLED
    else if (Type == (uint8)ADC_MODE_DMA)
    {
        ADC_SetDma();
    }
#endif
}


/*
***************************************************************************************************
*                                          ADC_Read
*
*
* @param    Channel [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

uint32 ADC_Read
(
    ADCChannel_t                        Channel
)
{
    uint32 data;
    uint32 i;
    uint32 timeout;

    data    = (uint32)0xFFFF;
    timeout = (uint32)ADC_CONV_CYCLE * (uint32)2;

    //if((ADC_CHANNEL_0 <= Channel) && (Channel < ADC_CHANNEL_MAX))
    if(Channel < ADC_CHANNEL_MAX)
    {
        SAL_WriteReg((uint32)1 << ((uint32)Channel & (uint32)0x1FF), ADC_REG + ADC_SMP_CMD_STS_REG);

        if (ADC_CheckDoneBit() == (uint32)0)
        {
            for (i = 0 ; i < timeout ; i++)
            {
                if (ADC_CheckDoneBit() == (uint32)1)
                {
                    //ADC_E("wait done = %d\n",i); // too much log printed
                    break;
                }

                ADC_Delay(1);   /* wait 1 cycle for ADC peri */
            }
            if (i >= timeout)
            {
                //ADC_D("adc timeout waiting for sampling complete\n"); // too much log printed
            }
        }

        //ADC_D("(ch %d)cmd = 0x%x, data addr = 0x%x\n", Channel, SAL_ReadReg(ADC_REG + ADC_SMP_CMD_STS_REG), ADC_REG + ADC_SDOUT0_REG); // too much log printed
        //data = SAL_ReadReg(ADC_REG + ADC_SDOUT0_REG);
        data = SAL_ReadReg(ADC_REG + ADC_SDOUT0_REG);
    }
    else
    {
        ADC_E("Wrong param\n");
    }

    return data;
}


/*
***************************************************************************************************
*                                          ADC_AutoScan
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/

uint32 * ADC_AutoScan
(
    void
)
{
#ifdef ADC_AUTO_SCAN_ENABLED
    uint32 ch;
    uint32 read_data;
    static uint32 adc_scan_val[ADC_CHANNEL_MAX];

    read_data   = 0;

    for (ch = 0 ; ch < (uint32)ADC_CHANNEL_MAX ; ch++)
    {
        read_data           = ADC_Read((ADCChannel_t)ch);
        adc_scan_val[ch]    = read_data;
        //ADC_DBG("adc_data[%d] : %d\n", ch, read_data);
    }

    return adc_scan_val;
#else
    ADC_E("Not supported ADC auto scan\n");

    return NULL;
#endif
}

/*
***************************************************************************************************
*                                          ADC_DeInit
*
*
* @return
*
* Notes
*
***************************************************************************************************
*/
void ADC_DeInit
(
    void
)
{
    (void)ADC_ClockDisable();
    (void)GIC_IntSrcDis(GIC_ADC);
}

