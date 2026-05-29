/*
***************************************************************************************************
*
*   FileName : i2c.c
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

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

/*                                                                                               */
/*************************************************************************************************/
/*                                             LOCAL VARIABLES                                   */
/*************************************************************************************************/


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

static void I2C_Reinit
(
    uint8                               ucCh
);

static SALRetCode_t I2C_Enable
(
    uint8                               ucCh
);

static SALRetCode_t I2C_Reset
(
    uint8                               ucCh
);

static SALRetCode_t I2C_SetPort
(
    uint8                               ucCh
);

static void I2C_EnableIrq
(
    uint8                               ucCh
);

static void I2C_DisableIrq
(
    uint8                               ucCh
);

static void I2C_XferComplete
(
    uint8                               ucCh,
    uint32                              ucEvent
);

static void I2C_ClearIntr
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_EnableIrq
*
* I2C Enable interrupt function.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_EnableIrq
(
    uint8                               ucCh
)
{
    uint32  val;
    uint32  reg;

    val = 0UL;
    reg = 0UL;

    val = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CTRL)) | BSP_BIT_06;
    reg = (uint32)(i2c[ucCh].dBase + I2C_CTRL);
    SAL_WriteReg(val, reg);

    return;
}

/*
***************************************************************************************************
*                                          I2C_DisableIrq
*
* I2C Disnable interrupt function.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

void I2C_DisableIrq
(
    uint8                               ucCh
)
{
    uint32  val;
    uint32  reg;

    val = 0UL;
    reg = 0UL;

    val = (uint32)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CTRL)) & ~BSP_BIT_06);
    reg = (uint32)(i2c[ucCh].dBase + I2C_CTRL);
    SAL_WriteReg(val, reg);

    val = (uint32)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)) | BSP_BIT_00);
    reg = (uint32)(i2c[ucCh].dBase + I2C_CMD);
    SAL_WriteReg(val, reg);

    return;

}

/*
***************************************************************************************************
*                                          I2C_XferComplete
*
* I2C Call callback function for notification.
*
* @param    ucCh [in]       : Value of channel to control
*           ucEvent [in]    : Value of event
* @return
* Notes
*
***************************************************************************************************
*/

void I2C_XferComplete
(
    uint8   ucCh,
    uint32  ucEvent
)
{
    I2C_DisableIrq(ucCh);
    i2c[ucCh].dState = I2C_STATE_IDLE;

    if(i2c[ucCh].dComplete.ccCallBack != NULL_PTR)
    {
        i2c[ucCh].dComplete.ccCallBack(ucCh, (ucEvent), i2c[ucCh].dComplete.ccArg);
    }
}

/*
***************************************************************************************************
*                                          I2C_ClearIntr
*
* I2C Interrupt clear function.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

void I2C_ClearIntr
(
    uint8                               ucCh
)
{
    uint32  val;
    uint32  reg;

    val = 0UL;
    reg = 0UL;
    val = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)) | BSP_BIT_00;
    reg = i2c[ucCh].dBase + I2C_CMD;
    SAL_WriteReg(val, reg);

    return;
}


/*
***************************************************************************************************
*                                          I2C_ScanSlave
*
* Function to scan slave device address received ACK about selected channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   slave address that scaned second.
* Notes
*
***************************************************************************************************
*/

uint32 I2C_ScanSlave
(
    uint8                               ucCh
)
{
    uint32          i;
    uint32          j;
    uint8           dummy;
    uint32          rd_opt;
    SALRetCode_t    ret;
    uint32          detect_cnt;
    uint32          return_addr;

    i           = 0xffUL;
    j           = 0xffUL;
    dummy       = (uint8)0UL;
    rd_opt      = 0UL;
    ret         = SAL_RET_SUCCESS;
    detect_cnt  = 0UL;
    return_addr = 0UL;
    I2C_D("Start I2C scan slave \n");

    for(i = 0 ; i < 0xFFUL ; i+= 16UL)
    {
        for(j = 0 ; j < 16UL ; j++)
        {
            if(rd_opt != 0UL)
            {
                ret = I2C_XferCmd(ucCh, (uint8)(((i+j) & 0x7fUL) << 1UL), (uint8)1UL, (uint8 *)&dummy, NULL,
                            (uint8 *)NULL_PTR, (uint8)1UL, (uint8 *)&dummy, NULL, NULL);
            }
            else
            {
                ret = I2C_XferCmd(ucCh, (uint8)(((i+j) & 0x7fUL) << 1UL), (uint8)1UL, (uint8 *)&dummy, NULL,
                            (uint8 *)NULL_PTR, NULL, (uint8 *)NULL_PTR, NULL, NULL);
            }

            if(ret == SAL_RET_SUCCESS)
            {
                detect_cnt++;
                I2C_D("Detected I2C Slave Addr [%02x]\n", i+j);

                if(detect_cnt == 1UL)
                {
                    return_addr = i+j;
                }
            }
        }
    }

    I2C_D("Finish I2C scan slave. return address value : %x \n", return_addr);

    return return_addr;
}

/*
***************************************************************************************************
*                                          I2C_BusyCheck
*
* Fucntion to check whether i2c is busy.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_BusyCheck
(
    uint8                               ucCh
)
{
    uint32          status;
    SALRetCode_t    ret;
    uint32          cnt;

    status  = 0UL;
    ret     = SAL_RET_SUCCESS;
    cnt     = 0UL;
    status = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));

    while(status != (uint32)NULL)
    {
        if((status & I2C_SR_BUSY) != (uint32)NULL)
        {
            I2C_D("I2C Busy ! \n");
        }
        else
        {
            break;
        }

        if(cnt > (uint32)I2C_BUSY_TIMEOUT)
        {
            I2C_D("I2C Busy check timeout. \n");
            ret = SAL_RET_FAILED;

            return ret;
        }

        cnt++;
        status = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2c_Reinit
*
* Function to Re-initialize I2C for selected channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_Reinit
(
    uint8                               ucCh
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if (ucCh > (uint32)I2C_CH_NUM)
    {
        I2C_D("%s %d channel is wrong\n", __func__, ucCh);

        return;
    }

    (void)SAL_CoreCriticalEnter();
    i2c[ucCh].dState = I2C_STATE_DISABLED;
    (void)SAL_MemSet(&i2c[ucCh].dAsync, 0, sizeof(I2CAsyncXfer_t));
    (void)SAL_CoreCriticalExit();
    I2C_DisableIrq(ucCh);
    I2C_ClearIntr(ucCh);
    ret = I2C_Reset(ucCh);

    if (ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s ch %d failed to reset core\n", __func__, ucCh);
    }

    ret = I2C_SetPort(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s ch %d failed to set port\n", __func__, ucCh);
    }

    ret = I2C_Enable(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s ch %d failed to enable core\n", __func__, ucCh);
    }

    (void)SAL_CoreCriticalEnter();
    i2c[ucCh].dState = I2C_STATE_IDLE;
    (void)SAL_CoreCriticalExit();

    return;
}

/*
***************************************************************************************************
*                                          I2C_WaitTIP
*
* Function to check transfer status and to wait TIP (Transfer in progress) complete.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*           : Wait until cmd operation is working correctly
* Notice
*           : If timeout occur, check SCL and SDA signal status
*             If SCL is low by slave devices during the byte transfer, CMD is not completed
*
***************************************************************************************************
*/

static SALRetCode_t I2C_Wait
(
    uint8                               ucCh
)
{
    SALRetCode_t    ret;
    uint32          cnt;

    ret = SAL_RET_SUCCESS;
    cnt = 0UL;

    /* Check STA , STO , RD , WR is 0. */
    while((SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)) & I2C_CMD_PROGRESS_MASK) != 0UL)
    {
        if(cnt > I2C_POLL_TIMEOUT)
        {
            I2C_D("%s: ch %d non interrupt time out, cmd(0x%x)\n",
                            __func__, (uint32)ucCh, SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_CMD)));
            ret = SAL_RET_FAILED;

            break;
        }

        cnt++;
    }

    if(ret == SAL_RET_SUCCESS)
    {
        /* Check TIP (Transfer in progress) is 0. */
        while(((SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR)) & I2C_SR_TIP)) != 0UL)
        {
            if(cnt > I2C_POLL_TIMEOUT)
            {
                I2C_D("%s: ch %d non interrupt time out, status(0x%x)\n",
                                __func__, (uint32)ucCh, SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR)));
                ret = SAL_RET_FAILED;

                break;
            }

            cnt++;
        }
    }

    I2C_ClearIntr(ucCh);

#if 0
    if(ret != SAL_RET_SUCCESS)
    {
        I2C_Reinit(ucCh); /* If command is not completed, try to reinit the core */
    }
#endif

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_CheckAck
*
* Function to check receive ACK after transfer complete.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*           : Check whether receive the acknowledgement after send the byte
*
***************************************************************************************************
*/

static SALRetCode_t I2C_CheckAck
(
    uint8                               ucCh
)
{
    uint32  val;

    val = (uint32)(SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR)) & I2C_SR_RX_ACK);

    if(val == (uint32)NULL) /* Check RxACK */
    {
        return SAL_RET_SUCCESS;
    }

    return SAL_RET_FAILED;
}

/*
***************************************************************************************************
*                                          I2C_XferSync
*
* Function to sync transfer data via I2C channel with param info.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    ucCmdLen [in]   : command data length
* @param    ucpCmdBuf [out] : command buffer
* @param    ucOutLen  [in]  : out data length
* @param    ucpOutBuf [out] : out data buffer
* @param    ucInLen   [in]  : in data length (read data legnth)
* @param    ucpInBuf  [in]  : in data buffer (data store after I2C READ)
* @param    uiOpt     [in]  : I2C transfer option
* @return
* Notes
*           : this function will be called according ucAsync value in I2C_Xfer function params.
*
***************************************************************************************************
*/

static SALRetCode_t I2C_XferSync
(
    const uint8                         ucCh,
    const uint8                         ucSlaveAddr,
    const uint8                         ucCmdLen,
    const uint8 *                       ucpCmdBuf,
    const uint8                         ucOutLen,
    const uint8 *                       ucpOutBuf,
    const uint8                         ucInLen,
    uint8 *                             ucpInBuf,
    uint32                              uiOpt
)
{
    SALRetCode_t    ret;
    uint32          i;

    ret = SAL_RET_SUCCESS;
    i   = 0xffUL;

    /* Write*/
    if((ucOutLen > (uint8)NULL) || (ucCmdLen > (uint8)NULL))
    {
        /* Send START condition and slave address with write flag*/
        SAL_WriteReg(((uint32)ucSlaveAddr | I2C_WR), (uint32)(i2c[ucCh].dBase + I2C_TXR));
        SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR) , (uint32)(i2c[ucCh].dBase + I2C_CMD));
        ret = I2C_Wait(ucCh);

        if(ret != SAL_RET_SUCCESS)
        {
            return ret;
        }

        ret = I2C_CheckAck(ucCh);

        if(ret != SAL_RET_SUCCESS)
        {
            goto stop;
        }

        if(ucCmdLen > (uint8)NULL)
        {
            for(i = 0 ; i < ucCmdLen ; i++)
            {
                /* Send data */
                if(ucpCmdBuf != (uint8 *)NULL_PTR)
                {
                    SAL_WriteReg(ucpCmdBuf[i], (uint32)(i2c[ucCh].dBase + I2C_TXR));
                }

                SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                ret = I2C_Wait(ucCh);

                if(ret != SAL_RET_SUCCESS)
                {
                    return ret;
                }

                ret = I2C_CheckAck(ucCh);

                if(ret != SAL_RET_SUCCESS)
                {
                    goto stop;
                }
            }
        }

        if(ucOutLen > (uint8)NULL)
        {
            for(i = 0 ; i < ucOutLen ; i++)
            {
                /* Send data */
                if(ucpOutBuf != (uint8 *)NULL_PTR)
                {
                    SAL_WriteReg(ucpOutBuf[i], (uint32)(i2c[ucCh].dBase + I2C_TXR));
                }

                SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                ret = I2C_Wait(ucCh);

                if(ret != SAL_RET_SUCCESS)
                {
                    return ret;
                }

                ret = I2C_CheckAck(ucCh);

                if(ret != SAL_RET_SUCCESS)
                {
                    goto stop;
                }
            }
        }

        /* if read data is none, send STOP condition */
        if(ucInLen == (uint8)NULL)
        {
            if((uiOpt & I2C_WR_WITHOUT_STOP) != 0UL)
            {
                return SAL_RET_SUCCESS;
            }
        }
    }

    /* Read */
    if(ucInLen > (uint8)NULL)
    {
        /* Send START condition and slave address with read flag*/
        SAL_WriteReg((ucSlaveAddr | I2C_RD), (uint32)(i2c[ucCh].dBase + I2C_TXR));

        if(((uiOpt & I2C_RD_WITHOUT_RPEAT_START) != (uint32) NULL))
        {
            SAL_WriteReg(I2C_CMD_STA , (uint32)(i2c[ucCh].dBase + I2C_CMD));
        }
        else
        {
            SAL_WriteReg((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
        }

        ret = I2C_Wait(ucCh);

        if(ret != SAL_RET_SUCCESS)
        {
            return ret;
        }

        ret = I2C_CheckAck(ucCh);

        if(ret != SAL_RET_SUCCESS)
        {
            goto stop;
        }

        for(i = 0 ; i < ucInLen ; i++)
        {
            if (i == (ucInLen - 1UL))
            {
                /* Last byte to read */
                if((uiOpt & I2C_RD_WITHOUT_ACK) != (uint32)NULL)
                {
                    SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
                else
                {
                    SAL_WriteReg(((uint32)I2C_CMD_RD | (uint32)I2C_CMD_ACK), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
            }
            else
            {
                SAL_WriteReg(I2C_CMD_RD , (uint32)(i2c[ucCh].dBase + I2C_CMD));
            }

            ret = I2C_Wait(ucCh);

            if(ret != SAL_RET_SUCCESS)
            {
                return ret;
            }

            /* Store read data */
            if(ucpInBuf != NULL_PTR)
            {
                ucpInBuf[i] = (uint8)SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_RXR));
            }
        }

        if((uiOpt & I2C_RD_WITHOUT_STOP) != (uint32)NULL)
        {
            return SAL_RET_SUCCESS;
        }
    }

stop:   /* Send STOP condition */
    SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    ret = I2C_Wait(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s: i2c ch %d received NACK from 0x%x\n", __func__, ucCh, (ucSlaveAddr)>>1UL);
        return ret;
    }

    return SAL_RET_SUCCESS;
}


/*
***************************************************************************************************
*                                          I2C_XferAsync
*
* Function to async transfer data via I2C channel with param info.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    ucCmdLen [in]   : command data length
* @param    ucpCmdBuf [out] : command buffer
* @param    ucOutLen  [in]  : out data length
* @param    ucpOutBuf [out] : out data buffer
* @param    ucInLen   [in]  : in data length (read data legnth)
* @param    ucpInBuf  [in]  : in data buffer (data store after I2C READ)
* @param    uiOpt     [in]  : I2C transfer option
* @return
* Notes
*           : this function will be called according ucAsync value in I2C_Xfer function params.
*
***************************************************************************************************
*/

static SALRetCode_t I2C_XferAsync
(
    const uint8                          ucCh,
    const uint8                          ucSlaveAddr,
    const uint8                          ucCmdLen,
    uint8 *                              ucpCmdBuf,
    const uint8                          ucOutLen,
    uint8 *                              ucpOutBuf,
    const uint8                          ucInLen,
    uint8 *                              ucpInBuf,
    uint32                               uiOpt
)
{
    i2c[ucCh].dAsync.axSlaveAddr    = ucSlaveAddr;
    i2c[ucCh].dAsync.axCmdLen       = ucCmdLen;
    i2c[ucCh].dAsync.axCmdBuf       = ucpCmdBuf;
    i2c[ucCh].dAsync.axOutLen       = ucOutLen;
    i2c[ucCh].dAsync.axOutBuf       = ucpOutBuf;
    i2c[ucCh].dAsync.axInBuf        = ucpInBuf;
    i2c[ucCh].dAsync.axInLen        = ucInLen;
    i2c[ucCh].dAsync.axOpt          = uiOpt;
    I2C_EnableIrq(ucCh);

    if((i2c[ucCh].dAsync.axCmdLen > (uint8)NULL) || (i2c[ucCh].dAsync.axOutLen > (uint8)NULL))
    {
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_SEND;
        (void)SAL_CoreCriticalExit();
        I2C_D("%s ch %d I2C_STATE_SEND\n", __func__, ucCh);
        SAL_WriteReg((ucSlaveAddr | I2C_WR), (uint32)(i2c[ucCh].dBase + I2C_TXR));
        SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR) , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }
    else if(i2c[ucCh].dAsync.axInLen > (uint8)NULL)
    {
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_RECV_START;
        (void)SAL_CoreCriticalExit();
        I2C_D("%s ch %d I2C_STATE_RECV_START\n", __func__, ucCh);
        SAL_WriteReg((ucSlaveAddr | I2C_RD), (uint32)(i2c[ucCh].dBase + I2C_TXR));
        SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR) , (uint32)(i2c[ucCh].dBase + I2C_CMD));
    }
    else
    {
        I2C_DisableIrq(ucCh);
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_IDLE;
        (void)SAL_CoreCriticalExit();

        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_Xfer
*
* Function to transfer data.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    ucOutLen  [in]  : out data length
* @param    ucpOutBuf [out] : out data buffer
* @param    ucInLen   [in]  : in data length (read data legnth)
* @param    ucpInBuf  [in]  : in data buffer (data store after I2C READ)
* @param    uiOpt     [in]  : I2C transfer option
* @param    ucAsync   [in]  : I2C transfer mode
* @return
* Notes
*           : This function can be called by application to transfer data via I2C.
*
***************************************************************************************************
*/

SALRetCode_t I2C_Xfer
(
    const uint8                         ucCh,
    const uint8                         ucSlaveAddr,
    const uint8                         ucOutLen,
    uint8 *                             ucpOutBuf,
    const uint8                         ucInLen,
    uint8 *                             ucpInBuf,
    uint32                              uiOpt,
    uint8                               ucAsync
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;
    (void)SAL_CoreCriticalEnter();

    if(i2c[ucCh].dState != I2C_STATE_IDLE)
    {
        (void)SAL_CoreCriticalExit();
        I2C_D("[%s] invaild state\n", __func__);

        return SAL_RET_FAILED;
    }

    i2c[ucCh].dState = I2C_STATE_RUNNING;
    (void)SAL_CoreCriticalExit();
    (void)I2C_BusyCheck(ucCh);

    if(ucAsync != (uint8)NULL)
    {
        ret = I2C_XferAsync(ucCh, ucSlaveAddr, 0, NULL, ucOutLen, ucpOutBuf, ucInLen, ucpInBuf, uiOpt);
    }
    else
    {
        ret = I2C_XferSync(ucCh, ucSlaveAddr, 0, NULL, ucOutLen, ucpOutBuf, ucInLen, ucpInBuf, uiOpt);
        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_IDLE;
        (void)SAL_CoreCriticalExit();
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_XferCmd
*
* Function to transfer data with command.
*
* @param    ucCh [in]       : Value of channel to control
* @param    ucSlaveAddr [in]: specific slave address to transfer
* @param    ucCmdLen [in]   : command data length
* @param    ucpCmdBuf [out] : command buffer
* @param    ucOutLen  [in]  : out data length
* @param    ucpOutBuf [out] : out data buffer
* @param    ucInLen   [in]  : in data length (read data legnth)
* @param    ucpInBuf  [in]  : in data buffer (data store after I2C READ)
* @param    uiOpt     [in]  : I2C transfer option
* @param    ucAsync   [in]  : I2C transfer mode
* @return
* Notes
*
***************************************************************************************************
*/

SALRetCode_t I2C_XferCmd
(
    const uint8                         ucCh,
    const uint8                         ucSlaveAddr,
    const uint8                         ucCmdLen,
    uint8 *                             ucpCmdBuf,
    const uint8                         ucOutLen,
    uint8 *                             ucpOutBuf,
    const uint8                         ucInLen,
    uint8 *                             ucpInBuf,
    uint32                              uiOpt,
    uint8                               ucAsync
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;
    (void)SAL_CoreCriticalEnter();

    if(i2c[ucCh].dState != I2C_STATE_IDLE)
    {
        (void)SAL_CoreCriticalExit();
        ret = SAL_RET_FAILED;
    }

    i2c[ucCh].dState = I2C_STATE_RUNNING;
    (void)SAL_CoreCriticalExit();
    (void)I2C_BusyCheck(ucCh);

    if(ucAsync != (uint8)NULL)
    {
        ret = I2C_XferAsync(ucCh, ucSlaveAddr, ucCmdLen, ucpCmdBuf, ucOutLen, ucpOutBuf, ucInLen, ucpInBuf, uiOpt);
    }
    else
    {
        ret = I2C_XferSync(ucCh, ucSlaveAddr, ucCmdLen, ucpCmdBuf, ucOutLen, ucpOutBuf, ucInLen, ucpInBuf, uiOpt);

        if(I2C_CheckAck(ucCh) == SAL_RET_SUCCESS)
        {
            //I2C_D("Receive ACK ! addr : %x \n", (ucSlaveAddr >> (uint8)1UL));
        }
        else
        {
        	ret = SAL_RET_FAILED;
        }

        (void)SAL_CoreCriticalEnter();
        i2c[ucCh].dState = I2C_STATE_IDLE;
        (void)SAL_CoreCriticalExit();
    }

    return ret;
}

/*
***************************************************************************************************
*                                          I2C_GetMachineState
*
* Function to get machine state value
*
* @param    ucCh [in]       : Value of channel to control
* @return   Value of I2C channel machine state.
* Notes
*
***************************************************************************************************
*/

uint32 I2C_GetMachineState
(
    uint8                               ucCh
)
{
    uint32  reg;
    uint32  status;
    uint32  sm;

    reg     = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));
    status  = (reg >> 19UL) & 0x7UL;
    sm      = (reg >> 16UL) & 0x7UL;

    switch(status)
    {
        case I2C_STATUS_IDLE :
        {
            I2C_D("I2C Machine state : IDLE \n");
            break;
        }

        case I2C_STATUS_START :
        {
            I2C_D("I2C Machine state : START \n");
            break;
        }

        case I2C_STATUS_READ :
        {
            I2C_D("I2C Machine state : READ \n");
            break;
        }

        case I2C_STATUS_WRITE :
        {
            I2C_D("I2C Machine state : WRITE \n");
            break;
        }

        case I2C_STATUS_READ_AFTER_ACK :
        {
            I2C_D("I2C Machine state : READ_AFTER_ACK \n");
            break;
        }

        case I2C_STATUS_WRITE_AFTER_ACK :
        {
            I2C_D("I2C Machine state : WRITE_AFTER_ACK \n");
            break;
        }

        case I2C_STATUS_STOP :
        {
            I2C_D("I2C Machine state : STOP \n");
            break;
        }

        default :
        {
            I2C_D("I2C Machine state : Unknown value. \n");
            break;
        }
    }

    switch(sm)
    {
        case I2C_SM_STATE_IDLE :
        {
            I2C_D("I2C SM State : IDLE\n");
            break;
        }

        case I2C_SM_STATE_THD_DAT :
        {
            I2C_D("I2C SM State : DATA HOLD\n");
            break;
        }

        case I2C_SM_STATE_TSU_DTA :
        {
            I2C_D("I2C SM State : DATA SETUP\n");
            break;
        }

        case I2C_SM_STATE_TSU_STA :
        {
            I2C_D("I2C SM State : START SETUP\n");
            break;
        }

        case I2C_SM_STATE_TSU_STO :
        {
            I2C_D("I2C SM State : STOP SETUP\n");
            break;
        }

        case I2C_SM_STATE_DAT_ACK_STD :
        {
            I2C_D("I2C SM State : DATA/ACK steady state\n");
            break;
        }

        default :
        {
            I2C_D("I2C SM State : Unknown value.\n");
            break;
        }
    }

    return (reg >> 16UL);
}

/*
***************************************************************************************************
*                                          I2C_Isr
*
* Fucntion to process I2C interrupt.
*
* @param    ucCh [in]       : Value of channel to control
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_Isr
(
    uint8                               ucCh
)
{
    uint32  status;
    uint32  is_intr;
    uint32  is_ack;

    status  = 0;
    is_intr = 0;
    is_ack  = 0;
    status  = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_SR));
    is_intr = status & I2C_SR_IF;

    if(is_intr != (uint8)NULL)
    {
        is_ack = (I2C_CheckAck(ucCh) == SAL_RET_SUCCESS) ? 1UL : 0UL;
        I2C_ClearIntr(ucCh);

        if(i2c[ucCh].dState == I2C_STATE_SEND)
        {
            if(is_ack != (uint8)NULL)
            {
                if(i2c[ucCh].dAsync.axCmdLen != (uint8)NULL)
                {
                    /* Send cmd first, if exist */
                    SAL_WriteReg(*i2c[ucCh].dAsync.axCmdBuf, (uint32)(i2c[ucCh].dBase + I2C_TXR));
                    i2c[ucCh].dAsync.axCmdLen--;
                    i2c[ucCh].dAsync.axCmdBuf++;

                    SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
                else
                {
                    if(i2c[ucCh].dAsync.axOutLen != (uint8)NULL)
                    {
                        /* Send data next to the cmd */
                        SAL_WriteReg(*i2c[ucCh].dAsync.axOutBuf, (uint32)(i2c[ucCh].dBase + I2C_TXR));
                        i2c[ucCh].dAsync.axOutLen--;
                        i2c[ucCh].dAsync.axOutBuf++;

                        SAL_WriteReg(I2C_CMD_WR , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                    else
                    {
                        /* After send cmd and out data */
                        if(i2c[ucCh].dAsync.axInLen != (uint8)NULL)
                        {
                            /* Start to recevie data */
                            i2c[ucCh].dState = I2C_STATE_RECV_START;
                            I2C_D("%s ch %d I2C_STATE_RECV_START\n", __func__, ucCh);
                            SAL_WriteReg((i2c[ucCh].dAsync.axSlaveAddr | I2C_RD), (uint32)(i2c[ucCh].dBase + I2C_TXR));

                            if(((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_RPEAT_START) != (uint32)NULL))
                            {
                                SAL_WriteReg(I2C_CMD_STA, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                            }
                            else
                            {
                                SAL_WriteReg(((uint32)I2C_CMD_STA | (uint32)I2C_CMD_WR), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                            }
                        }
                        else
                        {
                            /* Finish the transfer */
                            i2c[ucCh].dState = I2C_STATE_SEND_DONE;

                            if((i2c[ucCh].dAsync.axOpt & I2C_WR_WITHOUT_STOP) == (uint32)NULL)
                            {
                                SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                            }
                            else
                            {
                                I2C_XferComplete(ucCh, I2C_EVENT_XFER_COMPLETE);
                            }
                        }
                    }
                }
            }
            else
            {
                SAL_WriteReg(BSP_BIT_06 ,(uint32)( i2c[ucCh].dBase + I2C_CMD));
                I2C_D("%s: i2c ch %d received NACK from 0x%x\n", __func__, ucCh, (i2c[ucCh].dAsync.axSlaveAddr >> (uint8)1UL));
                I2C_XferComplete(ucCh, I2C_EVENT_NACK);
            }
        }
        else if(i2c[ucCh].dState == I2C_STATE_SEND_DONE)
        {
            I2C_XferComplete(ucCh, I2C_EVENT_XFER_COMPLETE);
        }
        else if(i2c[ucCh].dState == I2C_STATE_RECV_START)
        {
            if(is_ack != (uint8)NULL)
            {
                if(i2c[ucCh].dAsync.axInLen != (uint8)NULL)
                {
                    i2c[ucCh].dState = I2C_STATE_RECV;
                    I2C_D("%s ch %d I2C_STATE_RECV\n", __func__, ucCh);

                    if(i2c[ucCh].dAsync.axInLen == (uint8)1UL)
                    {
                        /* Read last byte */
                        if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_ACK) != (uint32)NULL)
                        {
                            SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                        }
                        else
                        {
                            SAL_WriteReg(((uint32)I2C_CMD_RD | (uint32)I2C_CMD_ACK), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                        }
                    }
                    else
                    {
                        SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                }
                else
                {
                    i2c[ucCh].dState = I2C_STATE_RECV_DONE;
                    I2C_D("%s ch %d I2C_STATE_RECV_DONE\n", __func__, ucCh);

                    if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_STOP) == (uint32)NULL)
                    {
                        SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                }
            }
            else
            {
                SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                I2C_D("%s: i2c ch %d received NACK from 0x%x\n", __func__, ucCh, (i2c[ucCh].dAsync.axSlaveAddr >> (uint8)1UL));
                I2C_XferComplete(ucCh, I2C_EVENT_NACK);
            }
        }
        else if(i2c[ucCh].dState == I2C_STATE_RECV)
        {
            *i2c[ucCh].dAsync.axInBuf = (uint8)SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_RXR));
            i2c[ucCh].dAsync.axInLen--;
            i2c[ucCh].dAsync.axInBuf++;

            if(i2c[ucCh].dAsync.axInLen != (uint8)NULL)
            {
                if(i2c[ucCh].dAsync.axInLen == (uint8)1UL)
                {
                    if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_ACK) != (uint32)NULL)
                    {
                        /* Read last byte */
                        SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                    else
                    {
                        SAL_WriteReg(((uint32)I2C_CMD_RD | (uint32)I2C_CMD_ACK), (uint32)(i2c[ucCh].dBase + I2C_CMD));
                    }
                }
                else
                {
                    SAL_WriteReg(I2C_CMD_RD, (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
            }
            else
            {
                i2c[ucCh].dState = I2C_STATE_RECV_DONE;
                I2C_D("%s ch %d I2C_STATE_RECV_DONE\n", __func__, ucCh);

                if((i2c[ucCh].dAsync.axOpt & I2C_RD_WITHOUT_STOP) == (uint32)NULL)
                {
                    SAL_WriteReg(I2C_CMD_STO , (uint32)(i2c[ucCh].dBase + I2C_CMD));
                }
            }
        }
        else if(i2c[ucCh].dState == I2C_STATE_RECV_DONE)
        {
            I2C_XferComplete(ucCh, I2C_EVENT_XFER_COMPLETE);
        }
        else
        {
            /* Nothing to do */
        }
    }
}

/*
***************************************************************************************************
*                                          I2C_IsrCh0
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh0
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH0 ISR\n", __func__);
    I2C_Isr(0);
}

/*
***************************************************************************************************
*                                          I2C_IsrCh1
* @param    pArg
* @return
* Notes
*d
***************************************************************************************************
*/

static void I2C_IsrCh1
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH1 ISR\n", __func__);
    I2C_Isr(1);
}

/*
***************************************************************************************************
*                                          I2C_IsrCh2
* @param    pArg
* @return
* Notes
*
***************************************************************************************************
*/

static void I2C_IsrCh2
(
    void *                              pArg
)
{
    (void)pArg;
    I2C_D("%s: Enter CH2 ISR\n", __func__);
    I2C_Isr(2);
}

/*
***************************************************************************************************
*                                          I2C_Reset
*
* Function to reset I2C Channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_Reset
(
    uint8                               ucCh
)
{
    sint32  ret;

    ret = 0;

    /* SW reset */
    ret = CLOCK_SetSwReset((sint32)i2c[ucCh].dIobusName, TRUE);

    if(ret != (sint32)NULL)
    {
        return SAL_RET_FAILED;
    }

    ret = CLOCK_SetSwReset((sint32)i2c[ucCh].dIobusName, FALSE);

    if(ret != (sint32)NULL)
    {
        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_SetPort
*
* Fucntion to set chaneel port info (SDA, SCL).
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_SetPort
(
    uint8                               ucCh
)
{
    uint32          pcfg_val;
    uint32          port_val;
    uint32          sda;
    uint32          scl;
    SALRetCode_t    ret;
    uint32          match_port;
    uint32          port;
    uint32          i;

    pcfg_val    = 0;
    port_val    = 0;
    sda         = 0;
    scl         = 0;
    ret         = SAL_RET_SUCCESS;
    match_port  = 0;
    port        = 0;
    i           = 0xffUL;

    if(ucCh >= I2C_CH_NUM)
    {
        return SAL_RET_FAILED;
    }

    sda = i2c[ucCh].dSda;
    scl = i2c[ucCh].dScl;

    for (i = 0 ; i < I2C_PORT_NUM ; i++)
    {
        if (scl == (i2cport[i][I2C_SCL]))
        {
            if (sda == (i2cport[i][I2C_SDA]))
            {
                match_port  = 1UL;
                port        = i;
            }
        }
    }

    if (match_port == 1UL)
    {
        i2c[ucCh].dPort = port;
        //I2C_D("%s: i2c port %d\n", __func__, i2c[ucCh].dPort);
    }
    else
    {
        I2C_D("%s: i2c port is wrong\n", __func__);

        return SAL_RET_FAILED;
    }

    for (i = 0 ; i < I2C_CH_NUM ; i++)
    {
        if ((i != ucCh) && (i2c[i].dPort == port))
        {
            I2C_D("%s: %d port is already used by ch%d\n", __func__, port, i);

            return SAL_RET_FAILED;
        }
    }

    /* Set i2c pcfg */
    pcfg_val = SAL_ReadReg(i2c[0].dCfgAddr);
    pcfg_val &= ~((uint32)I2C_PORT_SEL_MASK << (ucCh * 8UL));
    port_val = (i2c[ucCh].dPort << (ucCh * 8UL));

    SAL_WriteReg(pcfg_val | port_val, i2c[0].dCfgAddr);
    I2C_D("%s: I2C_PORT_SEL: 0x%08X\n", __func__, SAL_ReadReg(i2c[0].dCfgAddr));

    /* Set gpio */
    ret = GPIO_Config(sda, GPIO_FUNC(i2cport[port][I2C_FUNC]) | GPIO_INPUT | GPIO_INPUTBUF_EN);

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = GPIO_Config(scl, GPIO_FUNC(i2cport[port][I2C_FUNC]) | GPIO_INPUT | GPIO_INPUTBUF_EN);

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_ClearPort
*
* Function to clear channel port info.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_ClearPort
(
    uint32                              ucCh
)
{
    uint32          pcfg_val;
    uint32          sda;
    uint32          scl;
    SALRetCode_t    ret;

    pcfg_val    = 0;
    sda         = 0;
    scl         = 0;
    ret         = SAL_RET_SUCCESS;

    if(ucCh >= I2C_CH_NUM)
    {
        return SAL_RET_FAILED;
    }

    sda = i2c[ucCh].dSda;
    scl = i2c[ucCh].dScl;

    /* Reset gpio */
    ret = GPIO_Config(sda, GPIO_FUNC(0UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    ret = GPIO_Config(scl, GPIO_FUNC(0UL));

    if(ret != SAL_RET_SUCCESS)
    {
        return SAL_RET_FAILED;
    }

    /* Reset i2c pcfg */
    pcfg_val = SAL_ReadReg(i2c[0].dBase);
    pcfg_val |= (uint32)((uint32)I2C_PORT_SEL_MASK << (ucCh * (uint32)8UL));
    SAL_WriteReg(pcfg_val, i2c[0].dBase);
    i2c[ucCh].dPort = NULL;

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_SetFilter
*
* Function to enable/disble filter and set filter value
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiValue [in]    : Value of filter
* @return   SAL_RET_SUCCESS
* Notes
*           : Bus clock freq = 300 Mhz ( 3.33 ns per tick)
*             FLCV value = 16
*             3.33ns * 16 = 53.28 ns
*             ignore noise signal width lower than 53.28 ns.
*
***************************************************************************************************
*/

static SALRetCode_t I2C_SetFilter
(
    uint8                               ucCh,
    uint32                              uiValue
)
{
    uint32  tmp;

    tmp = 0;
    tmp = SAL_ReadReg((uint32)(i2c[ucCh].dBase + I2C_TIME_0));
    tmp = tmp & ~(I2C_FCLV_MASK << I2C_FCLV_SHIFT);
    tmp = tmp | (((uiValue & I2C_FCLV_MASK) << I2C_FCLV_SHIFT));
    SAL_WriteReg(tmp, (uint32)(i2c[ucCh].dBase + I2C_TIME_0));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_SetClock
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiInputClkKHz   : Value of GPSB speed
* @return   SAL_RET_SUCCESS
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_SetClock
(
    uint8                               ucCh,
    uint32                              uiInputClkKHz
)
{
    uint32  prescale;

    prescale = 0;

    /* Set prescale */
    prescale = (uiInputClkKHz / ((i2c[ucCh].dClk) * (uint32)5UL)) - (uint32)1UL;
    SAL_WriteReg(prescale, (uint32)(i2c[ucCh].dBase + I2C_PRES));

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_Enable
* Function to enable I2C block
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

static SALRetCode_t I2C_Enable
(
    uint8                               ucCh
)
{
    uint32  input_clk_kHz;
    sint32  ret;

    input_clk_kHz   = 0;
    ret             = 0;

    /* enable i2c peri bus */
    ret = CLOCK_SetPeriRate((sint32)i2c[ucCh].dPeriName, I2C_PCLK);

    if (ret != (sint32)0UL)
    {
        return SAL_RET_FAILED;
    }

    (void) CLOCK_EnablePeri((sint32)i2c[ucCh].dPeriName);

    /* enable iobus */
    ret = CLOCK_SetIobusPwdn((sint32)i2c[ucCh].dIobusName, FALSE);

    if(ret != (sint32)0UL)
    {
        return SAL_RET_FAILED;
    }

    /* get i2c bus clock */
    input_clk_kHz = CLOCK_GetPeriRate((sint32)i2c[ucCh].dPeriName);        /* get I2C bus clock */
    I2C_D("%s: I2C bus clock %d\n", __func__, input_clk_kHz);

    /* set i2c clock */
    (void)I2C_SetClock(ucCh, (input_clk_kHz/1000UL)); //Codesonar Unused Value (err)

    /* Enable I2C core. Set 8bit mode*/
    SAL_WriteReg(I2C_CTRL_EN_CORE , (uint32)(i2c[ucCh].dBase + I2C_CTRL));

    /* Clear pending interrupt */
    I2C_ClearIntr(ucCh);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_Open
*
* Function to initialize I2C Channel
*
* @param    ucCh [in]         : Value of channel to control
* @param    iGpioScl [in]     : Index of scl gpio
* @param    iGpioSda [in]     : Index of sda gpio
* @param    uiSpeedInKHz [in] : Value of SCLK speed
* @param    fnCallback [in]   : Fucntion poionter of callback
* @param    pArg
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t I2C_Open
(
    uint8                               ucCh,
    uint32                              iGpioScl,
    uint32                              iGpioSda,
    uint32                              uiSpeedInKHz,
    I2CCallback                         fnCallback,
    void *                              pArg
)
{
    SALRetCode_t    ret;

    ret = SAL_RET_SUCCESS;

    if(ucCh >= I2C_CH_NUM)
    {
        I2C_D("%s: %d channel is wrong\n", __func__, ucCh);

        return SAL_RET_FAILED;
    }

    i2c[ucCh].dScl                      = iGpioScl;
    i2c[ucCh].dSda                      = iGpioSda;
    i2c[ucCh].dClk                      = uiSpeedInKHz;
    i2c[ucCh].dComplete.ccCallBack      = fnCallback;
    i2c[ucCh].dComplete.ccArg           = pArg;
    (void)SAL_MemSet(&i2c[ucCh].dAsync, 0, sizeof(I2CAsyncXfer_t));
    ret = I2C_Reset(ucCh);

    if (ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s: ch %d failed to reset core\n", __func__, ucCh);

        return ret;
    }

    ret = I2C_SetPort(ucCh);

    if (ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s: ch %d failed to set port\n", __func__, ucCh);

        return ret;
    }

    ret = I2C_Enable(ucCh);

    if (ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s: ch %d failed to enable core\n", __func__, ucCh);

        return ret;
    }

#ifdef I2C_USE_FILTER
    (void)I2C_SetFilter(ucCh, 16UL);
#endif

    (void)SAL_CoreCriticalEnter();
    i2c[ucCh].dState = I2C_STATE_IDLE;
    (void)SAL_CoreCriticalExit();

    //I2C_ScanSlave(ch);

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_Close
*
* Function to deinitialize I2C Channel
* @param    ucCh [in]         : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

SALRetCode_t I2C_Close
(
    uint8                               ucCh
)
{
    sint32          iRet;
    SALRetCode_t    ret;

    iRet = 0;
    ret = SAL_RET_SUCCESS;

    if (ucCh > I2C_CH_NUM)
    {
        I2C_D("%s %d channel is wrong\n", __func__, ucCh);
        return SAL_RET_FAILED;
    }

    (void)SAL_CoreCriticalEnter();
    i2c[ucCh].dState = I2C_STATE_DISABLED;
    (void)SAL_CoreCriticalExit();
    I2C_DisableIrq(ucCh);
    I2C_ClearIntr(ucCh);
    SAL_WriteReg(0x0, (uint32)(i2c[ucCh].dBase + I2C_CTRL));
    (void)CLOCK_DisablePeri((sint32)i2c[ucCh].dPeriName);
    iRet = CLOCK_SetIobusPwdn((sint32)i2c[ucCh].dIobusName, TRUE);

    if (iRet != (sint32)NULL)
    {
        return SAL_RET_FAILED;
    }

    ret = I2C_ClearPort((uint32)ucCh);

    if (ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s ch %d failed to reset port\n", __func__, ucCh);
        return ret;
    }

    ret = I2C_Reset(ucCh);

    if(ret != SAL_RET_SUCCESS)
    {
        I2C_D("%s ch %d failed to reset core\n", __func__, ucCh);
        return ret;
    }

    return SAL_RET_SUCCESS;
}

/*
***************************************************************************************************
*                                          I2C_Init
*
* Function to register I2C interrupt
*
* @return   SAL_RET_SUCCESS or SAL_ERR
* Notes
*
***************************************************************************************************
*/

void I2C_Init
(
    void
)
{
    /* Enter i2c irq handler */
    (void)GIC_IntVectSet((uint32)GIC_I2C, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh0, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C);

    (void)GIC_IntVectSet((uint32)GIC_I2C1, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh1, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C1);

    (void)GIC_IntVectSet((uint32)GIC_I2C2, (uint32)GIC_PRIORITY_NO_MEAN,
                        (uint8)GIC_INT_TYPE_LEVEL_HIGH, &I2C_IsrCh2, NULL_PTR);
    (void)GIC_IntSrcEn(GIC_I2C2);

    return;
}

/*
***************************************************************************************************
*                                          I2C_Deinit
*
* Function to unregister I2C interrupt
*
* @return
* Notes
*
***************************************************************************************************
*/

void I2C_Deinit
(
    void
)
{
    (void)GIC_IntSrcDis(GIC_I2C);
    (void)GIC_IntSrcDis(GIC_I2C1);
    (void)GIC_IntSrcDis(GIC_I2C2);

    return;
}

