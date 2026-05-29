// SPDX-License-Identifier: Apache-2.0
/*
 * LDAR VCP-G pin map (Phase 1 complete: GPIO + ADC + PWM + CAN).
 * Single source of truth — adjust here when wiring changes.
 *
 * GPIO — J8D102 (8-pin Female Header, GPIO only):
 *
 *   J8D102 pin | board signal | LDAR use
 *   -----------|--------------|------------------------
 *   1          | GPIO_B01     | LDAR_PIN_JOY_SW         (조이스틱 SW 브레이크)
 *   2          | GPIO_A13     | (reserved — Phase 3 status LED/buzzer)
 *   3          | GPIO_B10     | LDAR_PIN_MOTOR_IN1      (L298N IN1)
 *   4          | GPIO_B27     | LDAR_PIN_LED_TURN_L     (방향지시 LED 좌, 1Hz 깜박임)
 *   5          | GPIO_B11     | LDAR_PIN_MOTOR_IN2      (L298N IN2)
 *   6          | GPIO_B28     | LDAR_PIN_LED_TURN_R     (방향지시 LED 우, 1Hz 깜박임)
 *   7          | GPIO_B25     | LDAR_PIN_TURN_L         (방향지시 버튼 L 입력)
 *   8          | GPIO_B26     | LDAR_PIN_TURN_R         (방향지시 버튼 R 입력)
 *
 * ADC — J8D101 (8-pin Female Header, GPIO+ADC):
 *
 *   J8D101 pin | board signal | LDAR use
 *   -----------|--------------|------------------------
 *   1          | A0 / ADC03   | LDAR_ADC_CH_VRX       (조이스틱 좌우, 조향)
 *   2          | A1 / ADC04   | LDAR_ADC_CH_VRY       (조이스틱 앞뒤, 속도)
 *
 * PWM (PDM, GPA CH0) — J18D100 (36-pin GPIO+ADC):
 *
 *   J18D100 pin | board signal | LDAR use
 *   ------------|--------------|------------------------
 *   26          | GPIO_A10     | L298N EN (모터 속도)   — PWMSEL_0, FUNC(2)
 *   25          | GPIO_A11     | Servo (조향 각)        — PWMSEL_1, FUNC(2)
 *
 * CAN — J5D100 (10-pin Male, CAN, BSP can_porting.h 고정):
 *
 *   J5D100 pin | board signal | LDAR use
 *   -----------|--------------|------------------------
 *   3          | TX0 / GPK_08 | CAN channel 0 TX (→ D3-G)
 *   4          | RX0 / GPK_01 | CAN channel 0 RX
 *   1, 2       | 3.3V         | CAN transceiver VCC
 *   9, 10      | GND          | CAN GND
 *
 * Power/GND: J8D100 (3.3V #4, GND #6/#7).
 *  - 조이스틱: VCC→3.3V, GND→GND, VRx→J8D101 #1, VRy→J8D101 #2, SW→J8D102 #1
 *  - 택트 버튼(L/R): 한쪽 다리→3.3V, 반대쪽→J8D102 #7/#8
 *  - 방향지시 LED(L/R): (+)→J8D102 #4/#6, (−)→직렬저항 220~330Ω→GND
 *  - L298N: IN1→J8D102 #3, IN2→J8D102 #5, EN→J18D100 #26, 로직 전원→3.3V/GND, 모터 전원은 별도 배터리팩
 *  - Servo: 신호→J18D100 #25, 전원→별도 5V (서보는 3.3V 부족할 수 있음), GND 공통
 */

#ifndef LDAR_PINS_H
#define LDAR_PINS_H

#include <gpio.h>
#include <adc.h>

/* T1.1 Joystick — SW push button (digital, pulldown, active-high) */
#define LDAR_PIN_JOY_SW              GPIO_GPB(1)

/* T1.1 Joystick analog — VRx (steering) / VRy (throttle). 12-bit, 0~4095. */
#define LDAR_ADC_TYPE                (0U)
#define LDAR_ADC_MODULE              (0U)
#define LDAR_ADC_CH_VRX              ADC_CHANNEL_3   /* A0 = ADC03 */
#define LDAR_ADC_CH_VRY              ADC_CHANNEL_4   /* A1 = ADC04 */

/* T1.2 L298N motor direction (IN1/IN2) — GPIO output */
#define LDAR_PIN_MOTOR_IN1           GPIO_GPB(10)
#define LDAR_PIN_MOTOR_IN2           GPIO_GPB(11)

/* T1.2 PWM — PDM CH0 (GPA port). Channel index = PWMSEL_n suffix. */
#define LDAR_PWM_CH_MOTOR_EN         (0U)              /* PWMSEL_0 → GPA[10] */
#define LDAR_PWM_PORT_MOTOR_EN       (GPIO_PERICH_CH0)
#define LDAR_PWM_CH_SERVO            (1U)              /* PWMSEL_1 → GPA[11] */
#define LDAR_PWM_PORT_SERVO          (GPIO_PERICH_CH0)

/* T1.3 Turn signal — left / right tact buttons (pulldown, active-high) */
#define LDAR_PIN_TURN_L              GPIO_GPB(25)
#define LDAR_PIN_TURN_R              GPIO_GPB(26)

/* 방향지시 LED 2개 — 버튼 누를 동안 1Hz 깜박임 */
#define LDAR_PIN_LED_TURN_L          GPIO_GPB(27)
#define LDAR_PIN_LED_TURN_R          GPIO_GPB(28)

/* T1.3 CAN — upstream 0x120 to D3-G via CAN channel 0 (J5D100 TX0/RX0). */
#define LDAR_CAN_CH                  (0U)
#define LDAR_CAN_ID_DRIVER_INPUT     (0x120U)

#endif
