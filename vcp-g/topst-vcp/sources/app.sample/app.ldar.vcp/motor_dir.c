// SPDX-License-Identifier: Apache-2.0

#include "motor_dir.h"
#include "ldar_pins.h"
#include <gpio.h>

void MotorDir_Init(void)
{
    GPIO_Config(LDAR_PIN_MOTOR_IN1, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Config(LDAR_PIN_MOTOR_IN2, GPIO_FUNC(0) | GPIO_OUTPUT);
    GPIO_Set(LDAR_PIN_MOTOR_IN1, 0);
    GPIO_Set(LDAR_PIN_MOTOR_IN2, 0);
}

void MotorDir_Set(MotorDir_t dir)
{
    switch (dir) {
        case MOTOR_DIR_FORWARD:
            GPIO_Set(LDAR_PIN_MOTOR_IN1, 1);
            GPIO_Set(LDAR_PIN_MOTOR_IN2, 0);
            break;
        case MOTOR_DIR_REVERSE:
            GPIO_Set(LDAR_PIN_MOTOR_IN1, 0);
            GPIO_Set(LDAR_PIN_MOTOR_IN2, 1);
            break;
        case MOTOR_DIR_BRAKE:
            GPIO_Set(LDAR_PIN_MOTOR_IN1, 1);
            GPIO_Set(LDAR_PIN_MOTOR_IN2, 1);
            break;
        case MOTOR_DIR_STOP:
        default:
            GPIO_Set(LDAR_PIN_MOTOR_IN1, 0);
            GPIO_Set(LDAR_PIN_MOTOR_IN2, 0);
            break;
    }
}
