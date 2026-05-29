// SPDX-License-Identifier: Apache-2.0

#include "ldar_app.h"
#include "joystick_sw.h"
#include "motor_dir.h"
#include "turn_signal.h"

#include <sal_api.h>
#include <debug.h>
#include <stdint.h>

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

static MotorDir_t DecideDir(uint32_t sw, TurnSignal_t ts)
{
    if (sw != 0U) {
        return MOTOR_DIR_BRAKE;
    }
    if (ts == TURN_LEFT) {
        return MOTOR_DIR_REVERSE;
    }
    if (ts == TURN_RIGHT) {
        return MOTOR_DIR_FORWARD;
    }
    return MOTOR_DIR_STOP;
}

void LDAR_Run(void)
{
    JoystickSW_Init();
    MotorDir_Init();
    TurnSignal_Init();

    mcu_printf("\n[LDAR] Phase 1 GPIO loop started\n");

    uint32_t tick = 0U;
    MotorDir_t prev = MOTOR_DIR_STOP;

    while (1) {
        uint32_t sw     = JoystickSW_Read();
        TurnSignal_t ts = TurnSignal_Read();
        MotorDir_t   dir = DecideDir(sw, ts);

        MotorDir_Set(dir);

        if ((dir != prev) || ((tick % 50U) == 0U)) {
            mcu_printf("\n[LDAR] sw=%d turn=%s dir=%s",
                       (int)sw, TurnName(ts), DirName(dir));
        }
        prev = dir;
        tick++;
        (void)SAL_TaskSleep(20);
    }
}
