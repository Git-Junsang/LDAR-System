 /*******************************************************************************
  
*   Copyright (c) Telechips Inc.
 
 
*   TCC Version 1.0
  
This source code contains confidential information of Telechips.
 
Any unauthorized use without a written permission of Telechips including not
limited to re-distribution in source or binary form is strictly prohibited.
 
This source code is provided "AS IS" and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without
limitation, any warranty of merchantability, fitness for a particular purpose
or non-infringement of any patent, copyright or other third party intellectual
property right.
No warranty is made, express or implied, regarding the information's accuracy,
completeness, or performance.
 
In no event shall Telechips be liable for any claim, damages or other
liability arising from, out of or in connection with this source code or
the use in the source code.
  
This source code is provided subject to the terms of a Mutual Non-Disclosure
Agreement between Telechips and Company.
*
*******************************************************************************/
/*---------------------------------------------------------------------------*/
/*- NAME: Test firmware Linker Script File for RAM Workspace                 */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*- File source          : mC100_RAM.ld                                      */
/*- Object               : Linker Script File for RAM Workspace              */
/*- created              : Jongin, Cha                                        */
/*---------------------------------------------------------------------------*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

/* Memory Definitions */
__VECTOR_START = 0x07000000;

DMA_NC_SIZE = 0x2000;

TESTCODE_SIZE =   0x2000;

MEMORY
{
	/*=========================================================
	  Region for Executing at SRAM with cacheable : 256KB
	  =========================================================*/
	  SRAM					(rwx)	: ORIGIN = 0x07000000, LENGTH = (0x10000 - 0x680 - DMA_NC_SIZE) /* total(0x80000) -  SRAM_ARM_STACK(0x680) - NC_SRAM(0x2000 or...config) */

	/*=========================================================
	  Region for ARM STACK area : 0x680Byte
	  =========================================================*/
	  SRAM_ARM_TEST (rwx)	: ORIGIN = 0x07000000 + (0x10000 - 0x680 - DMA_NC_SIZE - TESTCODE_SIZE), LENGTH = TESTCODE_SIZE
	  SRAM_ARM_STACK (rwx)	: ORIGIN = 0x07000000 + (0x10000 - 0x680 - DMA_NC_SIZE), LENGTH = 0x680 
	  
	/*=========================================================
	  Region for Executing at SRAM with non-cacheable : 8KB, The dma address is valid only physical address(0xC0000000), it can not use mirrored mem address(0x06000000)
	  =========================================================*/
	  NC_SRAM					(rwx)	: ORIGIN = 0xC0000000+0x80000-DMA_NC_SIZE, LENGTH = DMA_NC_SIZE

	/*=====================================================
	  Region for Executing at NOR Flash with cacheable
	  =====================================================*/
	NOR					(rx)	: ORIGIN = 0x08200000, LENGTH = 0x00200000
}

/* Section Definitions */
SECTIONS
{
  /* first section is .text which is used for code */
  .text :
  {
  	__TEXT_START__ = .;
    *startup.o (.text)
    *(.text)
    __TEXT_END__ = .;
    . = ALIGN(4);
  } > NOR

  /* .rodata section which is used for read-only data (constants) */
  .rodata :
  {
    __RODATA_START__ = .;


    *(.rodata)
    *(.rodata.*)
    *(.const)
    *(.const.*)
    . = ALIGN(0x20);
    __RODATA_END__ = .;
  } > NOR

  /* .vector section which is used for vector table */
  .vector :
  {
  	__VECTOR_START_LOAD = LOADADDR (.vector);  
  	__VECTOR_START__ = .;
    *vector.o (.vector .text .rodata .rodata*)
    . = ALIGN(0x20);
    __VECTOR_END__ = .;
  } > SRAM AT > NOR


  .codeonsram :
  {
  	__TESTCODE_START_LOAD = LOADADDR (.codeonsram);  
  	__TESTCODE_SIZE = SIZEOF (.codeonsram);  
  	__TESTCODE_RAM_START__ = .;
    *sfmc_test.o (.text) 
    *pmu_sm.o (.text) 
    . = ALIGN(0x20);
  } > SRAM_ARM_TEST AT > NOR



  /* .data section which is used for initialized data */
  .data :
	AT ( LOADADDR (.codeonsram) + SIZEOF (.codeonsram) )  
  {
  	__DATA_START_LOAD = LOADADDR (.data);
  	__DATA_START__ = .;
    *(.data)
    *(.data.*)
    . = ALIGN(0x20);
    __DATA_END__ = .;
  } > SRAM
  
  /* .bss section which is used for uninitialized data */
  .bss (NOLOAD):
  {
    . = ALIGN(0x20);
    __BSS_START__ = .;
    *(.bss)
    *(.bss.*)
    . = ALIGN(0x20);
    __BSS_END__ = .;
  } > SRAM

/* arm_stack */
 .arm_stack (NOLOAD):
 {
 	  __stack_start = .;
 	  . += 0x680;
   	_stack_end = .;
 } > SRAM_ARM_STACK

 /* .nc_dma section which is used for non-cachable dma buffer */
 .nc_dma (NOLOAD):
  AT ( ADDR (.bss) + SIZEOF (.bss) )
 {
 	  __nc_dmastart = .;
 	  . += DMA_NC_SIZE;
   	_end_of_nc_dma = .;
 } > NC_SRAM

  _end = .;
  __end = _end;
  PROVIDE(end = .);
  /DISCARD/ : { *(.ARM.exidx* .ARM.extab* .glue* .vfp11_veneer .v4_bx .iplt .igot.* .rel.*) }
}
