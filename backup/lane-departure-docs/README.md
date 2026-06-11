# LDAR — Lane Departure Auto-Recovery System

**조이스틱 수동 주행 + AI 차선이탈 자동 복귀 SDV 축소 구현**

조이스틱으로 수동 주행하다 차선 이탈이 감지되면 시스템이 제어권을 가져와(Override) 원래 차선으로 자동 복귀한다. 상용차 LDWS/LKAS를 Telechips Zonal 아키텍처 모형 차량에서 재현하여 **카메라 → NPU 추론 → 판정 → CAN 제어 → 액추에이션** 을 End-to-End로 구현한다.

- 팀: **서준상**(전자전기 — 제어: VCP-G + D3-G), **정은진**(소프트웨어 — 인지: AI-G)
- 학기: 2026년 1학기 중앙대 고급프로젝트 (TOPST Advanced Project)

## 문서
| 문서　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　| 내용　　　　　　　　　　　　　　　　　　　　　　　　　　　　　|
| -----------------------------------------------------------------------------------------| ---------------------------------------------------------------|
| [docs/PROTOCOL.md](docs/PROTOCOL.md)　　　　　　　　　　　　　　　　　　　　　　　　　　| CAN 메시지 테이블 · IPC · Decision State Machine · VCP-G 핀맵 |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)　　　　　　　　　　　　　　　　　　　　　　　| 개발 환경 · 빌드 · 플래시 · 모듈 추가법　　　　　　　　　　　 |
| [docs/ROADMAP.md](docs/ROADMAP.md)　　　　　　　　　　　　　　　　　　　　　　　　　　　| 개발 현황 · Phase 태스크 · 자재 · 중간발표 차이　　　　　　　 |
| [d3-g/a72/README.md](d3-g/a72/README.md)　　　　　　　　　　　　　　　　　　　　　　　　| D3-G 판정·명령 앱 사용법　　　　　　　　　　　　　　　　　　　|
| [중간발표 PDF](documents/Project_Presentation/LDAR_차선이탈자동복귀시스템_중간발표.pdf) | (일부 구 설계 — ROADMAP의 "차이" 참조)　　　　　　　　　　　　|

## 시스템 구성 (3-Zonal)

| Zone    | Board              | 역할　　　　　　　　　　　　　　　　　　　　　　　　　　　| 하드웨어　　　　　　　　　　　　　　　　 | 담당　 |
| ---------| --------------------| -----------------------------------------------------------| ------------------------------------------| --------|
| Sensing | **AI-G**           | PiCam → NPU 추론(YOLOv8-seg) → 차선 좌표 + 실선/점선 추출 | A53 Quad + NPU 8TOPS, MIPI CSI-2　　　　 | 정은진 |
| HPC     | **D3-G** (TCC8050) | 이탈 판정 · 복귀각 계산　　　　　　　　　　　　　　　　　 | A72(Linux, 판정) + R5(FreeRTOS, IPC↔CAN) | 서준상 |
| Control | **VCP-G**          | 조이스틱 수동주행 + 제어권 중재 + 모터·서보·LED·버저　　　| MCU + FreeRTOS, ADC/GPIO/PDM/I2C　　　　 | 서준상 |

```
조이스틱(VRx/VRy/SW) ─ADC/GPIO─▶ VCP-G ──[USER]──▶ DC모터 · 서보  (로컬 즉시 제어)
                                    └─상향 CAN 0x120(방향지시)─▶ R5 ─IPC─▶ A72
Camera ─MIPI CSI-2─▶ AI-G ─Ethernet TCP─▶ D3-G A72  (이탈감지 · 상태머신 · P 복귀각)
                                            │ IPC
                                            ▼
                       D3-G R5 ─CAN─▶ 0x110/0x111 + [BOARD]0x106/0x107
                                            ▼
                       VCP-G (제어권 중재) ──[BOARD]──▶ 로컬 조이스틱 무시,
                                                      CAN 조향/속도 적용 + LED/버저
```

**핵심 설계** — 수동 주행 루프는 VCP-G 로컬에서 완결(저지연). D3-G는 판정·복귀각만 계산해 **CAN으로 제어권(Override)만 지시**. VCP-G가 `0x110 Control Authority`를 보고 USER(로컬 조이스틱)/BOARD(CAN 복귀각)를 중재한다.

## 통신 규격 (요약)

| 구간 | 규격 | 주요 데이터 |
|---|---|---|
| Camera → AI-G | MIPI CSI-2 | RAW 영상 |
| AI-G → D3-G(A72) | Ethernet TCP | 좌/우 차선 좌표, 실선(1)/점선(2), 위험도, 타임스탬프 |
| 조이스틱 → VCP-G | ADC(VRx/VRy)·GPIO(SW) | 조향·속도·버튼 |
| VCP-G → D3-G(R5) | Classical CAN 2.0 | 방향지시 의도(상향, 0x120) |
| D3-G A72 ↔ R5 | IPC (`/dev/tcc_ipc_micom`) | 헤더+명령+CRC |
| D3-G(R5) → VCP-G | Classical CAN 2.0 (11-bit) | 제어권·차선상태·조향·속도 (0x110/0x111/0x106/0x107) |
| VCP-G → Actuators | GPIO·PDM(PWM)·I2C | 방향핀·PWM·LED·버저 |

→ 전체 CAN 테이블·페이로드·핀맵은 [docs/PROTOCOL.md](docs/PROTOCOL.md).

## 디렉터리 구조

```
LDAR-System/
├── ai-g/                              # Sensing Zone (정은진): PiCam·NPU 추론(YOLOv8-seg)·후처리·TCP 송신
│   ├── data_pipeline/                 #   학습 데이터: PiCam 촬영(capture.py)·프레임추출(extract_frames.py)
│   ├── ai-g app/ · ai_model/ · qt/
│
├── d3-g/                              # HPC Zone (서준상)
│   ├── a72/                           #   판정·명령 앱 (Python)
│   │   ├── ldar_decision.py           #     TCP/Mock 입력 + 상태머신 + P 복귀각
│   │   ├── ldar_can.py                #     하향 CAN 명령(0x110/0x111/0x107/0x106) — IPC 송신
│   │   ├── Library/IPC_Library.py     #     IPC(CRC16) transport
│   │   ├── ldar_listener.c · Makefile #     상향 0x120 IPC 수신 확인(C)
│   │   └── README.md
│   ├── r5/sources/app.ldar.bridge/    #   R5 LDAR 모듈 (CAN↔IPC 브리지)
│   ├── shared/ldar_ipc_proto.h        #   A72↔R5 공용 IPC 헤더
│   ├── D3G-R5/                        #   R5 BSP 풀트리 (외부 git)
│   └── reference/                     #   유사 프로젝트 참고 원본 (src/ bin/ ipc-example/) — README 참조
│
├── vcp-g/                             # Control Zone (서준상)
│   ├── flash/                         #   플래시 패키지 (fwdn + .rom + flash.sh)
│   └── topst-vcp/                     #   VCP-G BSP (untracked; LDAR 소스·산출물만 git -f)
│       └── sources/app.sample/app.ldar.vcp/   # ldar_app/pins, joystick_adc/sw,
│           # motor_dir/pwm, servo_pwm, turn_signal/led/can, buzzer, override, pwm_util
│
├── documents/                         # tutorials(D01–D10 PDF, VCP-G Docs) · d3g_references · Project_Presentation
├── docs/                              # 분리된 상세 문서 (PROTOCOL/DEVELOPMENT/ROADMAP)
├── CLAUDE.md                          # 에이전트 작업 지침 (정의 출처는 README/docs)
└── README.md
```

- **R5 BSP 두 군데**: 풀트리 `d3-g/D3G-R5/`(외부 git), LDAR 모듈 `d3-g/r5/...app.ldar.bridge/` — 빌드 시 오버레이
- **VCP-G 두 군데**: BSP `vcp-g/topst-vcp/`(untracked), LDAR 모듈은 그 안 `app.ldar.vcp/`에 직접. 산출물·도구는 `vcp-g/flash/`로 모아 git 추적

## 현황 (요약)
- ✅ 환경/기초(D01–D06) + 제어 아키텍처 확정
- ✅ **VCP-G 수동주행**(조이스틱·모터·서보·방향지시 토글·부저) + **D3-G 판정앱**(Mock 검증)
- 🟧 **인지(AI-G)** — UFLD→**YOLOv8-seg fine-tune**으로 전환(고정 데모 환경: 검정배경·직선 3m). 데이터 파이프라인 코드 완료, 촬영·라벨·학습 진행
- 🟧 제어 Phase 3(R5 하향 + VCP-G CAN 수신 통합)
- ⬜ Phase 4 통합·튜닝, 데모

→ 상세 단계·자재는 [docs/ROADMAP.md](docs/ROADMAP.md).

## 참고 코드 출처
텔레칩스 교육 코드 + 유사 프로젝트(자율주행 RC)를 적응 — 원본은 [d3-g/reference/](d3-g/reference/)에 정리(파일명/내용 매핑은 그 README 참조).
