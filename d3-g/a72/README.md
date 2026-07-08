# D3-G A72 — 판정·명령 (LDAR Decision)

AI-G가 인식한 **교통표지판**을 받아 **속도제한/정지를 판정**하고, VCP-G에 **속도 오버라이드를
CAN으로 명령**한다. (인지=AI-G, 판정·명령=D3-G, 구동·중재=VCP-G. 차선은 다루지 않음.)

```
AI-G ──Ethernet TCP(표지판 JSON)──▶ ldar_decision.py (A72)
                                     │ 표지판 → 속도 명령 매핑(SIGN_TO_CMD)
                                     │ ldar_can.py → IPC(/dev/tcc_ipc_micom)
                                     ▼
                               R5 (CAN demo/bridge) ──CAN 0x110──▶ VCP-G
                                 Speed Override: [0]=mode, [1]=한계속도(km/h)
```

## 파일
| 파일 | 역할 |
|---|---|
| `ldar_decision.py` | 메인 앱 — TCP/Mock 표지판 입력, 속도 명령 매핑, 송신 |
| `ldar_can.py` | 하향 CAN 0x110(Speed Override) — IPC 전송 |
| `Library/IPC_Library.py` | 교육용 IPC 패킷(CRC16) transport (유사 프로젝트 검증본 재사용) |
| `ldar_listener.c` | 상향 0x120(방향지시) IPC 수신 확인용 (Phase 1) |

## 실행
```bash
# AI-G 없이 매핑 단독 검증 — 표지판 시나리오 순환(none→60→30→stop→clear)
python3 ldar_decision.py --source mock --dry-run     # 콘솔만
python3 ldar_decision.py --source mock               # 실제 IPC 송신

# 실제 AI-G TCP 수신
python3 ldar_decision.py --source tcp --port 9999
```
`/dev/tcc_ipc_micom` 접근은 root 필요 (`sudo`).

## 판정 매핑 (PROTOCOL.md 일치)
| 표지판 | 명령 | CAN 0x110 | VCP-G 동작 |
|---|---|---|---|
| speed_30 | LIMIT 30 | `[1,30]` | 듀티 상한 30%까지 부드럽게 감속 |
| speed_60 | LIMIT 60 | `[1,60]` | 듀티 상한 60%까지 부드럽게 감속 |
| stop / no_entry | STOP | `[2,0]` | 0%까지 부드럽게 감속 후 정지 |
| clear (제한구역 종료) | RELEASE | `[0,0]` | 상한 해제 |
| none (표지판 없음) | — | (송신 안 함) | 직전 명령 유지 |

- 신뢰도 `conf < CONF_THRESH`(0.5) 검출은 무시. 같은 명령이 `CONFIRM`(2) 프레임 연속이어야 전환.
- 임계값은 `CFG` 딕셔너리에서 튜닝.

## 표지판 JSON (AI-G → D3-G, 잠정)
줄단위 `JSON\n`. 모델 클래스 확정 시 동기화.
```json
{"ts":1234.5, "sign":"speed_30", "conf":0.92}
```
- `sign` : `speed_30` | `speed_60` | `stop` | `no_entry` | `clear` | `none` · `conf` : 0..1

## 통합 시 남은 작업
1. **D3-G R5 하향** — 코드 작성됨: [r5/.../ldar_downstream.c](../r5/sources/app.ldar.bridge/ldar_downstream.c)가
   A72 교육용 IPC 패킷(CMD1=0x05)을 파싱→CAN TX. 파서·CRC는 호스트 상호운용 테스트로 검증.
   **남은 것**: WSL2 R5 환경에서 빌드, `Tcc_Ipc_Recv` 플레이스홀더를 실제 BSP IPC 수신 API로 교체,
   `LdarDownstream_Init()` 호출 추가. (대안: 교육용 CAN demo 활성화 — 같은 CMD1=0x05를 BSP가 처리.)
   상향(0x120 CAN→IPC)은 `ldar_bridge.c`에 이미 있음.
2. **VCP-G** — 0x110 **CAN 수신** 완료([override_can.c](../../vcp-g/app.ldar.vcp/override_can.c)).
   수신 → `Override_SetLimit/SetStop/Release` → 부드러운 감속/정지(컴파일·ROM 검증).
3. **AI-G 연동(0x110 상위)** — 실제 표지판 검출을 `--source tcp`로 연결 (현재 Mock 시나리오).
