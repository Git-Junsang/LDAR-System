# SPDX-License-Identifier: Apache-2.0
#
# LDAR VCP-G Phase 1 (GPIO scope)

MCU_BSP_APP_LDAR_VCP_PATH := $(MCU_BSP_BUILD_CURDIR)

COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_LDAR_VCP=1

VPATH    += $(MCU_BSP_APP_LDAR_VCP_PATH)
INCLUDES += -I$(MCU_BSP_APP_LDAR_VCP_PATH)

SRCS += ldar_app.c
SRCS += joystick_sw.c
SRCS += motor_dir.c
SRCS += turn_signal.c
