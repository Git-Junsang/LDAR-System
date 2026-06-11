// SPDX-License-Identifier: Apache-2.0
/*
 * D3-G R5 IPC→CAN 하향 브리지 (downstream).
 *
 * A72(ldar_decision.py → ldar_can.py)가 교육용 IPC 패킷(CMD1=0x05 EDUCATION_CAN_DEMO)으로
 * 보낸 (canID, data)를 받아 CAN으로 송신한다. 0x110 Speed Override가 이 경로로 VCP-G에 닿는다.
 * R5는 canID/data를 해석하지 않는 **단순 포워더** — A72가 정한 프레임을 그대로 버스에 올린다.
 *
 * 상향(CAN 0x120 → IPC)은 ldar_bridge.c, 하향(IPC → CAN)은 이 파일. 짝을 이룬다.
 *
 * 통합 (R5 BSP, TCC8050, WSL2 빌드환경):
 *  1) 이 파일 + ldar_downstream.c 를 app.ldar.bridge/ 에 둔다(ldar_bridge.c와 같은 폴더).
 *  2) rules.mk 에 SRCS += ldar_downstream.c (이미 반영).
 *  3) R5 AppTaskCreate()에서 CAN_Init() 후 LdarDownstream_Init() 호출.
 *  4) 아래 .c의 extern Tcc_Ipc_Recv 플레이스홀더를 R5 BSP의 실제 IPC 수신 API로 교체.
 *     (또는 교육용 IPC-CAN 데모를 활성화하면 이 파일 없이도 같은 동작 — README 참조.)
 */
#ifndef LDAR_DOWNSTREAM_H
#define LDAR_DOWNSTREAM_H

#include <stdint.h>

/* IPC 수신 폴링 태스크를 생성한다. CAN_Init() 이후 1회 호출. */
void LdarDownstream_Init(void);

/* 수신한 IPC 패킷 1개(SYNC..CRC)를 파싱해 CAN으로 송신.
 * BSP가 IPC RX 콜백을 주면, 폴링 태스크 대신 이 함수에 바로 연결해도 된다. */
void LdarDownstream_OnIpcPacket(const uint8_t *buf, uint32_t len);

#endif
