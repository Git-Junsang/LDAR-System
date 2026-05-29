/*
***************************************************************************************************
*
*   FileName : gpio_test.c
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

#include <gpio_test.h>
#include <debug.h>
#include <gpio.h>
#include <gic.h>
#include <fmu.h>
#include <reg_phys.h>
#include <bsp.h>

/*
***************************************************************************************************
*                                             DEFINITIONS
***************************************************************************************************
*/
#define GPIO_OFFSET_DAT                 (0x000UL)
#define GPIO_OFFSET_OEN                 (0x004UL)
#define GPIO_OFFSET_SET                 (0x008UL)
#define GPIO_OFFSET_CLR                 (0x00cUL)
#define GPIO_OFFSET_XOR                 (0x010UL)
#define GPIO_OFFSET_DS0                 (0x014UL)
#define GPIO_OFFSET_DS1                 (0x018UL)
#define GPIO_OFFSET_PE                  (0x01cUL)
#define GPIO_OFFSET_PS                  (0x020UL)
#define GPIO_OFFSET_IEN                 (0x024UL)
#define GPIO_OFFSET_IS                  (0x028UL)
#define GPIO_OFFSET_SR                  (0x02cUL)
#define GPIO_OFFSET_FNC0                (0x030UL)
#define GPIO_OFFSET_FNC1                (0x034UL)
#define GPIO_OFFSET_FNC2                (0x038UL)
#define GPIO_OFFSET_FNC3                (0x03cUL)
#define GPIO_OFFSET_EIS                 (0x280UL) /* External Interrupt Selection register             */
#define GPIO_OFFSET_ECS                 (0x2B0UL) /* External Clock Selection register                 */
#define GPIO_OFFSET_FLE                 (0x300UL) /* Function Configuration Lock Enable register       */
#define GPIO_OFFSET_OLE                 (0x380UL) /* Output Enable Configuration Lock Enable register  */
#define GPIO_OFFSET_VIR                 (0x400UL)

#define GPIO_CFG_WR_PW                  (0x8C0UL)
#define GPIO_CFG_WR_LOCK                (0x8C4UL)
#define GPIO_SOFT_FAULT_EN              (0xD00UL)
#define GPIO_SF_CTRL_CFG                (0xF00UL)
#define GPIO_SF_CTRL_STS                (0xF04UL)
#define GPIO_SF_STS                     ((SALReg32 *)0x1b935f04UL)

#define HwHCLK_MASK0                    ((SALReg32 *)0x1b930000UL)
#define HwHCLK_MASK1                    ((SALReg32 *)0x1b930004UL)
#define HwSW_RESET0                     ((SALReg32 *)0x1b93000cUL)
#define HwSW_RESET1                     ((SALReg32 *)0x1b930010UL)

//#define TCC_MICOM_GPIO_BASE             0x1B935000UL

#define GPIO_A_MASK                     (0xffffffffUL)
#define GPIO_B_MASK                     (0x1fffffffUL)
#define GPIO_C_MASK                     (0x3fffffffUL)
#define GPIO_E_MASK                     (0x000fffffUL)
#define GPIO_G_MASK                     (0x000007ffUL)
#define GPIO_H_MASK                     (0x00000fffUL)
#define GPIO_K_MASK                     (0x0007ffffUL)
#define GPIO_MA_MASK                    (0x3fffffffUL)
#define GPIO_MB_MASK                    (0xffffffffUL)
#define GPIO_MC_MASK                    (0x3fffffffUL)
    //#define GPIO_MD_MASK                  0x0003ffffUL
#define GPIO_MD_MASK                    (0x3fffffffUL)

#define GPIO_MAX_NTH_IRQ                (10)//5

#define GPIO_ACTIV_HIGH                 (0)
#define GPIO_ACTIV_LOW                  (1)

#define GPIO_VI2O_OEN                   (0x00U)
#define GPIO_VI2I_DAT                   (0x04U)
#define GPIO_VI2O_EN                    (0x08U)
#define GPIO_VI2I_EN                    (0x0CU)
#define GPIO_MON_DO                     (0x10U)
#define GPIO_MON_OEN                    (0x14U)

#define GPIO_MAX_NUM                    (11)
#define GPIO_TEMP_NUM                   (8) //except GPIOH, GPIOMA

#define GPIO_CMD_NAME                   ("gpio")

#define GPIO_WriteReg(a,v)              (SAL_WriteReg(v,a))
#define GPIO_ReadReg(a)                 (SAL_ReadReg(a))

#ifdef TRACE32_TEST 


static uint32 nof_gpio[8]              = {32, /* orig:32, GPIO-A  bit-width */
                                        29,            // GPIO-B  bit-width
                                        30,            // GPIO-C  bit-width
                                        20,            // GPIO-E  bit-width
                                        11,            // GPIO-G  bit-width
                                        //12,          // GPIO-H  bit-width
                                        //19,          // GPIO-K  bit-width
                                        //30,            // GPIO-MA bit-width
                                        32,            // GPIO-MB bit-width
                                        30,            // GPIO-MC bit-width
                                        18};           // GPIO-MD bit-width


static uint32 gpio_port[8]             = { 0, /* orig:32, GPIO-A  bit-width */
                                        32,            // GPIO-B  bit-width
                                        61,            // GPIO-C  bit-width
                                        91,            // GPIO-E  bit-width
                                        111,           // GPIO-G  bit-width
                                        //122,         // GPIO-H  bit-width
                                        //134,         // GPIO-K  bit-width
                                        //134,           // GPIO-MA bit-width
                                        164,           // GPIO-MB bit-width
                                        196,           // GPIO-MC bit-width
                                        226};          // GPIO-MD bit-width
#endif

static uint32 reset_bit[8]             = { 0xffffffffUL,       // GPIO-A
                                        0x1fffffffUL,           // GPIO-B
                                        0x3fffffffUL,           // GPIO-C
                                        0x000fffffUL,           // GPIO-E
                                        0x000007ffUL,           // GPIO-G
                                        //0x00000fffUL,         // GPIO-H
                                        //0x0007ffffUL,         // GPIO-K
                                        //0x3fffffffUL,           // GPIO-MA
                                        0xffffffffUL,           // GPIO-MB
                                        0x3fffffffUL,           // GPIO-MC
                                        0x0003ffffUL};          // GPIO-MD  0x03000000

static uint32 addr_gpio[8]             = { 0x1b935000UL, /* orig:0x1b935000, GPIO-A  base address */
                                        0x1b935040UL,                     // GPIO-B  base address
                                        0x1b935080UL,                     // GPIO-C  base address
                                        0x1b935100UL,                     // GPIO-E  base address
                                        0x1b935180UL,                     // GPIO-G  base address
                                        //0x1b935640UL,                   // GPIO-H  base address
                                        //0x1b935680UL,                   // GPIO-K  base address
                                        //0x1b9356c0UL,                     // GPIO-MA base address
                                        0x1b935700UL,                     // GPIO-MB base address
                                        0x1b935740UL,                     // GPIO-MC base address
                                        0x1b935780UL};                    // GPIO-MD base address




#define GPIO_PASSWORD                   (0x5afeace5UL)


static uint32 wr_reg[378]               = {0, 0xffffffffUL,   // 0x00_0         REG_GPA_DAT           000
                                          1, 0xffffffffUL,    // 0x00_4         REG_GPA_OEN           001
                                          5, 0xffffffffUL,    // 0x01_4         REG_GPA_DS0           002
                                          6, 0xffffffffUL,    // 0x01_8         REG_GPA_DS1           003
                                          7, 0xffffffffUL,    // 0x01_c         REG_GPA_PE            004
                                          8, 0xffffffffUL,    // 0x02_0         REG_GPA_PS            005
                                          9, 0xffffffffUL,    // 0x02_4         REG_GPA_IEN           006
                                         10, 0xffffffffUL,    // 0x02_8         REG_GPA_IS            007
                                         11, 0xffffffffUL,    // 0x02_c         REG_GPA_SR            008
                                         12, 0xffffffffUL,    // 0x03_0         REG_GPA_FNC0          009
                                         13, 0xffffffffUL,    // 0x03_4         REG_GPA_FNC1          010
                                         14, 0xffffffffUL,    // 0x03_8         REG_GPA_FNC2          011
                                         15, 0xffffffffUL,    // 0x03_c         REG_GPA_FNC3          012

                                         16, 0x1fffffffUL,    // 0x04_0         REG_GPB_DAT           013
                                         17, 0x1fffffffUL,    // 0x04_4         REG_GPB_OEN           014
                                         21, 0xffffffffUL,    // 0x05_4         REG_GPB_DS0           015
                                         22, 0x03ffffffUL,    // 0x05_8         REG_GPB_DS1           016
                                         23, 0x1fffffffUL,    // 0x05_c         REG_GPB_PE            017
                                         24, 0x1fffffffUL,    // 0x06_0         REG_GPB_PS            018
                                         25, 0x1fffffffUL,    // 0x06_4         REG_GPB_IEN           019
                                         26, 0x1fffffffUL,    // 0x06_8         REG_GPB_IS            020
                                         27, 0x1fffffffUL,    // 0x06_c         REG_GPB_SR            021
                                         28, 0xffffffffUL,    // 0x07_0         REG_GPB_FNC0          022
                                         29, 0xffffffffUL,    // 0x07_4         REG_GPB_FNC1          023
                                         30, 0xffffffffUL,    // 0x07_8         REG_GPB_FNC2          024
                                         31, 0x000fffffUL,    // 0x07_c         REG_GPB_FNC3          025

                                         32, 0x3fffffffUL,    // 0x08_0         REG_GPC_DAT           026
                                         33, 0x3fffffffUL,    // 0x08_4         REG_GPC_OEN           027
                                         37, 0xffffffffUL,    // 0x09_4         REG_GPC_DS0           028
                                         38, 0x0fffffffUL,    // 0x09_8         REG_GPC_DS1           029
                                         39, 0x3fffffffUL,    // 0x09_c         REG_GPC_PE            030
                                         40, 0x3fffffffUL,    // 0x0a_0         REG_GPC_PS            031
                                         41, 0x3fffffffUL,    // 0x0a_4         REG_GPC_IEN           032
                                         42, 0x3fffffffUL,    // 0x0a_8         REG_GPC_IS            033
                                         43, 0x3fffffffUL,    // 0x0a_c         REG_GPC_SR            034
                                         44, 0xffffffffUL,    // 0x0b_0         REG_GPC_FNC0          035
                                         45, 0xffffffffUL,    // 0x0b_4         REG_GPC_FNC1          036
                                         46, 0xffffffffUL,    // 0x0b_8         REG_GPC_FNC2          037
                                         47, 0x00ffffffUL,    // 0x0b_c         REG_GPC_FNC3          038

                                         64, 0x000fffffUL,    // 0x10_0         REG_GPE_DAT           039
                                         65, 0x000fffffUL,    // 0x10_4         REG_GPE_OEN           040
                                         69, 0xffffffffUL,    // 0x11_4         REG_GPE_DS0           041
                                         70, 0x000000ffUL,    // 0x11_8         REG_GPE_DS1           042
                                         71, 0x000fffffUL,    // 0x11_c         REG_GPE_PE            043
                                         72, 0x000fffffUL,    // 0x12_0         REG_GPE_PS            044
                                         73, 0x000fffffUL,    // 0x12_4         REG_GPE_IEN           045
                                         74, 0x000fffffUL,    // 0x12_8         REG_GPE_IS            046
                                         75, 0x000fffffUL,    // 0x12_c         REG_GPE_SR            047
                                         76, 0xffffffffUL,    // 0x13_0         REG_GPE_FNC0          048
                                         77, 0xffffffffUL,    // 0x13_4         REG_GPE_FNC1          049
                                         78, 0x0000ffffUL,    // 0x13_8         REG_GPE_FNC2          050

                                         96, 0x000007ffUL,    // 0x18_0         REG_GPG_DAT           051
                                         97, 0x000007ffUL,    // 0x18_4         REG_GPG_OEN           052
                                        101, 0x003fffffUL,    // 0x19_4         REG_GPG_DS0           053
                                        103, 0x000007ffUL,    // 0x19_c         REG_GPG_PE            054
                                        104, 0x000007ffUL,    // 0x1a_0         REG_GPG_PS            055
                                        105, 0x000007ffUL,    // 0x1a_4         REG_GPG_IEN           056
                                        106, 0x000007ffUL,    // 0x1a_8         REG_GPG_IS            057
                                        107, 0x000007ffUL,    // 0x1a_c         REG_GPG_SR            058
                                        108, 0xffffffffUL,    // 0x1b_0         REG_GPG_FNC0          059
                                        109, 0x00000fffUL,    // 0x1b_4         REG_GPG_FNC1          060

                                        //400, 0x00000fffUL,  // 0x64_0         REG_GPH_DAT           061
                                        401, 0x00000fffUL,    // 0x64_4         REG_GPH_OEN           062
                                        405, 0x00ffffffUL,    // 0x65_4         REG_GPH_DS0           063
                                        407, 0x00000fffUL,    // 0x65_c         REG_GPH_PE            064
                                        408, 0x00000fffUL,    // 0x66_0         REG_GPH_PS            065
                                        //409, 0x00000fffUL,  // 0x66_4         REG_GPH_IEN           066
                                        410, 0x00000fffUL,    // 0x66_8         REG_GPH_IS            067
                                        411, 0x00000fffUL,    // 0x66_c         REG_GPH_SR            068
                                        //412, 0xffffffffUL,  // 0x67_0         REG_GPH_FNC0          069
                                        //413, 0x0000ffffUL,  // 0x67_4         REG_GPH_FNC1          070

                                        416, 0x0007ffffUL,    // 0x68_0         REG_GPK_DAT           071
                                        417, 0x0007ffffUL,    // 0x68_4         REG_GPK_OEN           072
                                        //428, 0xffffffffUL,  // 0x6b_0         REG_GPK_FNC0          073
                                        //429, 0xffffffffUL,  // 0x6b_4         REG_GPK_FNC1          074
                                        //430, 0x00000fffUL,  // 0x6b_8         REG_GPK_FNC2          075

                                        //432, 0x3fffffffUL,  // 0x6c_0         REG_GPMA_DAT          076
                                        //433, 0x3fffffffUL,  // 0x6c_4         REG_GPMA_OEN          077
                                        437, 0xffffffffUL,    // 0x6d_4         REG_GPMA_DS0          078
                                        438, 0x0fffffffUL,    // 0x6d_8         REG_GPMA_DS1          079
                                        439, 0x3fffffffUL,    // 0x6d_c         REG_GPMA_PE           080
                                        440, 0x3fffffffUL,    // 0x6e_0         REG_GPMA_PS           081
                                        //441, 0x3fffffffUL,  // 0x6e_4         REG_GPMA_IEN          082
                                        442, 0x3fffffffUL,    // 0x6e_8         REG_GPMA_IS           083
                                        443, 0x3fffffffUL,    // 0x6e_c         REG_GPMA_SR           084
                                        //444, 0xffffffffUL,  // 0x6f_0         REG_GPMA_FNC0         085
                                        //445, 0xffffffffUL,  // 0x6f_4         REG_GPMA_FNC1         086
                                        //446, 0xffffffffUL,  // 0x6f_8         REG_GPMA_FNC2         087
                                        //447, 0x00ffffffUL,  // 0x6f_c         REG_GPMA_FNC3         088

                                        448, 0xffffffffUL,    // 0x70_0         REG_GPMB_DAT          089
                                        449, 0xffffffffUL,    // 0x70_4         REG_GPMB_OEN          090
                                        453, 0xffffffffUL,    // 0x71_4         REG_GPMB_DS0          091
                                        454, 0xffffffffUL,    // 0x71_8         REG_GPMB_DS1          092
                                        455, 0xffffffffUL,    // 0x71_c         REG_GPMB_PE           093
                                        456, 0xffffffffUL,    // 0x72_0         REG_GPMB_PS           094
                                        457, 0xffffffffUL,    // 0x72_4         REG_GPMB_IEN          095
                                        458, 0xffffffffUL,    // 0x72_8         REG_GPMB_IS           096
                                        459, 0xffffffffUL,    // 0x72_c         REG_GPMB_SR           097
                                        460, 0xffffffffUL,    // 0x73_0         REG_GPMB_FNC0         098
                                        461, 0xffffffffUL,    // 0x73_4         REG_GPMB_FNC1         099
                                        462, 0xffffffffUL,    // 0x73_8         REG_GPMB_FNC2         100
                                        463, 0xffffffffUL,    // 0x73_c         REG_GPMB_FNC3         101

                                        464, 0x3fffffffUL,    // 0x74_0         REG_GPMC_DAT          102
                                        465, 0x3fffffffUL,    // 0x74_4         REG_GPMC_OEN          103
                                        469, 0xffffffffUL,    // 0x75_4         REG_GPMC_DS0          104
                                        470, 0x0fffffffUL,    // 0x75_8         REG_GPMC_DS1          105
                                        471, 0x3fffffffUL,    // 0x75_c         REG_GPMC_PE           106
                                        472, 0x3fffffffUL,    // 0x76_0         REG_GPMC_PS           107
                                        473, 0x3fffffffUL,    // 0x76_4         REG_GPMC_IEN          108
                                        474, 0x3fffffffUL,    // 0x76_8         REG_GPMC_IS           109
                                        475, 0x3fffffffUL,    // 0x76_c         REG_GPMC_SR           110
                                        476, 0xffffffffUL,    // 0x77_0         REG_GPMC_FNC0         111
                                        477, 0xffffffffUL,    // 0x77_4         REG_GPMC_FNC1         112
                                        478, 0xffffffffUL,    // 0x77_8         REG_GPMC_FNC2         113
                                        479, 0x00ffffffUL,    // 0x77_c         REG_GPMC_FNC3         114

                                        480, 0x0003ffffUL,    // 0x78_0         REG_GPMD_DAT          115     //0xffff
                                        481, 0x0003ffffUL,    // 0x78_4         REG_GPMD_OEN          116     //0xffff
                                        485, 0xffffffffUL,    // 0x79_4         REG_GPMD_DS0          117
                                        486, 0x0000000fUL,    // 0x79_8         REG_GPMD_DS1          118
                                        487, 0x0003ffffUL,    // 0x79_c         REG_GPMD_PE           119     //0xffff
                                        488, 0x0003ffffUL,    // 0x7a_0         REG_GPMD_PS           120     //0xffff
                                        489, 0x0003ffffUL,    // 0x7a_4         REG_GPMD_IEN          121     //0xffff
                                        490, 0x0003ffffUL,    // 0x7a_8         REG_GPMD_IS           122     //0xffff
                                        491, 0x0003ffffUL,    // 0x7a_c         REG_GPMD_SR           123     //0xffff
                                        492, 0xffffffffUL,    // 0x7b_0         REG_GPMD_FNC0         124
                                        493, 0xffffffffUL,    // 0x7b_4         REG_GPMD_FNC1         125
                                        494, 0x000000ffUL,    // 0x7b_8         REG_GPMD_FNC2         126
                                        

                                        160, 0xffffffffUL,    // 0x28_0         REG_EINT_CFG0         127
                                        161, 0xffffffffUL,    // 0x28_4         REG_EINT_CFG1         128
                                        162, 0x0000ffffUL,    // 0x28_8         REG_EINT_CFG2         129

                                        172, 0xffffffffUL,    // 0x2B_0         REG_EXTCLK_CFG        130

                                        192, 0xffffffffUL,    // 0x30_0         REG_GPA_FNC_LOCK      131
                                        193, 0x1fffffffUL,    // 0x30_4         REG_GPB_FNC_LOCK      132
                                        194, 0x3fffffffUL,    // 0x30_8         REG_GPC_FNC_LOCK      133
                                        196, 0x000fffffUL,    // 0x31_0         REG_GPE_FNC_LOCK      134
                                        198, 0x000007ffUL,    // 0x31_8         REG_GPG_FNC_LOCK      135
                                        203, 0x00000fffUL,    // 0x32_c         REG_GPH_FNC_LOCK      136
                                        204, 0x0007ffffUL,    // 0x33_0         REG_GPK_FNC_LOCK      137
                                        205, 0x3fffffffUL,    // 0x33_4         REG_GPMA_FNC_LOCK     138
                                        206, 0xffffffffUL,    // 0x33_8         REG_GPMB_FNC_LOCK     139
                                        207, 0x3fffffffUL,    // 0x33_c         REG_GPMC_FNC_LOCK     140
                                        208, 0x0000ffffUL,    // 0x34_0         REG_GPMD_FNC_LOCK     141

                                        224, 0xffffffffUL,    // 0x38_0         REG_GPA_OEN_LOCKn     142
                                        225, 0x1fffffffUL,    // 0x38_4         REG_GPB_OEN_LOCKn     143
                                        226, 0x3fffffffUL,    // 0x38_8         REG_GPC_OEN_LOCKn     144
                                        228, 0x000fffffUL,    // 0x39_0         REG_GPE_OEN_LOCKn     145
                                        230, 0x000007ffUL,    // 0x39_8         REG_GPG_OEN_LOCKn     146
                                        235, 0x00000fffUL,    // 0x3a_c         REG_GPH_OEN_LOCKn     147
                                        236, 0x0007ffffUL,    // 0x3b_0         REG_GPK_OEN_LOCKn     148
                                        237, 0x3fffffffUL,    // 0x3b_4         REG_GPMA_OEN_LOCKn    149
                                        238, 0xffffffffUL,    // 0x3b_8         REG_GPMB_OEN_LOCKn    150
                                        239, 0x3fffffffUL,    // 0x3b_c         REG_GPMC_OEN_LOCKn    151
                                        240, 0x0000ffffUL,    // 0x3c_0         REG_GPMD_OEN_LOCKn    152

                                        256, 0xffffffffUL,    // 0x40_0         REG_GPA_VI2O_OEN      153
                                        257, 0xffffffffUL,    // 0x40_4         REG_GPA_VI2I_DAT      154
                                        258, 0xffffffffUL,    // 0x40_8         REG_GPA_VI2O_EN       155
                                        259, 0xffffffffUL,    // 0x40_c         REG_GPA_VI2I_EN       156

                                        264, 0x1fffffffUL,    // 0x42_0         REG_GPB_VI2O_OEN      157
                                        265, 0x1fffffffUL,    // 0x42_4         REG_GPB_VI2I_DAT      158
                                        266, 0x1fffffffUL,    // 0x42_8         REG_GPB_VI2O_EN       159
                                        267, 0x1fffffffUL,    // 0x42_c         REG_GPB_VI2I_EN       160

                                        272, 0x3fffffffUL,    // 0x44_0         REG_GPC_VI2O_OEN      161
                                        273, 0x3fffffffUL,    // 0x44_4         REG_GPC_VI2I_DAT      162
                                        274, 0x3fffffffUL,    // 0x44_8         REG_GPC_VI2O_EN       163
                                        275, 0x3fffffffUL,    // 0x44_c         REG_GPC_VI2I_EN       164

                                        288, 0x000fffffUL,    // 0x48_0         REG_GPE_VI2O_OEN      165
                                        289, 0x000fffffUL,    // 0x48_4         REG_GPE_VI2I_DAT      166
                                        290, 0x000fffffUL,    // 0x48_8         REG_GPE_VI2O_EN       167
                                        291, 0x000fffffUL,    // 0x48_c         REG_GPE_VI2I_EN       168

                                        304, 0x000007ffUL,    // 0x4c_0         REG_GPG_VI2O_OEN      169
                                        305, 0x000007ffUL,    // 0x4c_4         REG_GPG_VI2I_DAT      170
                                        306, 0x000007ffUL,    // 0x4c_8         REG_GPG_VI2O_EN       171
                                        307, 0x000007ffUL,    // 0x4c_c         REG_GPG_VI2I_EN       172

                                        344, 0x00000fffUL,    // 0x56_0         REG_GPH_VI2O_OEN      173
                                        345, 0x00000fffUL,    // 0x56_4         REG_GPH_VI2I_DAT      174
                                        //346, 0x00000fffUL,  // 0x56_8         REG_GPH_VI2O_EN       175
                                        //347, 0x00000fffUL,  // 0x56_c         REG_GPH_VI2I_EN       176

                                        352, 0x0007ffffUL,    // 0x58_0         REG_GPK_VI2O_OEN      177
                                        353, 0x0007ffffUL,    // 0x58_4         REG_GPK_VI2I_DAT      178
                                        354, 0x0007ffffUL,    // 0x58_8         REG_GPK_VI2O_EN       179
                                        355, 0x0007ffffUL,    // 0x58_c         REG_GPK_VI2I_EN       180

                                        360, 0x3fffffffUL,    // 0x5a_0         REG_GPMA_VI2O_OEN     181
                                        361, 0x3fffffffUL,    // 0x5a_4         REG_GPMA_VI2I_DAT     182
                                        //362, 0x3fffffffUL,  // 0x5a_8         REG_GPMA_VI2O_EN      183
                                        //363, 0x3fffffffUL,  // 0x5a_c         REG_GPMA_VI2I_EN      184

                                        368, 0xffffffffUL,    // 0x5c_0         REG_GPMB_VI2O_OEN     185
                                        369, 0xffffffffUL,    // 0x5c_4         REG_GPMB_VI2I_DAT     186
                                        370, 0xffffffffUL,    // 0x5c_8         REG_GPMB_VI2O_EN      187
                                        371, 0xffffffffUL,    // 0x5c_c         REG_GPMB_VI2I_EN      188

                                        376, 0x3fffffffUL,    // 0x5e_0         REG_GPMC_VI2O_OEN     189
                                        377, 0x3fffffffUL,    // 0x5e_4         REG_GPMC_VI2I_DAT     190
                                        378, 0x3fffffffUL,    // 0x5e_8         REG_GPMC_VI2O_EN      191
                                        379, 0x3fffffffUL,    // 0x5e_c         REG_GPMC_VI2I_EN      192

                                        384, 0x0003ffffUL,    // 0x60_0         REG_GPMD_VI2O_OEN     193     0xffff
                                        385, 0x0003ffffUL,    // 0x60_4         REG_GPMD_VI2I_DAT     194     0xffff
                                        386, 0x0003ffffUL,    // 0x60_8         REG_GPMD_VI2O_EN      195     0xffff
                                        387, 0x0003ffffUL,    // 0x60_c         REG_GPMD_VI2I_EN      196     0xffff

                                        512, 0x07ffffffUL,    // 0x80_0         REG_EXT_IRQ0_FIL      197
                                        513, 0x07ffffffUL,    // 0x80_4         REG_EXT_IRQ1_FIL      198
                                        514, 0x07ffffffUL,    // 0x80_8         REG_EXT_IRQ2_FIL      199
                                        515, 0x07ffffffUL,    // 0x80_c         REG_EXT_IRQ3_FIL      200
                                        516, 0x07ffffffUL,    // 0x81_0         REG_EXT_IRQ4_FIL      201
                                        517, 0x07ffffffUL,    // 0x81_4         REG_EXT_IRQ5_FIL      202
                                        518, 0x07ffffffUL,    // 0x81_8         REG_EXT_IRQ6_FIL      203
                                        519, 0x07ffffffUL,    // 0x81_c         REG_EXT_IRQ7_FIL      204
                                        520, 0x07ffffffUL,    // 0x82_0         REG_EXT_IRQ8_FIL      205
                                        521, 0x07ffffffUL};   // 0x82_4         REG_EXT_IRQ9_FIL      206


static uint32 fault_index;



#ifdef TRACE32_TEST 


//uint32  fault_inj_test;
static uint32 gpio_irq_done;

static uint32 tpg_reg[8]                = {0x1b935400UL,    //GPIO_A
                                        0x1b935420UL,       //GPIO_B
                                        0x1b935440UL,       //GPIO_C
                                        0x1b935480UL,       //GPIO_E
                                        0x1b9354C0UL,       //GPIO_G
                                        //0x1b935560UL,     //GPIO_H
                                        //0x1b935580UL,       //GPIO_K
                                        //0x1b9355A0UL,       //GPIO_MA
                                        0x1b9355C0UL,       //GPIO_MB
                                        0x1b9355E0UL,       //GPIO_MC
                                        0x1b935600UL};      //GPIO_MD
static uint32 din_reg[8]                = {0x1b935840UL,    //GPIO_A
                                        0x1b935844UL,       //GPIO_B
                                        0x1b935848UL,       //GPIO_C
                                        0x1b935850UL,       //GPIO_E
                                        0x1b935858UL,       //GPIO_G
                                        //0x1b93586CUL,     //GPIO_H
                                        //0x1b935870UL,       //GPIO_K
                                        //0x1b935874UL,       //GPIO_MA
                                        0x1b935878UL,       //GPIO_MB
                                        0x1b93587CUL,       //GPIO_MC
                                        0x1b935880UL};      //GPIO_MD


static uint32 wait_irq_p[10]            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint32 wait_irq_n[10]            = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/*
***************************************************************************************************
*                                         FUNCTION PROTOTYPES
***************************************************************************************************
*/



static void GpioTestCommandList
(
    uint8                               ucArgc,
    const void * const                  pArgv[]
);


static void ext_irq_p0_isr
(
    void *                              arg
);

static void ext_irq_p1_isr
(
    void *                              arg
);

static void ext_irq_p2_isr
(
    void *                              arg
);

static void ext_irq_p3_isr
(
    void *                              arg
);

static void ext_irq_p4_isr
(
    void *                              arg
);

static void ext_irq_p5_isr
(
    void *                              arg
);

static void ext_irq_p6_isr
(
    void *                              arg
);

static void ext_irq_p7_isr
(
    void *                              arg
);

static void ext_irq_p8_isr
(
    void *                              arg
);

static void ext_irq_p9_isr
(
    void *                              arg
);

static void ext_irq_n0_isr
(
    void *                              arg
);

static void ext_irq_n1_isr
(
    void *                              arg
);

static void ext_irq_n2_isr
(
    void *                              arg
);

static void ext_irq_n3_isr
(
    void *                              arg
);

static void ext_irq_n4_isr
(
    void *                              arg
);

static void ext_irq_n5_isr
(
    void *                              arg
);

static void ext_irq_n6_isr
(
    void *                              arg
);

static void ext_irq_n7_isr
(
    void *                              arg
);

static void ext_irq_n8_isr
(
    void *                              arg
);

static void ext_irq_n9_isr
(
    void *                              arg
);

static void wait_irq_done
(
    void
);

static FMUErrTypes_t open_irq_p
(
    uint32                              nth_irq,
    uint32                              port,
    uint32                              i
);

static FMUErrTypes_t open_irq_n
(
    uint32                              uiNnthIrq,
    uint32                              uiPort,
    uint32                              iIndex
);

static void close_irq_p
(
    uint32                              uiNnthIrq
);

static void close_irq_n
(
    uint32                              uiNnthIrq
);

static void ext_irq_filter_cfg
(
    uint32                              uiNth,
    uint32                              uiPolarity,
    uint32                              uiDepth
);

static void ext_irq_filter_en
(
    uint32                              uiNth
);

static void ext_irq_filter_clr
(
    uint32                              uiNth
);

static void ext_irq_sel_p
(
    uint32                              uiPort
);


static void ext_irq_sel_n
(
    uint32                              uiPort
);


/*
***************************************************************************************************
*                                         FUNCTIONS - SUB
***************************************************************************************************
*/


void wait_irq_done(void)
{
    while (1)
    {
        if (gpio_irq_done == 1UL)
        {
            break;
        }
    }
}


void ext_irq_p0_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P0");
    wait_irq_p[0] = 0;
}

void ext_irq_p1_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P1");
    wait_irq_p[1] = 0;
}

void ext_irq_p2_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P2");
    wait_irq_p[2] = 0;
}

void ext_irq_p3_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P3");
    wait_irq_p[3] = 0;
}

void ext_irq_p4_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P4");
    wait_irq_p[4] = 0;
}

void ext_irq_p5_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P5");
    wait_irq_p[5] = 0;
}

void ext_irq_p6_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P6");
    wait_irq_p[6] = 0;
}

void ext_irq_p7_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P7");
    wait_irq_p[7] = 0;
}

void ext_irq_p8_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P8");
    wait_irq_p[8] = 0;
}

void ext_irq_p9_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> P9");
    wait_irq_p[9] = 0;
}

void ext_irq_n0_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N0");
    wait_irq_n[0] = 0;
}

void ext_irq_n1_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N1");
    wait_irq_n[1] = 0;
}

void ext_irq_n2_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N2");
    wait_irq_n[2] = 0;
}

void ext_irq_n3_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N3");
    wait_irq_n[3] = 0;
}

void ext_irq_n4_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N4");
    wait_irq_n[4] = 0;
}

void ext_irq_n5_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N5");
    wait_irq_n[5] = 0;
}

void ext_irq_n6_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N6");
    wait_irq_n[6] = 0;
}

void ext_irq_n7_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N7");
    wait_irq_n[7] = 0;
}

void ext_irq_n8_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N8");
    wait_irq_n[8] = 0;
}

void ext_irq_n9_isr(void *arg)
{
    (void)arg;
    mcu_printf(" -> N9");
    wait_irq_n[9] = 0;
}

FMUErrTypes_t open_irq_p (uint32 nth_irq, uint32 port, uint32 i)
{

    FMUErrTypes_t ret;
    ret = FMU_OK;

#if 0
         if(nth_irq == 0) GIC_IntVectSet(INT_EXT0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p0_isr, NULL);
    else if(nth_irq == 1) GIC_IntVectSet(INT_EXT1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p1_isr, NULL);
    else if(nth_irq == 2) GIC_IntVectSet(INT_EXT2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p2_isr, NULL);
    else if(nth_irq == 3) GIC_IntVectSet(INT_EXT3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p3_isr, NULL);
    else if(nth_irq == 4) GIC_IntVectSet(INT_EXT4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p4_isr, NULL);
    else if(nth_irq == 5) GIC_IntVectSet(INT_EXT5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p5_isr, NULL);
    else if(nth_irq == 6) GIC_IntVectSet(INT_EXT6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p6_isr, NULL);
    else if(nth_irq == 7) GIC_IntVectSet(INT_EXT7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p7_isr, NULL);
    else if(nth_irq == 8) GIC_IntVectSet(INT_EXT8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p8_isr, NULL);
    else                  GIC_IntVectSet(INT_EXT9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_p9_isr, NULL);
#endif
    if(nth_irq == 0UL)
    {
        (void)GIC_IntVectSet(GIC_EXT0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p0_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT0);
    }
    else if(nth_irq == 1UL)
    {
        (void)GIC_IntVectSet(GIC_EXT1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p1_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT1);
    }
    else if(nth_irq == 2UL)
    {
        (void)GIC_IntVectSet(GIC_EXT2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p2_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT2);
    }
    else if(nth_irq == 3UL)
    {
        (void)GIC_IntVectSet(GIC_EXT3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p3_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT3);
    }
    else if(nth_irq == 4UL)
    {
        (void)GIC_IntVectSet(GIC_EXT4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p4_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT4);
    }
    else if(nth_irq == 5UL)
    {
        (void)GIC_IntVectSet(GIC_EXT5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p5_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT5);
    }
    else if(nth_irq == 6UL)
    {
        (void)GIC_IntVectSet(GIC_EXT6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p6_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT6);
    }
    else if(nth_irq == 7UL)
    {
        (void)GIC_IntVectSet(GIC_EXT7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p7_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT7);
    }
    else if(nth_irq == 8UL)
    {
        (void)GIC_IntVectSet(GIC_EXT8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p8_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT8);
    }
    else
    {
        (void)GIC_IntVectSet(GIC_EXT9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_p9_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXT9);
    }



#if 0
    if(port >7)
        wr_data_32b((0x1B935040),rd_data_32b(0x1B935040) & ~(0x1<<port));   // GPIO-MD TODO Confirm
    else
        wr_data_32b((0x1B935000),rd_data_32b(0x1B935000) & ~(0x1<<port));   // GPIO-MD TODO Confirm
#else


    //wr_data_32b((addr_gpio[i]),rd_data_32b(addr_gpio[i]) & ~(0x1<<port)); // GPIO-MD TODO Confirm
    GPIO_WriteReg((addr_gpio[i]),GPIO_ReadReg(addr_gpio[i]) | ((uint32)0x1<<port)); // GPIO-MD TODO Confirm


#endif
    return ret;

}

FMUErrTypes_t open_irq_n (uint32 uiNnthIrq,uint32 uiPort,uint32 iIndex)
{
    FMUErrTypes_t ret = FMU_OK;

#if 0
          if(uiNnthIrq == 0) GIC_IntVectSet(GIC_EXTn0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n0_isr, NULL);
     else if(uiNnthIrq == 1) GIC_IntVectSet(GIC_EXTn1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n1_isr, NULL);
     else if(uiNnthIrq == 2) GIC_IntVectSet(GIC_EXTn2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n2_isr, NULL);
     else if(uiNnthIrq == 3) GIC_IntVectSet(GIC_EXTn3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n3_isr, NULL);
     else if(uiNnthIrq == 4) GIC_IntVectSet(GIC_EXTn4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n4_isr, NULL);
     else if(uiNnthIrq == 5) GIC_IntVectSet(GIC_EXTn5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n5_isr, NULL);
     else if(uiNnthIrq == 6) GIC_IntVectSet(GIC_EXTn6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n6_isr, NULL);
     else if(uiNnthIrq == 7) GIC_IntVectSet(GIC_EXTn7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n7_isr, NULL);
     else if(uiNnthIrq == 8) GIC_IntVectSet(GIC_EXTn8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n8_isr, NULL);
     else                  GIC_IntVectSet(GIC_EXTn9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, ext_irq_n9_isr, NULL);
#endif

    if(uiNnthIrq == 0UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn0, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n0_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn0);
    }
    else if(uiNnthIrq == 1UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn1, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n1_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn1);   
    }
    else if(uiNnthIrq == 2UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn2, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n2_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn2);
    }
    else if(uiNnthIrq == 3UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn3, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n3_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn3);

    }
    else if(uiNnthIrq == 4UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn4, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n4_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn4);
    }
    else if(uiNnthIrq == 5UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn5, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n5_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn5);
    }
    else if(uiNnthIrq == 6UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn6, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n6_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn6);
    }
    else if(uiNnthIrq == 7UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn7, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n7_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn7);
    }
    else if(uiNnthIrq == 8UL)
    {
        (void)GIC_IntVectSet(GIC_EXTn8, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n8_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn8);
    }
    else
    {
        (void)GIC_IntVectSet(GIC_EXTn9, GIC_PRIORITY_NO_MEAN, GIC_INT_TYPE_EDGE_RISING, (GICIsrFunc)&ext_irq_n9_isr, (void *) 0);
        (void)GIC_IntSrcEn(GIC_EXTn9);
    }

    //GPIO_WriteReg((addr_gpio[iIndex]),GPIO_ReadReg(addr_gpio[iIndex]) | (0x1<<uiPort));  // GPIO-MD TODO Confirm
    GPIO_WriteReg((addr_gpio[iIndex]),GPIO_ReadReg(addr_gpio[iIndex]) & ~((uint32)0x1<<uiPort));   // GPIO-MD TODO Confirm

    return ret;
}


void close_irq_p (uint32 uiNnthIrq)
{

    if(uiNnthIrq == 0UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT0 );
    }
    else if(uiNnthIrq == 1UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT1 );
    }
    else if(uiNnthIrq == 2UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT2 );
    }
    else if(uiNnthIrq == 3UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT3 );
    }
    else if(uiNnthIrq == 4UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT4 );
    }
    else if(uiNnthIrq == 5UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT5 );
    }
    else if(uiNnthIrq == 6UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT6 );
    }
    else if(uiNnthIrq == 7UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT7 );
    }
    else if(uiNnthIrq == 8UL)
    {
        (void)GIC_IntSrcDis(GIC_EXT8 );
    }
    else
    {
        (void)GIC_IntSrcDis(GIC_EXT9 );
    }
}

void close_irq_n (uint32 uiNnthIrq)
{

    if(uiNnthIrq == 0UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn0);
    }
    else if(uiNnthIrq == 1UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn1);
    }
    else if(uiNnthIrq == 2UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn2);
    }
    else if(uiNnthIrq == 3UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn3);
    }
    else if(uiNnthIrq == 4UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn4);
    }
    else if(uiNnthIrq == 5UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn5);
    }
    else if(uiNnthIrq == 6UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn6);
    }
    else if(uiNnthIrq == 7UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn7);
    }
    else if(uiNnthIrq == 8UL)
    {
        (void)GIC_IntSrcDis(GIC_EXTn8);
    }
    else
    {
        (void)GIC_IntSrcDis(GIC_EXTn9);
    }
}




void ext_irq_filter_cfg (uint32 uiNth, uint32 uiPolarity, uint32 uiDepth)
{
    uint32 wr_addr;
    uint32 wr_data;

    wr_data = ((uiPolarity<<(uint32)26) | uiDepth);
    wr_addr = (MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2));
    GPIO_WriteReg(wr_addr, wr_data);
}

void ext_irq_filter_en (uint32 uiNth)
{
    uint32 wr_addr;
    uint32 rd_addr;
    uint32 rd_data;

    rd_addr = MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2);
    rd_data = GPIO_ReadReg(rd_addr);

    wr_addr = (MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2));

    GPIO_WriteReg(wr_addr, (rd_data | ((uint32)0x1<<(uint32)24)));
}

void ext_irq_filter_clr(uint32 uiNth)
{
    uint32 wr_addr;
    uint32 rd_addr;
    uint32 rd_data;

    rd_addr = MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2);
    rd_data = GPIO_ReadReg (rd_addr);

    wr_addr = (MCU_BSP_GPIO_BASE + 0x800UL + (uiNth << (uint32)2));
    
    GPIO_WriteReg(wr_addr, (rd_data & (0xffffffffUL - ((uint32)0x1<<(uint32)24))));
    // GPIO_WriteReg((MCU_BSP_GPIO_BASE + 0x800 + (uiNth << 2)),0x0);
}


void ext_irq_sel_p(uint32 uiPort)
{
    uint32 sel0 =0;
    uint32 sel1 =0;
    uint32 sel2 =0;
    uint32 base_addr;

    sel0 |= (uiPort << (uint32)0);
    sel0 |= (uiPort << (uint32)8);
    sel0 |= (uiPort << (uint32)16);
    sel0 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(base_addr,sel0);

    sel1 |= (uiPort << (uint32)0);
    sel1 |= (uiPort << (uint32)8);
    sel1 |= (uiPort << (uint32)16);
    sel1 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(base_addr,sel1);

    sel2 |= (uiPort << (uint32)0);
    sel2 |= (uiPort << (uint32)8);
    base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(base_addr,sel2);
#if 0
    HwMC_GPIO->pEINTCFG.bReg.EINT0  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT1  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT2  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT3  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT4  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT5  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT6  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT7  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT8  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT9  = uiPort;
#endif
}



static void ext_irq_sel_n(uint32 uiPort)
{
    uint32 sel0 =0;
    uint32 sel1 =0;
    uint32 sel2 =0;
    uint32 base_addr;

    sel0 |= (uiPort << (uint32)0);
    sel0 |= (uiPort << (uint32)8);
    sel0 |= (uiPort << (uint32)16);
    sel0 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x280UL);
    GPIO_WriteReg(base_addr,sel0);

    sel1 |= (uiPort << (uint32)0);
    sel1 |= (uiPort << (uint32)8);
    sel1 |= (uiPort << (uint32)16);
    sel1 |= (uiPort << (uint32)24);
    base_addr = (MCU_BSP_GPIO_BASE + 0x284UL);
    GPIO_WriteReg(base_addr,sel1);

    sel2 |= (uiPort << (uint32)0);
    sel2 |= (uiPort << (uint32)8);

    base_addr = (MCU_BSP_GPIO_BASE + 0x288UL);
    GPIO_WriteReg(base_addr,sel2);
#if 0
    HwMC_GPIO->pEINTCFG.bReg.EINT0  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT1  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT2  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT3  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT4  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT5  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT6  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT7  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT8  = uiPort;
    HwMC_GPIO->pEINTCFG.bReg.EINT9  = uiPort;
#endif
}



static void GPIO_InoutControl(void)
{
    uint32 i;

    mcu_printf("GPIO A : 0x%08X\n", GPIO_ReadReg(addr_gpio[0]));
    mcu_printf("GPIO B : 0x%08X\n", GPIO_ReadReg(addr_gpio[1]));
    mcu_printf("GPIO C : 0x%08X\n", GPIO_ReadReg(addr_gpio[2]));
    mcu_printf("GPIO E : 0x%08X\n", GPIO_ReadReg(addr_gpio[3]));
    mcu_printf("GPIO G : 0x%08X\n", GPIO_ReadReg(addr_gpio[4]));
    mcu_printf("GPIO MB: 0x%08X\n", GPIO_ReadReg(addr_gpio[5]));
    mcu_printf("GPIO MC: 0x%08X\n", GPIO_ReadReg(addr_gpio[6]));
    mcu_printf("GPIO MD: 0x%08X\n", GPIO_ReadReg(addr_gpio[7]));


    for(i=0 ; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT)) ;
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));


        (void)GPIO_Set(GPIO_GPA(i), 1UL);
        (void)GPIO_Set(GPIO_GPB(i), 1UL);
        (void)GPIO_Set(GPIO_GPC(i), 1UL);
        (void)GPIO_Set(GPIO_GPE(i), 1UL);
        (void)GPIO_Set(GPIO_GPG(i), 1UL);
        //(void)GPIO_Set(GPIO_GPH(i), 1);
        //(void)GPIO_Set(GPIO_GPK(i), 1UL);
        //(void)GPIO_Set(GPIO_GPMA(i), 1);
        (void)GPIO_Set(GPIO_GPMB(i), 1UL);
        (void)GPIO_Set(GPIO_GPMC(i), 1UL);
        (void)GPIO_Set(GPIO_GPMD(i), 1UL);


        //(void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0UL) | GPIO_INPUT));
        //(void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0UL) | GPIO_INPUT));

        //(void)GPIO_Get(GPIO_GPA(i));
        //(void)GPIO_Get(GPIO_GPB(i));
        //(void)GPIO_Get(GPIO_GPC(i));
        //(void)GPIO_Get(GPIO_GPE(i));
        //(void)GPIO_Get(GPIO_GPG(i));
        //GPIO_Get(GPIO_GPH(i));
        //(void)GPIO_Get(GPIO_GPK(i));
        //(void)GPIO_Get(GPIO_GPMA(i));
    }

    
    
}



static void GPIO_controlatfunction0(void)
{
    uint32    i;
    uint32    check_data;
    uint32    base_addr;
    uint32    comp_addr;

    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }

    check_data = 0;
    for(i=0; i<(uint32)GPIO_TEMP_NUM; i++)
    {
        comp_addr = addr_gpio[i] + GPIO_OFFSET_DAT;

        //GP_#_DAT
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("DAT write 0x00000000 fail!! [0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT write 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(base_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("DAT read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }


        //GP_#_SET
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("DAT write 0x00000000 fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT write 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_SET;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("SET write 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("SET write 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        //GP_#_CLR
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(base_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("DAT read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_CLR;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("CLR read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("CLR read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        //GP_#_XOR
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("DAT read 0x00000000 fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("DAT read 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_XOR;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("XOR read 0x00000000 fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("XOR read 0x00000000 sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_XOR;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ reset_bit[i]);

        if(check_data != 0UL)
        {
            mcu_printf("XOR read 0xffffffff fail!![0x%08X]\n",base_addr);
        }
        else
        {
            mcu_printf("XOR read 0xffffffff sucess!![0x%08X]\n",base_addr);
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_XOR;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(comp_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("XOR read 0xffffffff fail!![0x%08X]\n\n",base_addr);
        }
        else
        {
            mcu_printf("XOR read 0xffffffff sucess!![0x%08X]\n\n",base_addr);
        }
    }
}


static void GPIO_lockfunctionmultiplexer(void)
 {
     
     uint32    base_addr;

     

     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x55555555UL) & 0xffffffffUL;
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x55555555UL) & 0xffffffffUL;
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));

     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x000fffffUL));

     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00ffffffUL));

     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x0000ffffUL));

     base_addr = addr_gpio[4] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[4] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00000fffUL));

     base_addr = addr_gpio[5] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[5] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[5] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00000fffUL));

     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));

     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0x55555555UL);
     mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00ffffffUL));

     //base_addr = addr_gpio[8] + GPIO_OFFSET_FNC0;
     //GPIO_WriteReg(base_addr, 0x55555555UL);
     //mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     //base_addr = addr_gpio[8] + GPIO_OFFSET_FNC1;
     //GPIO_WriteReg(base_addr, 0x55555555UL);
     //mcu_printf("addr[0x%08X] write 0x55555555 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));


     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x4UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x8UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x10UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x18UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x2cUL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x30UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x34UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x38UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x3cUL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x40UL;
     GPIO_WriteReg(base_addr, 0xffffffffUL);


     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[0] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));

     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[1] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x000fffffUL));

     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[2] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00ffffffUL));

     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[3] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x0000ffffUL));

     base_addr = addr_gpio[4] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[4] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00000fffUL));

     base_addr = addr_gpio[5] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[5] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[5] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00000fffUL));

     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[6] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));

     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC0;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC1;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC2;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     base_addr = addr_gpio[7] + GPIO_OFFSET_FNC3;
     GPIO_WriteReg(base_addr, 0xffffffffUL);
     mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0x00ffffffUL));

     //base_addr = addr_gpio[8] + GPIO_OFFSET_FNC0;
     //GPIO_WriteReg(base_addr, 0xffffffffUL);
     //mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));
     //base_addr = addr_gpio[8] + GPIO_OFFSET_FNC1;
     //GPIO_WriteReg(base_addr, 0xffffffffUL);
     //mcu_printf("addr[0x%08X] write 0xffffffff reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ (0x55555555UL & 0xffffffffUL));


     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x4UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x1fffffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x8UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x10UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x000fffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x18UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x000007ff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x2cUL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000fff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x30UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x0007ffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x34UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x38UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0xffffffffUL);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x3cUL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffff);
     base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x40UL;
     GPIO_WriteReg(base_addr, 0x00000000UL);
     mcu_printf("addr[0x%08X] write 0x00000000 reg val [0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
     //check_data |= (GPIO_ReadReg(base_addr) ^ 0x0003ffff);


}


static void GPIO_Tpg(void)
{
    uint32    i;
    uint32    base_addr;

    for(i=0; i<(uint32)8; i++)
    {
        base_addr = tpg_reg[i] + GPIO_VI2O_EN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_VI2O_OEN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_MON_OEN;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN; 
            mcu_printf("Normal OEN Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("Normal OEN Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_VI2I_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr =tpg_reg[i] + GPIO_MON_DO;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("Normal DO Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("Normal DO Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        base_addr = tpg_reg[i] + GPIO_VI2O_EN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_VI2O_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr= tpg_reg[i] + GPIO_MON_OEN;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("TPG OEN Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("TPG OEN Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_OEN, GPIO_ReadReg(base_addr));
        }

        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_VI2I_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_MON_DO;
        if(GPIO_ReadReg(base_addr) != reset_bit[i])
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;
            mcu_printf("TPG DO Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO;           
            mcu_printf("TPG DO Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }

        base_addr = tpg_reg[i] + GPIO_VI2O_EN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        base_addr = tpg_reg[i] + GPIO_VI2I_DAT;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        base_addr = tpg_reg[i] + GPIO_MON_DO;
        if(GPIO_ReadReg(base_addr) != 0x00000000UL)
        {
            base_addr = tpg_reg[i] + GPIO_MON_OEN;
            mcu_printf("Normal OEM / TPG Do Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        else
        {
            base_addr = tpg_reg[i] + GPIO_MON_DO; 
            mcu_printf("Normal OEM / TPG Do Sucess [%d] addr[0x%08X] value[0x%08X]\n", i, tpg_reg[i] + GPIO_MON_DO, GPIO_ReadReg(base_addr));
        }
        base_addr = tpg_reg[i] + GPIO_VI2I_EN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        if(GPIO_ReadReg(din_reg[i]) != reset_bit[i])
        {
            mcu_printf("TPG LoopBack Fail !!![%d] addr[0x%08X] value[0x%08X]\n", i, din_reg[i], GPIO_ReadReg(din_reg[i]));
        }
        else
        {
            mcu_printf("TPG LoopBack Sucess[%d] addr[0x%08X] value[0x%08X]\n", i, din_reg[i], GPIO_ReadReg(din_reg[i]));
        }

    }
}


static void GPIO_SmWriteProtection(void)
{
    uint32    i;
    uint32    check_data;
    uint32    rd_data;
    uint32    base_addr;

    check_data = 0;
    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }

    for(i=0; i<(uint32)189; i++)
    {
        fault_index = wr_reg[i<<1UL];

        if((fault_index < (uint32)192) || (fault_index > (uint32)240))
        {        // *_FNC_LOCK, *_OEN_LOCKn skip
            base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);
            GPIO_WriteReg(base_addr, 0xffffffffUL);
            check_data |= (GPIO_ReadReg(base_addr) ^ wr_reg[(i << 1UL) + 1UL]);

            if(check_data != 0UL)
            {
                check_data = 0;
            }
        }
    }

    //CFG_WR_PW
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_PW;
    GPIO_WriteReg(base_addr, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_LOCK;
    GPIO_WriteReg(base_addr, 0x00000001UL);

    for(i=0; i<(uint32)189 ;i++)
    {
        fault_index = wr_reg[i<<1UL];

        if((fault_index < (uint32)192) || (fault_index > (uint32)240))
        {        // *_FNC_LOCK, *_OEN_LOCKn skip
            base_addr = MCU_BSP_GPIO_BASE + (fault_index<< 2UL);
            rd_data = GPIO_ReadReg(base_addr);
            GPIO_WriteReg(base_addr, 0x55555555UL);

            if(GPIO_ReadReg(base_addr) != rd_data)
            {
                mcu_printf("CFG_WR_LOCK fail   [%d]ori[0x%08X]write[0x%08X]\n",
                       fault_index,
                       rd_data,
                       GPIO_ReadReg(base_addr));
            }
            else
            {
                mcu_printf("CFG_WR_LOCK sucess [%d]ori[0x%08X]write[0x%08X]\n",
                       fault_index,
                       rd_data,
                       GPIO_ReadReg(base_addr));
            }
        }
    }

    //CFG_WR_PW
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_PW;
    GPIO_WriteReg(base_addr, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_LOCK;
    GPIO_WriteReg(base_addr, 0x00000000UL);

    for(i=0; i<(uint32)189 ;i++)
    {
        fault_index = wr_reg[i<<1UL];

        if((fault_index < (uint32)192) || (fault_index > (uint32)240))
        {        // *_FNC_LOCK, *_OEN_LOCKn skip
            base_addr = MCU_BSP_GPIO_BASE + (fault_index<< 2UL);
            rd_data = GPIO_ReadReg(base_addr);
            GPIO_WriteReg(base_addr, 0x55555555UL);

            if(GPIO_ReadReg(base_addr) != rd_data)
            {
                mcu_printf("CFG_WR_Release sucess [%d]ori[0x%08X]write[0x%08X]\n",
                       fault_index,
                       rd_data,
                       GPIO_ReadReg(base_addr));
            }
            else
            {
                mcu_printf("CFG_WR_Release fail   [%d]ori[0x%08X]write[0x%08X]\n",
                       fault_index,
                       rd_data,
                       GPIO_ReadReg(base_addr));
            }
        }
    }
}


static void GPIO_SmExtclitchintFilter(void)
{
    uint32 i;
    uint32 nth_irq;
    uint32 port;
    uint32 base_addr;
    uint32 uiK1;
    uint32 uiK2;
    const uint32  uiDelaycnt = (5000000UL);

    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS + 4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS + 8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_OFFSET_EIS + 12UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr= MCU_BSP_GPIO_BASE + GPIO_OFFSET_ECS;
    GPIO_WriteReg(base_addr, 0xffffffffUL);

        // Setup PAD mux
    base_addr =  (0x1B936000 + 0x148);   
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-A
    base_addr = (0x1B936000 + 0x14c);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-B
    base_addr= (0x1B936000 + 0x150);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-C
    base_addr = (0x1B936000 + 0x154);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-E
    base_addr = (0x1B936000 + 0x158);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-G
    //GPIO_WriteReg((0x1B936000 + 0x15c), 0x00000000UL);    // GPIO-H
    //GPIO_WriteReg((0x1B936000 + 0x160), 0x00000000UL);    // GPIO-MA
    base_addr = (0x1B936000 + 0x164);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-MB TODO Confirm
    base_addr = (0x1B936000 + 0x168);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-MC TODO Confirm
    base_addr = (0x1B936000 + 0x16c);
    GPIO_WriteReg(base_addr, 0x00000000UL);  // GPIO-MD TODO Confirm
    GPIO_WriteReg((0x1B935044UL), 0xffffffffUL);  // GPIO A Input
    GPIO_WriteReg((0x1B935040UL), 0xffffffffUL);  // GPIO A Input
    //GPIO_WriteReg((0x1B935000), 0x00000000UL);    // GPIO A Input

    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        if((i != (uint32)24) && (i != (uint32)25))//8050 mc uart at the port mux board
    {
            (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));

        (void)GPIO_Set(GPIO_GPA(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPB(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPC(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPE(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPG(i), 0x0UL);
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Set(GPIO_GPK(i), 0x0UL);
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Set(GPIO_GPMA(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPMB(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPMC(i), 0x0UL);
        (void)GPIO_Set(GPIO_GPMD(i), 0x0UL);
    }
    (void)GPIO_Config(GPIO_GPH(6UL), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    (void)GPIO_Config(GPIO_GPH(7UL), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    (void)GPIO_Set(GPIO_GPH(6UL), 0x0UL);
    (void)GPIO_Set(GPIO_GPH(7UL), 0x0UL);
    
    for(nth_irq=0; nth_irq<(uint32)GPIO_MAX_NTH_IRQ; nth_irq++)
    {
        for(i =0;i <(uint32)10; i++)
        {
            for(port=0; port<nof_gpio[i]; port++)
            {
                //if(((i == (uint32)8) && (((port == (uint32)24)) || (port == (uint32)25)))||(i == (uint32)5 && ((port < (uint32)6) || (port == (uint32)8) || (port == (uint32)9))))
                if(i==(uint32)8)
                {
                    ;//skip
                }
                else
                {
                    //ext_irq_sel_p(port);                // For GPIO-A
                    ext_irq_sel_p(port + gpio_port[i]);               // For GPIO-B
                    //ext_irq_sel_p(130);              // For GPIO-A


                    //for(type=0; type<GPIO_MAX_TYPE; type++) {
                    mcu_printf(" \n filter-%d, 0x%08X[%d] ", nth_irq, addr_gpio[i], port);

                    //ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_HIGH, ((0x1 << ((type + 8) +1)) -1)); // Active High
                    //ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_HIGH, 0xFFFFF); // Active High
                    //ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_LOW, 0x0); // Active Low
                    ext_irq_filter_cfg(nth_irq, GPIO_ACTIV_HIGH, 0x0UL); //  GPIO_ACTIV_HIGH
                    ext_irq_filter_en (nth_irq);

                    if(GPIO_Get(GPIO_GPH(3UL)) == (uint8)1UL){  //avoid code sonar warning always 1
                        
                        wait_irq_p[nth_irq] = 1;
                        (void)open_irq_p(nth_irq,port,i);

                        wait_irq_n[nth_irq] = 1;
                        (void)open_irq_n(nth_irq,port,i);

                    }

                    //mc_sim_ext_irq_test(SET_IRQ_PORT | IRQ_ENABLE | (port<<4) | type);

                    // Wait positive edge interrupt

                    for (uiK1 = 0 ; (uiK1 < uiDelaycnt) ; uiK1++)
                    {
                        if(wait_irq_p[nth_irq] == 0UL)
                        {
                            break;
                        }        
                        BSP_NOP_DELAY();
                    }
                    

                    close_irq_p(nth_irq);

                    // Wait negative edge interrupt

                    for (uiK2 = 0 ; (uiK2 < uiDelaycnt) ; uiK2++)
                    {
                        if(wait_irq_n[nth_irq] == 0UL)
                        {
                            break;
                        }        
                        BSP_NOP_DELAY();
                    }
                    close_irq_n(nth_irq);

                    ext_irq_filter_clr(nth_irq);
                    //mc_sim_ext_irq_test(SET_IRQ_PORT | IRQ_DISABLE | (port<<4) | type);
                }
            }
        }
    }

    mcu_printf("\n glitch test DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

}


static void GPIO_SmSwFaultInjection(void)
{
    uint32    i;
    uint32    base_addr_pw;
    uint32    base_addr;
    //uint32 uiK1;
    //const uint32  uiDelaycnt = (5000000UL);

    for(i=0; i<(uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0UL) | GPIO_OUTPUT));
    }

    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS + 0x4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS + 0x8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);

    // Fault inection test enable
    base_addr_pw = MCU_BSP_GPIO_BASE + GPIO_CFG_WR_PW;
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN;
    GPIO_WriteReg(base_addr_pw, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0xCUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x14UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x18UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x1CUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x20UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x24UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x28UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x2CUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr= MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x30UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr= MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x34UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x38UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x3cUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x40UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SOFT_FAULT_EN + 0x44UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);

#if 0
    for(i=0; i<18; i++) {
        HwMC_GPIO->CFG_WR_PW                        = GPIO_PASSWORD;
        HwMC_GPIO->SOFT_FAULT_EN[i]                 = 0xffffffff;
    }

    HwMC_GPIO->CFG_WR_PW                                = GPIO_PASSWORD;//;//GPIO_PASSWORD;
    HwMC_GPIO->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_TEST_EN = 1;

    HwMC_GPIO->CFG_WR_PW                            = GPIO_PASSWORD;
    HwMC_GPIO->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_EN  = 1;
#endif

    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SF_CTRL_CFG;

    GPIO_WriteReg(base_addr, GPIO_ReadReg(base_addr) | 0x00000011UL);

    for(i=0; i<(uint32)189; i++)
    {
        fault_index = wr_reg[i<<1];

        if((fault_index < (uint32)192) || (fault_index > (uint32)240))
        {        // *_FNC_LOCK, *_OEN_LOCKn skip
            base_addr = MCU_BSP_GPIO_BASE + (fault_index<< 2);
            GPIO_WriteReg(base_addr,
            GPIO_ReadReg(base_addr) ^ 0x11111111UL);
            wait_irq_done();

            mcu_printf("GPIO Inturrupt done [%d][0x%08X] \n",fault_index,base_addr);
            gpio_irq_done = 0;
        }
    }

    // Fault inection test disable
    //HwMC_GPIO->CFG_WR_PW                                 = GPIO_PASSWORD;//GPIO_PASSWORD;
    //HwMC_GPIO->uSOFT_FAULT_CTRL.bReg.SOFT_FAULT_TEST_EN  = 0;
    GPIO_WriteReg(base_addr_pw, GPIO_PASSWORD);
    base_addr = MCU_BSP_GPIO_BASE + GPIO_SF_CTRL_CFG;
    GPIO_WriteReg(base_addr,
    GPIO_ReadReg(base_addr) ^ 0x00000011UL);
}
#endif

static void GPIO_RegisterBank(void) /* based on SoC test bench */
{
    uint32    i;
    uint32    check_data = 0; 
    uint32    rw_error = 0;
    uint32    base_addr;

    mcu_printf("ip_test_GPIO_register_bank Test Start!!\n");
#if 0

    //micom select
    for(i=0; i<32 ; i++)
    {
        (void)GPIO_Config(TCC_GPA(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC(0) | GPIO_OUTPUT));

        (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
    }
#endif
    //----------------------------------------------------------------------------------------------------------------------------
    // Data(DAT)
    //----------------------------------------------------------------------------------------------------------------------------
    //check_data = 0;
    for(i=0; i<(uint32)GPIO_TEMP_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_DAT;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }           
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_DS0;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_DS1;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_PE;
        if(GPIO_ReadReg(base_addr)!= reset_bit[i])
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),reset_bit[i]);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_PS;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_IEN;
        if(GPIO_ReadReg(base_addr)!= reset_bit[i])
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),reset_bit[i]);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_IS;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_SR;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_FNC0;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_FNC1;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_FNC2;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
        base_addr = addr_gpio[i] + GPIO_OFFSET_FNC3;
        if(GPIO_ReadReg(base_addr)!= 0x00000000UL)
        {
            mcu_printf("reset fail addr[0x%08X] data[0x%08X] ori[0x%08X] \n",base_addr,GPIO_ReadReg(base_addr),0x00000000UL);
        }
        //check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000);
    }


    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS + 0x4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_EIS + 0x8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_ECS;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x10UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x18UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x2cUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x30UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x34UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x38UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x3cUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_FLE + 0x40UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0xffffffffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x1fffffffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x10UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x000fffffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x18UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x000007ffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x2cUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000fffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x30UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x0007ffffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x34UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x38UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0xffffffffUL);
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x3cUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x3fffffffUL);

    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x40UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x0003ffffUL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    
    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x8UL; 
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0xCUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x20UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x24UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x28UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x2CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x40UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x44UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x48UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x4CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x80UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x84UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x88UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x8CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0xC0UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0xC4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0xC8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0xCCUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x180UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x184UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr = addr_gpio[0] + GPIO_OFFSET_VIR + 0x188UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + GPIO_OFFSET_VIR + 0x18CUL; 
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5A0UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5A4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5A8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5ACUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5C0UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5C4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5C8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5C8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5E0UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5E4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5E8UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x5ECUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x600UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x604UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x608UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x60CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x800UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x804UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x808UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x80CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x810UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x814UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x818UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x81CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x820UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x824UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x8C0UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x8C4UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0x8C0UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD00UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD04UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD08UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD0CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD14UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD18UL;  
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD1CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD20UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD24UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD28UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD2CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD30UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD34UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD38UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD3CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD40UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xD44UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE00UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE04UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE08UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);  
    base_addr= addr_gpio[0] + 0xE0CUL;  
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE14UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE18UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE1CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE20UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE24UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE28UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE2CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE30UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE34UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE38UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE3CUL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE40UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE04UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xE44UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);  
    base_addr= addr_gpio[0] + 0xF00UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
    base_addr= addr_gpio[0] + 0xF04UL;
    check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL); 

    if(check_data != 0UL)
    {
        mcu_printf("GPx.DAT       check : %x\r\n", check_data);
    }
    else
    {
        mcu_printf("GPIO RESET VALUE CHECK TEST SUCESS!!\n\n");
    }

    //check_data = 0;

#if 1
    //micom select
    for(i=0; i< (uint32)32 ; i++)
    {
        (void)GPIO_Config(GPIO_GPA(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPB(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPC(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPE(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPG(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPH(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPK(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        //(void)GPIO_Config(GPIO_GPMA(i), (GPIO_FUNC(0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMB(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));

        (void)GPIO_Config(GPIO_GPMC(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
        (void)GPIO_Config(GPIO_GPMD(i), (GPIO_FUNC((uint32)0) | GPIO_OUTPUT));
    }
#endif

    mcu_printf("---- GPIO REG READ/WRITE TEST START \n");

    for(i=0; i<(uint32)189; i++)
    {
        fault_index = wr_reg[i<<1UL];
        base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);

        if((fault_index < (uint32)192) || (fault_index > (uint32)240))
        {        // *_FNC_LOCK, *_OEN_LOCKn skip
            if(fault_index != (uint32)494)
            {
                base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL); 
                GPIO_WriteReg(base_addr, 0x00000000UL);
                base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);
                rw_error |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);
            }

            base_addr = MCU_BSP_GPIO_BASE + (fault_index << 2UL);
            GPIO_WriteReg(base_addr, 0xffffffffUL);
            rw_error |= (GPIO_ReadReg(base_addr) ^ wr_reg[(i << 1UL) + 1UL]);

            if(rw_error != 0UL)
            {
                mcu_printf("table 0x%08X , actual reg 0x%08X  fault_id %d \n", wr_reg[(i<<1UL) + 1UL], GPIO_ReadReg(base_addr), fault_index);
                rw_error = 0;
            }
        }
        //mcu_printf ("****** index [%d] reg [0x%x] checkdata [%d]******** \n",fault_index, MCU_BSP_GPIO_BASE + (fault_index<< 2),check_data);
    }
#if 0
    if(rw_error == 0UL)
    {
        mcu_printf("GPIO REG READ/WRITE TEST SUCESS!!\n");
    }
    else
    {
        mcu_printf("GPIO REG READ/WRITE TEST FAIL!![%x]\r\n", rw_error);
    }
#endif 
    mcu_printf("++++ GPIO REG READ/WRITE TEST END \n");
}


static void GPIO_lockoutputconfigure(void)
{
    uint32    i;
    uint32    check_data;
    uint32    base_addr;

    check_data =0;

    mcu_printf("[ip_test_GPIO_lockoutputconfigure start!!]\n");

    mcu_printf("EN register set 0x1 \n");

    for(i=0; i<(uint32)GPIO_TEMP_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    }

    mcu_printf("EN register set 0x0 \n");

    for(i=0; i<(uint32)GPIO_TEMP_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0x00000000UL);
        mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    }

    mcu_printf("EN_LOCK_ENn regitse 0x0 \n");
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x4UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x8UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x10UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x18UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x2cUL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x30UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x34UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x38UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x3cUL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x40UL;
    GPIO_WriteReg(base_addr, 0x00000000UL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));

    mcu_printf("EN register set 0x1 \n");
    for(i=0; i<(uint32)GPIO_TEMP_NUM; i++)
    {
        base_addr = addr_gpio[i] + GPIO_OFFSET_OEN;
        GPIO_WriteReg(base_addr, 0xffffffffUL);
        check_data |= (GPIO_ReadReg(base_addr) ^ 0x00000000UL);

        if(check_data != 0UL)
        {
            mcu_printf("EN register set fail [0x%08X][0x%08X]\n",base_addr, GPIO_ReadReg(base_addr) );
        }
        else
        {
            mcu_printf("EN register set sucess [0x%08X][0x%08X]\n", base_addr, GPIO_ReadReg(base_addr) );
        }
    }

    mcu_printf("EN_LOCK_ENn regitser write 0x1 and read\n");
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x4UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x8UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x10UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x18UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x2cUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x30UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x34UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x38UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x3cUL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));
    base_addr =  addr_gpio[0] + GPIO_OFFSET_OLE + 0x40UL;
    GPIO_WriteReg(base_addr, 0xffffffffUL);
    mcu_printf("addr[0x%08X] value[0x%08X]\n",base_addr, GPIO_ReadReg(base_addr));

    mcu_printf("[ip_test_GPIO_lockoutputconfigure End!!]\n");
}


void GPIO_StartGpioTest(int32 ucMode)
{
    switch (ucMode)
    {

        case 1 :
        {
            GPIO_RegisterBank();
            break;
        }
#ifdef TRACE32_TEST         
        case 2 :
        {
            GPIO_InoutControl();  
            break;
        }
        case 3:
        {
            GPIO_controlatfunction0();
            break;
        }
        case 4:
        {
            GPIO_lockfunctionmultiplexer();
            break;
        }
#endif          
        case 5:
        {
            GPIO_lockoutputconfigure();
            break;
        }
#ifdef TRACE32_TEST         
        case 9:
        {
            GPIO_Tpg();
            break;
        }
        case 10:
        {
            GPIO_SmWriteProtection();
            break;
        }
    
        case 11:
        {
            GPIO_SmSwFaultInjection();
            break;
        }
        case 12:
        {               
            GPIO_SmExtclitchintFilter();
            break;          
        }
#endif      
        default:
        {
            mcu_printf("invalid test mode param\n");
            break;
        }
    }
}

