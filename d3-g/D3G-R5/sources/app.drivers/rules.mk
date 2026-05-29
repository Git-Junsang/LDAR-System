###################################################################################################
#
#   FileName : ruls.mk
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

MCU_BSP_APP_DRIVERS_PATH := $(MCU_BSP_BUILD_CURDIR)

# IPC Application Driver
include $(MCU_BSP_APP_DRIVERS_PATH)/ipc/rules.mk

# Local Interconnect Network
include $(MCU_BSP_APP_DRIVERS_PATH)/lin/rules.mk

# Safe Display Manager
include $(MCU_BSP_APP_DRIVERS_PATH)/sdm/rules.mk

# HSM Manager
include $(MCU_BSP_APP_DRIVERS_PATH)/hsm/rules.mk

# Trust RVC
ifeq ($(MCU_BSP_BUILD_FLAGS_T_RVC_INCLUDED), 1)
    include $(MCU_BSP_APP_DRIVERS_PATH)/trvc/rules.mk
endif

