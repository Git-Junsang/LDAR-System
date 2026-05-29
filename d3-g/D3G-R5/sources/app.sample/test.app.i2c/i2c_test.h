/*
***************************************************************************************************
*
*   FileName : i2c_test.h
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
#if !defined(I2C_TEST_HEADER)
#define I2C_TEST_HEADER

/*
***************************************************************************************************
*                                             INCLUDE FILES
***************************************************************************************************
*/
#include <i2c_reg.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/

// tcc8050
#define I2C_TEST_SCL                    (GPIO_GPMB(30UL))
#define I2C_TEST_SDA                    (GPIO_GPMB(31UL))

// tcc8059
//#define I2C_TEST_SCL                    (GPIO_GPMA(22UL))
//#define I2C_TEST_SDA                    (GPIO_GPMA(23UL))


#define I2C_TEST_CLK_RATE_100           (100)
#define I2C_TEST_CLK_RATE_400           (400)

#define I2C_TEST_MASTER_CH              (0)
#define I2C_TEST_SLAVE_CH               (0)

/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/

/*
***************************************************************************************************
*                                          I2C_TestSetLoopback
*
* Function to set internal/external loopback.
*
* @param    uiEnable [in]   : Value to set/clear LoopbackEN bit. (0 or 1)
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           uiEnable == 0   : I2C Master Internal LoopBack
*           uiEnable == 1   : I2C Master External LoopBack
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestSetLoopback
(
    uint32                              uiEnable
);

/*
***************************************************************************************************
*                                          I2C_TestSetSlaveAddr
*
* Function to set address about virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @param    uiAddr [in]     : Value of address
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           addrees bit [7:1]
**************************************************************************************************
*/

static SALRetCode_t I2C_TestSetSlaveAddr
(
    uint8                               ucCh,
    uint32                              uiAddr
);

/*
***************************************************************************************************
*                                          I2C_TestEnableSlave
*
* Function to enable virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestEnableSlave
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestDisableSlave
*
* Function to disable virtual slave channel.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestDisableSlave
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestEnableLock
*
* Function to enable write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           related register : PORT_SEL , I2C_LB_CFG
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestEnableLock
(
    void
);

/*
***************************************************************************************************
*                                          I2C_TestDisableLock
*
* Function to disable write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*           related register : PORT_SEL , I2C_LB_CFG
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestDisableLock
(
    void
);

/*
***************************************************************************************************
*                                          I2C_TestGetSlaveAddr
*
* Function to get virtual slave address.
*
* @param    ucCh [in]       : Value of channel to control
* @return   address of virtual slave channel
* Notes
*
**************************************************************************************************
*/

static uint32 I2C_TestGetSlaveAddr
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestLoopback
*
* Function to test loopback.
*
* @param    ucCh [in]       : Value of channel to control
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestLoopback
(
    uint8                               ucCh
);

/*
***************************************************************************************************
*                                          I2C_TestWriteLock
*
* Function to test write lock.
*
* @return   SAL_RET_SUCCESS or SAL_RET_FAILED
* Notes
*
**************************************************************************************************
*/

static SALRetCode_t I2C_TestWriteLock
(
    void
);

/*
***************************************************************************************************
*                                          I2C_TestMain
*
* Function to call all of test functions.
*
* @return
* Notes
*
**************************************************************************************************
*/

void I2C_TestMain
(
    void
);

#endif
