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

/* 서보 미세 중립 트림. horn을 기계적으로 정렬(축 재장착)하므로 기본 0.
 * 스플라인 간격상 정확히 정면이 안 나오면 ±10000ns(≈1°)씩만 미세 조정.
 *  - 정면에서 우측으로 틀어졌으면 한쪽 부호, 좌측이면 반대 (실측 후 ±) */
#define SERVO_ANGLE_CENTER (SERVO_ANGLE_MAX / 2U)   /* 63 → 정확히 CENTER 펄스 */
#define SERVO_TRIM_NS      (0)

static uint8_t s_lastAngle = 0xFFU;     /* sentinel */

static uint32_t ComputeDutyNs(uint8_t angle)
{
    if (angle > SERVO_ANGLE_MAX) { angle = SERVO_ANGLE_MAX; }
    /* angle=CENTER(63) → 정확히 1.5ms. 좌우 대칭: ±63스텝 = ∓SWING. */
    int32_t delta = (int32_t)angle - (int32_t)SERVO_ANGLE_CENTER;   /* -63..+64 */
    int32_t duty  = (int32_t)SERVO_PULSE_CENTER_NS
                  + ((delta * (int32_t)SERVO_PULSE_SWING_NS) / (int32_t)SERVO_ANGLE_CENTER)
                  + (int32_t)SERVO_TRIM_NS;
    if (duty < (int32_t)SERVO_PULSE_MIN_NS) { duty = (int32_t)SERVO_PULSE_MIN_NS; }
    if (duty > (int32_t)SERVO_PULSE_MAX_NS) { duty = (int32_t)SERVO_PULSE_MAX_NS; }
    return (uint32_t)duty;
}

void ServoPwm_Init(void)
{
    /* PDM_Init is idempotent and already called by MotorPwm_Init in typical
     * boot order. Calling again is harmless. */
    PwmUtil_Apply(LDAR_PWM_CH_SERVO, LDAR_PWM_PORT_SERVO,
                  SERVO_PERIOD_NS,
                  ComputeDutyNs(SERVO_ANGLE_CENTER));
    s_lastAngle = SERVO_ANGLE_CENTER;
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
