// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_SERVO_PWM_H
#define LDAR_SERVO_PWM_H

#include <stdint.h>

/* Hobby servo: 50 Hz frame, 1.0 ms ~ 2.0 ms pulse.
 * Angle 0..127 → 1.0 ms ~ 2.0 ms (matches CAN 0x107 spec, center ≈ 63). */
void ServoPwm_Init(void);
void ServoPwm_SetAngle(uint8_t angle);

#endif
