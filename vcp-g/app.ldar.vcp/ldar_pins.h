// SPDX-License-Identifier: Apache-2.0
/*
 * LDAR VCP-G pin map (Phase 1 complete: GPIO + ADC + PWM + CAN).
 * Single source of truth — adjust here when wiring changes.
 *
 * GPIO — J8D102 (8-pin Female Header, GPIO only):
 *
 *   J8D102 pin | board signal | LDAR use
 *   -----------|--------------|------------------------
 *   1          | GPIO_B01     | LDAR_PIN_JOY_SW         ([7] 조이스틱 SW → 부저)
 *   3          | GPIO_B10     | LDAR_PIN_MOTOR_IN1      (L298N IN1)
 *   5          | GPIO_B11     | LDAR_PIN_MOTOR_IN2      (L298N IN2)
 *   (2,4,6,7,8 미사용 — 버튼/LED/부저는 J18D100·J10D100로 이전)
 *
 * 버튼·LED·부저 — J18D100(36핀) / J10D100. [n] = 디지털 핀 번호(VCP-G Docs Port Name):
 *
 *   [n]  | board signal | LDAR use
 *   -----|--------------|------------------------
 *   [48] | GPIO_A05     | LDAR_PIN_TURN_L         (좌회전 버튼)
 *   [49] | GPIO_A04     | LDAR_PIN_TURN_R         (우회전 버튼)
 *   [42] | GPIO_A17     | LDAR_PIN_LED_TURN_L     (좌 녹색 — 방향지시)
 *   [41] | GPIO_A18     | LDAR_PIN_LED_TURN_R     (우 녹색 — 방향지시)
 *   [43] | GPIO_A16     | LDAR_PIN_LED_OVR_L      (좌 적색 — 오버라이드 보상)
 *   [40] | GPIO_K11     | LDAR_PIN_LED_OVR_R      (우 적색 — 오버라이드 보상)
 *   [11] | GPIO_C14     | LDAR_PIN_BUZZER         (피에조 부저)
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
 *  - 방향지시 버튼(L/R): [48]/[49]→버튼→GND (active-low, 내부 풀업)
 *  - 녹색 LED(L/R)→[42]/[41], 적색 LED(L/R)→[43]/[40]: (+)→핀, (−)→직렬저항 220~330Ω→GND
 *  - 피에조 부저: (+)→[11] GPIO_C14, (−)→GND (액티브 부저 가정)
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

/* 방향지시 버튼 (택트, pulldown, active-high) — J18D100 */
#define LDAR_PIN_TURN_L              GPIO_GPA(5)    /* [48] 좌회전 */
#define LDAR_PIN_TURN_R              GPIO_GPA(4)    /* [49] 우회전 */

/* 방향지시 LED (녹색) — 활성 쪽 1Hz 깜박임 */
#define LDAR_PIN_LED_TURN_L          GPIO_GPA(17)   /* [42] 좌 녹색 */
#define LDAR_PIN_LED_TURN_R          GPIO_GPA(18)   /* [41] 우 녹색 */

/* 오버라이드 LED (적색) — CAN 오버라이드 보상 방향 점등 */
#define LDAR_PIN_LED_OVR_L           GPIO_GPA(16)   /* [43] 좌 적색 */
#define LDAR_PIN_LED_OVR_R           GPIO_GPK(11)   /* [40] 우 적색 */

/* 피에조 부저 — SW 누름 / 오버라이드 시 발음 */
#define LDAR_PIN_BUZZER              GPIO_GPC(14)   /* [11] */

/* CAN — channel 0 (J5D100 TX0/RX0).
 *   상향 TX 0x120 : Driver Input(방향지시 의도) → D3-G
 *   하향 RX 0x110 : Speed Override(표지판 속도제한/정지) ← D3-G
 *                   표준ID RANGE 필터(0x101~0x200)→RXFIFO1 로 수신(can_par.c). */
#define LDAR_CAN_CH                  (0U)
#define LDAR_CAN_ID_DRIVER_INPUT     (0x120U)
#define LDAR_CAN_ID_SPEED_OVERRIDE   (0x110U)

#endif
