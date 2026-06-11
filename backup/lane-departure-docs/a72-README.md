# D3-G A72 — 판정·명령 (LDAR Decision)

AI-G가 인식한 차선을 받아 **이탈·복귀를 판정**하고, 차선을 벗어나면 **VCP-G에 서보를
좌/우로 돌리라고 CAN으로 명령**한다. (인지=AI-G, 판정·명령=D3-G, 구동·중재=VCP-G)

```
AI-G ──Ethernet TCP(차선 JSON)──▶ ldar_decision.py (A72)
                                    │ 판정(SAFE/WARN/OVERRIDE/CRITICAL) + P 복귀각
                                    │ ldar_can.py → IPC(/dev/tcc_ipc_micom)
                                    ▼
                              R5 (CAN demo/bridge) ──CAN──▶ VCP-G
                                0x110 제어권 · 0x111 차선상태 · 0x107 조향 · 0x106 속도
```

## 파일
| 파일 | 역할 |
|---|---|
| `ldar_decision.py` | 메인 앱 — TCP/Mock 차선 입력, 상태머신, P 복귀각, 명령 송신 |
| `ldar_can.py` | 하향 CAN 명령 계층 (0x106/0x107/0x110/0x111) — IPC 전송 |
| `Library/IPC_Library.py` | 교육용 IPC 패킷(CRC16) transport (유사 프로젝트 검증본 재사용) |
| `ldar_listener.c` | 상향 0x120(방향지시) IPC 수신 확인용 (Phase 1) |

## 실행
```bash
# AI-G 없이 상태머신 단독 검증 (Phase 2) — 삼각파로 좌(점선)/우(실선) 이탈 번갈아 유발
python3 ldar_decision.py --source mock --dry-run     # 콘솔만
python3 ldar_decision.py --source mock               # 실제 IPC 송신

# 실제 AI-G TCP 수신 (Phase 4)
python3 ldar_decision.py --source tcp --port 9999
```
`/dev/tcc_ipc_micom` 접근은 root 필요 (`sudo`).

## 판정 매트릭스 (CLAUDE.md 일치)
| 상태 | 조건 | 제어권(0x110) | 동작 |
|---|---|---|---|
| SAFE | 거리 > T_OUT | USER | 로컬 조이스틱 유지 |
| WARNING | 거리 < T_IN & 점선 | USER | 경고만(0x111 warn) |
| OVERRIDE | 거리 < T_IN & 실선 & 방향지시 없음 | BOARD | 0x107 P복귀각 + 0x106 주행속도 |
| CRITICAL | 차선 접촉(\|offset\|≥0.95) | BOARD | 0x107 복귀각 + 0x106 감속(0) |

복귀 자격(BOARD→USER): 거리 > T_OUT, heading 평행, T_HOLD 유지. 임계값·게인은
`CFG` 딕셔너리에서 Phase 4 트랙 실측으로 튜닝.

## 차선 JSON (AI-G → D3-G, 잠정)
줄단위 `JSON\n`. 모델 출력 텐서 확정 시 Phase 4에서 동기화.
```json
{"ts":1234.5, "offset":0.1, "heading":0.0, "left_type":1, "right_type":2, "risk":0.2}
```
- `offset` −1(좌끝)..0(중앙)..+1(우끝) · `heading` +면 우향 · `*_type` 1=실선 2=점선

## 통합 시 남은 작업
1. **D3-G R5** — A72의 IPC 프레임을 CAN으로 송신해야 함. 교육용 CAN demo
   (`MCU_BSP_SUPPORT_CAN_DEMO`)를 R5에 올리거나 `r5/.../ldar_bridge.c`에 하향
   (IPC→CAN TX) 경로를 추가. 상향(0x120 CAN→IPC)은 `ldar_bridge.c`에 이미 있음.
2. **VCP-G** — 0x106/0x107/0x110/0x111 **CAN 수신** 핸들러(Phase 3). 현재 VCP-G는
   `override.c`로 적색 LED·부저만 스텁 상태 → 수신부가 `Override_Set()` + 서보/모터에
   복귀 명령을 적용하면 E2E 완성.
3. **방향지시 의도(0x120)** — 점선 이탈 시 Override 분기에 쓰려면 상향 IPC를
   `ldar_decision.step(lane, driver_intent=...)`에 연결 (현재 0 고정).
