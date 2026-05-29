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

MCU_BSP_APP_SAMPLE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Main
include $(MCU_BSP_APP_SAMPLE_PATH)/app.base/rules.mk

# CAN Demo Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.can.demo/rules.mk

# Console Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.console/rules.mk

# FWUG Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.fwug/rules.mk

# Key Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.key/rules.mk

# System Monitoring Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.system.monitoring/rules.mk

# Boot Control Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.boot.ctrl/rules.mk

# Trust RVC application
ifeq ($(MCU_BSP_BUILD_FLAGS_T_RVC_INCLUDED), 1)
    include $(MCU_BSP_APP_SAMPLE_PATH)/app.trust.rvc/rules.mk
endif

# Reset Demo Application
include $(MCU_BSP_APP_SAMPLE_PATH)/app.reset.demo/rules.mk

# ADC Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.adc/rules.mk

# DSE Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.dse/rules.mk

# FMU Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.fmu/rules.mk

# GDMA Sample Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gdma/rules.mk

# GIC Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gic/rules.mk

# GPIO Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gpio/rules.mk

# GPSB Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.gpsb/rules.mk

# HSM Sample Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.hsm/rules.mk

# I2C Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.i2c/rules.mk

# ICTC Sample Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.ictc/rules.mk

# LIN Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.lin/rules.mk

# PDM Sample Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.pdm/rules.mk

# PMIO Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.pmio/rules.mk

# Timer Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.timer/rules.mk

# Watchdog Verification Application
include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.wdt/rules.mk

# UART Verification  Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.uart/rules.mk

# Writelock Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.writelock/rules.mk

# CPU Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.cpu/rules.mk

# CAN Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.can/rules.mk

# IMC Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.imc/rules.mk

# MIDF Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.midf/rules.mk

# MC TOP Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.top/rules.mk

# SM CRD Test Application
#include $(MCU_BSP_APP_SAMPLE_PATH)/test.app.crd/rules.mk

# MP Tool
include $(MCU_BSP_APP_SAMPLE_PATH)/app.mptool/rules.mk

# AUTOMOTIVE_IVI_CLUSTER_DEMO
include $(MCU_BSP_APP_SAMPLE_PATH)/app.ivi.cluster.demo/rules.mk