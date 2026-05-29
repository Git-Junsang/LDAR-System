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

MCU_BSP_DEV_DRIVERS_PATH := $(MCU_BSP_BUILD_CURDIR)

# Ananlog-to-Digital Converter
include $(MCU_BSP_DEV_DRIVERS_PATH)/adc/rules.mk

# Controller Area Network
include $(MCU_BSP_DEV_DRIVERS_PATH)/can/rules.mk

# CLOCK Control Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/clock/rules.mk

# Common
include $(MCU_BSP_DEV_DRIVERS_PATH)/common/rules.mk

# Fault Management Unut
include $(MCU_BSP_DEV_DRIVERS_PATH)/fmu/rules.mk

# Direct Memory Access
include $(MCU_BSP_DEV_DRIVERS_PATH)/gdma/rules.mk

# General Interrupt Controller
include $(MCU_BSP_DEV_DRIVERS_PATH)/gic/rules.mk

# General Purpose I/O
include $(MCU_BSP_DEV_DRIVERS_PATH)/gpio/rules.mk

# Inter-Integrated Circuit
include $(MCU_BSP_DEV_DRIVERS_PATH)/i2c/rules.mk

# Input Capture Time Counter
include $(MCU_BSP_DEV_DRIVERS_PATH)/ictc/rules.mk

# Low-Voltage Differential Signaling
#include $(MCU_BSP_DEV_DRIVERS_PATH)/lvds/rules.mk

# Mail-Box
include $(MCU_BSP_DEV_DRIVERS_PATH)/mailbox/rules.mk

# Memory Protection Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/mpu/rules.mk

# Pulse Density Modulator
include $(MCU_BSP_DEV_DRIVERS_PATH)/pdm/rules.mk

# Power Management I/O
include $(MCU_BSP_DEV_DRIVERS_PATH)/pmio/rules.mk

# Serial Peripheral Interface
include $(MCU_BSP_DEV_DRIVERS_PATH)/gpsb/rules.mk

# Timer
include $(MCU_BSP_DEV_DRIVERS_PATH)/timer/rules.mk

# Unuversal Asynchronous Receiver/Transimitter
include $(MCU_BSP_DEV_DRIVERS_PATH)/uart/rules.mk

# Ultra Direct Memory Access
include $(MCU_BSP_DEV_DRIVERS_PATH)/udma/rules.mk

# Watchdog
include $(MCU_BSP_DEV_DRIVERS_PATH)/watchdog/rules.mk

# Default Slave Error
include $(MCU_BSP_DEV_DRIVERS_PATH)/dse/rules.mk

# Memory ID Filter(TZC400)
include $(MCU_BSP_DEV_DRIVERS_PATH)/midf/rules.mk

# Power Manager Unit
include $(MCU_BSP_DEV_DRIVERS_PATH)/pmu/rules.mk