// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_PWM_UTIL_H
#define LDAR_PWM_UTIL_H

#include <stdint.h>

/* Reconfigure a PDM channel (disable → wait idle → setconfig → enable).
 * Heavy: do not call every tick. Used by MotorPwm/ServoPwm on duty change. */
void PwmUtil_Apply(uint32_t channel,
                   uint32_t portCh,
                   uint32_t periodNs,
                   uint32_t dutyNs);

#endif
