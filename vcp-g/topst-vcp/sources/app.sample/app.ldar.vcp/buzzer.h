// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_BUZZER_H
#define LDAR_BUZZER_H

#include <stdint.h>

/* 피에조 부저 on/off (GPIO High=발음, 액티브 부저 가정).
 * 패시브 피에조라 DC로 소리가 안 나면 톤(구형파) 생성이 필요 — 별도 처리. */
void Buzzer_Init(void);
void Buzzer_Set(uint32_t on);

#endif
