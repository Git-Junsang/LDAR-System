// SPDX-License-Identifier: Apache-2.0

#include "motor_pwm.h"
#include "pwm_util.h"
#include "ldar_pins.h"

#include <pdm.h>

#define MOTOR_PERIOD_NS   (1000000U)   /* 1 ms = 1 kHz */
#define MOTOR_DUTY_CAP    (80U)        /* CLAUDE.md spec: cap at 80% */

static uint8_t s_lastDuty = 0xFFU;     /* sentinel — force first apply */

void MotorPwm_Init(void)
{
    PDM_Init();
    PwmUtil_Apply(LDAR_PWM_CH_MOTOR_EN, LDAR_PWM_PORT_MOTOR_EN, MOTOR_PERIOD_NS, 0U);
    s_lastDuty = 0U;
}

void MotorPwm_SetDutyPercent(uint8_t pct)
{
    if (pct > MOTOR_DUTY_CAP) {
        pct = MOTOR_DUTY_CAP;
    }
    if (pct == s_lastDuty) {
        return;
    }
    uint32_t dutyNs = ((uint32_t)pct * MOTOR_PERIOD_NS) / 100U;
    PwmUtil_Apply(LDAR_PWM_CH_MOTOR_EN, LDAR_PWM_PORT_MOTOR_EN, MOTOR_PERIOD_NS, dutyNs);
    s_lastDuty = pct;
}
