// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_OVERRIDE_CAN_H
#define LDAR_OVERRIDE_CAN_H

/* CAN 0x110(Speed Override) 수신 → override 모듈 상태 갱신.
 *
 * data[0] = mode : 0x00 RELEASE / 0x01 LIMIT / 0x02 STOP
 * data[1] = limit(km/h ≈ duty%) — LIMIT 일 때만 유효 (예: 30, 60)
 *
 * CAN 채널·필터·콜백·CAN_Init()은 cmain의 CAN_DemoInitialize()가 이미 수행한다.
 * 0x110은 표준ID RANGE 필터(0x101~0x200)→RXFIFO1로 적재되어 ISR가 링버퍼에 넣고,
 * 메인 루프가 매 틱 OverrideCan_Poll()로 드레인한다(별도 태스크/인터럽트 핸들러 불필요). */
void OverrideCan_Init(void);
void OverrideCan_Poll(void);

#endif
