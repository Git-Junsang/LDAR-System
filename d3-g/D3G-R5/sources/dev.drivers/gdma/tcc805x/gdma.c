/*
***************************************************************************************************
*
*   FileName : gdma.c
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

#include "gdma.h"
#include "clock.h"
#include "clock_dev.h"
#include <gic.h>
#include <debug.h>
#include "gdma_dev.h"

#if (DEBUG_ENABLE)
    #define GDMA_LOGD(fmt, args...)     {LOGD(DBG_TAG_GDMA, fmt, ## args)}
    #define GDMA_LOGE(fmt, args...)     {LOGE(DBG_TAG_GDMA, fmt, ## args)}
#else
    #define GDMA_LOGD(fmt, args...)
    #define GDMA_LOGE(fmt, args...)
#endif
/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

// GDMA Interrupt Flag Group
#define GDMA_EVT_FLAG_GRP_TIMEOUT       (1000UL)

#define GDMA_INC                        (1UL)
#define GDMA_NO_INC                     (0UL)

#define GDMA_TRANSFER_SIZE_BYTE         (0UL)
#define GDMA_TRANSFER_SIZE_HALF         (1UL)
#define GDMA_TRANSFER_SIZE_WORD         (2UL)

#define GDMA_MAX_LLI_NUM                (64UL)

#define GDMA_BURST_SIZE_1               (0UL)
#define GDMA_BURST_SIZE_4               (1UL)
#define GDMA_BURST_SIZE_8               (2UL)
#define GDMA_BURST_SIZE_16              (3UL)
#define GDMA_BURST_SIZE_32              (4UL)
#define GDMA_BURST_SIZE_64              (5UL)
#define GDMA_BURST_SIZE_128             (6UL)
#define GDMA_BURST_SIZE_256             (7UL)

#define GDMA_FLOW_TYPE_M2M              (0UL)
#define GDMA_FLOW_TYPE_M2P              (1UL)
#define GDMA_FLOW_TYPE_P2M              (2UL)
#define GDMA_FLOW_TYPE_P2P              (3UL)

/*
***************************************************************************************************
                                             LOCAL VARIABLES
***************************************************************************************************
*/
static uint32 uiGDMAEvtFlgGrpID;
static uint32 uiGDMAEnabled = 0UL;

/*
***************************************************************************************************
                                         LOCAL FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void GDMA_ISR
(
    void *                              arg
);

static SALRetCode_t GDMA_WaitInterrupt
(
    const GDMAControlData_t *           pGdmaControl
);

static void GDMA_TcintEnable
(
    const GDMAControlData_t *           pGdmaControl
);

static void GDMA_SetIncrement
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              dest_inc,
    uint32                              src_inc
);

static void GDMA_SetTransferwidth
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              dest_width,
    uint32                              src_width
);

static void GDMA_SetBurstsize
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              dest_burst,
    uint32                              src_burst
);

static void GDMA_SetTransferSize
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              transfer_size
);

static void GDMA_ChannelEnable
(
    const GDMAControlData_t *           pGdmaControl
);

//static void GDMA_ChannelDisable(const GDMAControlData_t *pGdmaControl); // declared but never referenced

static void GDMA_SetSourcAddress
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              addr
);

static void GDMA_SetDestinationAddress
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              addr
);

//static void GDMA_SetLLIAddressr(const GDMAControlData_t *pGdmaControl, uint32 lli_addr);// declared but never referenced

static void GDMA_SetFlowcontrol
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              flow
);

//static void GDMA_SetPeripheral(const GDMAControlData_t *pGdmaControl, uint8 dest_peri, uint8 src_peri);// declared but never referenced
//static void GDMA_TcintDisable(const GDMAControlData_t *pGdmaControl);// declared but never referenced


/*
***************************************************************************************************
*                                         FUNCTION DECLEARATION
***************************************************************************************************
*/

 /*
***************************************************************************************************
*                                          GDMA_TcintEnable
*
* Enable for terminal count interrupt.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @return   void
*
* Notes
*
***************************************************************************************************
*/
static void GDMA_TcintEnable
(
    const GDMAControlData_t *           pGdmaControl
)
{
    uint32 read_val;
    uint32 read_addr;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        read_addr   = GDMA_BASE_ADDRESS + DMA_CH_CON(pGdmaControl->cdChannel);
        read_val    = SAL_ReadReg(read_addr);
        // Enable terminal count interrupt
        SAL_WriteReg((read_val | (1UL<<31)), read_addr);
        // Mask terminal count interrupt
        read_addr   = (GDMA_BASE_ADDRESS + DMA_CH_CONFIG(pGdmaControl->cdChannel));
        read_val    = SAL_ReadReg(read_addr);
        SAL_WriteReg(read_val | (1UL<<15), read_addr);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_TcintEnable
*
* Disable for terminal count interrupt.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @return   void
*
* Notes
*
***************************************************************************************************
*/
//  declared but never referenced
//static void GDMA_TcintDisable
//(
//    const GDMAControlData_t *pGdmaControl
//)
//{
//    // Disable terminal count interrupt
//    SAL_WriteReg(SAL_ReadReg(GDMA_BASE_ADDRESS+DMA_CH_CON(pGdmaControl->cdChannel)) & ~(1UL<<31),
//                 (GDMA_BASE_ADDRESS+DMA_CH_CON(pGdmaControl->cdChannel)));
//    // Unask terminal count interrupt
//    SAL_WriteReg(SAL_ReadReg(GDMA_BASE_ADDRESS+DMA_CH_CONFIG(pGdmaControl->cdChannel)) & ~(1UL<<15),
//                 GDMA_BASE_ADDRESS+DMA_CH_CONFIG(pGdmaControl->cdChannel));
//}


 /*
***************************************************************************************************
*                                          GDMA_SetIncrement
*
* Set increment option for source and destination.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    dest_inc [in] increment option for source.
* @param    src_inc [in] increment option for destination
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetIncrement
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              dest_inc,
    uint32                              src_inc
)
{
    uint32 inc;
    uint32 temp;
    uint32 reg_addr;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        reg_addr    = GDMA_BASE_ADDRESS + DMA_CH_CON(pGdmaControl->cdChannel);
        temp        = SAL_ReadReg(reg_addr);
        //clear desc, src increment
        temp        &= ~(0x3UL<<26);
        inc         = ((dest_inc&0x1UL)<<1) | (src_inc&0x1UL);
        temp        |= (inc<<26);
        SAL_WriteReg(temp, reg_addr);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetTransferwidth
*
* Set transfer width for source and destination.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    dest_width [in] transfer width for destination.
* @param    src_width [in] transfer width for source
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetTransferwidth
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              dest_width,
    uint32                              src_width
)
{
    uint32 transfer_width;
    uint32 temp;
    uint32 reg_addr;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        reg_addr        = GDMA_BASE_ADDRESS + DMA_CH_CON(pGdmaControl->cdChannel);
        temp            = SAL_ReadReg(reg_addr);
        //clear desc, src transfer width
        temp            &= ~(0x3fUL<<18);
        transfer_width  = ((dest_width&0x7UL)<<3) | (src_width&0x7UL);
        temp            |= (transfer_width<<18);
        SAL_WriteReg(temp, reg_addr);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetBurstsize
*
* Set size of burst transfer for source and destination
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    dest_burst [in] size of destination bust.
* @param    src_burst [in] size of source bust.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetBurstsize
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              dest_burst,
    uint32                              src_burst
)
{
    uint32 burst_size;
    uint32 temp;
    uint32 reg_addr;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        reg_addr    = GDMA_BASE_ADDRESS + DMA_CH_CON(pGdmaControl->cdChannel);
        temp        = SAL_ReadReg(reg_addr);
        //clear desc, src burst size
        temp        &= ~(0x3fUL<<12);
        burst_size  = ((dest_burst&0x7UL)<<3) | (src_burst&0x7UL);
        temp        |= (burst_size<<12);
        SAL_WriteReg(temp, reg_addr);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetTransferSize
*
* set transfer size.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    transfer_size [in] lenghth of transfer.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetTransferSize
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              transfer_size
)
{
    uint32 temp;
    uint32 reg_addr;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        reg_addr    = GDMA_BASE_ADDRESS + DMA_CH_CON(pGdmaControl->cdChannel);
        temp        = SAL_ReadReg(reg_addr);
        //clear transfer size
        temp        &= ~(0x0fffUL);
        temp        |= transfer_size & 0x0fffUL;
        SAL_WriteReg(temp, reg_addr);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetFlowcontrol
*
* Setting for flow control mode.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    flow [in] mode of flow control.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetFlowcontrol
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              flow
)
{
    uint32 temp;
    uint32 reg_addr;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        reg_addr    = GDMA_BASE_ADDRESS + DMA_CH_CONFIG(pGdmaControl->cdChannel);
        temp        = SAL_ReadReg(reg_addr);
        temp        &= ~(0x7UL<<11);
        temp        |= (flow & 0x7UL)<<11;
        SAL_WriteReg(temp, reg_addr);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetPeripheral
*
* Setting peripheral id for p2p transfer mode.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    dest_peri [in] ID of destination peripheral.
* @param    src_peri [in] ID of source peripheral
* @return
*
* Notes
*
***************************************************************************************************
*/
//  declared but never referenced
//static void GDMA_SetPeripheral
//(
//    const GDMAControlData_t *pGdmaControl,
//    uint8 dest_peri,
//    uint8 src_peri
//)
//{
//    uint32 temp;
//
//    temp = SAL_ReadReg(GDMA_BASE_ADDRESS+DMA_CH_CONFIG(pGdmaControl->cdChannel));
//    temp &= ~(0x7feUL);
//    temp |= (dest_peri & 0xfUL)<<6;
//    temp |= (src_peri & 0xfUL)<<1;
//    SAL_WriteReg(temp, GDMA_BASE_ADDRESS+DMA_CH_CONFIG(pGdmaControl->cdChannel));
//}


 /*
***************************************************************************************************
*                                          GDMA_ISR
*
* ISR hander function.
*
* @param    arg [in] argument of ISR handler function.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_ISR
(
    void *                              arg
)
{
    uint32          status;
    uint32          flags;
    uint32          i;
    SALRetCode_t    err;
    uint32          regBase;

    (void)arg;//unused
    flags   = 0UL;
    regBase = GDMA_BASE_ADDRESS + GDMA_ITCSR;
    status  = SAL_ReadReg(regBase);

    if (status != 0UL)
    {
        for (i = 0UL ; i < GDMA_CH_MAX ; i++)
        {
            flags = (uint32)((uint32)1UL << (uint32)i);

            if ((status & flags) != 0UL)
            {
                err = (SALRetCode_t)SAL_EventSet(uiGDMAEvtFlgGrpID, flags, SAL_EVENT_OPT_FLAG_SET);

                if (err == SAL_RET_SUCCESS)
                {
                    // Clear terminal count interrupt
                    regBase = GDMA_BASE_ADDRESS + GDMA_ITCCR;
                    SAL_WriteReg(flags, regBase);
                }
                else
                {
                    break;
                }
            }
        }
    }
}

 /*
***************************************************************************************************
*                                          GDMA_WaitInterrupt
*
* function that wait for transferring to complete.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @return
*
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t GDMA_WaitInterrupt
(
    const GDMAControlData_t *           pGdmaControl
)
{
    SALRetCode_t    err;
    uint32          flags;
    uint32          retFlags;

    err = SAL_RET_SUCCESS;

    if ( pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        flags   = (uint32)((uint32)1UL << (uint32)pGdmaControl->cdChannel);
        err     = (SALRetCode_t)SAL_EventGet
        (
            uiGDMAEvtFlgGrpID,
            flags,
            0UL,
            ((uint32)SAL_EVENT_OPT_SET_ALL | (uint32)SAL_EVENT_OPT_CONSUME | (uint32)SAL_OPT_BLOCKING),
            &retFlags
        );

        //check return of event set.
        if (err == SAL_RET_SUCCESS)
        {
            if ((retFlags & flags) != 0UL) //Msg(4:3344) Value of control expression is not derived from an explicit logical operation.MISRA-C:2004 Rule 13.2
            {
                //Msg(4:3200) 'OSAL_SetEvent' returns a value which is not being used. MISRA-C:2004 Rule 16.10
                err = (SALRetCode_t)SAL_EventSet(uiGDMAEvtFlgGrpID, flags, SAL_EVENT_OPT_CLR_ALL);
            }
            else
            {
                retFlags = 1;
            }
        }
    }

    return err;
}

 /*
***************************************************************************************************
*                                          GDMA_ChannelEnable
*
* Registers  functions needed by specific application.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_ChannelEnable
(
    const GDMAControlData_t *           pGdmaControl
)
{
    uint32 regBase;
    uint32 temp;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        regBase = GDMA_BASE_ADDRESS + DMA_CH_CONFIG(pGdmaControl->cdChannel);
        temp    = SAL_ReadReg(regBase);
        SAL_WriteReg(temp|0x1UL, regBase);
        GDMA_LOGE("GDMA Transfer Start");
        (void)GDMA_WaitInterrupt(pGdmaControl);
        GDMA_LOGE("GDMA Transfer Finished");
    }
}

 /*
***************************************************************************************************
*                                          GDMA_ChannelDisable
*
* Registers  functions needed by specific application.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @return
*
* Notes
*
***************************************************************************************************
*/
// declared but never referenced
//static void GDMA_ChannelDisable
//(
//    const GDMAControlData_t *pGdmaControl
//)
//{
//   SAL_WriteReg(SAL_ReadReg(GDMA_BASE_ADDRESS+DMA_CH_CONFIG(pGdmaControl->cdChannel)) & ~(0x1UL),
//                 GDMA_BASE_ADDRESS+DMA_CH_CONFIG(pGdmaControl->cdChannel));
//}

 /*
***************************************************************************************************
*                                          GDMA_SetSourcAddress
*
* Registers  functions needed by specific application.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    addr [in] Pointer to function that corresponds with function index.
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetSourcAddress
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              addr
)
{
    uint32 regBase;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        regBase = GDMA_BASE_ADDRESS + DMA_CH_SRC_ADDR(pGdmaControl->cdChannel);
        SAL_WriteReg(addr, regBase);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetDestinationAddress
*
* Registers  functions needed by specific application.
*
* @param    uiFunctionIndex [in] Index of function to register.
* @param    pFunc [in] Pointer to function that corresponds with function index.
* @param    pPreFunc [in] Pointer to function that
* @return
*
* Notes
*
***************************************************************************************************
*/

static void GDMA_SetDestinationAddress
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              addr
)
{
    uint32 regBase;

    if (pGdmaControl->cdChannel < GDMA_CH_MAX)
    {
        regBase = GDMA_BASE_ADDRESS + DMA_CH_DST_ADDR(pGdmaControl->cdChannel);
        SAL_WriteReg(addr, regBase);
    }
}

 /*
***************************************************************************************************
*                                          GDMA_SetLLIAddressr
*
* Registers  functions needed by specific application.
*
* @param    uiFunctionIndex [in] Index of function to register.
* @param    pFunc [in] Pointer to function that corresponds with function index.
* @param    pPreFunc [in] Pointer to function that
* @return
*
* Notes
*
***************************************************************************************************
*/
// declared but never referenced
//static void GDMA_SetLLIAddressr
//(
//    const GDMAControlData_t *pGdmaControl,
//    uint32 lli_addr
//)
//{
//    SAL_WriteReg(lli_addr, GDMA_BASE_ADDRESS+DMA_CH_LLI(pGdmaControl->cdChannel));
//}

 /*
***************************************************************************************************
*                                          GDMA_TransferFunctionForM2M
*
* Request DMA transfer from source address to destination address with given data length.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    uiSourceAddress [in] source address. This address is valid only for physical address.
* @param    uiDestinationAddress [in] destination address. This address is valid only for physical address.
* @param    uiLength [in] lengh fo data requested for transfer.
* @return   the result of transfer.
*
* Notes
*
***************************************************************************************************
*/

sint32 GDMA_TransferFunctionForM2M
(
    const GDMAControlData_t *           pGdmaControl,
    uint32                              uiSourceAddress,
    uint32                              uiDestinationAddress,
    uint32                              uiLength
)
{
    sint32 ret;
    uint32 word_len;
    uint32 word_len_rest;
    uint32 i;
    uint32 part_num;
    uint32 part_rest;
    uint32 src_addr_rest;
    uint32 dest_addr_rest;

    ret = (sint32)SAL_RET_SUCCESS;

    if ( (uiGDMAEnabled == 1UL) && (pGdmaControl != NULL_PTR))
    {
        if (((uiSourceAddress % sizeof(sint32)) == 0UL) && ((uiDestinationAddress % sizeof(sint32)) == 0UL))
        {
            word_len        = (uiLength / 4UL);
            word_len_rest   = uiLength % 4UL;
            part_num        = word_len / DMA_MAX_XFER_SIZE;
            part_rest       = word_len % DMA_MAX_XFER_SIZE;
            src_addr_rest   = uiSourceAddress;
            dest_addr_rest  = uiDestinationAddress;

            GDMA_SetFlowcontrol(pGdmaControl, GDMA_FLOW_TYPE_M2M);
            GDMA_SetIncrement(pGdmaControl, GDMA_INC, GDMA_INC);
            GDMA_SetBurstsize(pGdmaControl, GDMA_BURST_SIZE_256, GDMA_BURST_SIZE_256);

            /* max transfer */
            for (i = 0UL ; i < part_num ; i++)
            {
                GDMA_SetSourcAddress(pGdmaControl, src_addr_rest);
                GDMA_SetDestinationAddress(pGdmaControl, dest_addr_rest);
                GDMA_TcintEnable(pGdmaControl);
                GDMA_SetTransferwidth(pGdmaControl, GDMA_TRANSFER_SIZE_WORD, GDMA_TRANSFER_SIZE_WORD);
                GDMA_SetTransferSize(pGdmaControl, DMA_MAX_XFER_SIZE);
                GDMA_ChannelEnable(pGdmaControl);
                src_addr_rest   += (DMA_MAX_XFER_SIZE * 4UL);
                dest_addr_rest  += (DMA_MAX_XFER_SIZE * 4UL);
            }

            /* word transfer */
            if (part_rest != 0UL)
            {
                GDMA_SetSourcAddress(pGdmaControl, src_addr_rest);
                GDMA_SetDestinationAddress(pGdmaControl, dest_addr_rest);
                GDMA_TcintEnable(pGdmaControl);
                GDMA_SetTransferwidth(pGdmaControl, GDMA_TRANSFER_SIZE_WORD, GDMA_TRANSFER_SIZE_WORD);
                GDMA_SetTransferSize(pGdmaControl, part_rest);
                GDMA_ChannelEnable(pGdmaControl);
                src_addr_rest   += (part_rest * 4UL);
                dest_addr_rest  += (part_rest * 4UL);
            }

            /* byte transfer */
            if (word_len_rest != 0UL)
            {
                GDMA_SetSourcAddress(pGdmaControl, src_addr_rest);
                GDMA_SetDestinationAddress(pGdmaControl, dest_addr_rest);
                GDMA_TcintEnable(pGdmaControl);
                GDMA_SetTransferwidth(pGdmaControl, GDMA_TRANSFER_SIZE_BYTE, GDMA_TRANSFER_SIZE_BYTE);
                GDMA_SetTransferSize(pGdmaControl, word_len_rest);
                GDMA_ChannelEnable(pGdmaControl);
            }

            ret = (sint32)SAL_RET_SUCCESS;
        }
        else
        {
            /* mis align check */
            GDMA_LOGE("SAL_ERR_INVALID_PARAMETER uiSourceAddress %x, uiDestinationAddress %x", uiSourceAddress, uiDestinationAddress);
            ret = (sint32)SAL_ERR_INVALID_PARAMETER;
        }
    }
    else
    {
        ret = (sint32)SAL_ERR_UNDEF_STATE;
    }

    GDMA_LOGD("GDMA_TransferFunctionForM2M ret %d", ret);

    return ret;
}

 /*
***************************************************************************************************
*                                          GDMA_Initialize
*
* Initialize the gdma driver.
*
* @param    pGdmaControl [in] pointer of gdma control data.
* @param    uiInterruptPriority [in] priority of gdma interrupt.
* @return
*
* Notes
*
***************************************************************************************************
*/

sint32 GDMA_Initialize
(
    GDMAControlData_t *                 pGdmaControl,
    uint32                              uiInterruptPriority
)
{
    SALRetCode_t    err;
    uint32          regBase;
    uint32          regValue;

    (void)CLOCK_EnableIobus((sint32)CLOCK_IOBUS_DMA_GRP, SALEnabled);

    // Disable DMA Contorller
    regBase     = GDMA_BASE_ADDRESS + GDMA_CR;
    regValue    = SAL_ReadReg(regBase);
    SAL_WriteReg(regValue & ~(0x1UL), regBase);
    // Clear Interrupt
    regBase = GDMA_BASE_ADDRESS + GDMA_ITCCR;
    SAL_WriteReg(0x7F, regBase);
    regBase = GDMA_BASE_ADDRESS + GDMA_IECR;
    SAL_WriteReg(0x7F, regBase);
    err = (SALRetCode_t)SAL_EventCreate((uint32 *)&uiGDMAEvtFlgGrpID, (const uint8 *)"GDMA_Event_Group", 0UL);

    if (err == SAL_RET_SUCCESS)
    {
        (void)GIC_IntVectSet((uint32)GIC_DMA_PL080, uiInterruptPriority, (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&GDMA_ISR, (void *)pGdmaControl);
        (void)GIC_IntSrcEn((uint32)GIC_DMA_PL080);

        // Enable DMA Controller
        regBase = GDMA_BASE_ADDRESS + GDMA_CR;
        SAL_WriteReg(0x1UL, regBase);
        uiGDMAEnabled   = 1UL;
    }

    GDMA_LOGD("GDMA_Initialize ret %d", err);

    return (sint32)err;
}

