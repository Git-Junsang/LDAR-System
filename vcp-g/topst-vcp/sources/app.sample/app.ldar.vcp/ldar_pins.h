// SPDX-License-Identifier: Apache-2.0
/*
 * LDAR VCP-G pin map (Phase 1, GPIO scope).
 * Single source of truth — adjust here when wiring changes.
 *
 * All 5 GPIO signals live on J8D102 (8-pin Female Header, GPIO only):
 *
 *   J8D102 pin | board signal | LDAR use
 *   -----------|--------------|------------------------
 *   1          | GPIO_B01     | LDAR_PIN_JOY_SW
 *   2          | GPIO_A13     | (reserved — Phase 3 LED/buzzer)
 *   3          | GPIO_B10     | LDAR_PIN_MOTOR_IN1
 *   4          | GPIO_B27     | (reserved — Phase 3 LED)
 *   5          | GPIO_B11     | LDAR_PIN_MOTOR_IN2
 *   6          | GPIO_B28     | (reserved — Phase 3 LED)
 *   7          | GPIO_B25     | LDAR_PIN_TURN_L
 *   8          | GPIO_B26     | LDAR_PIN_TURN_R
 *
 * Power/GND for joystick, buttons, L298N logic: J8D100 (3.3V #4, GND #6/#7).
 */

#ifndef LDAR_PINS_H
#define LDAR_PINS_H

#include <gpio.h>

/* T1.1 Joystick — SW push button (digital, pulldown, active-high) */
#define LDAR_PIN_JOY_SW          GPIO_GPB(1)

/* T1.2 L298N motor direction (IN1/IN2). EN goes to PWM in D02-T06 (PDM). */
#define LDAR_PIN_MOTOR_IN1       GPIO_GPB(10)
#define LDAR_PIN_MOTOR_IN2       GPIO_GPB(11)

/* T1.3 Turn signal — left / right tact buttons (pulldown, active-high) */
#define LDAR_PIN_TURN_L          GPIO_GPB(25)
#define LDAR_PIN_TURN_R          GPIO_GPB(26)

#endif
