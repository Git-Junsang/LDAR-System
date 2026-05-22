# LDAR — Lane Departure Auto-Recovery System

## Project Overview

조이스틱 수동 주행과 AI 기반 차선이탈 자동 개입을 결합한 SDV(Software-Defined Vehicle) 축소 구현. 상용차의 LDWS/LKAS 기능을 Telechips Zonal 아키텍처 기반 모형 차량에서 재현하여, 카메라 → NPU 추론 → 차량 제어로 이어지는 데이터 흐름을 End-to-End로 구축한다.

- 팀: 서준상(전자전기), 정은진(소프트웨어)
- 학기: 2026년 1학기 중앙대 고급프로젝트 (TOPST Advanced Project)
- 중간발표 자료: [LDAR_차선이탈자동복귀시스템_중간발표.pdf](LDAR_차선이탈자동복귀시스템_중간발표.pdf)

## Architecture (3-Zonal)

| Zone | Board | Role | Hardware |
|---|---|---|---|
| Sensing | **AI-G** | Perception — 카메라 영상 → NPU 추론 → 차선 좌표 추출 | A53 Quad + NPU 8TOPS, 2GB LPDDR4X, MIPI CSI-2 |
| HPC | **D3-G** (TCC8050) | Decision — 입력·인지 통합 → 제어 명령 생성 | A72 (Linux, 판정·UI) + R5 (FreeRTOS, IPC↔CAN) |
| Control | **VCP-G** | Actuation — CAN 수신 → 모터·LED·버저 제어 | MCU + FreeRTOS, GPIO/PDM/I2C |

데이터 흐름:
```
Camera ─MIPI CSI-2─▶ AI-G ─Ethernet TCP─▶ D3-G(A72) ─IPC─▶ D3-G(R5) ─CAN 2.0─▶ VCP-G ─GPIO/PDM─▶ Actuators
                                              ▲
                                     USB HID │
                                          Joystick
```

## Communication Spec

| 구간 | 규격 | 주요 데이터 |
|---|---|---|
| Camera → AI-G | MIPI CSI-2 | RAW 영상 프레임 |
| AI-G → D3-G(A72) | Ethernet TCP | 좌/우 차선 좌표, 실선(1)/점선(2) 라벨, 위험도(0.0~1.0), 타임스탬프 |
| Joystick → D3-G(A72) | USB HID | 조향 축(X), 속도 축(Y), 트리거 버튼 |
| D3-G A72 ↔ R5 | IPC (`/dev/tcc_ipc_micom`) | 제어 명령 패킷: 헤더 + 명령 + CRC |
| D3-G(R5) → VCP-G | Classical CAN 2.0 (11-bit ID) | ID별 제어 메시지 |
| VCP-G → Actuators | GPIO · PDM(PWM) · I2C | 방향 핀, PWM 듀티, LCD |

## CAN Message Table

| 메시지 | CAN ID | Data 구조 | 비고 |
|---|---|---|---|
| Brake Light | 0x101 | `[0]` on=0x01 / off=0x02 | 기존 |
| Turn Signal | 0x102 | `[0]` left=0x01, right=0x02 / `[1]` on/off | 기존 |
| Emergency Signal | 0x103 | `[0]` on=0x01 (경고음 연동) | 기존 |
| Head Light | 0x104 | `[0]` on=0x01 / off=0x02 | 기존 |
| Vehicle Speed | 0x106 | `[0]` 0~80 (DC 모터 duty %) | 기존 |
| Wheel Angle | 0x107 | `[0]` 0~127 (서보 각도) | 기존 |
| **Control Authority** | **0x110** | `[0]` USER=0x01 / BOARD=0x02 | **신규** |
| **Lane Status** | **0x111** | `[0]` safe/warn/depart / `[1]` solid/dashed | **신규** |

신규 메시지 의미:
- `0x110 Control Authority` — 현재 제어권 주체. 계기판 UI 색상·아이콘, 조향 입력 무시 여부 결정 근거
- `0x111 Lane Status` — 차선 상태와 종류. 계기판 실선/점선 경고 차별 표시, 경고음 패턴 분기

## Decision State Machine (D3-G A72)

```
USER_CONTROL ──(점선 접근 · 거리<Tin)──▶ WARNING
USER_CONTROL ──(실선 접근 · 거리<Tin · 접근속도>0 · 방향지시 없음)──▶ BOARD_OVERRIDE
BOARD_OVERRIDE ──(거리>Tout · 홀드시간 Th · heading 평행)──▶ USER_CONTROL
```

판정 매트릭스:

| 상태 | 조건 | 동작 |
|---|---|---|
| SAFE | 거리 > Tout | 사용자 제어 유지 |
| WARNING | 거리 < Tin & 점선 쪽 | 경고음만 |
| OVERRIDE | 거리 < Tin & 실선 쪽 | 제어권 이양 + 조향 복귀 |
| CRITICAL | 차선 접촉/통과 | 즉시 감속 + 조향 복귀 |

임계값 `Tin`, `Tout`, 홀드시간 `Th`, heading 임계각은 Phase 4(DEBUG)에서 튜닝 확정.

## Current Status

- ✅ 완료: D01-D02 환경 구성, D02-D03 VCP-G 주변장치, D03-D04 CAN/IPC, D05 Control Zone, D06 AI-G+NPU
- 🟧 진행 중: **D07 Sensing Zone AI 모델 탑재** — UFLD 차선 검출, 추론 결과 확인
- ⬜ 예정: D08-D09 HPC Zone 통합, D10 추가 구현 / 발표

## Roadmap (Phase / Task)

### Phase 1 — SENSING: AI-G 모델 탑재 및 검증
- [ ] T1.1 MobileNet-V2 + U-Net 모델 변환 (ONNX → tc-nn-toolkit 입력)
- [ ] T1.2 양자화 + NPU 컴파일 (tc-nn-toolkit)
- [ ] T1.3 AI-G 보드에 모델 배포, tc-nn-app 런타임으로 실행
- [ ] T1.4 카메라 입력 → 추론 → 후처리 파이프라인 완성 (좌표 + 실선/점선 라벨 출력)
- [ ] T1.5 추론 결과 터미널 로그 검증
- **Exit criteria**: 실시간 차선 인식 좌표가 콘솔에 정상 출력

### Phase 2 — CONTROL: 주행 시스템 및 이탈 감지
- [ ] T2.1 조이스틱(USB HID) 입력 파싱 → 조향/속도 매핑
- [ ] T2.2 D3-G A72에서 좌표 맵 기반 이탈 감지 알고리즘 (거리·접근속도·차선종류)
- [ ] T2.3 Decision Matrix 상태 머신 구현 (SAFE/WARNING/OVERRIDE/CRITICAL)
- [ ] T2.4 자동 복귀(조향 복귀) 알고리즘 구현
- [ ] T2.5 A72 → R5 IPC 패킷 정의 및 송수신 (헤더+명령+CRC)
- **Exit criteria**: 이탈 감지 → 자동 복귀가 D3-G 단독으로 동작

### Phase 3 — INTEGRATION: 보드 간 통합
- [ ] T3.1 AI-G → D3-G Ethernet TCP 송수신 (패킷 포맷 확정 후 구현)
- [ ] T3.2 D3-G(R5) → VCP-G CAN 송신 — 기존 ID(0x101~0x107) + 신규(0x110, 0x111)
- [ ] T3.3 VCP-G CAN 수신 → 모터/서보/LED/버저 액추에이션
- [ ] T3.4 제어권 전환 상태 머신 End-to-End 검증
- [ ] T3.5 QT 계기판 App (제어권 상태·속도 시각화)
- **Exit criteria**: 카메라 → NPU → 판정 → CAN → 액추에이션 전체 흐름 동작

### Phase 4 — DEBUG: 안정화 및 튜닝
- [ ] T4.1 엣지 케이스 디버깅 (곡선 구간, 조명 변화)
- [ ] T4.2 임계값 튜닝 (Tin, Tout, 홀드시간 Th, heading 임계각)
- [ ] T4.3 추론 속도 / 응답 지연 측정 및 최적화
- [ ] T4.4 채터링·오작동 시나리오 회귀 테스트
- **Exit criteria**: 시연 시나리오 5종(정상/실선이탈/점선이탈 무방향/점선이탈+방향지시/제어권복귀) 안정 통과

### Phase 5 — DEMO: 시연 및 산출물
- [ ] T5.1 테스트 트랙 제작 (실선+점선, 직선+완만 곡선)
- [ ] T5.2 최종 데모 시연 + 영상 촬영
- [ ] T5.3 최종 발표 자료 정리

## Development Environment (확정: WSL2, 단일 PC)

- 호스트: Windows 11 + **WSL2 (Ubuntu)** — D01-T02 학습 환경 그대로
- VCP-G(USB UART) 연결:
  ```powershell
  usbipd list                          # 디바이스 BUSID 확인
  usbipd bind --busid <ID>             # 최초 1회 (관리자)
  usbipd attach --busid <ID> --wsl     # 매 부팅 시 필요
  ```
  WSL 내부에서 `/dev/ttyUSB0` 또는 `/dev/ttyACM0`로 인식
- D3-G / AI-G(Ethernet): 호스트 NIC 공유, WSL2 자동 인식
- **빌드 위치 주의**: 본 저장소는 외장 Z 드라이브에 있음. Yocto / R5 / Linux 빌드 산출물은 반드시 **WSL 홈(`~/`)에서 수행** — `/mnt/z/`(NTFS bridge)는 빌드 I/O 매우 느림. 본 repo에는 문서·정의·통합 코드만 두고 대용량 산출물은 `.gitignore`로 제외
- 도구: tc-nn-toolkit, tc-nn-app (보유), Yocto, FreeRTOS toolchain

## Required Materials

| 분류 | 항목 | 상태 |
|---|---|---|
| Boards | TOPST AI-G / D3-G / VCP-G, MIPI CSI 카메라, Ethernet 허브·케이블, USB-to-TTL UART | 보유 |
| Vehicle | 모형 섀시, DC 모터, 서보 모터, L298N, LED, 점퍼선·저항 | 보유 |
| Input | 조이스틱 (USB HID), 피에조 스피커 | **구매 필요** |
| Input | 모터 전용 배터리 팩 | 보유/확인 |
| Env | 실선&점선 차선 트랙 | **제작** |
| Env | 차선 학습 데이터셋 | **수집/선정** |
| Env | tc-nn-toolkit, tc-nn-app, Yocto / FreeRTOS | 보유 |

## Repository Layout

- [ai-g/](ai-g/) — Sensing Zone: 카메라 캡처, NPU 추론 (MobileNet-V2 + U-Net), 후처리, Ethernet 송신
- [d3-g/](d3-g/) — HPC Zone: A72(판정·UI·IPC 송신) + R5(IPC 수신·CAN 송신)
- [vcp-g/](vcp-g/) — Control Zone: CAN 수신, GPIO/PDM/I2C로 모터·서보·LED·버저 제어
- [documents/](documents/) — 텔레칩스 팹리스 교육과정 D01-D10 PDF 38개 (참고 자료)
- [LDAR_차선이탈자동복귀시스템_중간발표.pdf](LDAR_차선이탈자동복귀시스템_중간발표.pdf) — 중간발표 자료 (모든 정의의 출처)

## Notes

- CAN ID 0x110(Control Authority), 0x111(Lane Status)은 본 프로젝트 신규 메시지 — 기존 `can_vcp_ctrl.c` 확장
- AI-G → D3-G 패킷 포맷은 추론 모델 출력 텐서 구조 확인 후 확정 (현재 설계 단계)
- 추론은 AI-G NPU 전담, CPU는 전·후처리·전송만 — 실시간성 확보
- 인지(AI-G) ↔ 판정(D3-G) ↔ 구동(VCP-G) 책임 분리 — Zonal 아키텍처 원칙
