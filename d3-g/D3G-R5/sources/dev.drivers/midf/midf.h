/*
***************************************************************************************************
*
*   FileName : midf.h
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

#ifndef MIDF_HEADER_H
#define MIDF_HEADER_H

#define MIDF_SFATEY_MECHANISM

/**
 * MID Filter ID list.
 */
#define MIDF_ID_DMA0                    (0UL)
#define MIDF_ID_DMA1                    (1UL)
#define MIDF_ID_CAN0                    (2UL)
#define MIDF_ID_CAN1                    (3UL)
#define MIDF_ID_CAN2                    (4UL)
#define MIDF_ID_GPSB0                   (5UL)
#define MIDF_ID_GPSB1                   (6UL)
#define MIDF_ID_GPSB2                   (7UL)
#define MIDF_ID_GPSB3_4_5               (8UL)
#define MIDF_ID_UART0                   (9UL)
#define MIDF_ID_UART1                   (10UL)
#define MIDF_ID_UART2                   (11UL)
#define MIDF_ID_UART3_4_5               (12UL)
#define MIDF_ID_CR5                     (13UL)
#define MIDF_ID_JTAG                    (14UL)
#define MIDF_ID_AP                      (15UL)

#ifdef MIDF_SFATEY_MECHANISM
/**
 * MID Soft Fault Check Group Enable
 */
#define MIDF_CHECK_GRP_ALL                    (0x1FFUL)
#define MIDF_CHECK_GRP_XS_MID                    (1UL << 0)
#define MIDF_CHECK_GRP_HS2_MID                    (1UL << 1)
#define MIDF_CHECK_GRP_HS3_MID                    (1UL << 2)
#define MIDF_CHECK_GRP_HS4_MID0                    (1UL << 3)
#define MIDF_CHECK_GRP_HS4_MID1                    (1UL << 4)
#define MIDF_CHECK_GRP_HS5_MID0                    (1UL << 5)
#define MIDF_CHECK_GRP_HS5_MID_CFG_WR_PW                    (1UL << 6)
#define MIDF_CHECK_GRP_HS2_MID_CFG_WR_LOCK                    (1UL << 7)
#define MIDF_GHECK_GRP_NUM                    (8UL)
/**
 * MID Soft Fault Check Group Status
 */
#define MIDF_CHECK_GRP_STS_XS_MID                    (1UL << 0)
#define MIDF_CHECK_GRP_STS_HS2_MID                    (1UL << 1)
#define MIDF_CHECK_GRP_STS_HS3_MID                    (1UL << 2)
#define MIDF_CHECK_GRP_STS_HS4_MID0                    (1UL << 3)
#define MIDF_CHECK_GRP_STS_HS4_MID1                    (1UL << 4)
#define MIDF_CHECK_GRP_STS_HS5_MID0                    (1UL << 5)
#define MIDF_CHECK_GRP_STS_HS5_MID_CFG_WR_PW                    (1UL << 6)
#define MIDF_CHECK_GRP_STS_HS2_MID_CFG_WR_LOCK                    (1UL << 7)
#endif

#define MIDF_REGION_MIN_SIZE            (0x1000UL)
/**
 * MID Filter Action Type
 */
#define MIDF_TYPE_READ                  (1UL << 0)
#define MIDF_TYPE_WRITE                 (1UL << 1)

/**
 * MID Result
 */
#define MIDF_SUCCESS                    (0L)
#define MIDF_ERROR                      (-1L)
#define MIDF_ERROR_NOT_INITIALIZE       (-2L)
#define MIDF_ERROR_NO_RESOURCE          (-3L)
#define MIDF_ERROR_BAD_PARAMETERS       (-4L)
#define MIDF_ERROR_BAD_RANGES           (-5L)
#define MIDF_ERROR_NOT_FOUND            (-6L)
#define MIDF_ERROR_NOT_SET            (-7L)

/**
 * Initialize MID Filter
 */
/*
***************************************************************************************************
*                                          MIDF_FilterInitialize
*
* Register MID Filter
*
* @param    None
* @return   None
*
* Notes
*
***************************************************************************************************
*/
void MIDF_FilterInitialize
(
    void
);

/*
***************************************************************************************************
*                                          MIDF_RegisterFilterRegion
*
* Register MID Filter
*
* @param    uiMemoryBase [in] Base address of memory buffer for 'id'. it should be 4kb aligned
* @param    uiSize [in] Size of memory buffer for 'id'. it should be 4kb aligned
* @param    uiID[in] Master id.
* @param    uiType [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/
int32 MIDF_RegisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
);

/*
***************************************************************************************************
*                                          MIDF_UnregisterFilterRegion
*
* Unregister MID Filter
*
* @param    uiMemoryBase [in] Base address of memory buffer for 'id'.
* @param    uiSize [in] Size of memory buffer for 'id'.
* @param    uiID [in] Master id.
* @param    uiType [in] Read or Write type of memory buffer for 'id'
* @return   MID Result
*
* Notes
*
***************************************************************************************************
*/
int32 MIDF_UnregisterFilterRegion
(
    uint32                              uiMemoryBase,
    uint32                              uiSize,
    uint32                              uiID,
    uint32                              uiType
);

#ifdef MIDF_SFATEY_MECHANISM
int32 MIDF_RegisterWriteLockSafetyMechanism
(
    uint32                              uiLock
);

int32 MIDF_ConfigSoftFaultCheckSafetyMechanism
(
    uint32 uiSvlLevel,
    uint32 uiFaultCheckGroup
);
#endif
#endif /* MIDF_HEADER_H */

