// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_JOYSTICK_ADC_H
#define LDAR_JOYSTICK_ADC_H

#include <stdint.h>

typedef struct {
    int16_t  vrx;       /* -127..+127, center 0 (steering axis) */
    int16_t  vry;       /* -127..+127, center 0 (throttle axis) */
    uint16_t raw_vrx;   /* 12-bit raw 0..4095 (for diagnostics) */
    uint16_t raw_vry;   /* 12-bit raw 0..4095 */
} JoystickAxes_t;

void JoystickAdc_Init(void);
JoystickAxes_t JoystickAdc_Read(void);

#endif
