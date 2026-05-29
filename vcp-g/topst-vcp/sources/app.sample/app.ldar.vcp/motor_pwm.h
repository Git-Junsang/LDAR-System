// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_MOTOR_PWM_H
#define LDAR_MOTOR_PWM_H

#include <stdint.h>

/* L298N EN channel — 1 kHz PWM. Duty 0..100 (%); capped at 80 per CLAUDE.md. */
void MotorPwm_Init(void);
void MotorPwm_SetDutyPercent(uint8_t pct);

#endif
