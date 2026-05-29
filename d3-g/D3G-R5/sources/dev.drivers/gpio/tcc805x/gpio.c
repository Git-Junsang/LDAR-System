/*
***************************************************************************************************
*
*   FileName : gpio.c
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

#include <gpio.h>
#include <reg_phys.h>
#include <bsp.h>
#include "debug.h"

/*
***************************************************************************************************
*                                             LOCAL DEFINES
***************************************************************************************************
*/

#ifndef ENABLE
#   define GPIO_ENABLE                  (1)
#endif

#ifndef DISABLE
#   define GPIO_DISABLE                 (0)
#endif

#define GPIO_PMGPIO_BASE                (MCU_BSP_PMIO_BASE + (0x800UL))

#define GPIO_REG_BASE(x)                (MCU_BSP_GPIO_BASE + ((((x) & GPIO_PORT_MASK) >> (uint32)GPIO_PORT_SHIFT) * 0x40UL))

#define GPIO_GPIOA_SEL_OFFSET           (0x148UL)
#define GPIO_GPIOB_SEL_OFFSET           (0x14CUL)
#define GPIO_GPIOC_SEL_OFFSET           (0x150UL)
#define GPIO_GPIOE_SEL_OFFSET           (0x154UL)
#define GPIO_GPIOG_SEL_OFFSET           (0x158UL)
#define GPIO_GPIOH_SEL_OFFSET           (0x15CUL)
#define GPIO_GPIOMA_SEL_OFFSET          (0x160UL)
#define GPIO_GPIOMB_SEL_OFFSET          (0xA8UL)
#define GPIO_GPIOMC_SEL_OFFSET          (0xACUL)
#define GPIO_GPIOMD_SEL_OFFSET          (0xB0UL)


#define GPIO_A_SEL_BASE                 (0x1B936000UL + 0x148UL)
#define GPIO_B_SEL_BASE                 (MCU_BSP_PMU_BASE + 0x14CUL)
#define GPIO_C_SEL_BASE                 (MCU_BSP_PMU_BASE + GPIO_GPIOC_SEL_OFFSET)
#define GPIO_E_SEL_BASE                 (MCU_BSP_PMU_BASE + GPIO_GPIOE_SEL_OFFSET)
#define GPIO_G_SEL_BASE                 (MCU_BSP_PMU_BASE + GPIO_GPIOG_SEL_OFFSET)
#define GPIO_H_SEL_BASE                 (MCU_BSP_PMU_BASE + GPIO_GPIOH_SEL_OFFSET)
#define GPIO_MA_SEL_BASE                (MCU_BSP_PMU_BASE + GPIO_GPIOMA_SEL_OFFSET)
#define GPIO_MB_SEL_BASE                (MCU_BSP_PMU_BASE + GPIO_GPIOMB_SEL_OFFSET)
#define GPIO_MC_SEL_BASE                (MCU_BSP_PMU_BASE + GPIO_GPIOMC_SEL_OFFSET)
#define GPIO_MD_SEL_BASE                (MCU_BSP_PMU_BASE + GPIO_GPIOMD_SEL_OFFSET)

#define GPIO_IS_GPIOK(x)                ((((x) & GPIO_PORT_MASK) == GPIO_PORT_K) ? 1 : 0)

#define GPIO_REG_DATA(x)                (GPIO_REG_BASE(x) + 0x00UL)
#define GPIO_REG_OUTEN(x)               (GPIO_REG_BASE(x) + 0x04UL)
#define GPIO_REG_DATA_OR(x)             (GPIO_REG_BASE(x) + 0x08UL)
#define GPIO_REG_DATA_BIC(x)            (GPIO_REG_BASE(x) + 0x0CUL)
#define GPIO_REG_PULLEN(x)              (GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x10UL) : (GPIO_REG_BASE(x) + 0x1CUL))
#define GPIO_REG_PULLSEL(x)             (GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x14UL) : (GPIO_REG_BASE(x) + 0x20UL))
#define GPIO_REG_CD(x,pin)              ((GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x18UL) : (GPIO_REG_BASE(x) + 0x14UL)) + (0x4UL * ((pin) / (uint32)16)))
#define GPIO_REG_IEN(x)                 (GPIO_IS_GPIOK(x) ? (GPIO_PMGPIO_BASE + 0x0CUL) :( GPIO_REG_BASE(x) + 0x24UL))
#define GPIO_REG_IS(x)                  (GPIO_REG_BASE(x) + 0x28UL)
#define GPIO_REG_SR(x)                  (GPIO_REG_BASE(x) + 0x2CUL)
#define GPIO_REG_FN(x,pin)              ((GPIO_REG_BASE(x) + 0x30UL) + (0x4UL * ((pin)/(uint32)8)))

#define GPIO_PMGPIO_SEL                 (GPIO_PMGPIO_BASE + 0x8UL)

#define GPIO_LIST_NUM                   (6)
#define GPIO_MAX_STRING                 (20)
#define GPIO_SCAN_DATA                  (0)
#define GPIO_SCAN_OUTPUT_ENABLE         (1)
#define GPIO_SCAN_INPUT_ENABLE          (2)
#define GPIO_SCAN_PULL_UP_DOWN          (3)
#define GPIO_SCAN_DRIVE_STRENGTH        (4)
#define GPIO_SCAN_SCHMITT               (5)
#define GPIO_SCAN_ALL                   (6)

#define GPIO_MD_FNC2					(0x1B9357B4UL)

//#define GPIO_SCAN_DEBUG

/*
***************************************************************************************************
*                                             VARIABLES
***************************************************************************************************
*/


#ifdef GPIO_SCAN_DEBUG
static const sint8 scan_list[GPIO_LIST_NUM][GPIO_MAX_STRING] =
{
    "DATA",
    "OUTPUT ENABLE",
    "INPUT ENABLE",
    "PULL UP/DOWN",
    "DRIVE STRENGH",
    "SCHMITT"
};

static sint32 scan_offset[] = { 0x0, 0x04, 0x24, 0x1C, 0x14, 0x28 };
#endif


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void GPIO_Select
(
    uint32                              uiPort,
    uint32                              uiFlag
);


/*
***************************************************************************************************
*                                          GPIO_Select
*
* @param    uiPort [in]
* @param    uiFlag [in]
*
*
* Notes
*
***************************************************************************************************
*/
static void GPIO_Select(uint32 uiPort, uint32 uiFlag)
{
    uint32 pin;
    uint32 port;
    uint32 port_addr;

    pin     = uiPort & (uint32)GPIO_PIN_MASK;
    port    = uiPort & (uint32)GPIO_PORT_MASK;

    switch (port)
    {
        case GPIO_PORT_A:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            { //QAC : brace
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOA_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);

            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOA_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1UL << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_B:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOB_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOB_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1<<pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_C:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOC_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOC_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_E:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOE_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOE_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32) 0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_G:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOG_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOG_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_H:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOH_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOH_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_MA:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMA_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMA_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_MB:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMB_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMB_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_MC:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMC_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMC_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        case GPIO_PORT_MD:
        {
            if(uiFlag == (uint32)GPIO_ENABLE)
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMD_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) | ((uint32)0x1 << pin), port_addr);
            }
            else
            {
                port_addr = MCU_BSP_PMU_BASE + GPIO_GPIOMD_SEL_OFFSET;
                SAL_WriteReg(SAL_ReadReg(port_addr) & ~((uint32)0x1 << pin), port_addr);
            }

            break;
        }

        default:
        {
            GPIO_E("\n Can't find GPIO Port select\n");
            break;
        }
    }
}

/*
***************************************************************************************************
*                                          GPIO_Config
*
* @param    uiPort [in]
* @param    uiConfig [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

/*
 * GPIO_Config() function descripiton.
 * Parameters
 *  - port  : gpio index
 *  - config: default setting values are below when config is 0.
 *            * slew rate          : fast (gpio_k is not support)
 *            * input type         : cmos (gpio_k is not support)
 *            * input buffer       : no change
 *            * direction          : input
 *            * driver strength    : no change
 *            * pull up/down       : no pull (pull disable)
 *            * function selection : normal gpio
 */
SALRetCode_t GPIO_Config
(
    uint32                              uiPort,
    uint32                              uiConfig
)
{
    uint32 pin;
    uint32 bit;
    uint32 func;
    uint32 pull;
    uint32 cd;
    uint32 ien;
    uint32 base_val;
    uint32 comp_val;
    uint32 set_val;
    uint32 reg_fn;
    uint32 pullen_addr;
    uint32 pullsel_addr;
    uint32 cd_addr;
    uint32 outen_addr;
    uint32 ien_addr;
    uint32 is_addr;
    uint32 sr_addr;

    pin     = uiPort & (uint32)GPIO_PIN_MASK;
    bit     = (uint32)1 << pin;
    func    = uiConfig & (uint32)GPIO_FUNC_MASK;
    pull    = uiConfig & ((uint32)GPIO_PULL_MASK << (uint32)GPIO_PULL_SHIFT);
    cd      = uiConfig & ((uint32)GPIO_CD_MASK << (uint32)GPIO_CD_SHIFT);
    ien     = uiConfig & ((uint32)GPIO_INPUTBUF_MASK << (uint32)GPIO_INPUTBUF_SHIFT);
    GPIO_Select(uiPort, GPIO_DISABLE); // gpio select change to micom

    /* function */
    reg_fn      = GPIO_REG_FN(uiPort,pin);
    base_val    = SAL_ReadReg(reg_fn) & (~((uint32)0xF<<((pin%(uint32)8)*(uint32)4)));
    set_val     = base_val | (func<<((pin%(uint32)8)*(uint32)4));
    SAL_WriteReg(set_val, reg_fn);
    /* configuration check */
    comp_val    = SAL_ReadReg(reg_fn);

    if(comp_val != set_val)
    {
		/*GPIO_MD_FNC2 register is only writerable for TCC805x*/
		if(reg_fn != GPIO_MD_FNC2)
		{
        	return SAL_RET_FAILED;
		}
    }

    /* pull-up/down */
    if (pull == GPIO_PULLUP)
    {
        if(GPIO_IS_GPIOK(uiPort))
        {
            pullen_addr = (GPIO_PMGPIO_BASE + 0x10UL);
        }
        else
        {
            pullen_addr = (GPIO_REG_BASE(uiPort) + 0x1CUL);
        }

        base_val    = SAL_ReadReg(pullen_addr);
        set_val     = (base_val | bit);
        SAL_WriteReg(set_val, pullen_addr);
        comp_val    = SAL_ReadReg(pullen_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }

        if(GPIO_IS_GPIOK(uiPort))
        {
            pullsel_addr = (GPIO_PMGPIO_BASE + 0x14UL);
        }
        else
        {
            pullsel_addr = (GPIO_REG_BASE(uiPort) + 0x20UL);
        }

        base_val    = SAL_ReadReg(pullsel_addr);
        set_val     = base_val | bit;
        SAL_WriteReg(set_val, pullsel_addr);
        comp_val    = SAL_ReadReg(pullsel_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else if (pull == GPIO_PULLDN)
    {
        if(GPIO_IS_GPIOK(uiPort))
        {
            pullen_addr = (GPIO_PMGPIO_BASE + 0x10UL);
        }
        else
        {
            pullen_addr = (GPIO_REG_BASE(uiPort) + 0x1CUL);
        }

        base_val    = SAL_ReadReg(pullen_addr);
        set_val     = base_val | bit;
        SAL_WriteReg(set_val, pullen_addr);
        comp_val    = SAL_ReadReg(pullen_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }

        if(GPIO_IS_GPIOK(uiPort))
        {
            pullsel_addr = (GPIO_PMGPIO_BASE + 0x14UL);
        }
        else
        {
            pullsel_addr = (GPIO_REG_BASE(uiPort) + 0x20UL);
        }

        base_val    = SAL_ReadReg(pullsel_addr);
        set_val     = base_val & ~bit;
        SAL_WriteReg(set_val, pullsel_addr);
        comp_val    = SAL_ReadReg(pullsel_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else
    {
        if(GPIO_IS_GPIOK(uiPort))
        {
            pullen_addr = (GPIO_PMGPIO_BASE + 0x10UL);
        }
        else
        {
            pullen_addr = (GPIO_REG_BASE(uiPort) + 0x1CUL);
        }

        base_val    = SAL_ReadReg(pullen_addr);
        set_val     = base_val & ~bit;
        /* set pull disable. */
        SAL_WriteReg(set_val, pullen_addr);
        comp_val    = SAL_ReadReg(pullen_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }

    /* drive strength */
    if (cd != 0UL)
    {
        if(GPIO_IS_GPIOK(uiPort))
        {
            cd_addr = (GPIO_PMGPIO_BASE + 0x18UL) + (0x4UL * ((pin) / (uint32)16));
        }
        else
        {
            cd_addr = (GPIO_REG_BASE(uiPort) + 0x14UL) + (0x4UL * ((pin) / (uint32)16));
        }

        cd          = cd >> (uint32)GPIO_CD_SHIFT;
        base_val    = SAL_ReadReg(cd_addr) & ~((uint32)3 << ((pin % (uint32)16) * (uint32)2));
        set_val     = base_val | ((cd & (uint32)0x3) << ((pin % (uint32)16) * (uint32)2));
        SAL_WriteReg(set_val, cd_addr);
        comp_val    = SAL_ReadReg(cd_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }

    /* direction */
    if ((uiConfig&GPIO_OUTPUT) != 0UL)
    {
        outen_addr  = GPIO_REG_OUTEN(uiPort);
        base_val    = SAL_ReadReg(outen_addr);
        set_val     = base_val | bit;
        SAL_WriteReg(set_val, outen_addr);
        comp_val    = SAL_ReadReg(outen_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else
    {
        outen_addr  = GPIO_REG_OUTEN(uiPort);
        base_val    = SAL_ReadReg(outen_addr);
        set_val     = base_val & ~bit;
        SAL_WriteReg(set_val, outen_addr);
        comp_val    = SAL_ReadReg(outen_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }

    /* input buffer enable */
    if (ien == GPIO_INPUTBUF_EN)
    {
        if(GPIO_IS_GPIOK(uiPort))
        {
            ien_addr = (GPIO_PMGPIO_BASE + 0x0CUL);
        }
        else
        {
            ien_addr = (GPIO_REG_BASE(uiPort) + 0x24UL);
        }

        base_val    = SAL_ReadReg(ien_addr);
        set_val     = base_val | bit;
        SAL_WriteReg(set_val, ien_addr);
        comp_val    = SAL_ReadReg(ien_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else if (ien == GPIO_INPUTBUF_DIS)
    {
        if(GPIO_IS_GPIOK(uiPort))
        {
            ien_addr = (GPIO_PMGPIO_BASE + 0x0CUL);
        }
        else
        {
            ien_addr = (GPIO_REG_BASE(uiPort) + 0x24UL);
        }

        base_val    = SAL_ReadReg(ien_addr);
        set_val     = base_val & ~bit;
        SAL_WriteReg(set_val, ien_addr);
        comp_val    = SAL_ReadReg(ien_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else //QAC
    {
        ; // no statement
    }

    /* input type */
    if ((uiConfig & GPIO_INPUT_SCHMITT) != 0UL)
    {
        is_addr     = GPIO_REG_IS(uiPort);
        base_val    = SAL_ReadReg(is_addr);
        set_val     = base_val | bit;
        SAL_WriteReg(set_val, is_addr);
        comp_val    = SAL_ReadReg(is_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else
    {
        is_addr     = GPIO_REG_IS(uiPort);
        base_val    = SAL_ReadReg(is_addr);
        set_val     =  base_val & ~bit;
        SAL_WriteReg(set_val, is_addr);
        comp_val    = SAL_ReadReg(is_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }

    /* slew rate */
    if ((uiConfig & GPIO_SLEW_RATE_SLOW) != 0UL)
    {
        sr_addr     = GPIO_REG_SR(uiPort);
        base_val    = SAL_ReadReg(sr_addr);
        set_val     = base_val | set_val;
        SAL_WriteReg(set_val, sr_addr);
        comp_val    = SAL_ReadReg(sr_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }
    else
    {
        sr_addr     = GPIO_REG_SR(uiPort);
        base_val    = SAL_ReadReg(sr_addr);
        set_val     = base_val & ~bit;
        SAL_WriteReg(set_val, sr_addr);
        comp_val    = SAL_ReadReg(sr_addr);

        if(comp_val != set_val)
        {
            return SAL_RET_FAILED;
        }
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          GPIO_Get
*
* @param    uiPort [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

uint8 GPIO_Get
(
    uint32                              uiPort
)
{
    uint32 reg_data;
    uint32 value;

    reg_data    = GPIO_REG_DATA(uiPort);
    value       = (SAL_ReadReg(reg_data) & ((uint32)1 << (uiPort & GPIO_PIN_MASK)));

    if (value != 0UL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*
***************************************************************************************************
*                                          GPIO_Set
*
* @param    uiPort [in]
* @param    uiData [in]
* @return
*
* Notes
*
***************************************************************************************************
*/

SALRetCode_t GPIO_Set
(
    uint32                              uiPort,
    uint32                              uiData
)
{
    uint32 bit;
    uint32 data_or;
    uint32 data_bic;

    bit = (uint32)1 << (uiPort & GPIO_PIN_MASK);
    GPIO_Select(uiPort, GPIO_DISABLE);

    if (uiData > 1UL)
    {
        return SAL_RET_FAILED;
    }

    /* set data */
    if (uiData!=0UL)
    {
        data_or = GPIO_REG_DATA_OR(uiPort);
        SAL_WriteReg(bit, data_or);
    }
    else
    {
        data_bic = GPIO_REG_DATA_BIC(uiPort);
        SAL_WriteReg(bit, data_bic);
    }

    //gpio_select(port, ENABLE);

    return SAL_RET_SUCCESS;
}


/*
***************************************************************************************************
*                                          GPIO_ToNum
*
* @param    uiPort [in]
* @return
*
* Notes
*
***************************************************************************************************
*/
uint32 GPIO_ToNum
(
    uint32                              uiPort
)
{
    uint32 pin;

    pin = uiPort&GPIO_PIN_NUM_MASK;

    switch (uiPort & GPIO_PORT_MASK)
    {
        case GPIO_PORT_A:
        {
            if (pin <= (uint32)31)
            {  //removed codesonar warning
                return pin;
            }

            break;
        }

        case GPIO_PORT_B:
        {
            if (pin <= (uint32)28)
            {
                return (0x20UL + pin);
            }

            break;
        }
        case GPIO_PORT_C:
        {
            if (pin <= (uint32)29)
            {
                return (0x3dUL + pin);
            }

            break;
        }

        case GPIO_PORT_E:
        {
            if (pin <= (uint32)19)
            {
                return (0x5bUL + pin);
            }

            break;
        }

        case GPIO_PORT_G:
        {
            if (pin <= (uint32)10)
            {
                return (0x6fUL + pin);
            }

            break;
        }

        case GPIO_PORT_H:
        {
            if (pin <= (uint32)11)
            {
                return (0x7aUL + pin);
            }

            break;
        }

        case GPIO_PORT_K:
        {
            if (pin <= (uint32)18)
            {
                return (0x86UL + pin);
            }

            break;
        }

        case GPIO_PORT_MA:
        {
            if (pin <= (uint32)29)
            {
                return (0x99UL + pin);
            }

            break;
        }

        case GPIO_PORT_MB:
        {
            if (pin <= (uint32)31)
            {
                return (0xB7UL + pin);
            }

            break;
        }

        case GPIO_PORT_MC:
        {
            if (pin <= (uint32)29)
            {
                return (0xD7UL + pin);
            }

            break;
        }

        case GPIO_PORT_MD:
        {
            if (pin <= (uint32)17)
            {
                return (0xF5UL + pin);
            }

            break;
        }

        default:
        {
            GPIO_E("\n Can't find GPIO Port \n");
            break;
        }
    }

    return (uint32)SAL_RET_FAILED;
}

#ifdef GPIO_SCAN_DEBUG
/*
***************************************************************************************************
*                                          GPIO_PrintOut
*
* @param    uiReg [in]
* @param    uiFlag [in]
*
*
* Notes
*
***************************************************************************************************
*/
void GPIO_PrintOut
(
    uint32                              uiReg,
    uint32                              uiFlag
)
{

    uint32  gpio_reg_data;
    sint8   gpio_reg_buf[32];
    sint32  i;

    gpio_reg_buf[32]    = {0,};
    gpio_reg_data       = SAL_ReadReg(uiReg + scan_offset[uiFlag]);
    GPIO_D("%s : ", scan_list[uiFlag]);

    for(i = 0 ; i < 32 ; i++)
    {
        gpio_reg_buf[31 - i]    = (gpio_reg_data & 0x1) + '0';
        gpio_reg_data           = (gpio_reg_data >> 1);
    }

    GPIO_D("%s\n", gpio_reg_buf);
}

/*
***************************************************************************************************
*                                          GPIO_Scan
*
* @param    uiPort [in]
* @param    uiFlag [in]
*
*
* Notes
*
***************************************************************************************************
*/
void GPIO_Scan
(
    uint32                              uiPort,
    uint32                              uiFlag
)
{

    uint32  gpio_reg;
    sint32  i;

    gpio_reg    = GPIO_REG_BASE(uiPort);

    switch(uiPort & GPIO_PORT_MASK)
    {
        case GPIO_PORT_A :
        {
            GPIO_D("\nGPIO A REG DATA \n");
            break;
        }

        case GPIO_PORT_B :
        {
            GPIO_D("\nGPIO B REG DATA \n");
            break;
        }

        case GPIO_PORT_C :
        {
            GPIO_D("\nGPIO C REG DATA \n");
            break;
        }

        case GPIO_PORT_E :
        {
            GPIO_D("\nGPIO E REG DATA \n");
            break;
        }

        case GPIO_PORT_G :
        {
            GPIO_D("\nGPIO G REG DATA \n");
            break;
        }

        case GPIO_PORT_H :
        {
            GPIO_D("\nGPIO H REG DATA \n");
            break;
        }

        case GPIO_PORT_MA :
        {
            GPIO_D("\nGPIO MA REG DATA \n");
            break;
        }

        case GPIO_PORT_MB :
        {
            GPIO_D("\nGPIO MB REG DATA \n");
            break;
        }

        case GPIO_PORT_MC :
        {
            GPIO_D("\nGPIO MC REG DATA \n");
            break;
        }

        case GPIO_PORT_MD :
        {
            GPIO_D("\nGPIO MD REG DATA \n");
            break;
        }
    }

    if(uiFlag != GPIO_SCAN_ALL)
    {
        GPIO_PrintOut(gpio_reg, uiFlag);
    }
    else
    {
        for(i = 0 ; i < GPIO_LIST_NUM ; i++)
        {
            GPIO_PrintOut(gpio_reg, i);
        }
    }
}
#endif

