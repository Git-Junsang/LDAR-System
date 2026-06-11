# LDAR — 표지판 인식 속도 오버라이드 시스템

**조이스틱 수동 주행 + AI 표지판 인식 속도/정지 자동 오버라이드 SDV 축소 구현**

조이스틱으로 수동 주행하다 카메라가 교통표지판(속도제한 30/60, 정지·진입금지)을 인식하면
시스템이 **속도를 강제로(부드럽게) 제한하거나 정지**시킨다. 상용차 표지판 인식·속도보조(ISA)를
Telechips Zonal 아키텍처 모형 차량에서 재현하여 **카메라 → NPU 추론 → 판정 → CAN 제어 →
액추에이션**을 End-to-End로 구현한다. 조향은 항상 운전자(조이스틱)가 쥔다 — 개입은 속도뿐.

> **코드네임 LDAR** — 원래 "Lane Departure Auto-Recovery"(차선이탈 자동복귀)였으나 표지판 기반
> 속도 오버라이드로 방향 전환. 차선은 더 이상 인식하지 않는다. 구 설계 자료는 [backup/](backup/).

- 팀: **서준상**(전자전기 — 제어: VCP-G + D3-G), **정은진**(소프트웨어 — 인지: AI-G)
- 학기: 2026년 1학기 중앙대 고급프로젝트 (TOPST Advanced Project)

## 문서
| 문서 | 내용 |
|---|---|
| [docs/PROTOCOL.md](docs/PROTOCOL.md) | CAN 메시지 테이블 · IPC · 판정 매핑 · VCP-G 핀맵 |
| [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) | 개발 환경 · 빌드 · 플래시 · 모듈 추가법 |
| [docs/ROADMAP.md](docs/ROADMAP.md) | 개발 현황 · Phase 태스크 · 자재 |
| [d3-g/a72/README.md](d3-g/a72/README.md) | D3-G 판정·명령 앱 사용법 |
| [backup/README.md](backup/README.md) | 구 차선이탈 설계 자료(참고용 보관) |

## 시스템 구성 (3-Zonal)

| Zone | Board | 역할 | 하드웨어 | 담당 |
|---|---|---|---|---|
| Sensing | **AI-G** | PiCam → NPU 추론(YOLOv8) → **교통표지판 검출**(클래스+신뢰도) | A53 Quad + NPU 8TOPS, MIPI CSI-2 | 정은진 |
| HPC | **D3-G** (TCC8050) | 표지판 → **속도제한/정지 판정** | A72(Linux, 판정) + R5(FreeRTOS, IPC↔CAN) | 서준상 |
| Control | **VCP-G** | 조이스틱 수동주행 + **속도 오버라이드 중재** + 모터·서보·LED·부저 | MCU + FreeRTOS, ADC/GPIO/PDM/I2C | 서준상 |

```
조이스틱(VRx/VRy/SW) ─ADC/GPIO─▶ VCP-G ─────▶ DC모터 · 서보  (로컬 즉시 제어, 저지연)
                                    └─상향 CAN 0x120(방향지시)─▶ R5 ─IPC─▶ A72
Camera ─MIPI CSI-2─▶ AI-G ─Ethernet TCP─▶ D3-G A72  (표지판 검출 → 속도 판정)
                                            │ IPC
                                            ▼
                       D3-G R5 ─CAN 0x110─▶ Speed Override (mode + 한계속도)
                                            ▼
                       VCP-G ──▶ 한계속도까지 부드럽게 감속 / 정지 (조향은 조이스틱 유지)
```

**핵심 설계** — 수동 주행 루프는 VCP-G 로컬에서 완결(저지연). D3-G는 표지판→속도만 판정해
**CAN 0x110(Speed Override)으로 속도 상한/정지만 지시**. VCP-G는 그 명령을 받아 적용 상한을
틱마다 슬루-레이트로 이동시켜 **급변 없이 부드럽게** 감속/정지한다. 상한 아래에서는 조이스틱이
그대로 통과하고, **조향은 어떤 경우에도 운전자**가 쥔다.

## 통신 규격 (요약)

| 구간 | 규격 | 주요 데이터 |
|---|---|---|
| Camera → AI-G | MIPI CSI-2 | RAW 영상 |
| AI-G → D3-G(A72) | Ethernet TCP | 표지판 클래스, 신뢰도, 타임스탬프 |
| 조이스틱 → VCP-G | ADC(VRx/VRy)·GPIO(SW) | 조향·속도·버튼 |
| VCP-G → D3-G(R5) | Classical CAN 2.0 | 방향지시 의도(상향, 0x120) |
| D3-G A72 ↔ R5 | IPC (`/dev/tcc_ipc_micom`) | 헤더+명령+CRC |
| D3-G(R5) → VCP-G | Classical CAN 2.0 (11-bit) | **속도 오버라이드(0x110)** — mode + 한계속도 |
| VCP-G → Actuators | GPIO·PDM(PWM)·I2C | 방향핀·PWM·LED·부저 |

→ 전체 CAN 테이블·페이로드·핀맵은 [docs/PROTOCOL.md](docs/PROTOCOL.md).

## 표지판 → 동작

| 표지판 | D3-G 판정 | CAN 0x110 | VCP-G 동작 |
|---|---|---|---|
| 속도제한 30 | LIMIT, 30 | `[0]=1, [1]=30` | 듀티 상한 30%로 부드럽게 감속, 그 아래선 조이스틱 자유 |
| 속도제한 60 | LIMIT, 60 | `[0]=1, [1]=60` | 듀티 상한 60%로 부드럽게 감속 |
| 정지 / 진입금지 | STOP | `[0]=2` | 0%까지 부드럽게 감속 후 브레이크 정지 |
| (제한구역 종료) | RELEASE | `[0]=0` | 상한 해제 — 조이스틱 풀스로틀 복귀 |

> 한계속도(km/h)는 모형차 듀티%에 1:1로 대응(30→30%, 60→60%).

## 디렉터리 구조

```
LDAR-System/
├── ai-g/                              # Sensing Zone (정은진): PiCam·NPU 추론(YOLOv8)·표지판 검출·TCP 송신
│   ├── data_pipeline/                 #   학습 데이터: PiCam 촬영(capture.py)·프레임추출(extract_frames.py)
│   ├── ai-g app/ · ai_model/ · qt/
│
├── d3-g/                              # HPC Zone (서준상)
│   ├── a72/                           #   판정·명령 앱 (Python)
│   │   ├── ldar_decision.py           #     TCP/Mock 표지판 입력 → 속도 명령 매핑
│   │   ├── ldar_can.py                #     하향 CAN 0x110(Speed Override) — IPC 송신
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
│       └── sources/app.sample/app.ldar.vcp/   # ldar_app/pins, joystick_adc/sw, motor_dir/pwm,
│           # servo_pwm, turn_signal/led/can, override(+override_can), buzzer, pwm_util
│
├── backup/                            # 구 차선이탈 설계 자료(문서·D3-G 판정앱·VCP-G 조향 오버라이드)
├── documents/                         # tutorials(D01–D10 PDF, VCP-G Docs) · d3g_references · Project_Presentation
├── docs/                              # 분리된 상세 문서 (PROTOCOL/DEVELOPMENT/ROADMAP)
├── CLAUDE.md                          # 에이전트 작업 지침 (정의 출처는 README/docs)
└── README.md
```

- **R5 BSP 두 군데**: 풀트리 `d3-g/D3G-R5/`(외부 git), LDAR 모듈 `d3-g/r5/...app.ldar.bridge/` — 빌드 시 오버레이
- **VCP-G 두 군데**: BSP `vcp-g/topst-vcp/`(untracked), LDAR 모듈은 그 안 `app.ldar.vcp/`에 직접. 산출물·도구는 `vcp-g/flash/`로 모아 git 추적

## 현황 (요약)
- ✅ 환경/기초(D01–D06) + 제어 아키텍처 확정
- ✅ **VCP-G 수동주행**(조이스틱·모터·서보·방향지시 토글·부저)
- ✅ **VCP-G 속도 오버라이드** — CAN 0x110 수신 → 부드러운 감속/정지(컴파일·ROM 검증) + **D3-G 판정앱**(표지판→명령, Mock 검증)
- 🟧 **인지(AI-G)** — YOLOv8 표지판 검출 학습·보드 탑재 진행
- 🟧 제어 Phase 3 통합(R5 하향 IPC→CAN 0x110 송신, AI-G→D3-G TCP 연동)
- ⬜ Phase 4 통합·튜닝, 데모

→ 상세 단계·자재는 [docs/ROADMAP.md](docs/ROADMAP.md).

## 참고 코드 출처
텔레칩스 교육 코드 + 유사 프로젝트(자율주행 RC)를 적응 — 원본은 [d3-g/reference/](d3-g/reference/)에 정리(파일명/내용 매핑은 그 README 참조).
