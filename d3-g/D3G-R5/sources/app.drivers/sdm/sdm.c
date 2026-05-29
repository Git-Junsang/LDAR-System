/*
***************************************************************************************************
*
*   FileName : sdm.c
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

#include "sdm.h"
#include "gpsb_reg.h"
#include "pmu_reset.h"

/*
 ***************************************************************************************************
 *                                             DEFINITIONS
 ***************************************************************************************************
 */
#if (DEBUG_ENABLE)
    #include "debug.h"
    #define SDM_D(fmt, args...)          {LOGD(DBG_TAG_SDM, fmt, ## args)}
    #define SDM_E(fmt, args...)          {LOGE(DBG_TAG_SDM, fmt, ## args)}
#else
    #define SDM_D(fmt, args...)
    #define SDM_E(fmt, args...)
#endif

#define SDM_VA_SPI_DMA_SIZE             (256UL)
#define SDM_VA_SPI_CH                   (uint8)(GPSB_CH_3)
#define SDM_VA_SPI_SPEED_HZ             ((20UL) * (1000UL) * (1000UL))
#define SDM_VA_SPI_MAX_SIZE             (128UL)
#define SDM_VA_MAX_ADDR                 (0x910000UL)

 /*************************************************************************************************/
 /*                                             STATIC VARIABLE                                   */
 /*************************************************************************************************/
static uint8 gucSdmDrvSpiRun = 0U;

 /*************************************************************************************************/
 /*                                             STATIC FUNCTION                                   */
 /*************************************************************************************************/
static SDMError_t SDM_OpenSpi
(
    void
);

static void SDM_CloseSpi
(
    void
);

static SDMError_t SDM_LockReg
(
    void
);

static SDMError_t SDM_UnLockReg
(
    void
);

static SDMError_t SDM_OpenSpi
(
    void
)  /* SWU-SDM_D.019 */
{
    SDMError_t      ucRet;
    SALRetCode_t    iSpiRet;

    ucRet   = SDM_ERR_NONE;
    iSpiRet = GPSB_Open(SDM_VA_SPI_CH, 0L, 0L, 0L, NULL, NULL, \
            (uint32)SDM_VA_SPI_DMA_SIZE, NULL, NULL_PTR);

    if(iSpiRet != SAL_RET_SUCCESS)
    {
        SDM_E("ch %d open error ret %d\n",SDM_VA_SPI_CH, iSpiRet);
        ucRet = SDM_ERR_SPI_NOT_INIT;
    }
    else
    {
        iSpiRet = GPSB_SetSpeed(SDM_VA_SPI_CH, SDM_VA_SPI_SPEED_HZ);

        if(iSpiRet != SAL_RET_SUCCESS)
        {
            SDM_E("%s ch %d setting speed error ret %d\n", __func__, (uint32)SDM_VA_SPI_CH, (uint32)iSpiRet);
            ucRet = SDM_ERR_SPI_NOT_INIT;
        }
        else
        {
            /* not working noswap mode
            iSpiRet = GPSB_SetBpwNoSwap(ch, bpw);
            */
            (void) GPSB_SetBpw(SDM_VA_SPI_CH, 32UL);
            (void) GPSB_SetMode(SDM_VA_SPI_CH, (uint32)GPSB_MODE_0);

            gucSdmDrvSpiRun = 1U;
        }
    }

    if(ucRet == SDM_ERR_SPI_NOT_INIT)
    {
        SDM_CloseSpi();
    }

    return ucRet;
}

static void SDM_CloseSpi
(
    void
)
{
    SALRetCode_t iSpiRet;

    iSpiRet = GPSB_Close(SDM_VA_SPI_CH);

    if(iSpiRet == SAL_RET_SUCCESS)
    {
        gucSdmDrvSpiRun = 0U;
    }
}

SDMError_t SDM_ReadReg
(
    uint32                              uiAddr,
    uint32 *                            piValue
)
{
    uint8           ucOpCode[SDM_VA_SPI_MAX_SIZE + 3UL];
    uint8           ucDummy[SDM_VA_SPI_MAX_SIZE + 3UL];
    SDMError_t      ucRet;
    SALRetCode_t    iRet;
    uint32          uhwCheckSum;
    uint32          uhwReadCs;
    uint32          uiIdx;

    ucRet       = SDM_ERR_NONE;
    iRet        = SAL_RET_SUCCESS;
    uhwCheckSum = 0;
    uhwReadCs   = 0;
    uiIdx       = 0;
    (void)SAL_MemSet(ucOpCode, 0x0U, SDM_VA_SPI_MAX_SIZE + 3UL);
    (void)SAL_MemSet(ucDummy, 0x0U,  SDM_VA_SPI_MAX_SIZE + 3UL);

    if(gucSdmDrvSpiRun == 0U)
    {
        SDM_E("%s] spi not opened\n",__func__);
        ucRet = SDM_ERR_SPI_NOT_INIT;
    }
    else if((piValue == (uint32*)NULL) || (uiAddr > (uint32)SDM_VA_MAX_ADDR) )
    {
        SDM_E("%s] Invalid param is invalid err\n",__func__);
        ucRet = SDM_ERR_INVALID_PARAM;
    }
    else
    {
        //SAL_MemSet((void*)ucOpCode, 0U, (SDM_VA_SPI_MAX_SIZE+3UL));
        /* OP
         * code
         * */
        ucOpCode[uiIdx] = 0x0BU;
        uiIdx++;
        ucOpCode[uiIdx] = (uint8)(uiAddr >> (uint32)16UL);
        uiIdx++;
        ucOpCode[uiIdx] = (uint8)(uiAddr >> (uint32)8UL);
        uiIdx++;
        ucOpCode[uiIdx] = (uint8)(uiAddr & (uint32)0xffUL);
        uiIdx++;
        ucOpCode[uiIdx] = (uint8)(3U);
        uiIdx++;
        /* write
         * data
         * */
        iRet = GPSB_Xfer(SDM_VA_SPI_CH, (const void*)ucOpCode, (void*)ucDummy, \
                12UL, (uint8)GPSB_XFER_MODE_WITHOUT_INTERRUPT);
        uhwCheckSum = (uint32)((uint32)(ucDummy[uiIdx]) + (uint32)(ucDummy[uiIdx+ 1UL]) + \
                      (uint32)(ucDummy[uiIdx+2UL]) + (uint32)(ucDummy[uiIdx+ 3UL]));
        uhwReadCs = ((uint32)ucDummy[uiIdx + 4UL] << (uint32)8UL) + (uint32)ucDummy[uiIdx + 5UL];

        if((iRet == SAL_RET_SUCCESS) && (uhwCheckSum == uhwReadCs))
        {
            *piValue =  (uint32)ucDummy[uiIdx] << (uint32)24UL;
            *piValue |= (uint32)ucDummy[uiIdx + 1UL] << (uint32)16UL;
            *piValue |= (uint32)ucDummy[uiIdx + 2UL] << (uint32)8UL;
            *piValue |= (uint32)ucDummy[uiIdx + 3UL];
        }
        else
        {
            SDM_E("%s] Invalid crc calulate x%x, readed x%x or xfer ret x%x\n", \
                    __func__,uhwCheckSum,uhwReadCs,(uint32)iRet);
            ucRet = SDM_ERR_XFER_REG;
        }
    }

    return ucRet;
}

SDMError_t SDM_WriteReg
(
    uint32                              uiAddr,
    const uint8 *                       pValue,
    uint32                              uiSize
)
{
    SDMError_t      ucRet;
    SALRetCode_t    iSpiRet;
    uint32          uiAddr_temp;
    uint32          uiSize_temp;
    uint8           ucOpCode[SDM_VA_SPI_MAX_SIZE + 16UL];
    uint8           ucDummy[SDM_VA_SPI_MAX_SIZE + 16UL];
    uint16          uhwChecksum;
    uint32          uiIdx;
    uint32          uiSendDataSize;
    uint32          uiTempIdx;
    uint32          uiPos;

    ucRet           = SDM_ERR_NONE;
    iSpiRet         = SAL_RET_SUCCESS;
    uiAddr_temp     = uiAddr;
    uiSize_temp     = uiSize;
    uhwChecksum     = 0;
    uiIdx           = 0UL;
    uiSendDataSize  = 4UL;
    uiTempIdx       = 0UL;
    uiPos           = 0UL;

    if(gucSdmDrvSpiRun == 0U)
    {
        SDM_E("%s] spi not opened\n",__func__);
        ucRet = SDM_ERR_SPI_NOT_INIT;
    }
    else if((pValue == (uint8*)NULL) || \
            (uiSize == 0UL) || \
            (uiAddr > SDM_VA_MAX_ADDR) || \
            ((uiAddr + uiSize ) > SDM_VA_MAX_ADDR ) || \
            ((uiSize % 4UL) != 0UL ))
    {
        SDM_E("%s] Invalid param is invalid err,(addr x%x, Size %d)\n",__func__,uiAddr, uiSize);
        ucRet = SDM_ERR_INVALID_PARAM;
    }
    else
    {

        while(uiSize_temp > 0UL)
        {
            (void)SAL_MemSet(ucOpCode, 0U, (SDM_VA_SPI_MAX_SIZE+6UL));

            /* OP code */
            ucOpCode[uiIdx] = 0x02U;
            uiIdx++;
            ucOpCode[uiIdx] = (uint8)(uiAddr_temp >> (uint32)16UL);   /*MSB */
            uiIdx++;
            ucOpCode[uiIdx] = (uint8)(uiAddr_temp >> (uint32)8UL);
            uiIdx++;
            ucOpCode[uiIdx] = (uint8)(uiAddr_temp & (uint32)0xffUL);
            uiIdx++;

            if(uiSize_temp > SDM_VA_SPI_MAX_SIZE)
            {
                uiSendDataSize = (uint32)(SDM_VA_SPI_MAX_SIZE);
            }
            else
            {
                uiSendDataSize = (uint32)(uiSize_temp);
            }

            uiSendDataSize  = ((uiSendDataSize + 3UL) / 4UL) * 4UL;
            ucOpCode[uiIdx] = (uint8)(uiSendDataSize -1UL);
            uiIdx++;
            /* write data */
            (void)SAL_MemCopy((void*)(ucOpCode+uiIdx), (const void*)(pValue+uiPos), (SALSize)uiSendDataSize);

            for(uiTempIdx = uiIdx ; uiTempIdx < (uiSendDataSize + uiIdx) ; uiTempIdx++)
            {
                uhwChecksum += (uint16)ucOpCode[uiTempIdx];
            }

            uiIdx += uiSendDataSize;
            ucOpCode[uiIdx] = (uint8)(uhwChecksum >> (uint16)8);    /*MSB */
            uiIdx++;
            ucOpCode[uiIdx] = (uint8)(uhwChecksum & (uint16)0xff);
            uiIdx++;
            uiIdx = ((uiIdx + 3UL) / 4UL) * 4UL;
            uiSize_temp -= uiSendDataSize;

            //SDM_DBG("%s]2 uiAddr_temp x%x write uiIdx [%d] uiSendDataSize %d\n",__func__,uiAddr_temp, uiIdx, uiSendDataSize);

            iSpiRet = GPSB_Xfer(SDM_VA_SPI_CH, (const void*)ucOpCode, (void*)ucDummy, uiIdx, \
                    (uint8)GPSB_XFER_MODE_WITHOUT_INTERRUPT);
            uiAddr_temp += uiSendDataSize;
            uiPos += uiSendDataSize;
            uiIdx = 0UL;
            uhwChecksum = 0;

            if(iSpiRet != SAL_RET_SUCCESS)
            {
                SDM_E("%s] spi transfer err, (addr x%x, Size %d)\n",__func__,uiAddr, uiSize);
                ucRet = SDM_ERR_XFER_REG;
                break;
            }
        }
    }

    return ucRet;
}

static SDMError_t SDM_LockReg
(
    void
)   /* SWU-SDM_D.015 */
{
    const uint8 ucSdmLock[4] = {0xff, 0xff, 0xff, 0xff};
    SDMError_t  ucRet;

    ucRet = SDM_ERR_NONE;
    ucRet = SDM_WriteReg((uint32)SDM_VA_VS_ADDR, ucSdmLock, 4UL);

    return ucRet;
}

static SDMError_t SDM_UnLockReg
(
    void
)   /* SWU-SDM_D.016 */
{
    const uint8 ucSdmUnLock[4] = {0x1a, 0xcc, 0xe5, 0x51};
    SDMError_t  ucRet;

    ucRet = SDM_ERR_NONE;
    ucRet = SDM_WriteReg((uint32)SDM_VA_VS_ADDR, ucSdmUnLock, 4UL);

    return ucRet;
}

SDMError_t SDM_Init
(
    void
)
{
    const uint8 ucVa[4]         = {0,0,0,2}; /*DDI sel */
    const uint8 ucVaTmCtl4[4]   = {0x07,0x80,0x00,0x00};
    const uint8 ucVaTmCtl6[4]   = {0x02,0xd0,0x00,0x00};
    SDMError_t  ucRet;

    ucRet   = SDM_ERR_NONE;

    if(gucSdmDrvSpiRun == 0U)
    {
        ucRet = SDM_OpenSpi();
    }

    if( ucRet == SDM_ERR_NONE)
    {
        ucRet = SDM_UnLockReg();

        if(ucRet == SDM_ERR_NONE)
        {
            PMU_ResetSDMRstMskCfg(); /* Set reset mask when SDM is running. It's depend on reset scenario */

            (void)SDM_WriteReg(0x820040UL, (const uint8*)ucVaTmCtl4, 4UL);
            (void)SDM_WriteReg(0x820048UL, (const uint8*)ucVaTmCtl6, 4UL);
            (void)SDM_WriteReg(0x10004cUL, (const uint8*)ucVa, 4UL);

            ucRet = SDM_LockReg();
            SDM_D("SDM sets default display output.\n");

            #if 0
            if(ucRet == SDM_ERR_NONE)
            {
                SDM_CloseSpi();
            }
            #endif
        }
    }

    return ucRet;
}

