// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_OVERRIDE_H
#define LDAR_OVERRIDE_H

/* 제어권 오버라이드 보상 방향.
 * Phase 3에서 CAN 0x110(Control Authority)·0x107(Wheel Angle) 수신 → Override_Set.
 * 현재(Phase 1)는 CAN 수신이 없어 항상 OVR_NONE. */
typedef enum {
    OVR_NONE = 0,
    OVR_LEFT,    /* 좌측으로 복귀 조향 보상 */
    OVR_RIGHT,   /* 우측으로 복귀 조향 보상 */
} OverrideDir_t;

void          Override_Init(void);
void          Override_Set(OverrideDir_t dir);   /* Phase 3 CAN RX가 호출 */
OverrideDir_t Override_Get(void);

#endif
