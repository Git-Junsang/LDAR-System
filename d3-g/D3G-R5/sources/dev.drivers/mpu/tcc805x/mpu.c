/*
***************************************************************************************************
*
*   FileName : mpu.c
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

//#include <cache.h>
#include <bsp.h>
#include <mpu.h>
//#include <reg_phys.h>

#ifdef __MPU_ENABLED__

/****************************************************************************
*                               USER SETTING
****************************************************************************/

/*** 1. SELECT DMA SIZE - CHOICE ONLY ONE ***/

#define MPU_REGION_DMA                  (MPU_REGION_16KB)

#if ( MPU_REGION_DMA == MPU_REGION_4KB )
    #define MPU_DMA_ADDR                (MPU_SRAM0_ADDR + 0x7F000U)
#elif ( MPU_REGION_DMA == MPU_REGION_8KB )
    #define MPU_DMA_ADDR                (MPU_SRAM0_ADDR + 0x7E000U)
#elif ( MPU_REGION_DMA == MPU_REGION_16KB )
    #define MPU_DMA_ADDR                (MPU_SRAM0_ADDR + 0x7C000U)
#elif ( MPU_REGION_DMA == MPU_REGION_32KB )
    #define MPU_DMA_ADDR                (MPU_SRAM0_ADDR + 0x78000U)
#elif ( MPU_REGION_DMA == MPU_REGION_64KB )
    #define MPU_DMA_ADDR                (MPU_SRAM0_ADDR + 0x70000U)
#endif

/*** 2. SET RESION ATTRIBUTES ***/
const MPUConfig_t sMPUTable[ MPU_MAX_REGION ] =
{
    { MPU_REGION_ENABLE,  MPU_BOOT_ADDR,        MPU_REGION_4GB,     MPU_EXECUTE_NEVER         | MPU_PRIV_RW_USER_NA | MPU_STRONG_ORDERED_SHARED },
    { MPU_REGION_ENABLE,  MPU_BOOT_ADDR,        MPU_REGION_256MB,   MPU_NORM_SHARED_WB_WA     | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_PERI_ADDR,        MPU_REGION_16MB,    MPU_STRONG_ORDERED_SHARED | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_SRAM0_ADDR,       MPU_REGION_512KB,   MPU_NORM_SHARED_WB_WA     | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_SRAM1_ADDR,       MPU_REGION_64KB,    MPU_STRONG_ORDERED_SHARED | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_DMA_ADDR,         MPU_REGION_DMA,     MPU_STRONG_ORDERED_SHARED | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_SNOR_ADDR,        MPU_REGION_128MB,   MPU_NORM_NSHARED_WB_NWA   | MPU_PRIV_RO_USER_RO },
    { MPU_REGION_ENABLE,  MPU_VECTOR_ADDR,      MPU_REGION_1KB,     MPU_NORM_SHARED_WB_WA     | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_A53_BOOT_ADDR,    MPU_REGION_256MB,   MPU_NORM_SHARED_WB_WA     | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_A53_BACKUP_ADDR,  MPU_REGION_256MB,   MPU_NORM_SHARED_WB_WA     | MPU_PRIV_RO_USER_RO },
    { MPU_REGION_ENABLE,  MPU_CBUS_ADDR,        MPU_REGION_4MB,     MPU_STRONG_ORDERED_SHARED | MPU_PRIV_RW_USER_RW },
    { MPU_REGION_ENABLE,  MPU_VIOC_ADDR,        MPU_REGION_4MB,     MPU_STRONG_ORDERED_SHARED | MPU_PRIV_RW_USER_RW }
};

static void MPU_EnableRegion
(
    uint32                              uiRegionNumber,
    uint32                              uiAddr,
    uint32                              uiSize,
    uint32                              uiAttr
);

static void MPU_EnableRegion
(
    uint32                              uiRegionNumber,
    uint32                              uiAddr,
    uint32                              uiSize,
    uint32                              uiAttr
)
{
    uint32  uiAddress;
    uint32  uiRegionsize;

    uiAddress       = uiAddr;
    uiRegionsize    = 0;

    if ( uiRegionNumber < MPU_MAX_REGION ) //QAC : Dereference of an invalid pointer value of uiRegionNumber
    {
        MPU_WriteControlRegisterConfigurationData( MPU_ARM_CP15_MPU_MEMORY_REG_NUMBER, uiRegionNumber );
        uiRegionsize    = uiSize;
        uiRegionsize    <<= 1;
        uiRegionsize    |= MPU_REGION_EN;
        ARM_DSB();

        MPU_WriteControlRegisterConfigurationData( MPU_ARM_CP15_MPU_REG_BASEADDR,    uiAddress );
        MPU_WriteControlRegisterConfigurationData( MPU_ARM_CP15_MPU_REG_ACCESS_CTRL, uiAttr );
        MPU_WriteControlRegisterConfigurationData( MPU_ARM_CP15_MPU_REG_SIZE_EN,     uiRegionsize );

        ARM_DSB();
    }
    else
    {
        // Error Print
    }
}

void MPU_Init
(
    void
)
{
    uint32      uiIndex;
    for( uiIndex = 0; uiIndex < ( sizeof( sMPUTable ) / sizeof( sMPUTable[ 0 ] ) ) ; uiIndex++ )
    {
        MPU_EnableRegion( uiIndex, sMPUTable[ uiIndex ].uiRegionBase, sMPUTable[ uiIndex ].uiRegionSize, sMPUTable[ uiIndex ].uiRegionAttr);
    }
}

uint32 MPU_GetDMABaseAddress
(
    void
)
{
    // 1. Ths "__nc_dmastart" value already contains the physical base(0xC0000000). Please reference the linker(your) script file(.ld)
    // 2. The dma address is valid only physical address(memory and peripheral point of view).

    uint32  uiDMAStart;

    uiDMAStart  = ( uint32 )( &__nc_dmastart ); // Physical Offset for 512KB SRAM for execution zero base

    return uiDMAStart;
}

void MPU_SetRegion
(
    uint32 uiRegionAddr,
    uint32 uiRegionAttr
)
{
    uint32      uiIndex;

    for( uiIndex = 0; uiIndex < ( sizeof( sMPUTable ) / sizeof( sMPUTable[ 0 ] ) ) ; uiIndex++ )
    {
        if(sMPUTable[ uiIndex ].uiRegionBase == uiRegionAddr)
        {
            MPU_EnableRegion( uiIndex, sMPUTable[ uiIndex ].uiRegionBase, sMPUTable[ uiIndex ].uiRegionSize, uiRegionAttr);
        }
    }
}

void MPU_Enable
(
    void
)
{
    #if defined(__GNUC__)
    __asm volatile (
            "mrc p15, 0, r0, c1, c0, 0 \n"  // Read CP15 SCTLR
            "orr r0, r0, #(0x1<<0) \n"      // Enable MPU
            "orr r0, r0, #(0x1<<2) \n"      // Enable D-Cache
            "orr r0, r0, #(0x1<<12) \n"     // Enable I-Cache
            "dsb \n"
            "mcr p15, 0, r0, c1, c0, 0 \n"  // Write CP15 SCTLR
            "isb \n"
    );
#else //Green Hills case
    #pragma asm
    mrc p15, 0, r0, c1, c0, 0       ; Read CP15 SCTLR
	orr r0, r0, #(0x1<<0)           ; Enable MPU
	orr r0, r0, #(0x1<<2)           ; Enable D-Cache
	orr r0, r0, #(0x1<<12)          ; Enable I-Cache
	dsb
	mcr p15, 0, r0, c1, c0, 0       ; Write CP15 SCTLR
	isb
    #pragma endasm
#endif
}

void MPU_Disable
(
    void
)
{
#if defined(__GNUC__)
    __asm volatile (
            "mrc p15, 0, r0, c1, c0, 0 \n"  // Read CP15 SCTLR
            "bic r0, r0, #(0x1<<0) \n"      // Disable MPU
            "bic r0, r0, #(0x1<<2) \n"      // Disable D-Cache
            "bic r0, r0, #(0x1<<12) \n"     // Disable I-Cache
            "dsb \n"
            "mcr p15, 0, r0, c1, c0, 0 \n"  // Write CP15 SCTLR
            "isb \n"

            "mov r0, #0 \n"
            "dsb \n"
            "mcr p15, 0, r0, c7, c5, 0 \n"  // Invalidate I-Cache All
            "mcr p15, 0, r0, c15, c5, 0 \n" // Invalidate D-Cache All
            "isb \n"
    );
#else //Green Hills case
    #pragma asm
    mrc p15, 0, r0, c1, c0, 0       ; Read CP15 SCTLR
	bic r0, r0, #(0x1<<0)           ; Disable MPU
	bic r0, r0, #(0x1<<2)           ; Disable D-Cache
	bic r0, r0, #(0x1<<12)          ; Disable I-Cache
	dsb
	mcr p15, 0, r0, c1, c0, 0       ; Write CP15 SCTLR
	isb

    mov r0, #0
	dsb
	mcr p15, 0, r0, c7, c5, 0       ; Invalidate I-Cache All
	mcr p15, 0, r0, c15, c5, 0      ; Invalidate D-Cache All
	isb
    #pragma endasm
#endif
}

#endif

