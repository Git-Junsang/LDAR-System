# PROTOCOL — 통신 규격 · 판정 · 핀맵

> [README](../README.md)의 상세 규격 분리본. 불일치 시 코드(`ldar_pins.h`, `ldar_can.py`)가 기준.

## CAN 메시지 테이블

11-bit ID. 하향(R5→VCP)은 D3-G 판정 결과, 상향(VCP→R5)은 운전자 의도.

| 메시지 | CAN ID | 방향 | Data 구조 | 비고 |
|---|---|---|---|---|
| Brake Light | 0x101 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존 |
| Turn Signal | 0x102 | R5→VCP | `[0]` L=0x01 R=0x02 · `[1]` on/off | 기존 |
| Emergency Signal | 0x103 | R5→VCP | `[0]` on=0x01 (경고음 연동) | 기존 |
| Head Light | 0x104 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존 |
| Vehicle Speed | 0x106 | R5→VCP | `[0]` 0~80 (DC 모터 duty %) | BOARD 복귀 속도 |
| Wheel Angle | 0x107 | R5→VCP | `[0]` 0~127 (서보 각, center 63) | BOARD 복귀 조향 |
| **Control Authority** | **0x110** | R5→VCP | `[0]` USER=0x01 / BOARD=0x02 | **신규** — 제어권 주체 |
| **Lane Status** | **0x111** | R5→VCP | `[0]` safe/warn/depart · `[1]` solid/dashed | **신규** — 차선 상태·종류 |
| **Driver Input** | **0x120** | VCP→R5 | `[0]` 방향지시(L/R/off) | **신규** — 상향 운전자 의도 |

- **0x110** — VCP-G 조향 소스 중재 기준(USER=로컬 조이스틱, BOARD=CAN 복귀각) + 상태 LED 색 결정
- **0x111** — 실선/점선 경고 차별 표시, 경고음 패턴 분기
- **0x120** — 점선 이탈 시 방향지시 여부로 Override 분기 (의도적 차선변경이면 개입 안 함)

> 0x106의 0~80은 CAN 스펙. VCP-G **로컬 조이스틱** 풀스로틀 듀티 상한은 펌웨어에서 90%(`MOTOR_DUTY_CAP_PCT`)로 별도.

## IPC (A72 ↔ R5)

`/dev/tcc_ipc_micom` 경유. 교육용 IPC 패킷(SYNC·CMD·LENGTH·DATA·CRC16)으로 CAN 프레임을 래핑.
- **하향**: A72가 `(canID, data)`를 `IPC_SendPacketWithIPCHeader`로 write → R5가 CAN 송신. → [a72/ldar_can.py](../d3-g/a72/ldar_can.py)
- **상향**: R5가 0x120 CAN 수신 → `LdarIpcUpstream_t`로 A72 전달. → [r5/.../ldar_bridge.c](../d3-g/r5/sources/app.ldar.bridge/ldar_bridge.c), [shared/ldar_ipc_proto.h](../d3-g/shared/ldar_ipc_proto.h)

## Decision State Machine (D3-G A72)

```
USER_CONTROL ──(점선 접근 · 거리<Tin)──────────────────────────▶ WARNING
USER_CONTROL ──(실선 접근 · 거리<Tin · 방향지시 없음)────────────▶ BOARD_OVERRIDE
BOARD_OVERRIDE ──(거리>Tout · 홀드시간 Th · heading 평행)─────────▶ USER_CONTROL
(차선 접촉/통과) ──────────────────────────────────────────────▶ CRITICAL
```

| 상태 | 조건 | 동작 | 상태 LED |
|---|---|---|---|
| SAFE | 거리 > Tout | 사용자 제어 유지 | 🟢 초록 |
| WARNING | 거리 < Tin & 점선 쪽 | 경고음만 (점선 허용) | 🟠 주황 |
| OVERRIDE | 거리 < Tin & 실선 쪽 & 방향지시 없음 | 제어권 이양 + 조향 복귀 | 🔴 빨강 |
| CRITICAL | 차선 접촉/통과 | 즉시 감속 + 조향 복귀 | 🔴 빨강 |

- 복귀 조향 = **P 비례 제어**(횡오프셋·heading 오차). 게인·임계값(Tin/Tout/Th/heading)은 Phase 4 트랙 실측으로 확정 → [a72/ldar_decision.py](../d3-g/a72/ldar_decision.py)의 `CFG`
- 구현·실행법: [a72/README.md](../d3-g/a72/README.md)

## 차선 데이터 (AI-G → D3-G, 잠정)

줄단위 `JSON\n`. 모델 출력 텐서 확정 시 Phase 4 동기화.
```json
{"ts":1234.5, "offset":0.1, "heading":0.0, "left_type":1, "right_type":2, "risk":0.2}
```
`offset` −1(좌끝)..0(중앙)..+1(우끝) · `heading` +면 우향 · `*_type` 1=실선 2=점선

## VCP-G 핀맵

단일 출처는 [ldar_pins.h](../vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/ldar_pins.h). `[n]`=디지털 핀 번호(VCP-G Docs Port Name).

**입력**
| 부품 | 핀 | GPIO/신호 | 헤더 | 설정 |
|---|---|---|---|---|
| 조이스틱 VRx(조향) | A0 | ADC03 | J8D101 #1 | ADC, 중심 2048 고정 |
| 조이스틱 VRy(속도) | A1 | ADC04 | J8D101 #2 | ADC |
| 조이스틱 SW | [7] | GPB01 | J8D102 #1 | **active-low/풀업** → 부저 |
| 턴 버튼 L | [48] | GPA05 | J18D100 | **active-low/풀업** |
| 턴 버튼 R | [49] | GPA04 | J18D100 | **active-low/풀업** |

**출력 — 구동**
| 부품 | 핀 | GPIO | 헤더 | 비고 |
|---|---|---|---|---|
| 모터 IN1 | [5] | GPB10 | J8D102 #3 | L298N 방향 |
| 모터 IN2 | [3] | GPB11 | J8D102 #5 | L298N 방향 |
| 모터 EN(PWM) | [45] | GPA10 | J18D100 #26 | PDM CH0, 듀티 0~90% |
| 서보(PWM) | [44] | GPA11 | J18D100 #25 | PDM CH1, 1.0~2.0ms (center 1.5ms=angle 63, 좌우 대칭) |

**출력 — 표시**
| 부품 | 핀 | GPIO | 비고 |
|---|---|---|---|
| LED 좌 녹색 (방향지시) | [42] | GPA17 | 버튼 토글, 1Hz 깜박 |
| LED 우 녹색 (방향지시) | [41] | GPA18 | 버튼 토글, 1Hz 깜박 |
| LED 좌 적색 (오버라이드) | [43] | GPA16 | Phase 3 (CAN 수신 시) |
| LED 우 적색 (오버라이드) | [40] | GPK11 | Phase 3 (CAN 수신 시) |
| 피에조 부저 | [11] | GPC14 | active-high (긴발+→핀) |

**통신** — CAN0 TX `GPK08`(J5D100 #3), RX `GPK01`(#4)
**전원** — 3.3V→J8D100 #4(조이스틱·로직), 5V→서보 별도, 모터→별도 배터리(L298N), GND 공통

> ⚠️ **모든 버튼·SW는 active-low** (핀→버튼→GND, 내부 풀업). 조이스틱 모듈/택트 버튼 모두 GND로 단락되는 방식이라 풀다운/active-high로 두면 영원히 안 눌림.
