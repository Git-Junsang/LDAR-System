// SPDX-License-Identifier: Apache-2.0

#include "servo_pwm.h"
#include "pwm_util.h"
#include "ldar_pins.h"

#define SERVO_PERIOD_NS        (20000000U)  /* 20 ms = 50 Hz */

/* 중립·풀스윙 분리. horn 정렬은 CENTER 펄스 기준. 좌·우 대칭 보장.
 *  - CENTER 만지면 정면 위치 시프트
 *  - SWING 만지면 양쪽 풀스윙 동시 확대/축소 (좌우 비대칭 안 생김) */
#define SERVO_PULSE_CENTER_NS  (1500000U)   /* 1.5 ms */
#define SERVO_PULSE_SWING_NS   (500000U)    /* ±0.5 ms */
#define SERVO_PULSE_MIN_NS     (SERVO_PULSE_CENTER_NS - SERVO_PULSE_SWING_NS)
#define SERVO_PULSE_MAX_NS     (SERVO_PULSE_CENTER_NS + SERVO_PULSE_SWING_NS)
#define SERVO_ANGLE_MAX        (127U)

/* 서보 기계적 중립 보정. 펄스 1.5ms일 때 우측으로 ~5° 틀어지는 경우 +50000ns(+50us)
 * 만큼 펄스를 늘려 반시계 보정. 부호·크기는 실측 후 조정.
 *  - 우측으로 틀어졌으면: 양수 증가 (+50000 → +100000 ...)
 *  - 좌측으로 틀어졌으면: 음수 (-50000 → -100000 ...)
 * 약 10us ≈ 1° (서보별 다름) */
#define SERVO_TRIM_NS      (50000)

static uint8_t s_lastAngle = 0xFFU;     /* sentinel */

static uint32_t ComputeDutyNs(uint8_t angle)
{
    int32_t base = (int32_t)SERVO_PULSE_MIN_NS +
        (((int32_t)angle * (int32_t)(SERVO_PULSE_MAX_NS - SERVO_PULSE_MIN_NS))
         / (int32_t)SERVO_ANGLE_MAX);
    int32_t trimmed = base + (int32_t)SERVO_TRIM_NS;
    if (trimmed < 0) { trimmed = 0; }
    if (trimmed > (int32_t)SERVO_PERIOD_NS) { trimmed = (int32_t)SERVO_PERIOD_NS; }
    return (uint32_t)trimmed;
}

void ServoPwm_Init(void)
{
    /* PDM_Init is idempotent and already called by MotorPwm_Init in typical
     * boot order. Calling again is harmless. */
    PwmUtil_Apply(LDAR_PWM_CH_SERVO, LDAR_PWM_PORT_SERVO,
                  SERVO_PERIOD_NS,
                  ComputeDutyNs(SERVO_ANGLE_MAX / 2U));
    s_lastAngle = SERVO_ANGLE_MAX / 2U;
}

void ServoPwm_SetAngle(uint8_t angle)
{
    if (angle > SERVO_ANGLE_MAX) {
        angle = SERVO_ANGLE_MAX;
    }
    if (angle == s_lastAngle) {
        return;
    }
    PwmUtil_Apply(LDAR_PWM_CH_SERVO, LDAR_PWM_PORT_SERVO,
                  SERVO_PERIOD_NS, ComputeDutyNs(angle));
    s_lastAngle = angle;
}
