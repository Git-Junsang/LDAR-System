###################################################################################################
#
#   FileName : Makefile
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################
#
#   TCC Version 1.0
#
#   This source code contains confidential information of Telechips.
#
#   Any unauthorized use without a written permission of Telechips including not limited to
#   re-distribution in source or binary form is strictly prohibited.
#
#   This source code is provided "AS IS" and nothing contained in this source code shall constitute
#   any express or implied warranty of any kind, including without limitation, any warranty of
#   merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
#   or other third party intellectual property right. No warranty is made, express or implied,
#   regarding the information's accuracy,completeness, or performance.
#
#   In no event shall Telechips be liable for any claim, damages or other liability arising from,
#   out of or in connection with this source code or the use in the source code.
#
#   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
#   Telechips and Company.
#   This source code is provided "AS IS" and nothing contained in this source code shall constitute
#   any express or implied warranty of any kind, including without limitation, any warranty
#   (of merchantability, fitness for a particular purpose or non-infringement of any patent,
#   copyright or other third party intellectual property right. No warranty is made, express or
#   implied, regarding the information's accuracy, completeness, or performance.
#   In no event shall Telechips be liable for any claim, damages or other liability arising from,
#   out of or in connection with this source code or the use in the source code.
#   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
#   between Telechips and Company.
#
###################################################################################################

# Find the local dir of the make file
MCU_BSP_BUILD_CURDIR        ?= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Paths for Build
MCU_BSP_TOP_PATH                        ?= $(shell cd ../../../../;pwd)
MCU_BSP_DOCUMENTS_PATH                  ?= $(MCU_BSP_TOP_PATH)/documents
MCU_BSP_SCRIPTS_PATH                    ?= $(MCU_BSP_TOP_PATH)/scripts
MCU_BSP_SOURCES_PATH                    ?= $(MCU_BSP_TOP_PATH)/sources
MCU_BSP_SOURCES_BUILD_PATH              ?= $(MCU_BSP_SOURCES_PATH)/build
MCU_BSP_SOURCES_BUILD_TCC805x_PATH      ?= $(MCU_BSP_SOURCES_BUILD_PATH)/tcc805x
MCU_BSP_SOURCES_BUILD_TCC805x_GCC_PATH  ?= $(MCU_BSP_SOURCES_BUILD_TCC805x_PATH)/gcc
MCU_BSP_TOOLS_PATH                      ?= $(MCU_BSP_TOP_PATH)/tools

# Build Flags
MCU_BSP_BUILD_FLAGS_CHIPSET_TCC805x           ?= 1
MCU_BSP_BUILD_FLAGS_CHIPSET_TCC8059           ?= 0

MCU_BSP_BUILD_FLAGS_SOC_REVISION_ES           ?= 0

MCU_BSP_BUILD_FLAGS_VERBOSE                   ?= 0
MCU_BSP_BUILD_FLAGS_DEBUG                     ?= 1

MCU_BSP_BUILD_FLAGS_TARGET_OS_FREERTOS        ?= 1
MCU_BSP_BUILD_FLAGS_TARGET_OS_UCOS            ?= 0

MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_NONE           ?= 1
MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_D16            ?= 0
MCU_BSP_BUILD_FLAGS_SNOR_BOOT                 ?= 1

MCU_BSP_BUILD_FLAGS_SELECT_MEMORY_SRAM_0      ?= 1
MCU_BSP_BUILD_FLAGS_SELECT_MEMORY_SRAM_1      ?= 0

MCU_BSP_BUILD_FLAGS_T_RVC_INCLUDED            ?= 0

# MP Tool
MCU_BSP_BUILD_FLAGS_MP_TOOL_INCLUDED          ?= 0

# AUTOMOTIVE_IVI_CLUSTER_DEMO 
MCU_BSP_BUILD_FLAGS_AUTOMOTIVE_IVI_CLUSTER_DEMO_INCLUDED          ?= 0

# EDUCATION_CAN_DEMO
MCU_BSP_BUILD_FLAGS_EDUCATION_CAN_DEMO_INCLUDED     ?= 0

# SOONGSIL_UNI_CAN_DEMO
MCU_BSP_BUILD_FLAGS_SOONGSIL_UNI_CAN_DEMO_INCLUDED  ?= 1

# Build Environment
MCU_BSP_TOOLCHAIN_PATH   ?= /opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi
MCU_BSP_TOOLCHAIN_PREFIX ?= $(MCU_BSP_TOOLCHAIN_PATH)/bin/arm-eabi-

LIBCDIR   := $(MCU_BSP_TOOLCHAIN_PATH)/arm-eabi/libc/usr/lib
LIBGCCDIR := $(MCU_BSP_TOOLCHAIN_PATH)/lib/gcc/arm-eabi/7.2.1
LIBGCC    := -L$(LIBCDIR) -L$(LIBGCCDIR) -lc -lm -lgcc
AS        := $(MCU_BSP_TOOLCHAIN_PREFIX)as
CC        := $(MCU_BSP_TOOLCHAIN_PREFIX)gcc
LD        := $(MCU_BSP_TOOLCHAIN_PREFIX)ld
SIZE      := $(MCU_BSP_TOOLCHAIN_PREFIX)size
OBJCOPY   := $(MCU_BSP_TOOLCHAIN_PREFIX)objcopy
OBJDUMP   := $(MCU_BSP_TOOLCHAIN_PREFIX)objdump
INCLUDES  += -I$(MCU_BSP_BUILD_ROOT_PATH)
INCLUDES  += -Iinclude
INCLUDES  += -I$(LIBCDIR)/../include
INCLUDES  += -I$(LIBGCCDIR)/include


ifeq ($(MCU_BSP_BUILD_FLAGS_CHIPSET_TCC805x), 1)
    MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8050_EVM ?= 1
else ifeq ($(MCU_BSP_BUILD_FLAGS_CHIPSET_TCC8059), 1)
    MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8059_EVM ?= 1
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8050_EVM), 1)
    MCU_BSP_BUILD_FLAGS_BOARD_VER_TCC8050_EVM_0_1 ?= 1
else ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8059_EVM), 1)
    MCU_BSP_BUILD_FLAGS_BOARD_VER_TCC8059_EVM_0_1 ?= 1
endif

ifneq ($(MCU_BSP_BUILD_FLAGS_VERBOSE), 1)
    MCU_BSP_BUILD_QUIET ?= @
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_CHIPSET_TCC805x), 1)
    MCU_BSP_CHIPSET_NAME ?= tcc805x
else ifeq ($(MCU_BSP_BUILD_FLAGS_CHIPSET_TCC8059), 1)
    MCU_BSP_CHIPSET_NAME ?= tcc8059
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEBUG), 1)
    MCU_BSP_BUILD_CONFIG ?= debug
else
    MCU_BSP_BUILD_CONFIG ?= release
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_TARGET_OS_FREERTOS), 1)
    MCU_BSP_TARGET_OS ?= freertos
else ifeq ($(MCU_BSP_BUILD_FLAGS_TARGET_OS_UCOS), 1)
    MCU_BSP_TARGET_OS ?= ucos
else
    $(error Please select corrent OS.)
endif

# TCC803x VFP soupport only 16 mode, not support 32 mode
ifeq ($(MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_NONE), 1)
    MCU_BSP_OS_CPU_VFP ?= none
else ifeq ($(MCU_BSP_BUILD_FLAGS_OS_CPU_VFP_D16), 1)
    MCU_BSP_OS_CPU_VFP ?= d16
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_SOC_REVISION_ES), 1)
    MCU_BSP_SOC_REVISION ?= es
else
    MCU_BSP_SOC_REVISION ?= cs
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_SELECT_MEMORY_SRAM_0), 1)
    MCU_BSP_SELECT_MEMORY ?= sram0
else ifeq ($(MCU_BSP_BUILD_FLAGS_SELECT_MEMORY_SRAM_1), 1)
    MCU_BSP_SELECT_MEMORY ?= sram1
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8050_EVM), 1)
    MCU_BSP_CONFIG_BOARD_NAME ?= TCC8050_EVM
else ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8059_EVM), 1)
    MCU_BSP_CONFIG_BOARD_NAME ?= TCC8059_EVM
endif

MCU_BSP_ARM_CORE_NAME       ?= ARM_CR5
MCU_BSP_CHIPSET_FAMILY_NAME ?= tcc805x
MCU_BSP_PROJECT_NAME        ?= boot

MCU_BSP_BUILD_ROOT_PATH     ?= $(MCU_BSP_SOURCES_BUILD_TCC805x_GCC_PATH)/$(MCU_BSP_CHIPSET_NAME)-$(MCU_BSP_TARGET_OS)-$(MCU_BSP_BUILD_CONFIG)
MCU_BSP_BUILD_OBJS_PATH     ?= $(MCU_BSP_BUILD_ROOT_PATH)/obj

COMMON_FLAGS += -O0
COMMON_FLAGS += -finline
COMMON_FLAGS += -fno-builtin
COMMON_FLAGS += -fno-common
COMMON_FLAGS += -fno-gcse
COMMON_FLAGS += -fno-strict-aliasing
COMMON_FLAGS += -marm
COMMON_FLAGS += -mcpu=cortex-r5
COMMON_FLAGS += -nostdinc
COMMON_FLAGS += -g
COMMON_FLAGS += -gdwarf-2
COMMON_FLAGS += -pipe
COMMON_FLAGS += -D__GNU_C__
#COMMON_FLAGS += -DDSE_DATAABORT
#COMMON_FLAGS += -DSUBSYS_USERMODETEST
#COMMON_FLAGS += -DGIC_FIQTEST
COMMON_FLAGS += -DTCC805x
COMMON_FLAGS += -DDMA_NC_SIZE=0x4000
COMMON_FLAGS += -DFMU_DRIVER

ifeq ($(MCU_BSP_OS_CPU_VFP), d16)
    COMMON_FLAGS += -mfpu=vfpv3-d16
    COMMON_FLAGS += -mfloat-abi=softfp
    COMMON_FLAGS += -DFPU_USE
    COMMON_FLAGS += -DFPU_D16
else ifeq ($(MCU_BSP_OS_CPU_VFP), none)
    COMMON_FLAGS += -mfloat-abi=soft
endif

ifeq ($(MCU_BSP_SOC_REVISION), es)
    COMMON_FLAGS += -DMCU_BSP_SOC_REV_TCC805x_ES
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_CHIPSET_TCC805x), 1)
    COMMON_FLAGS += -DMCU_BSP_CHIPSET_TCC805x
else ifeq ($(MCU_BSP_BUILD_FLAGS_CHIPSET_TCC8059), 1)
    COMMON_FLAGS += -DMCU_BSP_CHIPSET_TCC8059
endif

ifeq ($(MCU_BSP_TARGET_OS), freertos)
    COMMON_FLAGS += -DOS_FREERTOS
else ifeq ($(MCU_BSP_TARGET_OS), ucos)
    COMMON_FLAGS += -DOS_UCOS
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_SNOR_BOOT), 1)
    COMMON_FLAGS += -DSNOR_STANDALONE
endif

ifeq ($(MCU_BSP_SELECT_MEMORY), sram1)
    COMMON_FLAGS += -DMEMORY_SRAM1
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_DEBUG), 1)
    COMMON_FLAGS += -DDEBUG_ENABLE=1
else
    COMMON_FLAGS += -DDEBUG_ENABLE=0
endif

ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8050_EVM), 1)
    ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_VER_TCC8050_EVM_0_1), 1)
        COMMON_FLAGS += -DMCU_BSP_BOARD_VER_TCC8050_EVM_0_1
    endif
else ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_NAME_TCC8059_EVM), 1)
    ifeq ($(MCU_BSP_BUILD_FLAGS_BOARD_VER_TCC8059_EVM_0_1), 1)
        COMMON_FLAGS += -DMCU_BSP_BOARD_VER_TCC8059_EVM_0_1
    endif
endif

# MP Tool
ifeq ($(MCU_BSP_BUILD_FLAGS_MP_TOOL_INCLUDED), 1)
    COMMON_FLAGS += -DMPTOOL_EN=1
else
    COMMON_FLAGS += -DMPTOOL_EN=0
endif

# AUTOMOTIVE_IVI_CLUSTER_DEMO
ifeq ($(MCU_BSP_BUILD_FLAGS_AUTOMOTIVE_IVI_CLUSTER_DEMO_INCLUDED), 1)
    COMMON_FLAGS += -DAUTOMOTIVE_IVI_CLUSTER_DEMO_EN
endif

# EDUCATION_CAN_DEMO
ifeq ($(MCU_BSP_BUILD_FLAGS_EDUCATION_CAN_DEMO_INCLUDED), 1)
    COMMON_FLAGS += -DEDUCATION_CAN_DEMO_EN
endif

# SOONGSIL_UNI_CAN_DEMO
ifeq ($(MCU_BSP_BUILD_FLAGS_SOONGSIL_UNI_CAN_DEMO_INCLUDED), 1)
    COMMON_FLAGS += -DEDUCATION_CAN_DEMO_EN
    COMMON_FLAGS += -DSOONGSIL_UNI_CAN_DEMO_EN
endif

COMMON_WARNINGS += -W
COMMON_WARNINGS += -Wall
COMMON_WARNINGS += -Wempty-body
COMMON_WARNINGS += -Werror
COMMON_WARNINGS += -Wextra
COMMON_WARNINGS += -Wformat
COMMON_WARNINGS += -Wmissing-braces
COMMON_WARNINGS += -Wmissing-field-initializers
COMMON_WARNINGS += -Wmissing-prototypes
COMMON_WARNINGS += -Wno-cast-align
COMMON_WARNINGS += -Wno-trigraphs
COMMON_WARNINGS += -Wno-unused-function
COMMON_WARNINGS += -Wno-unused-parameter
COMMON_WARNINGS += -Wno-unused-result
COMMON_WARNINGS += -Wparentheses
COMMON_WARNINGS += -Wpointer-sign
COMMON_WARNINGS += -Wshadow
COMMON_WARNINGS += -Wsign-compare
COMMON_WARNINGS += -Wstrict-prototypes
COMMON_WARNINGS += -Wswitch
#COMMON_WARNINGS += -Wundef
COMMON_WARNINGS += -Wuninitialized
COMMON_WARNINGS += -Wunknown-pragmas
COMMON_WARNINGS += -Wunused-label
COMMON_WARNINGS += -Wunused-value
COMMON_WARNINGS += -Wunused-variable
CWARNS          += $(COMMON_WARNINGS)
CFLAGS          += $(INCLUDES) $(COMMON_FLAGS) $(CWARNS)
LDFLAGS         += --cref
LDFLAGS         += -Bstatic
LDFLAGS         += -nostdlib
LDFLAGS         += -nostartfiles
LDFLAGS         += -p
LDFLAGS         += -EL
LDFLAGS         += -Map $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME).map

ifeq ($(MCU_BSP_BUILD_FLAGS_SNOR_BOOT), 1)
    ifeq ($(MCU_BSP_SOC_REVISION), cs)
        ifeq ($(MCU_BSP_SELECT_MEMORY), sram0)
            LDFLAGS += -Tlinker.cs.snor.sram0.ld
        else ifeq ($(MCU_BSP_SELECT_MEMORY), sram1)
            LDFLAGS += -Tlinker.cs.snor.sram1.ld
        endif
    else ifeq ($(MCU_BSP_SOC_REVISION), es)
        ifeq ($(MCU_BSP_SELECT_MEMORY), sram0)
            LDFLAGS += -Tlinker.es.snor.sram0.ld
        else ifeq ($(MCU_BSP_SELECT_MEMORY), sram1)
            LDFLAGS += -Tlinker.es.snor.sram1.ld
        endif
    endif
else
    ifeq ($(MCU_BSP_SOC_REVISION), cs)
        LDFLAGS += -Tlinker.cs.sram.ld
    else ifeq ($(MCU_BSP_SOC_REVISION), es)
        LDFLAGS += -Tlinker.es.sram.ld
    endif
endif

OBJCOPYFLAGS = -O binary -R .note -R .comment -S
OBJS         = $(patsubst %.c,$(MCU_BSP_BUILD_OBJS_PATH)/%.o,$(SRCS)) $(patsubst %.S,$(MCU_BSP_BUILD_OBJS_PATH)/%.o,$(ASMSRCS))

# compile modules include
include $(MCU_BSP_SOURCES_PATH)/rules.mk

.PHONY : all
all: $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME).bin

$(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME).bin : $(OBJS)
	$(MCU_BSP_BUILD_QUIET)mkdir -p $(MCU_BSP_BUILD_ROOT_PATH)
	$(MCU_BSP_BUILD_QUIET)$(LD) $(LDFLAGS) -o $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME) $(OBJS) $(LIBGCC)
	$(MCU_BSP_BUILD_QUIET)$(OBJCOPY) $(OBJCOPYFLAGS) $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME) $@
	@echo --------------Make TC Image Format-----------------------------
	$(MCU_BSP_BUILD_QUIET)chmod 755 mkimg.sh
	./mkimg.sh TOOL_PATH=$(MCU_BSP_TOOLS_PATH) INPUT_PATH=$(MCU_BSP_BUILD_ROOT_PATH) OUTPUT_PATH=$(MCU_BSP_BUILD_ROOT_PATH) TARGET_ADDRESS=0x00000000 IMAGE_VERSION=0.0.0
	@echo --------------Make ROM Image Format----------------------------
	$(MCU_BSP_BUILD_QUIET)$(SIZE) $(MCU_BSP_BUILD_ROOT_PATH)/$(MCU_BSP_PROJECT_NAME)
	$(MCU_BSP_BUILD_QUIET)chmod 755 mkrom.cs.sh
	./mkrom.cs.sh BOARD_NAME=$(MCU_BSP_CONFIG_BOARD_NAME) OUTPUT_PATH=$(MCU_BSP_BUILD_ROOT_PATH)

	@echo  Finished ...

.PHONY : clean
clean:
	$(MCU_BSP_BUILD_QUIET)rm -rf $(MCU_BSP_BUILD_ROOT_PATH)/../../make_utility/tcc8050_snor.cs.rom
	$(MCU_BSP_BUILD_QUIET)rm -rf $(MCU_BSP_BUILD_ROOT_PATH) $@

$(OBJS): | $(MCU_BSP_BUILD_OBJS_PATH)

$(MCU_BSP_BUILD_OBJS_PATH):
	$(MCU_BSP_BUILD_QUIET)mkdir -p $@

# General rules
$(MCU_BSP_BUILD_OBJS_PATH)/%.o : %.S
	@echo compile: $<
	$(MCU_BSP_BUILD_QUIET)$(CC) -c $(CFLAGS) -o $@ $<

$(MCU_BSP_BUILD_OBJS_PATH)/%.o : %.c
	@echo compile: $<
	$(MCU_BSP_BUILD_QUIET)$(CC) -c $(CFLAGS) -o $@ $<

