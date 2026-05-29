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

#include <sal_api.h>
#include <debug.h>
#include <stdint.h>

#define VRY_GO_THRESHOLD   (10)   /* below this magnitude → coast (STOP) */
#define MOTOR_DUTY_CAP_PCT (80U)  /* CLAUDE.md spec */

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

    mcu_printf("\n[LDAR] Phase 1 full loop started (GPIO+ADC+PWM+CAN)\n");

    uint32_t   tick = 0U;
    MotorDir_t prev = MOTOR_DIR_STOP;

    while (1) {
        uint32_t       sw    = JoystickSW_Read();
        JoystickAxes_t axes  = JoystickAdc_Read();
        TurnSignal_t   ts    = TurnSignal_Read();

        MotorDir_t     dir   = DecideDir(sw, axes.vry);
        uint8_t        duty  = DutyFromVry(dir, axes.vry);
        uint8_t        angle = AngleFromVrx(axes.vrx);

        MotorDir_Set(dir);
        MotorPwm_SetDutyPercent(duty);
        ServoPwm_SetAngle(angle);
        TurnCan_SendIfChanged(ts);
        TurnLed_Update(ts, tick);

        if ((dir != prev) || ((tick % 50U) == 0U)) {
            mcu_printf("\n[LDAR] sw=%d vrx=%d vry=%d (raw %d/%d) duty=%d%% angle=%d turn=%s dir=%s",
                       (int)sw,
                       (int)axes.vrx, (int)axes.vry,
                       (int)axes.raw_vrx, (int)axes.raw_vry,
                       (int)duty, (int)angle,
                       TurnName(ts), DirName(dir));
        }
        prev = dir;
        tick++;
        (void)SAL_TaskSleep(20);
    }
}
