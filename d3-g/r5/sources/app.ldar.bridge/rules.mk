# SPDX-License-Identifier: Apache-2.0
#
# LDAR R5 CAN→IPC bridge (Phase 1 T1.4).
# Drop this dir into D3-G R5 BSP at sources/app.sample/app.ldar.bridge/ and
# add `include $(MCU_BSP_APP_SAMPLE_PATH)/app.ldar.bridge/rules.mk` to
# app.sample/rules.mk (unconditional, like LDAR VCP-G).

MCU_BSP_APP_LDAR_BRIDGE_PATH := $(MCU_BSP_BUILD_CURDIR)

COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_LDAR_BRIDGE=1

VPATH    += $(MCU_BSP_APP_LDAR_BRIDGE_PATH)
INCLUDES += -I$(MCU_BSP_APP_LDAR_BRIDGE_PATH)
INCLUDES += -I$(MCU_BSP_APP_LDAR_BRIDGE_PATH)/../../../../shared

SRCS += ldar_bridge.c
SRCS += ldar_downstream.c
