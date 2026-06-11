# PROTOCOL — 통신 규격 · 판정 · 핀맵

> [README](../README.md)의 상세 규격 분리본. 불일치 시 코드(`ldar_pins.h`, `ldar_can.py`)가 기준.

## CAN 메시지 테이블

11-bit ID, CAN 채널 0. 하향(R5→VCP)은 D3-G 판정 결과(표지판→속도), 상향(VCP→R5)은 운전자 의도.

| 메시지 | CAN ID | 방향 | Data 구조 | 비고 |
|---|---|---|---|---|
| Brake Light | 0x101 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존(미사용) |
| Turn Signal | 0x102 | R5→VCP | `[0]` L=0x01 R=0x02 · `[1]` on/off | 기존(미사용) |
| Head Light | 0x104 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존(미사용) |
| **Speed Override** | **0x110** | R5→VCP | `[0]` mode · `[1]` 한계속도(km/h) | **표지판 속도 명령** |
| **Driver Input** | **0x120** | VCP→R5 | `[0]` 방향지시(0 off/1 L/2 R) | 상향 운전자 의도 |

### 0x110 Speed Override (핵심)
- `[0]` mode : `0x00` RELEASE(상한 해제) / `0x01` LIMIT(상한 제한) / `0x02` STOP(정지)
- `[1]` 한계속도(km/h) : **LIMIT일 때만 유효** — VCP-G가 듀티% 상한으로 1:1 적용(30→30%, 60→60%)

| 표지판 | mode `[0]` | `[1]` | VCP-G 동작 |
|---|---|---|---|
| 속도제한 30 | 0x01 LIMIT | 30 | 듀티 상한 30%까지 부드럽게 감속 |
| 속도제한 60 | 0x01 LIMIT | 60 | 듀티 상한 60%까지 부드럽게 감속 |
| 정지 / 진입금지 | 0x02 STOP | – | 0%까지 부드럽게 감속 후 브레이크 정지 |
| (제한구역 종료) | 0x00 RELEASE | – | 상한 해제 — 조이스틱 풀스로틀 복귀 |

- 수신: VCP-G가 표준ID RANGE 필터(0x101~0x200)→RXFIFO1로 받아 메인 루프에서 `CAN_CheckNewRxMessage`/
  `CAN_GetNewRxMessage`로 드레인([override_can.c](../vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/override_can.c)).
- "부드럽게": VCP-G가 **적용 상한(cap)을 목표값까지 틱마다 슬루-레이트**로 이동(감속 ≈100%/s, 90%→0% 약 0.9s).
  상한 아래에서는 조이스틱 듀티가 그대로 통과(직결). **조향은 오버라이드하지 않는다**(항상 조이스틱).
- 송신: D3-G A72가 표지판 판정 결과를 `LdarCan.limit/stop/release`로 IPC 송신 → R5가 CAN TX.
  → [a72/ldar_can.py](../d3-g/a72/ldar_can.py)

> 0x110 한계속도(km/h)는 CAN 스펙. VCP-G **로컬 조이스틱** 풀스로틀 듀티 상한은 펌웨어에서 90%(`MOTOR_DUTY_CAP_PCT`).

## IPC (A72 ↔ R5)

`/dev/tcc_ipc_micom` 경유. 교육용 IPC 패킷(SYNC·CMD·LENGTH·DATA·CRC16)으로 CAN 프레임을 래핑.
- **하향**: A72가 `(canID, data)`를 `IPC_SendPacketWithIPCHeader`로 write → R5가 CAN 송신. → [a72/ldar_can.py](../d3-g/a72/ldar_can.py)
- **상향**: R5가 0x120 CAN 수신 → `LdarIpcUpstream_t`로 A72 전달. → [r5/.../ldar_bridge.c](../d3-g/r5/sources/app.ldar.bridge/ldar_bridge.c), [shared/ldar_ipc_proto.h](../d3-g/shared/ldar_ipc_proto.h)

## 판정 매핑 (D3-G A72)

표지판 클래스 → 속도 명령. 차선 상태머신·복귀각은 더 이상 없음(구 설계는 [backup/](../backup/)).

```
표지판 검출(conf ≥ 임계) ──▶ SIGN_TO_CMD ──▶ 0x110 송신
  speed_30 → LIMIT 30      stop/no_entry → STOP
  speed_60 → LIMIT 60      clear → RELEASE(선택)      none → 직전 명령 유지
```

| 표지판 | 명령 | 0x110 |
|---|---|---|
| speed_30 | 속도 상한 30 | `[1,30]` |
| speed_60 | 속도 상한 60 | `[1,60]` |
| stop / no_entry | 정지 | `[2,0]` |
| clear (제한구역 종료) | 해제 | `[0,0]` |

- 신뢰도 `conf < CONF_THRESH`(0.5) 검출은 무시. 같은 명령이 `CONFIRM`(2) 프레임 연속이어야 전환(검출 깜박임 방지).
- 새 표지판은 직전 명령을 덮어쓴다(30→60이면 상한 60). `none`(표지판 없음)은 직전 명령을 **유지**(지나가도 제한 지속).
- 구현·실행법: [a72/README.md](../d3-g/a72/README.md). 게인/임계값은 `CFG` 딕셔너리.

## 표지판 데이터 (AI-G → D3-G, 잠정)

줄단위 `JSON\n`. 모델 클래스 확정 시 동기화.
```json
{"ts":1234.5, "sign":"speed_30", "conf":0.92}
```
`sign` : `speed_30` | `speed_60` | `stop` | `no_entry` | `clear` | `none` · `conf` : 검출 신뢰도 0..1

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
| 모터 EN(PWM) | [45] | GPA10 | J18D100 #26 | PDM CH0, 듀티 0~90%(오버라이드 시 상한↓) |
| 서보(PWM) | [44] | GPA11 | J18D100 #25 | PDM CH1, 1.0~2.0ms (center 1.5ms=angle 63) — **조이스틱 전용, 오버라이드 없음** |

**출력 — 표시**
| 부품 | 핀 | GPIO | 비고 |
|---|---|---|---|
| LED 좌 녹색 (방향지시) | [42] | GPA17 | 버튼 토글, 1Hz 깜박 |
| LED 우 녹색 (방향지시) | [41] | GPA18 | 버튼 토글, 1Hz 깜박 |
| LED 좌 적색 (오버라이드) | [43] | GPA16 | LIMIT=1Hz 깜박 / STOP=점등 |
| LED 우 적색 (오버라이드) | [40] | GPK11 | LIMIT=1Hz 깜박 / STOP=점등 |
| 피에조 부저 | [11] | GPC14 | active-high — SW 누름 또는 오버라이드 중 발음 |

**통신** — CAN0 TX `GPK08`(J5D100 #3), RX `GPK01`(#4)
**전원** — 3.3V→J8D100 #4(조이스틱·로직), 5V→서보 별도, 모터→별도 배터리(L298N), GND 공통

> ⚠️ **모든 버튼·SW는 active-low** (핀→버튼→GND, 내부 풀업). 풀다운/active-high로 두면 영원히 안 눌림.
