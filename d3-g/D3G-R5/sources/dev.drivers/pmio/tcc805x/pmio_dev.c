/*
***************************************************************************************************
*
*   FileName : pmio_dev.c
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
***************************************************************************************************/

#include <pmio.h>
#include <pmio_dev.h>
#include <reg_phys.h>
#include <bsp.h>
#include <gic.h>
#include <debug.h>
/*************************************************************************************************
 *                                             DEFINITIONS
 *************************************************************************************************/



/*PMIO EXTRA FEATURE=====================================================*/

#ifdef PMIO_CONF_DEBUG_ALONE
    #define PMIO_FEATURE_USE_DEBUG_ALONE
#endif

#ifdef PMIO_CONF_PREVENT_LEAK
    #define PMIO_FEATURE_LEAKAGE_REDEUCTION_GPK_PORT
#endif

#ifdef PMIO_CONF_DET_BU
    #define PMIO_VA_BU_DET_PORT             (PMIO_VA_BIT1)
#else
    #define PMIO_VA_BU_DET_PORT             (PMIO_VA_BITCLEAR)
#endif

#ifdef PMIO_CONF_DET_ACC
    #define PMIO_VA_ACC_DET_PORT            (PMIO_VA_BIT3)
#else
    #define PMIO_VA_ACC_DET_PORT            (PMIO_VA_BITCLEAR)
#endif

#ifdef PMIO_CONF_DET_CAN
    #define PMIO_VA_CAN_DET_PORT            (PMIO_VA_BIT14)
#else
    #define PMIO_VA_CAN_DET_PORT            (PMIO_VA_BITCLEAR)
#endif

#ifdef PMIO_CONF_MISC_DEV_FUNC
    //#define PMIO_FEATURE_DEV_TEST_BACKUP_RAM
        /* [En] It runs sample test about backup ram read/write.                                */

    //#define PMIO_FEATURE_DEV_TEST_STR_ITERATION
        /* [En] It runs sample test about repeat STR/wakeup.                                    */
        /* If you enter "pmio str" as MCU cmd, MCU can request str msg to APs.                  */
        /* Need to support S/W Mailbox driver.                                                  */
        /* Need to support AP's STR mode with RTC wkup.                                         */
        /* Need to support MCU's STR mode.                                                      */

    //#define PMIO_FEATURE_DEV_TEST_CRST
        /* [En] It runs sample test about cold reset.                                           */
        /* When A72,A53 sends a reset request msg,                                              */
        /* pmio immediately performs a cold reset.                                              */
        /* Need to support S/W Mailbox driver.                                                  */

    //#define PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
        /* [En] It runs sample test about R5 alone STR Entering                                 */
        /* If boot is OK, AP and R5 enter STR mode together through mbox communication.         */
        /* If boot is not OK, R5 enters STR mode alone.                                         */
        /* Need to support STR Mode                                                             */
        /* Need to support S/W Mailbox driver. (in R5 BCTRL app, in AP BL1)                     */
        /* Need to support S/W Boot check communication (in R5 BCTRL app, in AP BL1).           */

    #ifndef PMIO_FEATURE_USE_DEBUG_ALONE
        #define PMIO_FEATURE_USE_DEBUG_ALONE
    #endif
#endif

#ifdef PMIO_CONF_STR_MODE
    #define PMIO_FEATURE_USE_STR_MODE
    #define PMIO_VA_STR_DET_PORT            (PMIO_VA_BIT12)

    //#define PMIO_SUBFEATURE_STR_SINGLE_CORE_MODE
        /*AP Core number*/
        /*Default AP Core operation mode is dual mode (main:CA53, sub:CA72)*/
        /*You can change the mode from dual to single (main:CA72, sub:Disabled)*/

    /*#define PMIO_SUBFEATURE_STR_USE_BCONF_CORE_CHECK*/
        /* Not Supports this feature.*/
        /* Check core number for multi-core and single-core*/
        /* [En]: Automatic processing of message according to the number of cores is possible */
        /*      Core number check requires a prebuilt bconf.bin file in boot-firmware*/
        /*      This function should use the correct bconf file.*/
        /*      Make a snor rom image with r5-fw and a prebuilt to use.*/
        /* [Disable]: Static Processing of message [default: multi-core (main=ca72, sub=ca53)] */

    #define PMIO_VA_ACC_OFF_TIME_LIMIT          (PMIO_FUNC_SEC_TO_USEC(600UL))
    #define PMIO_VA_STR_REQ_TIME_LIMIT          (PMIO_FUNC_SEC_TO_USEC(120UL))
        /* When the system wakes up in ACC OFF state, it waits for ACC ON signal using timer. */
        /* If ACC ON does not occur during the time limit, suspend mode is entered. */
#endif

/*=======================================================PMIO EXTRA FEATURE*/





#if defined(DEBUG_ENABLE) || defined(PMIO_FEATURE_USE_DEBUG_ALONE)
    #define PMIO_D(fmt, args...)        {mcu_printf("[PMIO][%s:%d] " fmt, \
                                                __func__, __LINE__, ## args);}
#else
    #define PMIO_D(fmt, args...)
#endif

#define PMIO_E(fmt, args...)        {mcu_printf("[PMIO][%s:%d] Error ! " fmt, \
                                            __func__, __LINE__, ## args);}

#if defined(PMIO_FEATURE_USE_STR_MODE) || defined(PMIO_FEATURE_DEV_TEST_CRST)
    #include <mbox.h>
    #include <mbox_phy.h>
    #define PMIO_VA_MSG_TASK_STACK_SIZE (128UL)
#endif

#ifdef PMIO_FEATURE_USE_STR_MODE
    #include <timer.h>
    #define PMIO_VA_TIMER_CH            (1UL)
    #define PMIO_VA_SEND_EVENT_NUM      (0x00000001UL)
    #define PMIO_VA_RECV_EVENT_NUM      (0x00000002UL)
    #define PMIO_VA_READY_EVENT_NUM     (0x00000003UL)
#endif

#ifdef PMIO_FEATURE_DEV_TEST_CRST
    typedef enum
    {
        PMIO_EX_MSG_NOT_SET            = 0,
        PMIO_EX_MSG_COLD_RESET         = 7 //used      (AP->R5): for cold rst and dram test
    } PMIOExMsg_t;

    typedef enum
    {
        PMIO_EX_AP_CH_A72               = 0,
        PMIO_EX_AP_CH_A53               = 1,
        PMIO_EX_AP_CH_MAX               = 2
    } PMIOExApCh_t;
#endif

#ifdef PMIO_FEATURE_DEV_TEST_STR_ITERATION
    #undef  PMIO_VA_BU_DET_PORT
    #undef  PMIO_VA_ACC_DET_PORT
    #undef  PMIO_VA_CAN_DET_PORT
    #define PMIO_VA_BU_DET_PORT             (PMIO_VA_BITCLEAR)
    #define PMIO_VA_ACC_DET_PORT            (PMIO_VA_BITCLEAR)
    #define PMIO_VA_CAN_DET_PORT            (PMIO_VA_BITCLEAR)
#endif

/************************************************************************************************/
/*                                             STATIC FUNCTION                                  */
/************************************************************************************************/

static void PMIO_Delay
(
    uint32                              uiCnt
);

static void PMIO_SetPort
(
    PMIOPortSel_t                       uiPortType,
    uint32                              uiPort32
);

static void PMIO_InterruptHandler
(
    void *                              pArg
);

static void PMIO_SetInterruptHandler
(
    void
);

static void PMIO_RTCInterruptHandler
(
    void *                              pArg
);

static void PMIO_SetRTCInterruptHandler
(
    void
);

static void PMIO_SetGPKInterrupt
(
    uint32                              uiPort,
    uint8                               ucEdge,
    uint8                               ucHigh,
    uint8                               ucEn
);

static void PMIO_ReducePowerLeakageCurrent
(
    void
);


#ifdef PMIO_FEATURE_DEV_TEST_CRST
static void PMIO_RST_RunMsg
(
    void
);
static void PMIO_RST_RecvMsgTask_A72
(
    void *                              pArg
);
static void PMIO_RST_RecvMsgTask_A53
(
    void *                              pArg
);
static void PMIO_RST_RecvMsgTask
(
    PMIOExApCh_t                        tCh
);
#endif

#ifdef PMIO_FEATURE_USE_STR_MODE
    /*static STR function*/
    static PMIOMode_t PMIO_STR_GetBootMode
    (
        void
    );

    static void PMIO_STR_SetPowerStatus
    (
        PMIOMode_t                      tMode,
        PMIOReason_t                    tReason
    );

    static void PMIO_STR_GetCore
    (
        uint32 *                        pMainCore,
        uint32 *                        pSubCore
    );

    static void PMIO_STR_OpenMsg
    (
        void
    );

    static void PMIO_STR_RecvMsg
    (
        PMIOMsg_t                       uiMsgType
    );

    static void PMIO_STR_SendMsg
    (
        PMIOMsg_t                       uiMsgType
    );

    static void PMIO_STR_RecvMsgTask
    (
        void *                          pArg
    );

    static void PMIO_STR_SendMsgTask
    (
        void *                          pArg
    );

    static void PMIO_STR_SetTimer
    (
        uint32                          uiUsec,
        PMIOTimerReason_t               uiReason
    );

    static sint32 PMIO_STR_TimeoutInterruptHandler
    (
        TIMERChannel_t                  iChannel,
        void *                          pArgs
    );

    static uint32               gSTRTime            = 0UL;
    static PMIOTimerReason_t    gSTRTimerReason     = PMIO_TIMER_REASON_NOT_SET;

    static MBOXDvice_t *    gPmioMboxA72;
    static MBOXDvice_t *    gPmioMboxA53;
    static uint32           gMsgMainCore            = 0UL;
    static uint32           gMsgSubCore             = 0UL;

    static uint32           gRecvMsgTaskEventId     = 0UL;
    static uint32           gSendMsgTaskEventId     = 0UL;
    static uint32           gReadyMsgTaskEventId    = 0UL;

    static PMIOMsg_t        gSendMsgType            = PMIO_MSG_NOT_SET;
    static PMIOMsg_t        gRecvMsgType            = PMIO_MSG_NOT_SET;

    static uint8            gucOldMsgType           = 0UL;

    typedef struct
    {
        const PMIOMsg_t     msgType;
        const uint8         msgCmd1;
        const uint8         msgCmd2;
        const uint8         msgCmd3;
    } PMIOMsgDataList_t;

    static PMIOMsgDataList_t gMsgDataList[(uint32)PMIO_MSG_MAX] =
    {
        { PMIO_MSG_NOT_SET            , (const uint8)0x00, (const uint8)0x00, (const uint8)0x00},
        { PMIO_MSG_ACK                , (const uint8)0x03, (const uint8)0x01, (const uint8)0x00},
        { PMIO_MSG_ACC_ON             , (const uint8)0x02, (const uint8)0x01, (const uint8)0x00},
        { PMIO_MSG_STR                , (const uint8)0x05, (const uint8)0x01, (const uint8)0x00},
        { PMIO_MSG_STR_DONE           , (const uint8)0x04, (const uint8)0x00, (const uint8)0x00},
        { PMIO_MSG_POWER_DOWN         , (const uint8)0x00, (const uint8)0x00, (const uint8)0x00},
        { PMIO_MSG_POWER_DOWN_DONE    , (const uint8)0x00, (const uint8)0x00, (const uint8)0x00},
        { PMIO_MSG_AP_AWAKE           , (const uint8)0x07, (const uint8)0x00, (const uint8)0x00}
    };
        //for debugging power status
    static uint32   psMODE          = 0UL;
#endif
    static uint32   psBOOT          = 0UL;

#ifdef PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
    static uint32   gAPBootOk       = 0UL;
#endif


/*************************************************************************************************/
/*                                             Implementation                                    */
/* ***********************************************************************************************/
static void PMIO_Delay
(
    uint32                              uiCnt
)
{
    uint32 uiDesc;

    uiDesc  = uiCnt;

    if(uiDesc == (uint32)(0UL))
    {
        while(1)
        {
            BSP_NOP_DELAY();
        }
    }
    else
    {
        for(; uiDesc > (uint32)(0UL) ; uiDesc--)
        {
            BSP_NOP_DELAY();
        }
    }
}

static void PMIO_SetPort
(
    PMIOPortSel_t                       uiPortType,
    uint32                              uiPort32
)
{
    switch(uiPortType)
    {
        case PMIO_PORT_SEL_RELEASE:
            {
                //GPK port default is controled by PMGPIO
                PMIO_REG_PMGPIO_FS  &= ~(uiPort32);  //0: pin controlled by PMGPIO
                PMIO_REG_PMGPIO_EN  &= ~(uiPort32);  //0: input
                PMIO_REG_PMGPIO_IEN &= ~(uiPort32);  //0: input disable
                break;
            }
        case PMIO_PORT_SEL_GPIO:
            {
                PMIO_REG_PMGPIO_FS  |=  (uiPort32); //1: pin controlled by GPIO
                PMIO_REG_PMGPIO_IEN |=  (uiPort32);  //1: input enable
                break;
            }
        case PMIO_PORT_SEL_PMGPIO_IN:
            {
                PMIO_REG_PMGPIO_FS  &= ~(uiPort32);  //0: pin controlled by PMGPIO
                PMIO_REG_PMGPIO_EN  &= ~(uiPort32);  //0: input
                PMIO_REG_PMGPIO_IEN |=  (uiPort32);  //1: input enable
                break;
            }
        case PMIO_PORT_SEL_PMGPIO_OUT_L:
            {
                PMIO_REG_PMGPIO_FS  &= ~(uiPort32); //0: pin controlled by PMGPIO_DAT/EN
                PMIO_REG_PMGPIO_EN  |=  (uiPort32); //1: output
                PMIO_REG_PMGPIO_DAT &= ~(uiPort32); //0: Low
                break;
            }
        case PMIO_PORT_SEL_PMGPIO_OUT_H:
            {
                PMIO_REG_PMGPIO_FS  &= ~(uiPort32); //0: pin controlled by PMGPIO_DAT/EN
                PMIO_REG_PMGPIO_EN  |=  (uiPort32); //1: output
                PMIO_REG_PMGPIO_DAT |=  (uiPort32); //1: High
                break;
            }
        case PMIO_PORT_SEL_PMGPIO_PU:
            {
                PMIO_REG_PMGPIO_FS  &= ~(uiPort32);  //0: pin controlled by PMGPIO_DAT/EN
                PMIO_REG_PMGPIO_PS  |=  (uiPort32);  //1: Set to pull-up
                PMIO_REG_PMGPIO_PE  |=  (uiPort32); //pull up/down En
                break;
            }
        case PMIO_PORT_SEL_PMGPIO_PD:
            {
                PMIO_REG_PMGPIO_FS  &= ~(uiPort32);  //0: pin controlled by PMGPIO_DAT/EN
                PMIO_REG_PMGPIO_PS  &= ~(uiPort32); //0: Set to pull-down
                PMIO_REG_PMGPIO_PE  |=  (uiPort32); //pull up/down En
                break;
            }
        case PMIO_PORT_SEL_PCTLIO_IN:
            {
                PMIO_REG_PCTLIO_FS  &= ~(uiPort32);  //0: ctrld by PMGPIO_DAT/EN
                PMIO_REG_PCTLIO_EN  &= ~(uiPort32);  //0: PCTL[1:0] input
                PMIO_REG_PCTLIO_IEN |=  (uiPort32);  //0: PCTL[1:0] input
                break;
            }
        case PMIO_PORT_SEL_PCTLIO_OUT_H:
            {
                PMIO_REG_PCTLIO_FS  &= ~(uiPort32);  //0: ctrld by PMGPIO_DAT/EN
                PMIO_REG_PCTLIO_EN  |=  (uiPort32);  //PCTL[1:0] output
                PMIO_REG_PCTLIO_DAT |=  (uiPort32); // PCTL1:HIGH, PCTL0:LOW
                break;
            }
        case PMIO_PORT_SEL_PCTLIO_OUT_L:
            {
                PMIO_REG_PCTLIO_FS  &= ~(uiPort32);  //0: ctrld by PMGPIO_DAT/EN
                PMIO_REG_PCTLIO_EN  |=  (uiPort32);  //PCTL[1:0] output
                PMIO_REG_PCTLIO_DAT &= ~(uiPort32); // PCTL1:HIGH, PCTL0:LOW
                break;
            }
        case PMIO_PORT_SEL_PCTLIO_PU:
            {
                PMIO_REG_PCTLIO_FS  &= ~(uiPort32);  //0: pin controlled by PCTLIO_DAT/EN
                PMIO_REG_PCTLIO_PS  |=  (uiPort32);  //1: Set to pull-up
                PMIO_REG_PCTLIO_PE  |=  (uiPort32); //pull up/down En
                break;
            }
        case PMIO_PORT_SEL_PCTLIO_PD:
            {
                PMIO_REG_PCTLIO_FS  &= ~(uiPort32);  //0: pin controlled by PCTLIO_DAT/EN
                PMIO_REG_PCTLIO_PS  &= ~(uiPort32); //0: Set to pull-down
                PMIO_REG_PCTLIO_PE  |=  (uiPort32); //pull up/down En
                break;
            }
    }
}

static void PMIO_InterruptHandler
(
    void *                              pArg
)
{
    (void)pArg;

    if ((PMIO_REG_GPK_IRQST & PMIO_VA_BU_DET_PORT) != 0UL)
    {
        PMIO_REG_GPK_IRQST |= PMIO_VA_BU_DET_PORT;
        PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
    }

    if ((PMIO_REG_GPK_IRQST & PMIO_VA_ACC_DET_PORT) != 0UL)
    {
#ifdef PMIO_FEATURE_USE_STR_MODE
        if((PMIO_REG_GPK_IRQPOL & PMIO_VA_ACC_DET_PORT) != 0UL) // 1 inverted high to low
        {
            PMIO_D("irq: ACC OFF\n");
            PMIO_REG_GPK_IRQST |= PMIO_VA_ACC_DET_PORT;
        #ifdef PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
            if(gAPBootOk == 0UL)
            {
                PMIO_PowerDown(PMIO_MODE_STR);
            }
            else
        #endif
            {
                PMIO_STR_SendMsg(PMIO_MSG_STR);
                PMIO_STR_RecvMsg(PMIO_MSG_STR_DONE);
                PMIO_SetGPKInterrupt(PMIO_VA_ACC_DET_PORT, \
                        1/*0:level, 1:edge*/, 1/*0:low, 1:high*/, 1/*0:disable, 1:enable*/);
            }
        }
        else
        {
            PMIO_D("irq: ACC ON\n");
            /*0: not-inverted(low->high)*/
            if(gSTRTimerReason == PMIO_TIMER_REASON_ACC_OFF) /*power on but late acc on*/
            {
                PMIO_SetGPKInterrupt((PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT), \
                        1/*0:level, 1:edge*/, 0/*0:low, 1:high*/, 0/*0:disable, 1:enable*/);
                PMIO_STR_SetTimer(0UL, PMIO_TIMER_REASON_NOT_SET);
                PMIO_STR_SendMsg(PMIO_MSG_ACC_ON);
            }
            else if(gSTRTimerReason == PMIO_TIMER_REASON_STR_REQ) /*str req but acc back on)*/
            {
                PMIO_D("Already send a STR req msg. wait a STR_DONE msg\n");
            }
            else
            {
                gSendMsgType = PMIO_MSG_NOT_SET;
                PMIO_STR_SetTimer(0UL, PMIO_TIMER_REASON_NOT_SET);
                PMIO_D("Cancel a send msg.\n");
            }

            PMIO_SetGPKInterrupt((PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT), \
                    1/*0:level, 1:edge*/, 0/*0:low, 1:high*/, 1/*0:disable, 1:enable*/);
        }
#else
        PMIO_REG_GPK_IRQST |= PMIO_VA_ACC_DET_PORT;
        PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
#endif
    }
}

static void PMIO_SetInterruptHandler
(
    void
)
{
    static uint8 uiSet = 0U;

    if(uiSet == 0U)
    {
        (void)GIC_IntVectSet( \
                PMIO_VA_INTERRUPT_SRC, \
                GIC_PRIORITY_NO_MEAN, \
                GIC_INT_TYPE_LEVEL_HIGH, \
                &PMIO_InterruptHandler, \
                NULL);

        (void)GIC_IntSrcEn(PMIO_VA_INTERRUPT_SRC);
        uiSet = 1U;
    }
}

static void PMIO_RTCInterruptHandler
(
    void *                              pArg
)
{
    uint32 uiReadVa;

    (void)pArg;
    uiReadVa = SAL_ReadReg(0x16480000UL);

    uiReadVa &= ~(PMIO_VA_BIT7);
    SAL_WriteReg(uiReadVa, 0x16480000UL);
    #ifdef PMIO_FEATURE_USE_STR_MODE
    PMIO_D("Detect RTC Wakeup.\n");
    PMIO_STR_SetPowerStatus(PMIO_MODE_NOT_SET, PMIO_REASON_NOT_SET);/*Clear boot mode, boot reason*/
    PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
    #endif
}

static void PMIO_SetRTCInterruptHandler
(
    void
)
{
    static uint8 uiSet = 0U;

    if(uiSet == 0U)
    {
        (void)GIC_IntVectSet( \
                PMIO_VA_RTC_INTERRUPT_SRC, \
                GIC_PRIORITY_NO_MEAN, \
                GIC_INT_TYPE_LEVEL_HIGH, \
                &PMIO_RTCInterruptHandler, \
                NULL);

        (void)GIC_IntSrcEn(PMIO_VA_RTC_INTERRUPT_SRC);
        uiSet = 1U;
    }
}

static void PMIO_SetGPKInterrupt
(
    uint32                              uiPort,
    uint8                               ucEdge,
    uint8                               ucHigh,
    uint8                               ucEn
)
{
    const uint32 uiGpkFLvd = 0x1000UL;
    const uint32 uiGpkFDiv = 0x1000UL;

    PMIO_REG_GPK_IRQEN  &= ~(uiPort/*|external source*/); // interrupt disable
    PMIO_REG_GPK_IRQST  |=  (uiPort/*|external source*/);  // clear interrupt

    if(ucEn != 0U)
    {
        /*Interrupt for ACC DET*/
        PMIO_REG_GPK_FBP    &= ~(uiPort/*|external source*/);  //enable interrupt source & filter
        if(PMIO_REG_GPK_FCK == 0x0UL)
        {
            PMIO_REG_GPK_FCK    |=  ((uiGpkFLvd<<16)|(uiGpkFDiv));  //enable interrupt source & filter
        }

        if(ucHigh != 0U)
        {
            PMIO_REG_GPK_IRQPOL &= ~(uiPort/*|external source*/);  //interrupt polarity not inverted (when changed low->high)
        }
        else
        {
            PMIO_REG_GPK_IRQPOL |=  (uiPort/*|external source*/);  //interrupt polarity inverted (when changed high->low)
        }

        if(ucEdge != 0U)
        {
            PMIO_REG_GPK_IRQTM0 &= ~(uiPort/*|external source*/); // 0: edge trigger    1: level trigger
        }
        else
        {
            PMIO_REG_GPK_IRQTM0 |=  (uiPort/*|external source*/); // 0: edge trigger    1: level trigger
        }

        PMIO_REG_GPK_IRQEN  |=  (uiPort/*|external source*/); // interrupt enable

        PMIO_SetInterruptHandler();
    }
}

static void PMIO_ReducePowerLeakageCurrent
(
    void
)
{
    /* Power leakage current must be checked on user's hardware environment */
    /********************************************
    GPIO_K00    GPIO_K00
    GPIO_K01    BU_DET0
    GPIO_K02    GPIO_K02
    GPIO_K03    ACC_DET
    GPIO_K04    IGN_DET
    GPIO_K05    EMG_WAKE_IN
    GPIO_K06    CAN0_INH
    GPIO_K07    DOOR_OPEN_DET
    GPIO_K08    CAN0_STB
    GPIO_K09    LIN_TX
    GPIO_K10    LIN_RX
    GPIO_K11
    GPIO_K12    STR_MODE
    GPIO_K13    CAN0_TX
    GPIO_K14    CAN0_RX
    GPIO_K15    CAN1_TX
    GPIO_K16    CAN1_RX
    GPIO_K17    CAN2_TX
    GPIO_K18    CAN2_RX
    *********************************************/
    PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_IN, PMIO_VA_LEAKAGE_IN_PORT);

    PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_OUT_L, PMIO_VA_LEAKAGE_OUT_L_PORT);
    PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_OUT_H, PMIO_VA_LEAKAGE_OUT_H_PORT);

    PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_PU, PMIO_VA_LEAKAGE_PU_PORT);
    PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_PD, PMIO_VA_LEAKAGE_PD_PORT);
}


/* **********************************************************************************************/
/*                                             EXTERN FUNCTION                                  */
/* **********************************************************************************************/

void PMIO_ForceSTRModeTest
(
    void
)
{
    #ifdef PMIO_FEATURE_DEV_TEST_STR_ITERATION
    PMIO_D("req: suspend\n");
    PMIO_STR_SendMsg(PMIO_MSG_STR);
    PMIO_STR_RecvMsg(PMIO_MSG_STR_DONE);
    #else
    PMIO_D("str test config is disabled.\n");
    #endif
}

void PMIO_EarlyWakeUp
(
    void
)
{
    PMIOReason_t    tReason;
    #ifdef PMIO_FEATURE_USE_STR_MODE
    PMIOMode_t      tMode;
    #endif

    tReason     = PMIO_GetBootReason();
    psBOOT      = (uint32)tReason;

    #ifdef PMIO_FEATURE_USE_STR_MODE
    tMode       = PMIO_STR_GetBootMode();
    psMODE      = (uint32)tMode;

    PMIO_STR_SetPowerStatus(tMode, tReason);
    #endif

    PMIO_SetPort(PMIO_PORT_SEL_PCTLIO_OUT_H, PMIO_VA_PCTL_1_PORT);
    PMIO_SetPort(PMIO_PORT_SEL_PCTLIO_OUT_L, PMIO_VA_PCTL_0_PORT);

    PMIO_REG_PMGPIO_CTL &= ~PMIO_VA_BIT0;  //enable Latch
    PMIO_REG_PMGPIO_CTL &= ~PMIO_VA_BIT1;  //enable LVS
}

void PMIO_PowerDown
(
    PMIOMode_t tMode
)
{
    uint32 uiReadVa;

    uiReadVa = 0;

    PMIO_D("power down(%d) start\n", tMode);
    PMIO_SetGPKInterrupt(PMIO_VA_BITALL, \
            1/*0:level, 1:edge*/, 0/*0:low, 1:high*/, 0/*0:disable, 1:enable*/);

#ifdef PMIO_FEATURE_LEAKAGE_REDEUCTION_GPK_PORT
    PMIO_ReducePowerLeakageCurrent();
#endif

    /* set PCTL */
    PMIO_REG_PCTLIO_FS  =  ~(PMIO_VA_PCTL_1_PORT | PMIO_VA_PCTL_0_PORT); //0: ctrld by PCTLIO_DAT/EN
    PMIO_REG_PCTLIO_EN  =   (PMIO_VA_PCTL_1_PORT | PMIO_VA_PCTL_0_PORT); //1: output

#ifdef PMIO_FEATURE_DEV_TEST_BACKUP_RAM
    (void)PMIO_SetBackupRam(PMIO_VA_BACKUP_RAM_ADDR, 1UL);
#endif

    /*This bit must be set to “1” before core power (VDDI) is turned off.*/
    PMIO_REG_PMGPIO_CTL |=  (PMIO_VA_BIT1|PMIO_VA_BIT0); //disable Latch/LVS

    /* set wake-up event */
    PMIO_REG_PMGPIO_POL &= ~(PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT); //0:STR 1:STF
    PMIO_REG_PMGPIO_STR =  (PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT); // Clear evnet status

    PMIO_REG_PMGPIO_POL |=  (PMIO_VA_CAN_DET_PORT); //0:STR 1:STF
    PMIO_REG_PMGPIO_STF =  (PMIO_VA_CAN_DET_PORT); // Clear evnet status

    PMIO_REG_PMGPIO_EE0 =   (PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT|PMIO_VA_CAN_DET_PORT); //0:Disable 1: En

#ifdef PMIO_FEATURE_USE_STR_MODE
    PMIO_Delay(10000UL);

    if(tMode == PMIO_MODE_STR)
    {
        PMIO_REG_PMGPIO_CTL |= (PMIO_VA_BIT7|PMIO_VA_BIT6);
    	PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_OUT_H, PMIO_VA_STR_DET_PORT);
        PMIO_D("STR enable\n");
    }
    else if(tMode == PMIO_MODE_POWER_DOWN)
    {
        PMIO_REG_PMGPIO_CTL &= ~(PMIO_VA_BIT7|PMIO_VA_BIT6);
		PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_OUT_L, PMIO_VA_STR_DET_PORT);
        PMIO_D("STR disable\n");
    }
    else
    {
        PMIO_D("Unknown Power mode %d\n", (uint32)tMode);
    }
#endif

    if((PMIO_REG_PMGPIO_DI & PMIO_VA_ACC_DET_PORT) != 0UL)
    {
        /*Cold Rst*/
#ifdef PMIO_FEATURE_USE_STR_MODE
        PMIO_D("Check ACC:: Detect ACC back ON. \n");
        PMIO_D("Cold Reset.\n\n");
        PMIO_STR_SetPowerStatus(PMIO_MODE_NOT_SET, PMIO_REASON_ACC_ON);//Clear boot mode, boot reason
#endif
        uiReadVa = SAL_ReadReg(0x1b936008UL);
        SAL_WriteReg( ( uiReadVa & (~(0x1UL<<(uint32)10UL)) ), 0x1b936008UL );
        SAL_WriteReg(0x1UL, 0x1b93617CUL);
    }
    else
    {
        /*Power down*/
        PMIO_REG_PCTLIO_DAT = (~PMIO_VA_PCTL_1_PORT | PMIO_VA_PCTL_0_PORT); //PCTL1:LOW, PCTL0:HIGH
    }

    PMIO_Delay(0UL);
}

void PMIO_DebugPrintPowerStatus
(
    void
)
{
    PMIOReason_t tReason;
#ifdef PMIO_FEATURE_USE_STR_MODE
    PMIOMode_t tMode;
#endif

    tReason = (PMIOReason_t)psBOOT;
#ifdef PMIO_FEATURE_USE_STR_MODE
    tMode   = (PMIOMode_t)psMODE;
    mcu_printf("[PMIO: INFO ] Support STR mode\n");
#endif

#ifdef PMIO_FEATURE_DEV_TEST_BACKUP_RAM
    mcu_printf("[PMIO: INFO ] Support Test: BACKUP RAM TEST\n");
#endif

#ifdef PMIO_FEATURE_DEV_TEST_STR_ITERATION
    mcu_printf("[PMIO: INFO ] Support Test: STR ITERATION TEST\n");
#endif

#ifdef PMIO_FEATURE_DEV_TEST_CRST
    mcu_printf("[PMIO: INFO ] Support Test: COLDRESET TEST\n");
#endif

#ifdef PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
    mcu_printf("[PMIO: INFO ] Support Test: ALONE STR WITH AP BOOT READY TEST\n");
#endif



#ifdef PMIO_FEATURE_USE_STR_MODE
    switch(tMode)
    {
        case PMIO_MODE_NOT_SET :
        {
            PMIO_D("[PMIO: INFO ] mode PMIO_MODE_NOT_SET\n");
            break;
        }

        case PMIO_MODE_POWER_ON :
        {
            PMIO_D("[PMIO: INFO ] mode PMIO_MODE_POWER_ON\n");
            break;
        }

        case PMIO_MODE_ACTIVE :
        {
            PMIO_D("[PMIO: INFO ] mode PMIO_MODE_ACTIVE\n");
            break;
        }

        case PMIO_MODE_STR :
        {
            PMIO_D("[PMIO: INFO ] mode PMIO_MODE_STR\n");
            break;
        }

        case PMIO_MODE_POWER_DOWN :
        {
            PMIO_D("[PMIO: INFO ] mode PMIO_MODE_POWER_DOWN\n");
            break;
        }

        default :
        {
            PMIO_D("[PMIO: INFO ] mode I DONT KNOW\n");
            break;
        }
    }
#endif

    switch(tReason)
    {
        case PMIO_REASON_NOT_SET :
        {
            PMIO_D("[PMIO: INFO ] boot PMIO_REASON_NOT_SET\n");
            break;
        }

        case PMIO_REASON_BU_ON :
        {
            PMIO_D("[PMIO: INFO ] boot PMIO_REASON_BU_ON\n");
            break;
        }

        case PMIO_REASON_ACC_ON :
        {
            PMIO_D("[PMIO: INFO ] boot PMIO_REASON_ACC_ON\n");
            break;
        }

        case PMIO_REASON_CAN_ON :
        {
            PMIO_D("[PMIO: INFO ] boot PMIO_REASON_CAN_ON\n");
            break;
        }

        case PMIO_REASON_RTC_ON :
        {
            PMIO_D("[PMIO: INFO ] boot PMIO_REASON_RTC_ON\n");
            break;
        }

        default :
        {
            PMIO_D("[PMIO: INFO ] boot I DONT KNOW Boot\n");
            break;
        }
    }

#ifdef PMIO_FEATURE_USE_STR_MODE
    PMIO_D("[PMIO: INFO ] Timer %d\n", gSTRTime);
#endif
    if(PMIO_GetWdtReset() == 1U)
    {
        PMIO_D("[PMIO: INFO ] reset Occurred\n");
    }

#ifdef PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
    if(gAPBootOk == 1UL)
    {
        PMIO_D("[PMIO: INFO ] AP Boot status : OK.\n");
    }
    else
    {
        PMIO_D("[PMIO: INFO ] AP Boot status : NG.\n");
    }
#endif
}


void PMIO_Init
(
    void
)
{
    //OFFCTL[1:0], Once PCTL[1:0] output change from high to low state, keep low until PRST# active
    PMIO_REG_PMGPIO_CTL |= (PMIO_VA_BIT7|PMIO_VA_BIT6|PMIO_VA_BIT5);

    #ifdef PMIO_FEATURE_DEV_TEST_BACKUP_RAM
    if(psBOOT == PMIO_REASON_BU_ON)
    {
        PMIO_ClearBackupRAM(); //Clear power down reason information
    }
    #endif

    //Set PMGPIO port
    PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_IN, (PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT));

    //Clear Event
    PMIO_REG_PMGPIO_STR = PMIO_VA_BITALL;
    PMIO_REG_PMGPIO_STF = PMIO_VA_BITALL;

    #if (defined(PMIO_FEATURE_USE_STR_MODE) && (PMIO_VA_ACC_DET_PORT > PMIO_VA_BITCLEAR))
    if( (PMIO_REG_PMGPIO_DI & PMIO_VA_ACC_DET_PORT) == 0UL )
    {
        PMIO_SetRTCInterruptHandler();

        /*Interrupt for ACC DET*/
        PMIO_SetGPKInterrupt(PMIO_VA_ACC_DET_PORT, \
                1/*0:level, 1:edge*/, 1/*0:low, 1:high*/, 1/*0:disable, 1:enable*/);
        PMIO_STR_SetTimer(PMIO_VA_ACC_OFF_TIME_LIMIT, PMIO_TIMER_REASON_ACC_OFF);
    }
    else
    #endif
    {
        //Set Interrupt
        PMIO_SetGPKInterrupt((PMIO_VA_ACC_DET_PORT|PMIO_VA_BU_DET_PORT), \
                1/*0:level, 1:edge*/, 0/*0:low, 1:high*/, 1/*0:disable, 1:enable*/);
    }

    #ifdef PMIO_FEATURE_USE_STR_MODE
    PMIO_STR_OpenMsg();
    #endif

    #ifdef PMIO_FEATURE_DEV_TEST_CRST
    PMIO_RST_RunMsg();
    #endif
}

void PMIO_ClearBackupRAM
(
    void
)
{
    uint32 uiAddr;

    for(uiAddr = PMIO_VA_BACKUP_RAM_ADDR ;
            uiAddr < (PMIO_VA_BACKUP_RAM_ADDR + PMIO_VA_BACKUP_RAM_SIZE) ;
            uiAddr += 4UL)
    {
        SAL_WriteReg(PMIO_VA_BITCLEAR, uiAddr);
    }
}

uint32 PMIO_GetBackupRam
(
    uint32                              uiAddr
)
{
    if(uiAddr == 0UL)
    {
        return 0;
    }

    return SAL_ReadReg(uiAddr);
}

sint8 PMIO_SetBackupRam
(
    uint32                              uiAddr,
    uint32                              uiVa
)
{
    sint8 cRet;

    cRet    = 0;

    if(uiAddr < PMIO_VA_BACKUP_RAM_ADDR)
    {
        cRet = -1;
    }

    if(uiAddr > (PMIO_VA_BACKUP_RAM_ADDR + PMIO_VA_BACKUP_RAM_SIZE))
    {
        cRet = -1;
    }

    if(cRet == 0)
    {
        SAL_WriteReg(uiVa, uiAddr);
    }

    return cRet;
}

PMIOReason_t PMIO_GetBootReason
(
    void
)
{
    PMIOReason_t tBoot;
#ifdef PMIO_FEATURE_USE_STR_MODE
    uint32 uiReadVa;
#endif

    tBoot   = PMIO_REASON_NOT_SET;

#ifdef PMIO_FEATURE_USE_STR_MODE
    uiReadVa = (( SAL_ReadReg(PMIO_VA_PMU_MICOM_USER_ADDR) & 0x00000F00UL ) >> 8UL);

    if(uiReadVa == (uint32)PMIO_REASON_ACC_ON)
    {
        tBoot = PMIO_REASON_ACC_ON;
    }
    else
    {
#endif
        if((PMIO_REG_PMGPIO_STR & (uint32)PMIO_VA_BU_DET_PORT) == 0UL)
        {
            if((PMIO_REG_PMGPIO_STF & (uint32)PMIO_VA_BU_DET_PORT) == 0UL)
            {
                if((PMIO_REG_PMGPIO_STR & PMIO_VA_ACC_DET_PORT) != 0UL)
                {
                    tBoot = PMIO_REASON_ACC_ON;
                }
                else if((PMIO_REG_PMGPIO_STF & PMIO_VA_ACC_DET_PORT) != 0UL)
                {
                    tBoot = PMIO_REASON_ACC_ON;
                }
                else
                {
                    if((PMIO_REG_PMGPIO_STR & PMIO_VA_CAN_DET_PORT) != 0UL)
                    {
                        tBoot = PMIO_REASON_CAN_ON;
                    }

                    if((PMIO_REG_PMGPIO_STF & PMIO_VA_CAN_DET_PORT) != 0UL)
                    {
                        tBoot = PMIO_REASON_CAN_ON;
                    }
                }
            }
        }
        else
        {
            tBoot = PMIO_REASON_BU_ON;
        }
#ifdef PMIO_FEATURE_USE_STR_MODE
    }
#endif

    if(tBoot == PMIO_REASON_NOT_SET)
    {
        tBoot = PMIO_REASON_BU_ON;
    }

    #ifdef PMIO_FEATURE_DEV_TEST_STR_ITERATION
    tBoot = PMIO_REASON_ACC_ON;
    #endif

    return tBoot;
}

uint8 PMIO_GetWdtReset
(
    void
)
{
    const uint32 uiReset0 = 0x0F0F0F0FUL;
    const uint32 uiReset1 = 0x0F0F0F0FUL;
    const uint32 uiReset2 = 0x0F0F0F03UL;

    uint8 ucRet;
    uint32 uiRstSts0;
    uint32 uiRstSts1;
    uint32 uiRstSts2;

    ucRet = 0U;
    uiRstSts0 = SAL_ReadReg(PMIO_VA_PMU_RST_STS0_ADDR);
    uiRstSts1 = SAL_ReadReg(PMIO_VA_PMU_RST_STS1_ADDR);
    uiRstSts2 = SAL_ReadReg(PMIO_VA_PMU_RST_STS2_ADDR);

    if( (uiRstSts0 != uiReset0) || (uiRstSts1 != uiReset1) || (uiRstSts2 != uiReset2) )
    {
        //ucRet = 1U;
    }

    return ucRet;
}

void PMIO_SetGPK
(
    uint32 uiPort32
)
{
    PMIO_SetPort(PMIO_PORT_SEL_GPIO, uiPort32);
}

#ifdef PMIO_FEATURE_USE_STR_MODE
/*
 * FOR STR FUNCTION
 *
 * STR function part 1
 * Set/Get Power Status. (power mode, boot reason)
 */
static PMIOMode_t PMIO_STR_GetBootMode
(
    void
)
{
    /*
     *  PMIO_MODE_POWER_ON = 0,
     *  PMIO_MODE_ACTIVE = 1,
     *  PMIO_MODE_STR = 2,
     *  PMIO_MODE_POWER_DOWN = 3,
     */
    PMIOMode_t  tMode;

    tMode = PMIO_MODE_POWER_ON;

    if((PMIO_REG_PMGPIO_DI & PMIO_VA_STR_DET_PORT) != 0UL)
    {
        tMode = PMIO_MODE_STR;
    }

    return tMode;
}

void PMIO_STR_SetPowerStatus
(
    PMIOMode_t                          tMode,
    PMIOReason_t                        tReason
)
{
    /*[15:12]: tMode*/
    /*[11:8] : tReason*/
    uint32 uiReadVa;
    uint32 uiWriteVa;

    uiWriteVa   = (uint32)((((uint32)tMode & (uint32)0x0FUL) << (uint32)12UL) | \
                    (((uint32)tReason & (uint32)0x0FUL) << (uint32)8UL));
    uiReadVa    = ( SAL_ReadReg(PMIO_VA_PMU_MICOM_USER_ADDR) & 0xFFFF00FFUL);
    uiWriteVa |= uiReadVa;
    SAL_WriteReg(uiWriteVa, PMIO_VA_PMU_MICOM_USER_ADDR);
}

// STR function part 2
//
// Communication for Power Status. (send, recv)
//
static void PMIO_STR_GetCore
(
    uint32 *                            pMainCore,
    uint32 *                            pSubCore
)
{
    uint32  uiMain;
    uint32  uiSingle;

#ifdef PMIO_SUBFEATURE_STR_USE_BCONF_CORE_CHECK
    const uint32    uiBconfAddr = 0xD000000UL;

    uiMain      = ((readl(uiBconfAddr) >> 16UL) & 0x1UL);
    uiSingle    = ((readl(uiBconfAddr) >> 24UL) & 0x1UL);
#else
    uiMain      = 0UL;
    uiSingle    = 0UL;
    #ifdef PMIO_SUBFEATURE_STR_SINGLE_CORE_MODE
    uiSingle    = 1UL;
    #endif
#endif

    /*bconf*/
    /*main 0: ca72*/
    /*main 1: ca53*/
    /*single 0: multi core*/
    /*single 1: single core*/

    if((pMainCore != NULL_PTR) && (pSubCore != NULL_PTR))
    {
        if(uiMain == 0UL)
        {
            pMainCore[0] = 72UL;

            if(uiSingle == 0UL)
            {
                pSubCore[0] = 53UL;
            }
        }
        else
        {
            pMainCore[0] = 53UL;

            if(uiSingle == 0UL)
            {
                pSubCore[0] = 72UL;
            }
        }
    }
    else
    {
        PMIO_E("null param!\n");
    }
}

static void PMIO_STR_OpenMsg
(
    void
)
{
    static uint8    ucPmioOpenMsg       = 0U;
    static uint32   uiPmioReadMsgTaskId = 0UL;
    static uint32   uiPmioSendMsgTaskId = 0UL;
    static uint32   uiPmioReadMsgTaskStack[PMIO_VA_MSG_TASK_STACK_SIZE];
    static uint32   uiPmioSendMsgTaskStack[PMIO_VA_MSG_TASK_STACK_SIZE];
    int8            cMboxOpenName[7];

    cMboxOpenName[0] = 'P';
    cMboxOpenName[1] = 'O';
    cMboxOpenName[2] = 'W';
    cMboxOpenName[3] = 'E';
    cMboxOpenName[4] = 'R';
    cMboxOpenName[5] = '0';
    cMboxOpenName[6] = '\0';

    if(ucPmioOpenMsg == 0U)
    {
        ucPmioOpenMsg++;

        PMIO_STR_GetCore(&gMsgMainCore, &gMsgSubCore);

        gRecvMsgType = PMIO_MSG_NOT_SET;
        gSendMsgType = PMIO_MSG_NOT_SET;

        if((gMsgMainCore == 72UL) || (gMsgSubCore == 72UL))
        {
            gPmioMboxA72  = MBOX_DevOpen(MBOX_CH_CA72MP_NONSECURE, cMboxOpenName, 0U);
            PMIO_D("STR Msg Open : A72\n");
        }

        if((gMsgMainCore == 53UL) || (gMsgSubCore == 53UL))
        {
            gPmioMboxA53  = MBOX_DevOpen(MBOX_CH_CA53MP_NONSECURE, cMboxOpenName, 0U);
            PMIO_D("STR Msg Open : A53\n");
        }

        if ((SAL_EventCreate((uint32 *)&gRecvMsgTaskEventId, \
                        (const uint8 *)"msg signal event created", 0)) != SAL_RET_SUCCESS)
        {
            PMIO_E("pmio create event FAIL.\n");
        }

        if ((SAL_EventCreate((uint32 *)&gSendMsgTaskEventId, \
                        (const uint8 *)"msg signal event created", 0)) != SAL_RET_SUCCESS)
        {
            PMIO_E("pmio create event FAIL.\n");
        }

        if ((SAL_EventCreate((uint32 *)&gReadyMsgTaskEventId, \
                        (const uint8 *)"msg signal event created", 0)) != SAL_RET_SUCCESS)
        {
            PMIO_E("pmio create event FAIL.\n");
        }

        if((SAL_TaskCreate(&uiPmioReadMsgTaskId, \
                                (const uint8 *)"PMIO_STR_RecvMsgTask", \
                                (SALTaskFunc)&PMIO_STR_RecvMsgTask, \
                                &(uiPmioReadMsgTaskStack[0]), \
                                (uint32)PMIO_VA_MSG_TASK_STACK_SIZE, \
                                SAL_PRIO_POWER_MANAGER, \
                                NULL)) != SAL_RET_SUCCESS)
        {
            PMIO_E("pmio create task FAIL.\n");
        }

        if((SAL_TaskCreate(&uiPmioSendMsgTaskId, \
                                (const uint8 *)"PMIO_STR_SendMsgTask", \
                                (SALTaskFunc)&PMIO_STR_SendMsgTask, \
                                uiPmioSendMsgTaskStack, \
                                PMIO_VA_MSG_TASK_STACK_SIZE, \
                                SAL_PRIO_POWER_MANAGER, \
                                NULL)) != SAL_RET_SUCCESS)
        {
            PMIO_E("pmio create task FAIL.\n");
        }

        PMIO_STR_RecvMsg(PMIO_MSG_ACK); /*check AP Mbox ready*/
    }
    else
    {
        PMIO_D("pmio already open msg.\n");
    }
}
static PMIOMsg_t PMIO_STR_GetMsgData
(
    uint32                              *puiMsg
)
{
    PMIOMsg_t tRetMsg;
    uint32 uiIdx;
    uint8 uiCmd1;
    uint8 uiCmd2;
    uint8 uiCmd3;

    if(((PMIOMsg_t)puiMsg[0] == PMIO_MSG_ACK) && (gucOldMsgType == 0UL))
    {
        PMIO_D("Power message protocol change: [POWER MAILBOX] => [STR MSG] (old ver)\n");
        gucOldMsgType = 1UL;
    }

    if(gucOldMsgType == 1UL)
    {
        tRetMsg = (PMIOMsg_t)puiMsg[0];
    }
    else
    {
        tRetMsg = PMIO_MSG_NOT_SET;
        uiCmd1 = (uint8) ((puiMsg[0] >> 0UL ) & 0xffUL);
        uiCmd2 = (uint8) ((puiMsg[0] >> 8UL ) & 0xffUL);
        uiCmd3 = (uint8) ((puiMsg[0] >> 16UL) & 0xffUL);

        PMIO_D("msg %d, %d, %d\n", uiCmd1, uiCmd2, uiCmd3);

        for(uiIdx = 0UL; uiIdx < (uint32)PMIO_MSG_MAX; uiIdx++)
        {

            if(
                    (gMsgDataList[uiIdx].msgCmd1 == uiCmd1) &&
                    (gMsgDataList[uiIdx].msgCmd2 == uiCmd2) &&
                    (gMsgDataList[uiIdx].msgCmd3 == uiCmd3)
              )
            {
                tRetMsg = gMsgDataList[uiIdx].msgType;
            }
        }
    }

    return tRetMsg;
}

static void PMIO_STR_SetMsgData
(
    PMIOMsg_t                           uiMsgType,
    uint32                              *puiMsg
)
{
    uint32 uiCmd;

    if(gucOldMsgType == 1UL)
    {
        puiMsg[0] = uiMsgType;
    }
    else
    {
        uiCmd = (   (gMsgDataList[(uint32)uiMsgType].msgCmd1 << 0  ) |
                (gMsgDataList[(uint32)uiMsgType].msgCmd2 << 8  ) |
                (gMsgDataList[(uint32)uiMsgType].msgCmd3 << 16 )    );

        puiMsg[0] = uiCmd;
    }
}

static void PMIO_STR_RecvMsg
(
    PMIOMsg_t                           uiMsgType
)
{
    gRecvMsgType = uiMsgType;
    (void)SAL_EventSet(gRecvMsgTaskEventId, PMIO_VA_RECV_EVENT_NUM, SAL_EVENT_OPT_FLAG_SET);
}

static void PMIO_STR_SendMsg
(
    PMIOMsg_t                           uiMsgType
)
{
    gSendMsgType = uiMsgType;
    (void)SAL_EventSet(gSendMsgTaskEventId, PMIO_VA_SEND_EVENT_NUM, SAL_EVENT_OPT_FLAG_SET);
}

static void PMIO_STR_RecvMsgTask
(
    void *                              pArg
)
{
    static uint32   uiMboxAPReady = 0UL;
    SALRetCode_t    tEventRet;
    uint32          uiCmd[8];
    PMIOMsg_t       tMsgType;
    uint32          uiFlag;

    PMIO_STR_SetMsgData(PMIO_MSG_NOT_SET, uiCmd);
    uiFlag = 0UL;
    tMsgType = PMIO_MSG_NOT_SET;
    (void)pArg;

    while(1)
    {
        PMIO_STR_SetMsgData(PMIO_MSG_NOT_SET, uiCmd);
        tEventRet   = SAL_EventGet(gRecvMsgTaskEventId, PMIO_VA_RECV_EVENT_NUM, \
                0UL, \
                (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)),\
                &uiFlag);
        (void)SAL_EventSet(gRecvMsgTaskEventId, PMIO_VA_RECV_EVENT_NUM, SAL_EVENT_OPT_CLR_ALL);

        if (tEventRet == SAL_RET_SUCCESS)
        {
            PMIO_D("recv: wait (%s:%d)\n",
                    ((gRecvMsgType==PMIO_MSG_STR_DONE)?"PMIO_MSG_STR_DONE":
                     ((gRecvMsgType==PMIO_MSG_POWER_DOWN_DONE)?"PMIO_MSG_POWER_DOWN_DONE":
                      ((gRecvMsgType==PMIO_MSG_ACK)?"PMIO_MSG_ACK for AppReady":
                       "ETC Msg"))),
                    (uint32)gRecvMsgType);

            if(uiMboxAPReady != 0UL)
            {
                if(gMsgMainCore == 53UL)
                {
                    (void)MBOX_DevRecvCmd(gPmioMboxA53, uiCmd);
                }

                if(gMsgMainCore == 72UL)
                {
                    (void)MBOX_DevRecvCmd(gPmioMboxA72, uiCmd);
                }

                tMsgType = PMIO_STR_GetMsgData(uiCmd);

                if( tMsgType == gRecvMsgType)
                {
                    PMIO_D("recv: done (%s:%d)\n",
                            ((tMsgType==PMIO_MSG_STR_DONE) ?
                             "PMIO_MSG_STR_DONE" :
                             ((tMsgType==PMIO_MSG_POWER_DOWN_DONE) ?
                              "PMIO_MSG_POWER_DOWN_DONE" :
                              ((tMsgType==PMIO_MSG_ACK) ?
                               "PMIO_MSG_ACK for AppReady" :
                               "ETC Msg"))),
                            (uint32)tMsgType);

                    if(tMsgType == PMIO_MSG_POWER_DOWN_DONE)
                    {
                        PMIO_STR_SetTimer(0UL, PMIO_TIMER_REASON_NOT_SET);
                        PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
                    }
                    else if(tMsgType == PMIO_MSG_STR_DONE)
                    {
                        PMIO_STR_SetTimer(0UL, PMIO_TIMER_REASON_NOT_SET);
                        PMIO_PowerDown(PMIO_MODE_STR);
                    }
                    else
                    {
                        PMIO_E("Unexpected msg\n");
                    }

                }
                else
                {
                    PMIO_E("recv: Expect msg %d, Recv msg %d\n", (uint32)gRecvMsgType, (uint32)tMsgType);

                    if(tMsgType == PMIO_MSG_ACK)
                    {
                        PMIO_D("recv: Maybe AP reboot alone.\n");
                        (void)SAL_EventSet(gRecvMsgTaskEventId, PMIO_VA_RECV_EVENT_NUM, SAL_EVENT_OPT_FLAG_SET);
                    }
                }
            }
            else
            {
                if(gMsgMainCore == 53UL)
                {
                    (void)MBOX_DevRecvCmd(gPmioMboxA53, uiCmd);

                    tMsgType = PMIO_STR_GetMsgData(uiCmd);

                    if( (tMsgType == PMIO_MSG_ACK) )
                    {
                        if(gMsgSubCore == 72UL)
                        {
                            (void)MBOX_DevRecvCmd(gPmioMboxA72, uiCmd);

                            if( (PMIO_STR_GetMsgData(uiCmd) == PMIO_MSG_ACK) )
                            {
                                PMIO_D("A%d/A%d Multi core is READY\n", gMsgMainCore, gMsgSubCore);
                                uiMboxAPReady = 1UL;
                            }
                            else
                            {
                                PMIO_D("recv: A%d's MBOX is NOT READY. \n", gMsgSubCore);
                            }
                        }
                        else
                        {
                            PMIO_D("A%d Single core is READY\n", gMsgMainCore);
                            uiMboxAPReady = 1UL;
                        }
                    }
                    else if( (tMsgType == PMIO_MSG_AP_AWAKE) )
                    {
                        PMIO_D("AP system awakened. Set STR low\n");
                        PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_OUT_L, PMIO_VA_STR_DET_PORT);
                        (void)SAL_EventSet(gRecvMsgTaskEventId, PMIO_VA_RECV_EVENT_NUM, SAL_EVENT_OPT_FLAG_SET);
                    }
                    else
                    {
                        PMIO_D("recv: A%d's MBOX is NOT READY. %d\n", gMsgMainCore, (uint32)tMsgType );
                    }
                }
                else if(gMsgMainCore == 72UL)
                {
                    (void)MBOX_DevRecvCmd(gPmioMboxA72, uiCmd);

                    tMsgType = PMIO_STR_GetMsgData(uiCmd);

                    if( (tMsgType == PMIO_MSG_ACK) )
                    {
                        if(gMsgSubCore == 53UL)
                        {
                            (void)MBOX_DevRecvCmd(gPmioMboxA53, uiCmd);

                            if( (PMIO_STR_GetMsgData(uiCmd) == (uint32)PMIO_MSG_ACK) )
                            {
                                PMIO_D("A%d/A%d Multi core is READY\n", gMsgMainCore, gMsgSubCore);
                                uiMboxAPReady = 1UL;
                            }
                            else
                            {
                                PMIO_D("A%d's MBOX is NOT READY.\n", gMsgSubCore);
                            }
                        }
                        else
                        {
                            PMIO_D("A%d Single core is READY\n", gMsgMainCore);
                            uiMboxAPReady = 1UL;
                        }
                    }
                    else if( (tMsgType == PMIO_MSG_AP_AWAKE) )
                    {
                        PMIO_D("AP system awakened. Set STR low\n");
                        PMIO_SetPort(PMIO_PORT_SEL_PMGPIO_OUT_L, PMIO_VA_STR_DET_PORT);
                        (void)SAL_EventSet(gRecvMsgTaskEventId, PMIO_VA_RECV_EVENT_NUM, SAL_EVENT_OPT_FLAG_SET);
                    }
                    else
                    {
                        PMIO_D("A%d's MBOX is NOT READY. %d\n", gMsgMainCore, (uint32)tMsgType );
                    }
                }
                else
                {
                    PMIO_E("Cannot check the MainCore \n");
                }

                if(uiMboxAPReady == 1UL)
                {
                    //clear power status
                    PMIO_STR_SetPowerStatus(PMIO_MODE_NOT_SET, PMIO_REASON_NOT_SET);

                    (void)SAL_EventSet(gReadyMsgTaskEventId, \
                            PMIO_VA_READY_EVENT_NUM, SAL_EVENT_OPT_FLAG_SET);
                }
            }
        }
        else
        {
            PMIO_E("recv: event fail %d, %d\n", (uint32)tEventRet, uiFlag);
        }
    }
}

static void PMIO_STR_SendMsgTask
(
    void *                              pArg
)
{
    static uint32   uiSendBuf[8] = {0};
    SALRetCode_t    tEventRet;
    int32           iMboxRet;
    uint32          uiFlag;

    iMboxRet = (uint32)MBOX_SUCCESS;
    uiFlag = 0;
    (void)pArg;

    tEventRet = SAL_EventGet(gReadyMsgTaskEventId, PMIO_VA_READY_EVENT_NUM, \
            0UL, \
            (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)),\
            &uiFlag);

    if(tEventRet == SAL_RET_SUCCESS)
    {
        while(1)
        {
            tEventRet = SAL_EventGet(gSendMsgTaskEventId, PMIO_VA_SEND_EVENT_NUM, \
                    0UL, \
                    (((uint32) SAL_EVENT_OPT_SET_ANY)|((uint32) SAL_OPT_BLOCKING)),\
                    &uiFlag);
            (void)SAL_EventSet(gSendMsgTaskEventId, PMIO_VA_SEND_EVENT_NUM, SAL_EVENT_OPT_CLR_ALL);

            if(tEventRet == SAL_RET_SUCCESS)
            {
                if(gSendMsgType != PMIO_MSG_NOT_SET)
                {
                    PMIO_STR_SetMsgData(gSendMsgType, uiSendBuf);

                    if((gMsgMainCore == 72UL) || (gMsgSubCore == 72UL))
                    {
                        iMboxRet |= MBOX_DevSendCmd(gPmioMboxA72, uiSendBuf);
                    }

                    if((gMsgMainCore == 53UL) || (gMsgSubCore == 53UL))
                    {
                        iMboxRet |= MBOX_DevSendCmd(gPmioMboxA53, uiSendBuf);
                    }

                    if(iMboxRet == (uint32)MBOX_SUCCESS)
                    {
                        if(gSendMsgType == PMIO_MSG_STR || gSendMsgType == PMIO_MSG_POWER_DOWN)
                        {
                            PMIO_STR_SetTimer(PMIO_VA_STR_REQ_TIME_LIMIT, PMIO_TIMER_REASON_STR_REQ);
                        }
                    }
                    else
                    {
                        PMIO_E("send fail %d\n", iMboxRet);
                    }
                }
            }
            else
            {
                PMIO_E("send: event fail %d, %d\n", (uint32)tEventRet, uiFlag);
            }
        }
    }
    else
    {
        PMIO_E("send: app ready event fail %d, %d\n", (uint32)tEventRet, uiFlag);
    }
}

// STR function part 3
//
// Control Timer. (RTC, timer)
//
void PMIO_STR_SetTimer
(
    uint32                              uiUsec,
    PMIOTimerReason_t                   uiReason
)
{
    gSTRTime = uiUsec;
    gSTRTimerReason = uiReason;

    if(uiUsec != 0UL)
    {
        (void) TIMER_Enable((TIMERChannel_t)PMIO_VA_TIMER_CH, uiUsec, \
                &PMIO_STR_TimeoutInterruptHandler, NULL);

        (void) GIC_IntSrcEn((uint32)GIC_TIMER_0 + PMIO_VA_TIMER_CH);
        PMIO_D("pmio Set Timer %d, reason %d\n", uiUsec, uiReason);
    }
    else
    {
        (void) TIMER_Disable((TIMERChannel_t)PMIO_VA_TIMER_CH);
        PMIO_D("pmio Set Timer disable, reason clear\n");
    }
}
static sint32 PMIO_STR_TimeoutInterruptHandler
(
    TIMERChannel_t                      iChannel,
    void *                              pArgs
)
{
    uint32 uiTimeoutReason;

    uiTimeoutReason = gSTRTimerReason;
    (void)pArgs;

    if((iChannel == (TIMERChannel_t)PMIO_VA_TIMER_CH))
    {
        PMIO_STR_SetTimer(0UL, PMIO_TIMER_REASON_NOT_SET);

        PMIO_SetGPKInterrupt(PMIO_VA_BITALL, \
                1/*0:level, 1:edge*/, 0/*0:low, 1:high*/, 0/*0:disable, 1:enable*/);

        if(uiTimeoutReason == PMIO_TIMER_REASON_ACC_OFF)
        {
            PMIO_D("Timeout : ACC ON signal\n");
            PMIO_D("Request : STR mode\n");
        #ifdef PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
            if(gAPBootOk == 0UL)
            {
                PMIO_PowerDown(PMIO_MODE_STR);
            }
            else
        #endif
            {
                PMIO_STR_SendMsg(PMIO_MSG_STR);
                PMIO_STR_RecvMsg(PMIO_MSG_STR_DONE);
            }
        }
        else if(uiTimeoutReason == PMIO_TIMER_REASON_STR_REQ)
        {
            PMIO_D("Timeout : STR DONE msg\n");
            PMIO_D("Go to   : POWER DOWN mode\n");
            gRecvMsgType = PMIO_MSG_NOT_SET;
            PMIO_PowerDown(PMIO_MODE_POWER_DOWN);
        }
        else
        {
            PMIO_E("Unknown time out reason %d\n", uiTimeoutReason);
        }
    }
    else
    {
        PMIO_D("TimerIntr fail null or mismatched ch %d\n", (uint32)iChannel);
    }
    return (sint32)SAL_RET_SUCCESS;
}
#endif

void PMIO_APBootOk
(
    void
)
{
#ifdef PMIO_FEATURE_DEV_TEST_ALONE_STR_WITH_AP_BOOT_READY
    gAPBootOk = 1UL;
    PMIO_D("Ap boot is Ok.\n");
#endif
}

#ifdef PMIO_FEATURE_DEV_TEST_CRST
/*
 * FOR ColdReset test function
 *
 */
static void PMIO_RST_RunMsg
(
    void
)
{
    static uint8    ucPmioOpenMsg                           = 0U;
    static uint32   uiPmioReadMsgTaskId[PMIO_EX_AP_CH_MAX]  = {0UL, 0UL};
    static uint32   uiPmioReadMsgTaskStack[PMIO_EX_AP_CH_MAX][PMIO_VA_MSG_TASK_STACK_SIZE];

    if(ucPmioOpenMsg == 0U)
    {
        ucPmioOpenMsg++;

        if((SAL_TaskCreate(&(uiPmioReadMsgTaskId[(uint32)PMIO_EX_AP_CH_A72]), \
                                (const uint8 *)"PMIO_RST_RecvMsgTask_A72", \
                                (SALTaskFunc)&PMIO_RST_RecvMsgTask_A72, \
                                &(uiPmioReadMsgTaskStack[(uint32)PMIO_EX_AP_CH_A72][0]), \
                                (uint32)PMIO_VA_MSG_TASK_STACK_SIZE, \
                                SAL_PRIO_POWER_MANAGER, \
                                NULL)) != SAL_RET_SUCCESS)
        {
            PMIO_E("ResetMsg create task FAIL(A72).\n");
        }

        if((SAL_TaskCreate(&(uiPmioReadMsgTaskId[(uint32)PMIO_EX_AP_CH_A53]), \
                                (const uint8 *)"PMIO_RST_RecvMsgTask_A53", \
                                (SALTaskFunc)&PMIO_RST_RecvMsgTask_A53, \
                                &(uiPmioReadMsgTaskStack[(uint32)PMIO_EX_AP_CH_A53][0]), \
                                (uint32)PMIO_VA_MSG_TASK_STACK_SIZE, \
                                SAL_PRIO_POWER_MANAGER, \
                                NULL)) != SAL_RET_SUCCESS)
        {
            PMIO_E("ResetMsg create task FAIL(A53).\n");
        }
    }
    else
    {
        PMIO_D("ResetMsg already open msg.\n");
    }
}

static void PMIO_RST_RecvMsgTask_A72
(
    void *                              pArg
)
{
    (void)pArg;
    PMIO_RST_RecvMsgTask(PMIO_EX_AP_CH_A72);
}

static void PMIO_RST_RecvMsgTask_A53
(
    void *                              pArg
)
{
    (void)pArg;
    PMIO_RST_RecvMsgTask(PMIO_EX_AP_CH_A53);
}

static void PMIO_RST_RecvMsgTask
(
    PMIOExApCh_t    tCh
)
{
    MBOXDvice_t *           tPmioMbox;
    uint32                  uiCmd[8];
    uint32                  uiReadVa;
    PMIOExMsg_t             tMsg;
    int8                    cMboxOpenName[7];

    (void)SAL_MemSet(uiCmd, 0x0U, sizeof(uiCmd));
    uiReadVa    = 0UL;
    tMsg        = PMIO_EX_MSG_NOT_SET;
    cMboxOpenName[0] = 'R';
    cMboxOpenName[1] = 'E';
    cMboxOpenName[2] = 'S';
    cMboxOpenName[3] = 'E';
    cMboxOpenName[4] = 'T';
    cMboxOpenName[5] = '0';
    cMboxOpenName[6] = '\0';

    if(tCh == PMIO_EX_AP_CH_A72)
    {
        tPmioMbox  = MBOX_DevOpen(MBOX_CH_CA72MP_NONSECURE, cMboxOpenName, 0U);
        PMIO_D("Running Cold Reset msg task a72.\n");
    }
    else
    {
        tPmioMbox  = MBOX_DevOpen(MBOX_CH_CA53MP_NONSECURE, cMboxOpenName, 0U);
        PMIO_D("Running Cold Reset msg task a53.\n");
    }

    while(1)
    {
        uiCmd[0]    = 0;
        (void)MBOX_DevRecvCmd(tPmioMbox, uiCmd);
        tMsg = (PMIOExMsg_t)(uiCmd[0]);

        if(tMsg == PMIO_EX_MSG_COLD_RESET)
        {
            PMIO_D("recv(%s): PMIO_EX_MSG_COLD_RESET\n",
                    (tCh == PMIO_EX_AP_CH_A72 ? "A72" : "A53"));

            uiReadVa = SAL_ReadReg(0x1b936008UL);
            SAL_WriteReg( ( uiReadVa & (~(0x1UL<<(uint32)10UL)) ), 0x1b936008UL );
            SAL_WriteReg(0x1UL, 0x1b93617CUL);
        }
        else
        {
            PMIO_E("recv(%s): Unknown msg. (%d)\n",
                    (tCh == PMIO_EX_AP_CH_A72 ? "A72" : "A53"), uiCmd[0]);
        }
        //SAL_TaskSleep(500);
    }
}
#endif

