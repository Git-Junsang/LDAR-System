// SPDX-License-Identifier: Apache-2.0

#include "ldar_app.h"
#include "joystick_sw.h"
#include "joystick_adc.h"
#include "motor_dir.h"
#include "motor_pwm.h"
#include "servo_pwm.h"
#include "turn_signal.h"
#include "turn_can.h"
#include "turn_led.h"
#include "override.h"
#include "override_can.h"
#include "buzzer.h"

#include <sal_api.h>
#include <debug.h>
#include <stdint.h>

#define VRY_GO_THRESHOLD   (10)   /* below this magnitude → coast (STOP) */
#define MOTOR_DUTY_CAP_PCT (90U)  /* 조이스틱 풀스로틀 90% — 최고속 ~10%↓ (오픈루프라 토크도 약간 같이 감소) */

static const char *DirName(MotorDir_t d)
{
    switch (d) {
        case MOTOR_DIR_FORWARD: return "FWD";
        case MOTOR_DIR_REVERSE: return "REV";
        case MOTOR_DIR_BRAKE:   return "BRK";
        case MOTOR_DIR_STOP:
        default:                return "STP";
    }
}

static const char *TurnName(TurnSignal_t t)
{
    switch (t) {
        case TURN_LEFT:  return "L";
        case TURN_RIGHT: return "R";
        case TURN_OFF:
        default:         return "-";
    }
}

static const char *OvrName(OverrideMode_t m)
{
    switch (m) {
        case OVR_MODE_LIMIT: return "LIM";
        case OVR_MODE_STOP:  return "STP";
        case OVR_MODE_NONE:
        default:             return "-";
    }
}

static MotorDir_t DecideDir(uint32_t sw, int16_t vry)
{
    if (sw != 0U) {
        return MOTOR_DIR_BRAKE;
    }
    if (vry >  VRY_GO_THRESHOLD) {
        return MOTOR_DIR_FORWARD;
    }
    if (vry < -VRY_GO_THRESHOLD) {
        return MOTOR_DIR_REVERSE;
    }
    return MOTOR_DIR_STOP;
}

static uint8_t DutyFromVry(MotorDir_t dir, int16_t vry)
{
    if ((dir != MOTOR_DIR_FORWARD) && (dir != MOTOR_DIR_REVERSE)) {
        return 0U;
    }
    int32_t mag = (vry < 0) ? -(int32_t)vry : (int32_t)vry;
    if (mag > 127) { mag = 127; }
    /* |vry|/127 → 0..MOTOR_DUTY_CAP_PCT */
    return (uint8_t)((mag * (int32_t)MOTOR_DUTY_CAP_PCT) / 127);
}

static uint8_t AngleFromVrx(int16_t vrx)
{
    /* vrx 음수=우측 의도 → angle 큰 값(서보 우측 펄스). 부호 반전.
     * vrx -127..+127 → angle 127..0, center 63 */
    int32_t a = 63 - ((int32_t)vrx / 2);
    if (a <   0) { a = 0;   }
    if (a > 127) { a = 127; }
    return (uint8_t)a;
}

void LDAR_Run(void)
{
    JoystickSW_Init();
    JoystickAdc_Init();
    MotorDir_Init();
    MotorPwm_Init();
    ServoPwm_Init();
    TurnSignal_Init();
    TurnCan_Init();
    TurnLed_Init();
    Override_Init();
    OverrideCan_Init();
    Buzzer_Init();

    mcu_printf("\n[LDAR] manual drive + sign speed-override loop started (GPIO+ADC+PWM+CAN)\n");

    uint32_t   tick = 0U;
    MotorDir_t prev = MOTOR_DIR_STOP;

    while (1) {
        OverrideCan_Poll();   /* CAN 0x110(Speed Override) 수신 → override 상태 갱신 */

        uint32_t       sw    = JoystickSW_Read();
        JoystickAxes_t axes  = JoystickAdc_Read();
        TurnSignal_t   ts    = TurnSignal_Update();
        OverrideMode_t ovr   = Override_GetMode();

        MotorDir_t     dir   = DecideDir(sw, axes.vry);
        uint8_t        reqD  = DutyFromVry(dir, axes.vry);
        uint8_t        duty  = Override_ApplyDuty(reqD);  /* 상한/정지 적용(부드러운 슬루) */
        uint8_t        angle = AngleFromVrx(axes.vrx);    /* 조향은 항상 조이스틱(오버라이드 없음) */

        /* 정지 오버라이드가 0%까지 감속 완료되면 능동 브레이크로 정지 유지 */
        if ((ovr == OVR_MODE_STOP) && (duty == 0U)) {
            dir = MOTOR_DIR_BRAKE;
        }

        MotorDir_Set(dir);
        MotorPwm_SetDutyPercent(duty);
        ServoPwm_SetAngle(angle);
        TurnCan_SendIfChanged(ts);
        TurnLed_Update(ts, ovr, tick);
        Buzzer_Set(((sw != 0U) || (ovr != OVR_MODE_NONE)) ? 1U : 0U);

        if ((dir != prev) || ((tick % 50U) == 0U)) {
            mcu_printf("\n[LDAR] sw=%d vrx=%d vry=%d duty=%d%%(req %d cap %d) angle=%d turn=%s dir=%s ovr=%s",
                       (int)sw,
                       (int)axes.vrx, (int)axes.vry,
                       (int)duty, (int)reqD, (int)Override_GetCap(),
                       (int)angle,
                       TurnName(ts), DirName(dir), OvrName(ovr));
        }
        prev = dir;
        tick++;
        (void)SAL_TaskSleep(20);
    }
}
