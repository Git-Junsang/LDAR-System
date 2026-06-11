// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_OVERRIDE_H
#define LDAR_OVERRIDE_H

#include <stdint.h>

/*
 * 속도 오버라이드 — 표지판 인식 기반.
 *
 * AI-G가 표지판을 검출 → D3-G가 판정 → CAN 0x110(Speed Override)으로 VCP-G에 명령.
 * override_can.c(CAN RX)가 아래 Set 함수를 호출해 상태를 갱신하고, 메인 루프는 매 틱
 * Override_ApplyDuty()로 조이스틱 듀티에 오버라이드를 적용한다.
 *
 *   speed_30 표지판 → LIMIT, cap=30   (속도 상한 30%로 강제, 부드럽게 감속)
 *   speed_60 표지판 → LIMIT, cap=60
 *   정지/진입금지   → STOP            (cap=0 까지 부드럽게 감속 후 정지)
 *   (구역 해제)     → RELEASE         (상한 해제 — 조이스틱 자유)
 *
 * "부드럽게" = 적용 상한(cap)을 목표값까지 틱마다 슬루-레이트로 이동시켜
 * 듀티가 급변하지 않게 한다. 상한 아래에서는 조이스틱이 그대로 통과(직결).
 */
typedef enum {
    OVR_MODE_NONE = 0,   /* 오버라이드 없음 — 조이스틱 자유 */
    OVR_MODE_LIMIT,      /* 속도 상한 제한(cap%) */
    OVR_MODE_STOP,       /* 강제 정지(cap=0) */
} OverrideMode_t;

void           Override_Init(void);

/* CAN RX(0x110)가 호출 — 표지판 판정 결과 반영 */
void           Override_SetLimit(uint8_t capPct);   /* LIMIT, 목표 상한 capPct% */
void           Override_SetStop(void);              /* STOP, 목표 상한 0% */
void           Override_Release(void);              /* NONE, 상한 해제 */

OverrideMode_t Override_GetMode(void);
uint8_t        Override_GetCap(void);   /* 현재 적용 상한(슬루 후) % — 표시/로그용 */

/* 조이스틱 요청 듀티에 오버라이드를 적용해 실제 적용 듀티를 반환.
 * 매 제어 틱(20ms) 1회 호출 — 내부에서 적용 상한을 목표값으로 슬루한다.
 * 반환값 = min(requestedDuty, 슬루된 상한). */
uint8_t        Override_ApplyDuty(uint8_t requestedDuty);

#endif
