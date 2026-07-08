// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_MOTOR_DIR_H
#define LDAR_MOTOR_DIR_H

typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_FORWARD,
    MOTOR_DIR_REVERSE,
    MOTOR_DIR_BRAKE,
} MotorDir_t;

void MotorDir_Init(void);
void MotorDir_Set(MotorDir_t dir);

#endif
