# SPDX-License-Identifier: Apache-2.0
#
# LDAR VCP-G Phase 1 (GPIO + ADC + PWM + CAN)

MCU_BSP_APP_LDAR_VCP_PATH := $(MCU_BSP_BUILD_CURDIR)

COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_LDAR_VCP=1

VPATH    += $(MCU_BSP_APP_LDAR_VCP_PATH)
INCLUDES += -I$(MCU_BSP_APP_LDAR_VCP_PATH)

SRCS += ldar_app.c
SRCS += joystick_sw.c
SRCS += joystick_adc.c
SRCS += motor_dir.c
SRCS += motor_pwm.c
SRCS += servo_pwm.c
SRCS += turn_signal.c
SRCS += turn_can.c
SRCS += turn_led.c
SRCS += override.c
SRCS += buzzer.c
SRCS += pwm_util.c
