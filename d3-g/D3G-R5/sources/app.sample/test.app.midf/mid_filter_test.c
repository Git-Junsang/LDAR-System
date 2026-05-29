/****************************************************************************
 *   FileName    : mid_filter_test.c
 *   Description :
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall
constitute any express or implied warranty of any kind, including without limitation, any warranty
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
or other third party intellectual property right. No warranty is made, express or implied,
regarding the information's accuracy, completeness, or performance.
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of
or in connection with this source code or the use in the source code.
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
between Telechips and Company.
*
****************************************************************************/

#include <sal_internal.h>
#include <gic.h>
#include "mid_filter_test.h"
#include "debug.h"
#include "midf.h"
#ifdef MIDF_IP_TEST

#define TCC8050_TEST
#define MICOM_BASE_ADDR                 (0x1B000000UL)
#define MID_FILTER_BASE		0x1B93B000

//#define COWORK_DMA
#ifdef COWORK_DMA
#include "gdma_ip_test.h"
#endif
//#define HwMC_MID            ((volatile MC_MID_CONFG *)(MICOM_BASE_ADDR + 0x939000))
#define MID_TYPE_READ			(1<<0)
#define MID_TYPE_WRITE			(1<<1)
#define SRAM1_BASE          0xc1000000                      // 0xC1000000 ~ 0xC000FFFF

/**
 * Filter Register Descriptions
 */
#define TZC400_BASE_L(ch)	(MID_FILTER_BASE+0x100+((ch)*0x20))
#define TZC400_BASE_H(ch)	(MID_FILTER_BASE+0x104+((ch)*0x20))
#define TZC400_TOP_L(ch)	(MID_FILTER_BASE+0x108+((ch)*0x20))
#define TZC400_TOP_H(ch)	(MID_FILTER_BASE+0x10C+((ch)*0x20))
#define TZC400_ATTRIBUTES(ch)	(MID_FILTER_BASE+0x110+((ch)*0x20))
#define TZC400_ID_ACCESS(ch)	(MID_FILTER_BASE+0x114+((ch)*0x20))

/**
 * Other Definition
 */
#define TZC400_MAX_CH		(8)
#define MID_TYPE_ALL		(MID_TYPE_READ|MID_TYPE_WRITE)
#define TZC400_ACCESS_DEFAULT	(0x10001<<MID_ID_CR5)	/* CR5 can access whole area */
//----------------------------------------------------------------------------------------------------------------------------
// Register structure
//----------------------------------------------------------------------------------------------------------------------------
#define TZASC_032KB             0x0E
#define TZASC_064KB             0x0F
#define TZASC_128KB             0x10
#define TZASC_256KB             0x11
#define TZASC_512KB             0x12
#define TZASC_001MB             0x13
#define TZASC_002MB             0x14
#define TZASC_004MB             0x15
#define TZASC_008MB             0x16
#define TZASC_016MB             0x17
#define TZASC_032MB             0x18
#define TZASC_064MB             0x19
#define TZASC_128MB             0x1A
#define TZASC_256MB             0x1B
#define TZASC_512MB             0x1C
#define TZASC_001GB             0x1D
#define TZASC_002GB             0x1E
#define TZASC_004GB             0x1F
#define TZASC_008GB             0x20

#define TZASC_SECURE_READ       0x8
#define TZASC_SECURE_WRITE      0x4

#define TZASC_NONSECURE_READ    0x2
#define TZASC_NONSECURE_WRITE   0x1

#define TZASC_REACT_OKAY        (0x0<<0)
#define TZASC_REACT_DECERR      (0x1<<0)
#define TZASC_REACT_LOW         (0x0<<1)
#define TZASC_REACT_HIGH        (0x1<<1)

#define TZASC_SPECULATION_OFF   1
#define TZASC_SPECULATION_ON    0

typedef struct {
    uint32    no_of_regions   :  5;   //  [04:00]
    uint32                    :  3;   //  [07:05]
    uint32    address_width   :  6;   //  [13:08]
    uint32                    : 10;   //  [23:14]
    uint32    no_of_filters   :  2;   //  [25:24]
    uint32                    :  6;   //  [31:26]
} CTRL;
typedef    union {
    uint32    nReg;
    CTRL    bReg;
} CTRL_U;

typedef struct {
    uint32    reaction_value  :  2;   //  [01:00]
    uint32                    : 30;   //  [31:02]
} ACTION;
typedef    union {
    uint32    nReg;
    ACTION  bReg;
} ACTION_U;

typedef struct {
    uint32    open_request    :  8;   //  [07:00]
    uint32                    :  8;   //  [15:08]
    uint32    open_status     :  8;   //  [23:16]
    uint32                    :  8;   //  [31:24]
} GATEKEEP;

typedef    union {
    uint32        nReg;
    GATEKEEP    bReg;
} GATE_KEEP_U;

typedef struct {
    uint32    read_speculation    :  1;   // [   00]
    uint32    write_speculation   :  1;   // [   01]
    uint32                        : 30;   // [31:02]
} SPECULATION;

typedef    union {
    uint32         nReg;
    SPECULATION  bReg;
} SPECULATION_U;

typedef struct {
    uint32    status          :  4;   //  [03:00]
    uint32                    :  4;   //  [07:04]
    uint32    overrun         :  4;   //  [11:08]
    uint32                    :  4;   //  [15:12]
    uint32    overlap         :  4;   //  [19:16]
    uint32                    : 12;   //  [31:20]
} INT_STS;

typedef    union {
    uint32        nReg;
    INT_STS    bReg;
} INT_STATUS_U;

typedef struct {
    uint32    clear           :  4;   //  [03:00]
    uint32                    : 28;   //  [31:04]
} INT_CLR;

typedef union {
    uint32        nReg;
    INT_CLR     bReg;
} INT_CLEAR_U;

typedef struct {
    // Fail Address Low
    uint32    fail_addr_low   : 32;   // [31:00]
    // Fail Address High      
    uint32    fail_addr_high  : 32;   // [31:00]
    // Fail Ctrl              
    uint32                    : 20;   // [19:00]
    uint32    privileged      :  1;   // [   20]
    uint32    nonsecure       :  1;   // [   21]
    uint32                    :  2;   // [23:22]
    uint32    write           :  1;   // [24]
    uint32                    :  7;   // [31:25]
    // Fail ID                
    uint32    id              :  8;   // [07:00]
    uint32                    : 16;   // [23:08]
    uint32    vnet            :  4;   // [27:24]
    uint32                    :  4;   // [31:28]
} FAIL_CTRL;

typedef union {
    uint32        nReg[4];
    FAIL_CTRL   bReg;
} FAIL_CTRL_U;

typedef struct {
    // Base Address LOW /*nReg[0]*/
    uint32                    : 12;   //  [11:00]
    uint32    base_addr_low   : 20;   //  [31:12]

    // Base Address HIGH /*nReg[1]*/                
    uint32    base_addr_high  :  2;   //  [01:00]
    uint32                    : 30;   //  [31:02]

    // Top Address LOW  /*nReg[2]*/                
    uint32                    : 12;   //  [11:00]
    uint32    top_addr_low    : 20;   //  [31:12]

    // Top Address HIGH  /*nReg[3]*/                                
    uint32    top_addr_high   :  2;   //  [01:00]
    uint32                    : 30;   //  [31:02]

    // Attribute               /*nReg[4]*/                         
    uint32    filter0_en      :  1;   //  [   00]
    uint32          :  31;   //  [   01 : 31]

    // NSAID                     /*nReg[5]*/                       
    uint32    nsaid_rd_en     : 16;   //  [15:00]
    uint32    nsaid_wr_en     : 16;   //  [31:16]

    // Reserved			/*nReg[6,7]*/                
    uint32    reserved_0      : 32;   //  [31:00]
    uint32    reserved_1      : 32;   //  [63:32]
} REGION_CTRL;

typedef union {
    uint32        nReg[8];
    REGION_CTRL bReg;
} REGION_CTRL_U;

typedef struct {
    CTRL_U          uCTRL;                  // 0x00
    ACTION_U        uACTION;                // 0x04
    GATE_KEEP_U     uGATE_KEEP;             // 0x08
    SPECULATION_U   uSPECULATION;           // 0x0C
    INT_STATUS_U    uINT_STATUS;            // 0x10
    INT_CLEAR_U     uINT_CLEAR;             // 0x14
    uint32            reserved0[2];           // 0x18~0x1C
    FAIL_CTRL_U     uFAIL_CTRL[4];          // 0x20~0x2C,0x30~0x3C,0x40~0x4C,0x50~0x5C
    uint32            reserved1[40];          // 0x60~0xFC
    REGION_CTRL_U   uREGION_CTRL[9];        // 0x100~0x11C,0x120~0x13C, ... 0x200~0x21C
    uint32            reserved2[64*13+4*11];  // 0x220 ~ 0xFEC
    uint32            uPERIPH_ID_4;           // 0xFD0
    uint32            uPERIPH_ID_5;           // 0xFD4
    uint32            uPERIPH_ID_6;           // 0xFD8
    uint32            uPERIPH_ID_7;           // 0xFDC
    uint32            uPERIPH_ID_0;           // 0xFE0
    uint32            uPERIPH_ID_1;           // 0xFE4
    uint32            uPERIPH_ID_2;           // 0xFE8
    uint32            uPERIPH_ID_3;           // 0xFEC
    uint32            uCOMPONENT_ID_0;        // 0xFF0
    uint32            uCOMPONENT_ID_1;        // 0xFF4
    uint32            uCOMPONENT_ID_2;        // 0xFF8
    uint32            uCOMPONENT_ID_3;        // 0xFFC
} TZC400;

//----------------------------------------------------------------------------------------------------------------------------
// Register structure
//----------------------------------------------------------------------------------------------------------------------------
// MID_XS
typedef struct {
    uint32                MID_AR_XS0          :  4;   // [03:00], RW
    uint32                MID_AW_XS0          :  4;   // [07:00], RW
    uint32                reserved0           :  8;   // [15:08]
    uint32                MID_AR_XS1          :  4;   // [19:16], RW
    uint32                MID_AW_XS1          :  4;   // [23:20], RW
    uint32                reserved1           :  8;   // [31:24]
} MC_MID_XS;

typedef union {
    uint32                nReg;
    MC_MID_XS           bReg;
} MC_MID_XS_U;

// MID_HS2
typedef struct {
    uint32                MID_HR_S2_TO_HS2    :  4;   // [03:00], RW
    uint32                MID_HW_S2_TO_HS2    :  4;   // [07:00], RW
    uint32                MID_HR_S3_TO_HS2    :  4;   // [11:08], RW
    uint32                MID_HW_S3_TO_HS2    :  4;   // [15:12], RW
    uint32                MID_HR_S4_TO_HS2    :  4;   // [19:16], RW
    uint32                MID_HW_S4_TO_HS2    :  4;   // [23:20], RW
    uint32                reserved0           :  8;   // [31:24]
} MC_MID_HS2;

typedef union {
    uint32                nReg;
    MC_MID_HS2          bReg;
} MC_MID_HS2_U;

// MID_HS3
typedef struct {
    uint32                MID_HR_S0_TO_HS3    :  4;   // [03:00], RW
    uint32                MID_HW_S0_TO_HS3    :  4;   // [07:00], RW
    uint32                MID_HR_S1_TO_HS3    :  4;   // [11:08], RW
    uint32                MID_HW_S1_TO_HS3    :  4;   // [15:12], RW
    uint32                MID_HR_S2_TO_HS3    :  4;   // [19:16], RW
    uint32                MID_HW_S2_TO_HS3    :  4;   // [23:20], RW
    uint32                reserved0           :  8;   // [31:24]
} MC_MID_HS3;

typedef union {
    uint32                nReg;
    MC_MID_HS3          bReg;
} MC_MID_HS3_U;

// MID_HS4
typedef struct {
    uint32                MID_HR_S0_TO_HS4    :  4;   // [0][03:00], RW
    uint32                MID_HW_S0_TO_HS4    :  4;   // [0][07:00], RW
    uint32                MID_HR_S1_TO_HS4    :  4;   // [0][11:08], RW
    uint32                MID_HW_S1_TO_HS4    :  4;   // [0][15:12], RW
    uint32                MID_HR_S2_TO_HS4    :  4;   // [0][19:16], RW
    uint32                MID_HW_S2_TO_HS4    :  4;   // [0][23:20], RW
    uint32                MID_HR_S3_TO_HS4    :  4;   // [0][27:24], RW
    uint32                MID_HW_S3_TO_HS4    :  4;   // [0][31:28], RW
    uint32                MID_HR_S4_TO_HS4    :  4;   // [1][03:00], RW
    uint32                MID_HW_S4_TO_HS4    :  4;   // [1][07:00], RW
    uint32                MID_HR_S5_TO_HS4    :  4;   // [1][11:08], RW
    uint32                MID_HW_S5_TO_HS4    :  4;   // [1][15:12], RW
    uint32                                    : 16;   // [1][31:16], RW
} MC_MID_HS4;

typedef union {
    uint32                nReg[2];
    MC_MID_HS4          bReg;
} MC_MID_HS4_U;

// MID_HS5
typedef struct {
    uint32                MID_HR_S0_TO_HS5    :  4;   // [0][03:00], RW
    uint32                MID_HW_S0_TO_HS5    :  4;   // [0][07:00], RW
    uint32                MID_HR_S1_TO_HS5    :  4;   // [0][11:08], RW
    uint32                MID_HW_S1_TO_HS5    :  4;   // [0][15:12], RW
    uint32                MID_HR_S2_TO_HS5    :  4;   // [0][19:16], RW
    uint32                MID_HW_S2_TO_HS5    :  4;   // [0][23:20], RW
    uint32                MID_HR_S3_TO_HS5    :  4;   // [0][27:24], RW
    uint32                MID_HW_S3_TO_HS5    :  4;   // [0][31:28], RW
    uint32                MID_HR_S4_TO_HS5    :  4;   // [1][03:00], RW
    uint32                MID_HW_S4_TO_HS5    :  4;   // [1][07:00], RW
    uint32                MID_HR_S5_TO_HS5    :  4;   // [1][11:08], RW
    uint32                MID_HW_S5_TO_HS5    :  4;   // [1][15:12], RW
    uint32                                    : 16;   // [1][31:16], RW
} MC_MID_HS5;

typedef union {
    uint32                nReg[2];
    MC_MID_HS5          bReg;
} MC_MID_HS5_U;

// MID configuration soft fault status
typedef struct {
    uint32                REG_MID_XS          :  1;   // [   00], RW
    uint32                REG_MID_HS2         :  1;   // [   01], RW
    uint32                REG_MID_HS3         :  1;   // [   02], RW
    uint32                REG_MID_HS4_0       :  1;   // [   03], RW
    uint32                REG_MID_HS4_1       :  1;   // [   04], RW
    uint32                REG_MID_HS5_0       :  1;   // [   05], RW
    uint32                REG_MID_HS5_1       :  1;   // [   06], RW
    uint32                REG_MID_WR_PW       :  1;   // [   07], RW
    uint32                REG_MID_WR_LOCK     :  1;   // [   08], RW
    uint32                                    : 23;   // [31:09], Reserved
} MC_MID_SOFT_FAULT_EN;

typedef union {
    uint32                    nReg;
    MC_MID_SOFT_FAULT_EN    bReg;
} MC_MID_SOFT_FAULT_EN_U;

// Micom configuration soft control
typedef struct {
    uint32                SOFT_FAULT_EN       :  1;   // [   00]
    uint32                reserved0           :  3;   // [04:01] 
    uint32                SOFT_FAULT_TEST_EN  :  1;   // [   05]
    uint32                reserved1           :  3;   // [07:06] 
    uint32                SOFT_FAULT_REQ_INIT :  1;   // [   08]
    uint32                reserved2           :  7;   // [15:09] 
    uint32                TIMEOUT_VALUE       : 12;   // [27:16]
    uint32                reserved3           :  4;   // [31:28] 
} MC_MID_SOFT_FAULT_CTRL;

typedef union {
    uint32                    nReg;
    MC_MID_SOFT_FAULT_CTRL  bReg;
} MC_MID_SOFT_FAULT_CTRL_U;

// Micom configuration soft control status
typedef struct {
    uint32                SOFT_FAULT_STS      :  1;   // [   00]
    uint32                reserved0           : 15;   // [15:01] 
    uint32                SOFT_FAULT_REQ      :  1;   // [   16]
    uint32                reserved1           :  7;   // [23:17] 
    uint32                SOFT_FAULT_ACK      :  1;   // [   24]
    uint32                reserved2           :  7;   // [31:25] 
} MC_MID_SOFT_FAULT_STS;

typedef union {
    uint32                    nReg;
    MC_MID_SOFT_FAULT_STS   bReg;
} MC_MID_SOFT_FAULT_STS_U;

//----------------------------------------------------------------------------------------------------------------------------
// MID configuration register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MC_MID_XS_U                 uMID_XS;                // 0x00
    MC_MID_HS2_U                uMID_HS2;               // 0x04
    MC_MID_HS3_U                uMID_HS3;               // 0x08
    MC_MID_HS4_U                uMID_HS4;               // 0x0c ~ 0x10
    MC_MID_HS5_U                uMID_HS5;               // 0x14 ~ 0x18
    uint32                        CFG_WR_PW;              // 0x1c
    uint32                        CFG_WR_LOCK;            // 0x20
    uint32                        reserved[3];            // 0x24 ~ 0x2c
    MC_MID_SOFT_FAULT_EN_U      uSOFT_FAULT_EN;         // 0x30
    MC_MID_SOFT_FAULT_EN_U      uSOFT_FAULT_STS;        // 0x34
    MC_MID_SOFT_FAULT_CTRL_U    uSOFT_FAULT_CTRL;       // 0x38
    MC_MID_SOFT_FAULT_STS_U     uSOFT_FAULT_CTRL_STS;   // 0x3c
} MC_MID;

#ifndef COWORK_DMA
// Micom bus clock/rst mask-0~2
typedef struct {
    uint32                NSAID_FILTER        :  1;   // [0][   00], RW
    uint32                NSAID_FILTER_BUS    :  1;   // [0][   01], RW
    uint32                NSAID_FILTER_CFG    :  1;   // [0][   02], RW
    uint32                SGID_FILTER         :  1;   // [0][   03], RW
    uint32                SGID_FILTER_BUS     :  1;   // [0][   04], RW
    uint32                SGID_FILTER_CFG     :  1;   // [0][   05], RW
    uint32                SFMC_WRAP           :  1;   // [0][   06], RW
    uint32                SFMC_MID_FILTER_BUS :  1;   // [0][   07], RW
    uint32                SFMC_MID_FILTER_CFG :  1;   // [0][   08], RW
    uint32                SFMC_X2H            :  1;   // [0][   09], RW
    uint32                SFMC_CORE_BUS       :  1;   // [0][   10], RW
    uint32                IMC_WRAP            :  1;   // [0][   11], RW
    uint32                IMC_MID_FILTER_BUS  :  1;   // [0][   12], RW
    uint32                IMC_MID_FILTER_CFG  :  1;   // [0][   13], RW
    uint32                IMC_CORE_BUS        :  1;   // [0][   14], RW
    uint32                GIC400              :  1;   // [0][   15], RW
    uint32                GIC400_H2X          :  1;   // [0][   16], RW
    uint32                GPIO                :  1;   // [0][   17], RW
    uint32                SOC400              :  1;   // [0][   18], RW
    uint32                JTAG_DBG_IF         :  1;   // [0][   19], RW
    uint32                DMA                 :  1;   // [0][   20], RW
    uint32                CAN_WRAP            :  1;   // [0][   21], RW
    uint32                CAN0                :  1;   // [0][   22], RW
    uint32                CAN1                :  1;   // [0][   23], RW
    uint32                CAN2                :  1;   // [0][   24], RW
    uint32                CAN_CFG             :  1;   // [0][   25], RW
    uint32                                    :  6;   // [0][31:26], RW
    uint32                UART_WRAP           :  1;   // [1][   00], RW
    uint32                UART0               :  1;   // [1][   01], RW
    uint32                UART1               :  1;   // [1][   02], RW
    uint32                UART2               :  1;   // [1][   03], RW
    uint32                UART3               :  1;   // [1][   04], RW
    uint32                UART4               :  1;   // [1][   05], RW
    uint32                UART5               :  1;   // [1][   06], RW
    uint32                UART_DMA0           :  1;   // [1][   07], RW
    uint32                UART_DMA1           :  1;   // [1][   08], RW
    uint32                UART_DMA2           :  1;   // [1][   09], RW
    uint32                UART_DMA3           :  1;   // [1][   10], RW
    uint32                UART_DMA4           :  1;   // [1][   11], RW
    uint32                UART_DMA5           :  1;   // [1][   12], RW
    uint32                UART_CFG            :  1;   // [1][   13], RW
    uint32                I2C_WRAP            :  1;   // [1][   14], RW
    uint32                I2C0                :  1;   // [1][   15], RW
    uint32                I2C1                :  1;   // [1][   16], RW
    uint32                I2C2                :  1;   // [1][   17], RW
    uint32                I2C_CFG             :  1;   // [1][   18], RW
    uint32                PDM_WRAP            :  1;   // [1][   19], RW
    uint32                PDM0                :  1;   // [1][   20], RW
    uint32                PDM1                :  1;   // [1][   21], RW
    uint32                PDM2                :  1;   // [1][   22], RW
    uint32                PDM_CFG             :  1;   // [1][   23], RW
    uint32                ICTC_WRAP           :  1;   // [1][   24], RW
    uint32                ICTC0               :  1;   // [1][   25], RW
    uint32                ICTC1               :  1;   // [1][   26], RW
    uint32                ICTC2               :  1;   // [1][   27], RW
    uint32                ADC_WRAP            :  1;   // [1][   28], RW
    uint32                MBOX_WRAP           :  1;   // [1][   29], RW
    uint32                                    :  2;   // [1][31:30], RW
    uint32                TIMER_WRAP          :  1;   // [2][   00], RW
    uint32                TIMER0              :  1;   // [2][   01], RW
    uint32                TIMER1              :  1;   // [2][   02], RW
    uint32                TIMER2              :  1;   // [2][   03], RW
    uint32                TIMER3              :  1;   // [2][   04], RW
    uint32                TIMER4              :  1;   // [2][   05], RW
    uint32                TIMER5              :  1;   // [2][   06], RW
    uint32                WATCHDOG            :  1;   // [2][   07], RW
    uint32                X2X_MST             :  1;   // [2][   08], RW
    uint32                X2X_SLV             :  1;   // [2][   09], RW
    uint32                GPSB_WRAP           :  1;   // [2][   10], RW
    uint32                GPSB0               :  1;   // [2][   11], RW
    uint32                GPSB1               :  1;   // [2][   12], RW
    uint32                GPSB2               :  1;   // [2][   13], RW
    uint32                GPSB3               :  1;   // [2][   14], RW
    uint32                GPSB4               :  1;   // [2][   15], RW
    uint32                GPSB5               :  1;   // [2][   16], RW
    uint32                GPSB_CFG            :  1;   // [2][   17], RW
    uint32                GPSB_IO_MON         :  1;   // [2][   18], RW
    uint32                CMU                 :  1;   // [2][   19], RW
    uint32                SYS_CTRL_SM         :  1;   // [2][   20], RW
    uint32                                    : 11;   // [2][31:21], RW
} MC_BCLK_RST_MASK;

typedef union {
    uint32                nReg[3];
    MC_BCLK_RST_MASK    bReg;
} MC_BCLK_RST_MASK_U;

// Micom IMC configuration
typedef struct {
    uint32                RAM_RDW             :  1;   // [   00], RW
    uint32                ROM_RDW             :  1;   // [   01], RW
    uint32                                    : 30;   // [31:02], RW
} MC_IMC_CFG;

typedef union {
    uint32                nReg;
    MC_IMC_CFG          bReg;
} MC_IMC_CFG_U;

// Micom DMA single request configuration
typedef struct {
    uint32                ICTC0               :  1;   // [   00], RW
    uint32                ICTC1               :  1;   // [   01], RW
    uint32                ICTC2               :  1;   // [   02], RW
    uint32                ADC                 :  1;   // [   03], RW
    uint32                CAN0                :  1;   // [   04], RW
    uint32                CAN1                :  1;   // [   05], RW
    uint32                CAN2                :  1;   // [   06], RW
    uint32                                    : 25;   // [31:07], RW
} MC_DMA_REQ_CFG;

typedef union {
    uint32                nReg;
    MC_DMA_REQ_CFG      bReg;
} MC_DMA_REQ_CFG_U;

typedef struct {
    uint32                                    :  8;   // [0][07:00], RW
    uint32                                    :  8;   // [0][15:08], RW
    uint32                                    :  8;   // [0][23:16], RW
    uint32                DMA_REQ7_SEL        :  8;   // [0][31:24], RW
    uint32                DMA_REQ8_SEL        :  8;   // [1][07:00], RW
    uint32                DMA_REQ9_SEL        :  8;   // [1][15:08], RW
    uint32                DMA_REQ10_SEL       :  8;   // [1][23:16], RW
    uint32                DMA_REQ11_SEL       :  8;   // [1][31:24], RW
    uint32                DMA_REQ12_SEL       :  8;   // [2][07:00], RW
    uint32                DMA_REQ13_SEL       :  8;   // [2][15:08], RW
    uint32                DMA_REQ14_SEL       :  8;   // [2][23:16], RW
    uint32                DMA_REQ15_SEL       :  8;   // [2][31:24], RW
} MC_DMA_REQ_SEL;

typedef union {
    uint32                nReg[3];
    MC_DMA_REQ_SEL      bReg;
} MC_DMA_REQ_SEL_U;

// Micom asynchronous bridge power control
typedef struct {
    uint32                MC2MB_PWRDNREQN     :  1;   // [   00], RW
    uint32                MC2MB_PWRDNACKN     :  1;   // [   01], RO
    uint32                MC2MB_CACTIVES      :  1;   // [   02], RO
    uint32                                    :  1;   // [   03], Reserved
    uint32                MB2MC_PWRDNREQN     :  1;   // [   04], RW
    uint32                MB2MC_PWRDNACKN     :  1;   // [   05], RO
    uint32                MB2MC_CACTIVEM      :  1;   // [   06], RO
    uint32                                    :  9;   // [15:07], Reserved
    uint32                MC2MB_MST_RST       :  1;   // [   16], RW
    uint32                MC2MB_SLV_RST       :  1;   // [   17], RO
    uint32                                    :  2;   // [19:18], Reserved
    uint32                MB2MC_SLV_RST       :  1;   // [   20], RW
    uint32                MB2MC_MST_RST       :  1;   // [   21], RO
    uint32                                    : 10;   // [31:22], Reserved
} MC_X2X_PWR_CTRL;

typedef union {
    uint32                nReg;
    MC_X2X_PWR_CTRL     bReg;
} MC_X2X_PWR_CTRL_U;

// Micom external access filter power control
typedef struct {
    uint32                NSAID_FIL_PWRDNREQN :  1;   // [   00], RW
    uint32                NSAID_FIL_PWRDNACKN :  1;   // [   01], RO
    uint32                NSAID_FIL_CACTIVE   :  1;   // [   02], RO
    uint32                                    :  1;   // [   03], Reserved
    uint32                SGID_FIL_PWRDNREQN  :  1;   // [   04], RW
    uint32                SGID_FIL_PWRDNACKN  :  1;   // [   05], RO
    uint32                SGID_FIL_CACTIVE    :  1;   // [   06], RO
    uint32                                    :  1;   // [   07], Reserved
    uint32                NSAID_CFG_PWRDNREQN :  1;   // [   08], RW
    uint32                NSAID_CFG_PWRDNACKN :  1;   // [   09], RO
    uint32                NSAID_CFG_CACTIVE   :  1;   // [   10], RO
    uint32                                    :  1;   // [   11], Reserved
    uint32                SGID_CFG_PWRDNREQN  :  1;   // [   12], RW
    uint32                SGID_CFG_PWRDNACKN  :  1;   // [   13], RO
    uint32                SGID_CFG_CACTIVE    :  1;   // [   14], RO
    uint32                                    :  1;   // [   15], Reserved
    uint32                NSAID_FIL_FPID      :  1;   // [   16], RW
    uint32                                    :  3;   // [19:17], Reserved
    uint32                SGID_FIL_FPID       :  1;   // [   20], RW
    uint32                                    : 11;   // [31:21], RW
} MC_AID_PWR_CTRL;

typedef union {
    uint32                nReg;
    MC_AID_PWR_CTRL     bReg;
} MC_AID_PWR_CTRL_U;

// Micom internal access filter power control
typedef struct {
    uint32                SFMC_FIL_PWRDNREQN  :  1;   // [   00], RW
    uint32                SFMC_FIL_PWRDNACKN  :  1;   // [   01], RO
    uint32                SFMC_FIL_CACTIVE    :  1;   // [   02], RO
    uint32                                    :  1;   // [   03], Reserved
    uint32                IMC_FIL_PWRDNREQN   :  1;   // [   04], RW
    uint32                IMC_FIL_PWRDNACKN   :  1;   // [   05], RO
    uint32                IMC_FIL_CACTIVE     :  1;   // [   06], RO
    uint32                                    :  1;   // [   07], Reserved
    uint32                SFMC_CFG_PWRDNREQN  :  1;   // [   08], RW
    uint32                SFMC_CFG_PWRDNACKN  :  1;   // [   09], RO
    uint32                SFMC_CFG_CACTIVE    :  1;   // [   10], RO
    uint32                                    :  1;   // [   11], Reserved
    uint32                IMC_CFG_PWRDNREQN   :  1;   // [   12], RW
    uint32                IMC_CFG_PWRDNACKN   :  1;   // [   13], RO
    uint32                IMC_CFG_CACTIVE     :  1;   // [   14], RO
    uint32                                    :  1;   // [   15], Reserved
    uint32                SFMC_FIL_FPID       :  1;   // [   16], RW
    uint32                                    :  3;   // [19:17], Reserved
    uint32                IMC_FIL_FPID        :  1;   // [   20], RW
    uint32                                    : 11;   // [31:21], RW
} MC_MID_PWR_CTRL;

typedef union {
    uint32                nReg;
    MC_MID_PWR_CTRL     bReg;
} MC_MID_PWR_CTRL_U;

// Micom debug control
typedef struct {
    uint32                CR5_DBGEN           :  1;   // [   00], RW
    uint32                CR5_NIDEN           :  1;   // [   01], RW
    uint32                                    :  2;   // [03:02], Reserved
    uint32                SOC400_DBGEN        :  1;   // [   04], RW
    uint32                SOC400_NIDEN        :  1;   // [   05], RW
    uint32                SOC400_SPIDEN       :  1;   // [   06], RW
    uint32                                    :  9;   // [15:07], Reserved
    uint32                GIC_CFG_DIS         :  1;   // [   16], RW
    uint32                GIC_DFT_RST_DIS     :  1;   // [   17], RW
    uint32                                    :  6;   // [23:18], Reserved
    uint32                DP_APB_SEL          :  1;   // [   24], RW
    uint32                                    :  7;   // [31:25], Reserved
} MC_DBG_DP_CTRL;

typedef union {
    uint32                nReg;
    MC_DBG_DP_CTRL      bReg;
} MC_DBG_DP_CTRL_U;

// Micom system user mode write disable
typedef struct {
    uint32                NIC400_GPV          :  1;   // [0][   00], RW
    uint32                GIG400              :  1;   // [0][   01], RW
    uint32                SMFC                :  1;   // [0][   02], RW
    uint32                CMU                 :  1;   // [0][   03], RW
    uint32                SYS_SM_CTRL         :  1;   // [0][   04], RW
    uint32                MC_CFG              :  1;   // [0][   05], RW
    uint32                CCU                 :  1;   // [0][   06], RW
    uint32                CR5_CFG             :  1;   // [0][   07], RW
    uint32                TIMER0              :  1;   // [0][   08], RW
    uint32                TIMER1              :  1;   // [0][   09], RW
    uint32                TIMER2              :  1;   // [0][   10], RW
    uint32                TIMER3              :  1;   // [0][   11], RW
    uint32                TIMER4              :  1;   // [0][   12], RW
    uint32                TIMER5              :  1;   // [0][   13], RW
    uint32                WDT                 :  1;   // [0][   14], RW
    uint32                GPIO                :  1;   // [0][   15], RW
    uint32                PMU                 :  1;   // [0][   16], RW
    uint32                FMU                 :  1;   // [0][   17], RW
    uint32                PMIO                :  1;   // [0][   18], RW
    uint32                DEF_SLV_IRQ         :  1;   // [0][   19], RW
    uint32                MID_CFG             :  1;   // [0][   20], RW
    uint32                SFMC_MID_FILTER     :  1;   // [0][   21], RW
    uint32                IMC_MID_FILTER      :  1;   // [0][   22], RW
    uint32                SGID_FILTER         :  1;   // [0][   23], RW
    uint32                NSAID_FILTER        :  1;   // [0][   24], RW
    uint32                LVDS_DEMUX          :  1;   // [0][   25], RW
    uint32                MIPI_WRAP           :  1;   // [0][   26], RW
    uint32                DP_WRAP             :  1;   // [0][   27], RW
    uint32                                    :  4;   // [0][31:28]
    uint32                S_MBOX0             :  1;   // [1][   00], RW
    uint32                NS_MBOX0            :  1;   // [1][   01], RW
    uint32                S_MBOX1             :  1;   // [1][   02], RW
    uint32                NS_MBOX1            :  1;   // [1][   03], RW
    uint32                MBOX2               :  1;   // [1][   04], RW
    uint32                                    : 27;   // [1][31:05]
} MC_SYS_USER_WR_DIS;

typedef union {
    uint32                nReg[2];
    MC_SYS_USER_WR_DIS  bReg;
} MC_SYS_USER_WR_DIS_U;

// Micom configuration soft fault status
typedef struct {
    uint32                REG_HCLK_MASK0          :  1;   // [   00], RW, 0x0_0, HCLK clock mask-0
    uint32                REG_HCLK_MASK1          :  1;   // [   01], RW, 0x0_4, HCLK clock mask-1
    uint32                REG_HCLK_MASK2          :  1;   // [   02], RW, 0x0_8, HCLK clock mask-2
    uint32                REG_SW_HRESET0          :  1;   // [   03], RW, 0x0_c, SW HRESETn mask-0
    uint32                REG_SW_HRESET1          :  1;   // [   04], RW, 0x1_0, SW HRESETn mask-1
    uint32                REG_SW_HRESET2          :  1;   // [   05], RW, 0x1_4, SW HRESETn mask-2
    uint32                REG_IMC_CFG             :  1;   // [   06], RW, 0x1_8, Internal Memory controller config.
    uint32                REG_DMA_SREQ_EN         :  1;   // [   07], RW, 0x1_c, DMA single request enable
    uint32                REG_DMA_REQ_SEL0        :  1;   // [   08], RW, 0x2_0, DMA request select-0
    uint32                REG_DMA_REQ_SEL1        :  1;   // [   09], RW, 0x2_4, DMA request select-1
    uint32                REG_DMA_REQ_SEL2        :  1;   // [   10], RW, 0x2_8, DMA request select-2
    uint32                REG_X2X_PWR_CTRL        :  1;   // [   11], RW, 0x2_c, Axi to Axi Async. Bridge(MICOM/MEMBUS to MEMBUS/MICOM) power management signal
    uint32                REG_AID_PWR_CTRL        :  1;   // [   12], RW, 0x3_0, AID(NSAID, SGID) filter power management signal
    uint32                REG_MID_PWR_CTRL        :  1;   // [   13], RW, 0x3_4, MID( SFMC, IMC ) filter power management signal
    uint32                REG_DEBUG_DP_CTRL       :  1;   // [   14], RW, 0x3_8, DBG_EN/NIDEN/SPIDEN & DP APB control select
    uint32                REG_SYS_USER_WR_DIS0    :  1;   // [   15], RW, 0x3_c, System block user mode write disabla-0
    uint32                REG_SYS_USER_WR_DIS1    :  1;   // [   16], RW, 0x4_0, System block user mode write disable-1
    uint32                REG_MICOM_READY         :  1;   // [   17], RW, 0x4_4, MICOM ready
    uint32                REG_MC_CFG_WR_PW        :  1;   // [   18], RW, 0x4_8, Micom configuration write password
    uint32                REG_MC_CFG_WR_LOCK      :  1;   // [   19], RW, 0x4_c, Micom configuration write lock
    uint32                                        : 12;   // [31:20], Reserved
} MC_CFG_SOFT_FAULT_EN;

typedef union {
    uint32                    nReg;
    MC_CFG_SOFT_FAULT_EN    bReg;
} MC_CFG_SOFT_FAULT_EN_U;

// Micom configuration soft control
typedef struct {
    uint32                SOFT_FAULT_EN       :  1;   // [   00]
    uint32                                    :  3;   // [04:01] 
    uint32                SOFT_FAULT_TEST_EN  :  1;   // [   05]
    uint32                                    :  3;   // [07:06] 
    uint32                SOFT_FAULT_REQ_INIT :  1;   // [   08]
    uint32                                    :  7;   // [15:09] 
    uint32                TIMEOUT_VALUE       : 12;   // [27:16]
    uint32                                    :  4;   // [31:28] 
} MC_CFG_SOFT_FAULT_CTRL;

typedef union {
    uint32                    nReg;
    MC_CFG_SOFT_FAULT_CTRL  bReg;
} MC_CFG_SOFT_FAULT_CTRL_U;

// Micom configuration soft control status
typedef struct {
    uint32                SOFT_FAULT_STS      :  1;   // [   00]
    uint32                                    : 15;   // [15:01] 
    uint32                SOFT_FAULT_REQ      :  1;   // [   16]
    uint32                                    :  7;   // [23:17] 
    uint32                SOFT_FAULT_ACK      :  1;   // [   24]
    uint32                                    :  7;   // [31:25] 
} MC_CFG_SOFT_FAULT_STS;

typedef union {
    uint32                    nReg;
    MC_CFG_SOFT_FAULT_STS   bReg;
} MC_CFG_SOFT_FAULT_STS_U;

//----------------------------------------------------------------------------------------------------------------------------
// Micom configuration register structure
//----------------------------------------------------------------------------------------------------------------------------
typedef struct {
    MC_BCLK_RST_MASK_U          uBCLK_MASK;             // 0x00 ~ 0x08
    MC_BCLK_RST_MASK_U          uSW_RST_MASK;           // 0x0c ~ 0x14
    MC_IMC_CFG_U                uIMC_CFG;               // 0x18
    MC_DMA_REQ_CFG_U            uDMA_REQ_CFG;           // 0x1c
    MC_DMA_REQ_SEL_U            uDMA_REQ_SEL;           // 0x20 ~ 0x28
    MC_X2X_PWR_CTRL_U           uX2X_PWR_CTRL;          // 0x2c
    MC_AID_PWR_CTRL_U           uAID_PWR_CTRL;          // 0x30
    MC_MID_PWR_CTRL_U           uMID_PWR_CTRL;          // 0x34
    MC_DBG_DP_CTRL_U            uDBG_CTRL;           // 0x38
    MC_SYS_USER_WR_DIS_U        uCFG_USER_WR_DIS;       // 0x3c ~ 0x40
    uint32                        MICOM_READY;            // 0x44
    uint32                        CFG_WR_PW;              // 0x48
    uint32                        CFG_WR_LOCK;            // 0x4c
    MC_CFG_SOFT_FAULT_EN_U      uSOFT_FAULT_EN;         // 0x50
    uint32                        reserved1[3];           // 0x54 ~ 0x5c
    MC_CFG_SOFT_FAULT_EN_U      uSOFT_FAULT_STS;        // 0x60
    uint32                        reserved2[3];           // 0x64 ~ 0x6c
    MC_CFG_SOFT_FAULT_CTRL_U    uSOFT_FAULT_CTRL;       // 0x70
    MC_CFG_SOFT_FAULT_STS_U     uSOFT_FAULT_CTRL_STS;   // 0x74
} MC_CFG;
#endif

#define HwMC_CFG            ((volatile MC_CFG           *)(MICOM_BASE_ADDR + 0x930000))

#define HwMC_IMC_MID_FIL    ((volatile TZC400           *)(MICOM_BASE_ADDR + 0x93B000))
#define HwMC_SFMC_MID_FIL   ((volatile TZC400           *)(MICOM_BASE_ADDR + 0x93A000))

#define HwMC_MID            ((volatile MC_MID *)(MICOM_BASE_ADDR + 0x939000))

/**
 * MID Filter ID list.
 */
enum {
#ifdef TCC8050_TEST
	MID_ID_DMA0 = 0,
	MID_ID_DMA1,
	MID_ID_CAN0,
	MID_ID_CAN1,
	MID_ID_CAN2,
	MID_ID_GPSB0,		// 5
	MID_ID_GPSB1,
	MID_ID_GPSB2,
	MID_ID_GPSB3_4_5,
	MID_ID_UART0,
	MID_ID_UART1,		// 10
	MID_ID_UART2,
	MID_ID_UART3_4_5,
	MID_ID_CR5,
	MID_ID_JTAG,
	MID_ID_AP,		//15
#else
	MID_ID_DMA0 = 0,
	MID_ID_DMA1,
	MID_ID_CAN0,
	MID_ID_CAN1,
	MID_ID_CAN2,
	MID_ID_GPSB0,		// 5
	MID_ID_GPSB1,
	MID_ID_GPSB2,
	MID_ID_GPSB3,
	MID_ID_UART0,
	MID_ID_UART1,		// 10
	MID_ID_UART2,
	MID_ID_UART3,
	MID_ID_CR5,
	MID_ID_JTAG,
	MID_ID_AP,		//15

#endif	
};

#define MID_TYPE_READ			(1<<0)
#define MID_TYPE_WRITE			(1<<1)
#define SRAM1_BASE          0xc1000000                      // 0xC1000000 ~ 0xC000FFFF


static int32 MIDF_T01_ClockGatingAndSwResetTest(void);
static int32 MIDF_T02_GateKeeperTest(void);
static void MIDF_IMCTestConfig(void);
static void MIDF_SFMCTestConfig(void);
static int32 MIDF_T03_FilterActionTest(void);
static int32 MIDF_T04_SFMCFiterActionTest(void);
static void MIDF_IMCIsrHandler(void * param);
static void MIDF_IMCIsrHandler(void * param);
static void MIDF_SFMCIsrHandler(void * param);
static void MIDF_WaitIrqDone(void);
static int32 MIDF_ConfigSoftFaultCheckSafetyMechanism_FaultInjectTest( uint32 uiSvlLevel, uint32 uiFaultCheckGroup);

/**
 * Static Variables
 */
static uint32 midf_irq_done;

static void MIDF_IMCIsrHandler(void * param) 
{
	TZC400 * pIMC_MIDCtl=  ((TZC400 *)(MICOM_BASE_ADDR + 0x93B000));

	if(pIMC_MIDCtl->uINT_STATUS.nReg !=0)
	{
		pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
	}

	midf_irq_done = 1;
}

static void MIDF_SFMCIsrHandler(void * param) 
{
	TZC400 * pIMC_MIDCtl = ((TZC400 *)(HwMC_SFMC_MID_FIL));

	if ( pIMC_MIDCtl->uINT_STATUS.nReg !=0)
	{
		pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
	}

	midf_irq_done = 1;
}



int32 MIDF_IPTest(int32 test_case )
{
	int32 ret =0;
	switch (test_case)
	{
		case 0:
			mcu_printf("MIDF TC 00 : MIDF_T01_ClockGatingAndSwResetTest \n");
			ret = MIDF_T01_ClockGatingAndSwResetTest();
			break;
		case 1:
			mcu_printf("MIDF TC 01 : mid_filter_init/register_mid_filter \n");
			MIDF_FilterInitialize();
			MIDF_RegisterFilterRegion(0xc1000000, 0x4000, MID_ID_CAN0, (MID_TYPE_READ|MID_TYPE_WRITE));
			MIDF_UnregisterFilterRegion(0xc1000000, 0x4000, MID_ID_CAN0, (MID_TYPE_READ|MID_TYPE_WRITE));
			break;
		case 2:
			mcu_printf("MIDF TC 02 : MIDF_T02_GateKeeperTest \n");
			ret = MIDF_T02_GateKeeperTest();
			break;
		case 3:
			mcu_printf("MIDF TC 03 : MIDF_T03_FilterActionTest \n");
			ret = MIDF_T03_FilterActionTest();
			break;
		case 4:
			mcu_printf("MIDF TC 04 : MIDF_T03_SFMCFiterActionTest \n");
			ret = MIDF_T04_SFMCFiterActionTest();
			break;
		case 5:
			mcu_printf("MIDF TC 05 : Safety Mechanism - Write Lock \n");
			MIDF_FilterInitialize();
			ret = MIDF_RegisterWriteLockSafetyMechanism(1);

			// TEST D.S using T32 for XS_MID ~  HS5_MID0/1
			// Expected Result : Can not write the MID configuration register
			break;
			
		case 6:
			mcu_printf("MIDF TC 05 : Safety Mechanism - Soft Fault Check \n");
			MIDF_FilterInitialize();
			MIDF_ConfigSoftFaultCheckSafetyMechanism(0, MIDF_CHECK_GRP_ALL-1);

     //Test
			MIDF_ConfigSoftFaultCheckSafetyMechanism_FaultInjectTest(0, MIDF_CHECK_GRP_ALL);
			break;

			default:
			break;
	}
	return ret;	
}

static void MIDF_IMCTestConfig(void)
{
	uint32 ch;
	uint32 access = 0;
	TZC400 * pMIDCtl= (TZC400 *)(HwMC_IMC_MID_FIL);
	MC_MID * pMIDCfg = (MC_MID *)HwMC_MID;
	
	/* configure Master IDs */
	if ( pMIDCfg != NULL_PTR)
	{
		pMIDCfg->uMID_XS.nReg = (uint32)((MID_ID_CR5<<20)|(MID_ID_CR5<<16)|(MID_ID_AP<<4)|(MID_ID_AP<<0));
		pMIDCfg->uMID_HS2.nReg = (uint32)((MID_ID_JTAG<<20)|(MID_ID_JTAG<<16)|(MID_ID_DMA1<<12)|(MID_ID_DMA1<<8)| \
											(MID_ID_DMA0<<4)|(MID_ID_DMA0<<0));
		pMIDCfg->uMID_HS3.nReg = (uint32)((MID_ID_CAN2<<20)|(MID_ID_CAN2<<16)|(MID_ID_CAN1<<12)|(MID_ID_CAN1<<8)| \
											(MID_ID_CAN0<<4)|(MID_ID_CAN0<<0));
#ifdef TCC8050_TEST
		pMIDCfg->uMID_HS4.nReg[0] = (uint32)((uint32)(MID_ID_GPSB3_4_5<<28)|(MID_ID_GPSB3_4_5<<24)|(MID_ID_GPSB2<<20)|(MID_ID_GPSB2<<16)|\
											(MID_ID_GPSB1<<12)|(MID_ID_GPSB1<<8)|(MID_ID_GPSB0<<4)|(MID_ID_GPSB0<<0));
		pMIDCfg->uMID_HS4.nReg[1] = (uint32)((uint32)(MID_ID_GPSB3_4_5<<12)|(MID_ID_GPSB3_4_5<<8)|(MID_ID_GPSB3_4_5<<4)|(MID_ID_GPSB3_4_5<<0));

		pMIDCfg->uMID_HS5.nReg[0] = (uint32)((uint32)(MID_ID_UART3_4_5<<28)|(MID_ID_UART3_4_5<<24)|(MID_ID_UART2<<20)|(MID_ID_UART2<<16)|\
											(MID_ID_UART1<<12)|(MID_ID_UART1<<8)|(MID_ID_UART0<<4)|(MID_ID_UART0<<0));
		pMIDCfg->uMID_HS5.nReg[1] = (uint32)((uint32)(MID_ID_UART3_4_5<<12)|(MID_ID_UART3_4_5<<8)|(MID_ID_UART3_4_5<<4)|(MID_ID_UART3_4_5<<0));
		
#else
		pMIDCfg->uMID_HS4.nReg = (uint32)((uint32)(MID_ID_GPSB3<<28)|(MID_ID_GPSB3<<24)|(MID_ID_GPSB2<<20)|(MID_ID_GPSB2<<16)|\
											(MID_ID_GPSB1<<12)|(MID_ID_GPSB1<<8)|(MID_ID_GPSB0<<4)|(MID_ID_GPSB0<<0));
		pMIDCfg->uMID_HS5.nReg = (uint32)((uint32)(MID_ID_UART3<<28)|(MID_ID_UART3<<24)|(MID_ID_UART2<<20)|(MID_ID_UART2<<16)|\
											(MID_ID_UART1<<12)|(MID_ID_UART1<<8)|(MID_ID_UART0<<4)|(MID_ID_UART0<<0));
#endif
	}
	
	/* Initialize memory region filter */
	if ( pMIDCtl != NULL_PTR)
	{
		/* clear all region except region 0*/
		ch = TZC400_MAX_CH;
		while(ch!=0) 
		{
			if ( pMIDCtl != NULL_PTR)
			{
				pMIDCtl->uREGION_CTRL[ch].nReg[0] = 0x00000000U; /*BASE LOW*/
				pMIDCtl->uREGION_CTRL[ch].nReg[1] = 0x00000000U; /*BASE HIGH*/
				pMIDCtl->uREGION_CTRL[ch].nReg[2] = 0x00000000U; /*TOP LOW*/
				pMIDCtl->uREGION_CTRL[ch].nReg[3] = 0x00000000U; /*TOP HIGH*/
				pMIDCtl->uREGION_CTRL[ch].nReg[4] = 0x00000000U; /*ATTRIBUTE */
				pMIDCtl->uREGION_CTRL[ch].nReg[5] = 0x00000000U; /*ID ACCESS */
			}
			ch--;
		}

		/* set region0 by default config */
		pMIDCtl->uREGION_CTRL[0].nReg[0] = 0x00000000U; /*BASE LOW*/
		pMIDCtl->uREGION_CTRL[0].nReg[1] = 0x00000000U; /*BASE HIGH*/
		pMIDCtl->uREGION_CTRL[0].nReg[2] = 0xFFFFFFFFU; /*TOP LOW*/
		pMIDCtl->uREGION_CTRL[0].nReg[3] = 0x00000000U; /*TOP HIGH*/
		pMIDCtl->uREGION_CTRL[0].nReg[4] = 0x1; /*ATTRIBUTE : filter_en */
		//pMIDCtl->uREGION_CTRL[0].bReg.filter0_en = 0x1;
		access = (1<<MID_ID_CR5) | (1<<MID_ID_JTAG);
		access |= (access<<16);			
		pMIDCtl->uREGION_CTRL[0].nReg[5] = access; /*ID ACCESS */
	}
}


static void MIDF_SFMCTestConfig(void)
{
	uint32 ch;
	uint32 access = 0;
	TZC400 * pMIDCtl= (TZC400 *)(HwMC_SFMC_MID_FIL);
	MC_MID * pMIDCfg = (MC_MID *)HwMC_MID;
	
	/* configure Master IDs */
	if ( pMIDCfg != NULL_PTR)
	{
		pMIDCfg->uMID_XS.nReg = (uint32)((MID_ID_CR5<<20)|(MID_ID_CR5<<16)|(MID_ID_AP<<4)|(MID_ID_AP<<0));
		pMIDCfg->uMID_HS2.nReg = (uint32)((MID_ID_JTAG<<20)|(MID_ID_JTAG<<16)|(MID_ID_DMA1<<12)|(MID_ID_DMA1<<8)| \
											(MID_ID_DMA0<<4)|(MID_ID_DMA0<<0));
		pMIDCfg->uMID_HS3.nReg = (uint32)((MID_ID_CAN2<<20)|(MID_ID_CAN2<<16)|(MID_ID_CAN1<<12)|(MID_ID_CAN1<<8)| \
											(MID_ID_CAN0<<4)|(MID_ID_CAN0<<0));
#ifdef TCC8050_TEST
		pMIDCfg->uMID_HS4.nReg[0] = (uint32)((uint32)(MID_ID_GPSB3_4_5<<28)|(MID_ID_GPSB3_4_5<<24)|(MID_ID_GPSB2<<20)|(MID_ID_GPSB2<<16)|\
											(MID_ID_GPSB1<<12)|(MID_ID_GPSB1<<8)|(MID_ID_GPSB0<<4)|(MID_ID_GPSB0<<0));
		pMIDCfg->uMID_HS4.nReg[1] = (uint32)((uint32)(MID_ID_GPSB3_4_5<<12)|(MID_ID_GPSB3_4_5<<8)|(MID_ID_GPSB3_4_5<<4)|(MID_ID_GPSB3_4_5<<0));

		pMIDCfg->uMID_HS5.nReg[0] = (uint32)((uint32)(MID_ID_UART3_4_5<<28)|(MID_ID_UART3_4_5<<24)|(MID_ID_UART2<<20)|(MID_ID_UART2<<16)|\
											(MID_ID_UART1<<12)|(MID_ID_UART1<<8)|(MID_ID_UART0<<4)|(MID_ID_UART0<<0));
		pMIDCfg->uMID_HS5.nReg[1] = (uint32)((uint32)(MID_ID_UART3_4_5<<12)|(MID_ID_UART3_4_5<<8)|(MID_ID_UART3_4_5<<4)|(MID_ID_UART3_4_5<<0));
		
#else
		pMIDCfg->uMID_HS4.nReg = (uint32)((uint32)(MID_ID_GPSB3<<28)|(MID_ID_GPSB3<<24)|(MID_ID_GPSB2<<20)|(MID_ID_GPSB2<<16)|\
											(MID_ID_GPSB1<<12)|(MID_ID_GPSB1<<8)|(MID_ID_GPSB0<<4)|(MID_ID_GPSB0<<0));
		pMIDCfg->uMID_HS5.nReg = (uint32)((uint32)(MID_ID_UART3<<28)|(MID_ID_UART3<<24)|(MID_ID_UART2<<20)|(MID_ID_UART2<<16)|\
											(MID_ID_UART1<<12)|(MID_ID_UART1<<8)|(MID_ID_UART0<<4)|(MID_ID_UART0<<0));
#endif
	}
	
	/* Initialize memory region filter */
	if ( pMIDCtl != NULL_PTR)
	{
		/* clear all region except region 0*/
		ch = TZC400_MAX_CH;
		while(ch!=0) 
		{
			if ( pMIDCtl != NULL_PTR)
			{
				pMIDCtl->uREGION_CTRL[ch].nReg[0] = 0x00000000U; /*BASE LOW*/
				pMIDCtl->uREGION_CTRL[ch].nReg[1] = 0x00000000U; /*BASE HIGH*/
				pMIDCtl->uREGION_CTRL[ch].nReg[2] = 0x00000000U; /*TOP LOW*/
				pMIDCtl->uREGION_CTRL[ch].nReg[3] = 0x00000000U; /*TOP HIGH*/
				pMIDCtl->uREGION_CTRL[ch].nReg[4] = 0x00000000U; /*ATTRIBUTE */
				pMIDCtl->uREGION_CTRL[ch].nReg[5] = 0x00000000U; /*ID ACCESS */
			}
			ch--;
		}

		/* set region0 by default config */
		pMIDCtl->uREGION_CTRL[0].nReg[0] = 0x00000000U; /*BASE LOW*/
		pMIDCtl->uREGION_CTRL[0].nReg[1] = 0x00000000U; /*BASE HIGH*/
		pMIDCtl->uREGION_CTRL[0].nReg[2] = 0xFFFFFFFFU; /*TOP LOW*/
		pMIDCtl->uREGION_CTRL[0].nReg[3] = 0x00000000U; /*TOP HIGH*/
		pMIDCtl->uREGION_CTRL[0].nReg[4] = 0x1; /*ATTRIBUTE : filter_en */
		//pMIDCtl->uREGION_CTRL[0].bReg.filter0_en = 0x1;
		access = (1<<MID_ID_CR5) | (1<<MID_ID_JTAG);
		access |= (access<<16);			
		pMIDCtl->uREGION_CTRL[0].nReg[5] = access; /*ID ACCESS */
	}
}

// can not test on target system
static int32 MIDF_T01_ClockGatingAndSwResetTest(void)
{
	#if 0
	MC_CFG * pMcCfg = (MC_CFG *)HwMC_CFG;
	TZC400 * pMIDCtl= (TZC400 *)(HwMC_IMC_MID_FIL);

	MC_MID * pMIDCfg =  ((MC_MID           *)(MICOM_BASE_ADDR + 0x939000));
	TZC400 * pSFMC_MIDCtl=  ((TZC400           *)(MICOM_BASE_ADDR + 0x93A000));
	TZC400 * pIMC_MIDCtl=  ((TZC400           *)(MICOM_BASE_ADDR + 0x93B000));
	TZC400 * pSGID_Ctl = ((TZC400           *)(MICOM_BASE_ADDR + 0x93C000));
	TZC400 * pNSAID_Ctl = ((TZC400           *)(MICOM_BASE_ADDR + 0x93D000));

	uint32 test_data;
	int32 ret = 0;
	int32 i;
	

	for (i=0; i<4; i++)
	{
		TZC400 * pFilterCtl = ((TZC400           *)(MICOM_BASE_ADDR + 0x93A000 + i*0x1000));
		// clock gating test
		// test register set
		pFilterCtl->uREGION_CTRL[1].nReg[0] = 0xC1000000;
		test_data = pFilterCtl->uREGION_CTRL[1].nReg[0];
		if ( test_data != 0xC1000000)
			midf_error = 0x00000001;

		// clock gating on
		pMcCfg->uBCLK_MASK0.nReg = (pMcCfg->uBCLK_MASK0.nReg  & ~(0x1<<((i*5)+7)));
		test_data = pFilterCtl->uREGION_CTRL[1].nReg[0];
		if ( test_data == 0xC1000000)
			midf_error = 0x00000001;

		// clock gating off
		pMcCfg->uBCLK_MASK0.nReg = (pMcCfg->uBCLK_MASK0.nReg  | (0x1<<((i*5)+7)));
		test_data = pFilterCtl->uREGION_CTRL[1].nReg[0];
		if ( test_data != 0xC1000000)
			midf_error = 0x00000001;

		// sw reset test
		// test register set
		pFilterCtl->uREGION_CTRL[1].nReg[0] = 0xC1000000;
		test_data = pFilterCtl->uREGION_CTRL[1].nReg[0];
		if ( test_data != 0xC1000000)
			midf_error = 0x00000001;

		// dma sw reset
		pMcCfg->uSW_RST_MASK0.nReg = (pMcCfg->uSW_RST_MASK0.nReg  & ~((i*5)+7));
		pMcCfg->uSW_RST_MASK0.nReg = (pMcCfg->uSW_RST_MASK0.nReg  | ((i*5)+7));

		// check reset value
		test_data = pFilterCtl->uREGION_CTRL[1].nReg[0];
		if ( test_data == 0xC1000000)
			midf_error = 0x00000001;

		// re-setting and checking
		pFilterCtl->uREGION_CTRL[1].nReg[0] = 0xC1000000;
		test_data = pFilterCtl->uREGION_CTRL[1].nReg[0];
		if ( test_data != 0xC1000000)
			midf_error = 0x00000001;

	}
	if ( midf_error !=0)
		ret = -1;

	return ret;
	#else
	
	return 0;
	
	#endif
}

static int32 MIDF_T02_GateKeeperTest(void)
{
	TZC400 * pIMC_MIDCtl=  ((TZC400           *)(MICOM_BASE_ADDR + 0x93B000));

	pIMC_MIDCtl->uGATE_KEEP.bReg.open_request = 0;
	return 0;
}

//--------------------------------------------------------------
// Wait DMA IRQ done
//--------------------------------------------------------------
static void MIDF_WaitIrqDone(void) 
{
    //gdma_irq_done = 0;

    while(1) {
        if(midf_irq_done == 1)
            break;
    }
}

static int32 MIDF_T03_FilterActionTest(void)
{
	#define MDIF_SRC_REGION	1
	#define MDIF_DST_REGION	2
	#define TEST_MEM_BASE	(SRAM1_BASE)
	#define TEST_REGION_SIZE (0x1000) //4//  4k 
	TZC400 * pIMC_MIDCtl=  ((TZC400           *)(MICOM_BASE_ADDR + 0x93B000));
	uint32 access;//uint32 attr, access, ch, top, found;
	int32 result = 0;
	uint32 i, j;
	uint8 type= 0;

	// prepare dma
	#ifdef COWORK_DMA
    (void)GIC_IntVectSet((uint32)GIC_DMA_PL080, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&GDMA_IPTEST_IsrHandler, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA_PL080);
	
	// dma_AHB_BUS_1           ,   //  uint32    src_bus         : AHB-1  => DAM0
       // dma_AHB_BUS_2           ,   //  uint32    dst_bus         : AHB-2  => DMA1
    (void)GIC_IntVectSet((uint32)GIC_IMC_MID_FILTER, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&MIDF_IMCIsrHandler, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_IMC_MID_FILTER);

	GDMA_IPTEST_TestConfigure();
	#endif
	
	MIDF_IMCTestConfig();

	// SRC : region[1] for 0xc1000000+0x1000 for DMA R/W
	// configure memory id filter
	access = 0x00000000;
	type = (MID_TYPE_READ|MID_TYPE_WRITE);
	if ((type&MID_TYPE_READ) != 0) 
	{
		access |= 1U<<(MID_ID_DMA0&0xF);
	}
	if ((type&MID_TYPE_WRITE) != 0) 
	{
		access |= 1U<<((MID_ID_DMA0&0xF) + 16);
	}

	pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x0;
	pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[0] = TEST_MEM_BASE; /*BASE LOW*/
	pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[2] = (TEST_MEM_BASE+(TEST_REGION_SIZE-1)); /*TOP LOW*/
	access |= (TZC400_ACCESS_DEFAULT);			
	pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[5] = access; /*ID ACCESS */
	pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x1;

	//DEST : region[2] for 0xc1001000+0x2000 for DMA R/W
	access = 0x00000000;
	type = (MID_TYPE_READ|MID_TYPE_WRITE);
	if ((type&MID_TYPE_READ) != 0) 
	{
		access |= 1U<<(MID_ID_DMA1&0xF);
	}
	if ((type&MID_TYPE_WRITE) != 0) 
	{
		access |= 1U<<((MID_ID_DMA1&0xF) + 16);
	}

	pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].bReg.filter0_en = 0x0;
	pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[0] = (TEST_MEM_BASE + TEST_REGION_SIZE); /*BASE LOW*/
	pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[2] = ((TEST_MEM_BASE+ TEST_REGION_SIZE) +(TEST_REGION_SIZE-1)); /*TOP LOW*/
	access |= (TZC400_ACCESS_DEFAULT);			
	pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[5] = access; /*ID ACCESS */
	pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].bReg.filter0_en = 0x1;

	// normal access test
	for(i =0; i < TEST_REGION_SIZE/4; i++)
	{
		*((uint32 *)(SRAM1_BASE) + i) =  i;
	}
	#ifdef COWORK_DMA
	GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SRAM1_BASE), 0x1, (SRAM1_BASE + TEST_REGION_SIZE), 0x1, (TEST_REGION_SIZE/4));
	gdma_irq_done= 0;
	GDMA_IPTEST_SetChannelEnable(0x0);
	GDMA_IPTEST_WaitIrqDone();
	#endif
	
	// check mem
	for(i =0; i < TEST_REGION_SIZE/4; i++)
	{
		if (*((uint32 *)(SRAM1_BASE + TEST_REGION_SIZE) + i) !=  i)
		{
			result = -1;
			break;
		}
	}
	// out of range access

	/*
Controls how the MID Filter uses error response and interrupt signals when a region permission failure occurs, excluding region overlap errors. The settings for these bits are:
0b00: Sets interrupt LOW and issues an OKAY response. ==> Not Generated Interrupt, indicated by INT_STATUS
0b01: Sets interrupt LOW and issues a DECERR response. ==> Not Generated Interrupt, indicated by INT_STATUS
0b10: Sets interrupt HIGH and issues an OKAY response. ==> Generated Interrupt
0b11: Sets interrupt HIGH and issues a DECERR response. ==> Generated Interrupt
	*/
	for( j= 0; j <4; j++)
	{
		pIMC_MIDCtl->uACTION.bReg.reaction_value = (j);
		
		// clear dest mem
		for(i =0; i < TEST_REGION_SIZE/4; i++)
		{
			*((uint32 *)(SRAM1_BASE + TEST_REGION_SIZE) + i) =  0;
		}
		#ifdef COWORK_DMA
		GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SRAM1_BASE), 0x1, (SRAM1_BASE + TEST_REGION_SIZE*2), 0x1, (TEST_REGION_SIZE/4));
		gdma_irq_done= 0;
		#endif
		midf_irq_done= 0;

		#ifdef COWORK_DMA
		GDMA_IPTEST_SetChannelEnable(0x0);
		#endif
		// expected overrun error
		if( j > 1)
			MIDF_WaitIrqDone();
		else
		{
			if ( pIMC_MIDCtl->uINT_STATUS.nReg !=0)
			{
				pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
			}
			else
			{
				result = -1;
			}
		}

		// test for not permitted attribute
		// r
		// change attribute of SRC region to write only
		// SRC : region[1] for 0xc1000000+0x1000 for DMA R/W
		// configure memory id filter
		access = 0x00000000;
		type = (MID_TYPE_READ|MID_TYPE_WRITE);
		if ((type&MID_TYPE_READ) != 0) 
		{
			//access |= 1U<<(MID_ID_DMA0&0xF);
		}
		if ((type&MID_TYPE_WRITE) != 0) 
		{
			access |= 1U<<((MID_ID_DMA0&0xF) + 16);
		}

		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x0;
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[0] = TEST_MEM_BASE; /*BASE LOW*/
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[2] = (TEST_MEM_BASE+(TEST_REGION_SIZE-1)); /*TOP LOW*/
		access |= (TZC400_ACCESS_DEFAULT);			
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[5] = access; /*ID ACCESS */
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x1;
		for(i =0; i < TEST_REGION_SIZE/4; i++)
		{
			*((uint32 *)(SRAM1_BASE + TEST_REGION_SIZE) + i) =  0;
		}
		#ifdef COWORK_DMA
		GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SRAM1_BASE), 0x1, (SRAM1_BASE + TEST_REGION_SIZE), 0x1, (TEST_REGION_SIZE/4));
		midf_irq_done= 0;

		GDMA_IPTEST_SetChannelEnable(0x0);
		#endif
		// expected overrun error
		if( j > 1)
			MIDF_WaitIrqDone();
		else
		{
			if ( pIMC_MIDCtl->uINT_STATUS.nReg !=0)
			{
				pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
			}
			else
			{
				result = -1;
			}
		}
		// w
		// set SRC R/W,  Set DEST R
		
		// SRC : region[1] for 0xc1000000+0x1000 for DMA R/W
		// configure memory id filter
		access = 0x00000000;
		type = (MID_TYPE_READ|MID_TYPE_WRITE);
		if ((type&MID_TYPE_READ) != 0) 
		{
			access |= 1U<<(MID_ID_DMA0&0xF);
		}
		if ((type&MID_TYPE_WRITE) != 0) 
		{
			access |= 1U<<((MID_ID_DMA0&0xF) + 16);
		}

		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x0;
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[0] = TEST_MEM_BASE; /*BASE LOW*/
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[2] = (TEST_MEM_BASE+(TEST_REGION_SIZE-1)); /*TOP LOW*/
		access |= (TZC400_ACCESS_DEFAULT);			
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[5] = access; /*ID ACCESS */
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x1;

		//DEST : region[2] for 0xc1001000+0x2000 for DMA R/W
		access = 0x00000000;
		type = (MID_TYPE_READ|MID_TYPE_WRITE);
		if ((type&MID_TYPE_READ) != 0) 
		{
			access |= 1U<<(MID_ID_DMA1&0xF);
		}
		if ((type&MID_TYPE_WRITE) != 0) 
		{
	//		access |= 1U<<((MID_ID_DMA1&0xF) + 16);
		}

		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].bReg.filter0_en = 0x0;
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[0] = (TEST_MEM_BASE + TEST_REGION_SIZE); /*BASE LOW*/
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[2] = ((TEST_MEM_BASE+ TEST_REGION_SIZE) +(TEST_REGION_SIZE-1)); /*TOP LOW*/
		access |= (TZC400_ACCESS_DEFAULT);			
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[5] = access; /*ID ACCESS */
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].bReg.filter0_en = 0x1;
		
		for(i =0; i < TEST_REGION_SIZE/4; i++)
		{
			*((uint32 *)(SRAM1_BASE + TEST_REGION_SIZE) + i) =  0;
		}
		#ifdef COWORK_DMA
		GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SRAM1_BASE), 0x1, (SRAM1_BASE + TEST_REGION_SIZE), 0x1, (TEST_REGION_SIZE/4));
		midf_irq_done= 0;
		GDMA_IPTEST_SetChannelEnable(0x0);
		#endif
		// expected overrun error
		if( j > 1)
			MIDF_WaitIrqDone();
		else
		{
			if ( pIMC_MIDCtl->uINT_STATUS.nReg !=0)
			{
				pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
			}
			else
			{
				result = -1;
			}
		}

		// check normal access again
		// SRC : region[1] for 0xc1000000+0x1000 for DMA R/W
		// configure memory id filter
		access = 0x00000000;
		type = (MID_TYPE_READ|MID_TYPE_WRITE);
		if ((type&MID_TYPE_READ) != 0) 
		{
			access |= 1U<<(MID_ID_DMA0&0xF);
		}
		if ((type&MID_TYPE_WRITE) != 0) 
		{
			access |= 1U<<((MID_ID_DMA0&0xF) + 16);
		}

		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x0;
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[0] = TEST_MEM_BASE; /*BASE LOW*/
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[2] = (TEST_MEM_BASE+(TEST_REGION_SIZE-1)); /*TOP LOW*/
		access |= (TZC400_ACCESS_DEFAULT);			
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].nReg[5] = access; /*ID ACCESS */
		pIMC_MIDCtl->uREGION_CTRL[MDIF_SRC_REGION].bReg.filter0_en = 0x1;

		//DEST : region[2] for 0xc1001000+0x2000 for DMA R/W
		access = 0x00000000;
		type = (MID_TYPE_READ|MID_TYPE_WRITE);
		if ((type&MID_TYPE_READ) != 0) 
		{
			access |= 1U<<(MID_ID_DMA1&0xF);
		}
		if ((type&MID_TYPE_WRITE) != 0) 
		{
			access |= 1U<<((MID_ID_DMA1&0xF) + 16);
		}

		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].bReg.filter0_en = 0x0;
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[0] = (TEST_MEM_BASE + TEST_REGION_SIZE); /*BASE LOW*/
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[2] = ((TEST_MEM_BASE+ TEST_REGION_SIZE) +(TEST_REGION_SIZE-1)); /*TOP LOW*/
		access |= (TZC400_ACCESS_DEFAULT);			
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].nReg[5] = access; /*ID ACCESS */
		pIMC_MIDCtl->uREGION_CTRL[MDIF_DST_REGION].bReg.filter0_en = 0x1;
		
		// normal access
		for(i =0; i < TEST_REGION_SIZE/4; i++)
		{
			*((uint32 *)(SRAM1_BASE) + i) =  i;
		}
		#ifdef COWORK_DMA
		GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SRAM1_BASE), 0x1, (SRAM1_BASE + TEST_REGION_SIZE), 0x1, (TEST_REGION_SIZE/4));
		gdma_irq_done= 0;
		GDMA_IPTEST_SetChannelEnable(0x0);
		GDMA_IPTEST_WaitIrqDone();
		#endif
		
		// check mem
		for(i =0; i < TEST_REGION_SIZE/4; i++)
		{
			if (*((uint32 *)(SRAM1_BASE + TEST_REGION_SIZE) + i) !=  i)
			{
				result = -1;
				break;
			}
		}
	}
	return result;
}

static int32 MIDF_T04_SFMCFiterActionTest(void)
{
	#define SFMC_MDIF_SRC_REGION	1
	#define SFMC_MDIF_DST_REGION	2
	#define SFMC_TEST_MEM_BASE	(0xd0000000)
	#define SFMC_TEST_DEST_MEM_BASE	(SRAM1_BASE)
	#define SFMC_TEST_REGION_SIZE (0x1000) //4//  4k 
	TZC400 * pIMC_MIDCtl=  ((TZC400           *)(HwMC_SFMC_MID_FIL));
	uint32 access;//uint32 attr, access, ch, top, found;
	int32 result = 0;
	int32 i, j;
	uint8 type= 0;

	#ifdef COWORK_DMA
	// prepare dma
    (void)GIC_IntVectSet((uint32)GIC_DMA_PL080, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&GDMA_IPTEST_IsrHandler, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_DMA_PL080);
	
	// dma_AHB_BUS_1           ,   //  uint32    src_bus         : AHB-1  => DAM0
       // dma_AHB_BUS_2           ,   //  uint32    dst_bus         : AHB-2  => DMA1
    (void)GIC_IntVectSet((uint32)GIC_SFMC_MID_FILTER, (uint32)GIC_PRIORITY_NO_MEAN,
                         (uint8)GIC_INT_TYPE_LEVEL_HIGH, (GICIsrFunc)&MIDF_SFMCIsrHandler, NULL_PTR);
    (void)GIC_IntSrcEn((uint32)GIC_SFMC_MID_FILTER);

	GDMA_IPTEST_TestConfigure();
	#endif
	
	MIDF_SFMCTestConfig();

	// SRC : region[1] for 0xc1000000+0x1000 for DMA R/W
	// configure memory id filter
	access = 0x00000000;
	type = (MID_TYPE_READ|MID_TYPE_WRITE);
	if ((type&MID_TYPE_READ) != 0) 
	{
		access |= 1U<<(MID_ID_DMA0&0xF);
	}
	if ((type&MID_TYPE_WRITE) != 0) 
	{
		access |= 1U<<((MID_ID_DMA0&0xF) + 16);
	}

	// SRC : SNOR => DEST : SRAM1
	pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].bReg.filter0_en = 0x0;
	pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].nReg[0] = SFMC_TEST_MEM_BASE; /*BASE LOW*/
	pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].nReg[2] = (SFMC_TEST_MEM_BASE+(SFMC_TEST_REGION_SIZE-1)); /*TOP LOW*/
	access |= (TZC400_ACCESS_DEFAULT);			
	pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].nReg[5] = access; /*ID ACCESS */
	pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].bReg.filter0_en = 0x1;
	#ifdef COWORK_DMA
	GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SFMC_TEST_MEM_BASE), 0x1, (SRAM1_BASE ), 0x1, (SFMC_TEST_REGION_SIZE/4));
	gdma_irq_done= 0;
	GDMA_IPTEST_SetChannelEnable(0x0);
	GDMA_IPTEST_WaitIrqDone();
	#endif
	// check mem
	for(i =0; i < SFMC_TEST_REGION_SIZE/4; i++)
	{
		if (*((uint32 *)(SRAM1_BASE) + i) !=  *((uint32 *)(SFMC_TEST_MEM_BASE) + i))
		{
			result = -1;
			break;
		}
	}
	// out of range access

	/*
Controls how the MID Filter uses error response and interrupt signals when a region permission failure occurs, excluding region overlap errors. The settings for these bits are:
0b00: Sets interrupt LOW and issues an OKAY response. ==> Not Generated Interrupt, indicated by INT_STATUS
0b01: Sets interrupt LOW and issues a DECERR response. ==> Not Generated Interrupt, indicated by INT_STATUS
0b10: Sets interrupt HIGH and issues an OKAY response. ==> Generated Interrupt
0b11: Sets interrupt HIGH and issues a DECERR response. ==> Generated Interrupt
	*/
	for( j= 0; j <4; j++)
	{
		pIMC_MIDCtl->uACTION.bReg.reaction_value = (j);
		#ifdef COWORK_DMA
		GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SFMC_TEST_MEM_BASE + SFMC_TEST_REGION_SIZE*2), 0x1, (SRAM1_BASE), 0x1, (SFMC_TEST_REGION_SIZE/4));

		gdma_irq_done= 0;
		midf_irq_done= 0;
		
		GDMA_IPTEST_SetChannelEnable(0x0);
		#endif
		// expected overrun error
		if( j > 1)
			MIDF_WaitIrqDone();
		else
		{
			if ( pIMC_MIDCtl->uINT_STATUS.nReg !=0)
			{
				pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
			}
			else
			{
				result = -1;
			}
		}

		// test for not permitted attribute
		// r
		// change attribute of SRC region to write only
		// SRC : region[1] for 0xc1000000+0x1000 for DMA R/W
		// configure memory id filter
		access = 0x00000000;
		type = (MID_TYPE_READ|MID_TYPE_WRITE);
		if ((type&MID_TYPE_READ) != 0) 
		{
			//access |= 1U<<(MID_ID_DMA0&0xF);
		}
		if ((type&MID_TYPE_WRITE) != 0) 
		{
			access |= 1U<<((MID_ID_DMA0&0xF) + 16);
		}

		pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].bReg.filter0_en = 0x0;
		pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].nReg[0] = SFMC_TEST_MEM_BASE; /*BASE LOW*/
		pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].nReg[2] = (SFMC_TEST_MEM_BASE+(SFMC_TEST_REGION_SIZE-1)); /*TOP LOW*/
		access |= (TZC400_ACCESS_DEFAULT);			
		pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].nReg[5] = access; /*ID ACCESS */
		pIMC_MIDCtl->uREGION_CTRL[SFMC_MDIF_SRC_REGION].bReg.filter0_en = 0x1;
		for(i =0; i < SFMC_TEST_REGION_SIZE/4; i++)
		{
			*((uint32 *)(SRAM1_BASE + SFMC_TEST_REGION_SIZE) + i) =  0;
		}
		#ifdef COWORK_DMA
		GDMA_IPTEST_M2MNormalTransferTest   (0x0, (SFMC_TEST_MEM_BASE), 0x1, (SRAM1_BASE), 0x1, (SFMC_TEST_REGION_SIZE/4));
		
		midf_irq_done= 0;

		GDMA_IPTEST_SetChannelEnable(0x0);
		#endif
		// expected overrun error
		if( j > 1)
			MIDF_WaitIrqDone();
		else
		{
			if ( pIMC_MIDCtl->uINT_STATUS.nReg !=0)
			{
				pIMC_MIDCtl->uINT_CLEAR.nReg = pIMC_MIDCtl->uINT_STATUS.nReg;
			}
			else
			{
				result = -1;
			}
		}
	}

	return result;
}


static int32 MIDF_ConfigSoftFaultCheckSafetyMechanism_FaultInjectTest
(
    uint32 uiSvlLevel,
    uint32 uiFaultCheckGroup
)
{
	   MC_MID * pMIDCfg = (MC_MID *)HwMC_MID;
   		int32 uiLoop;
     // write password to modify configuration
     pMIDCfg->CFG_WR_PW = 0x5AFEACE5UL;

     pMIDCfg->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_TEST_EN = 1;
     
     pMIDCfg->uMID_XS.nReg = ~pMIDCfg->uMID_XS.nReg;

				for (uiLoop =0; uiLoop<50000; uiLoop++)
				{
           if(midf_irq_done == 1)
               break;;
				}
       return 0;
}
#endif
