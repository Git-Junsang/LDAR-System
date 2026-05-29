# LDAR — Lane Departure Auto-Recovery System

> **단일 출처**: 프로젝트 정의는 [README.md](README.md)를 따른다. 본 문서와 README가 불일치하면 **README가 기준**.
> 중간발표 PDF는 일부 구 설계가 남아 있다 — 하단 "중간발표 PDF와의 차이" 참조.

## Project Overview

조이스틱 수동 주행과 AI 기반 차선이탈 자동 개입을 결합한 SDV(Software-Defined Vehicle) 축소 구현. 조이스틱으로 수동 주행하다 차선 이탈이 감지되면 시스템이 제어권을 가져와(Override) 원래 차선으로 자동 복귀한다. 상용차의 LDWS/LKAS를 Telechips Zonal 아키텍처 기반 모형 차량에서 재현하여 **카메라 → NPU 추론 → 판정 → CAN 제어 → 액추에이션** 을 End-to-End로 구축한다.

- 팀: **서준상**(전자전기 — 제어 측: VCP-G + D3-G), **정은진**(소프트웨어 — 인지 측: AI-G)
- 학기: 2026년 1학기 중앙대 고급프로젝트 (TOPST Advanced Project)
- 중간발표 자료: [LDAR_차선이탈자동복귀시스템_중간발표.pdf](documents/Project_Presentation/LDAR_차선이탈자동복귀시스템_중간발표.pdf)

## Architecture (3-Zonal)

| Zone | Board | Role | Hardware | 담당 |
|---|---|---|---|---|
| Sensing | **AI-G** | Perception — 카메라 영상 → NPU 추론 → 차선 좌표 추출 | A53 Quad + NPU 8TOPS, 2GB LPDDR4X, MIPI CSI-2 | 정은진 |
| HPC | **D3-G** (TCC8050) | Decision — 차선 데이터 기반 이탈 판정·복귀각 계산 | A72(Linux, 판정) + R5(FreeRTOS, IPC↔CAN) | 서준상 |
| Control | **VCP-G** | Actuation — 조이스틱 수동주행 + 제어권 중재 + 모터·서보·LED·버저 | MCU + FreeRTOS, ADC/GPIO/PDM/I2C | 서준상 |

데이터 흐름:
```
조이스틱(VRx/VRy/SW) ─ADC/GPIO─▶ VCP-G ──[USER]──▶ DC모터 · 서보  (로컬 즉시 제어)
                                    │
                                    └─상향 CAN 0x120(방향지시 의도)─▶ R5 ─IPC─▶ A72
                                                                              │
Camera ─MIPI CSI-2─▶ AI-G ─Ethernet TCP─▶ D3-G A72  (이탈감지 · 상태머신 · P 복귀각)
                                                       │ IPC (헤더+명령+CRC)
                                                       ▼
                              D3-G R5 ─CAN─▶ 0x110/0x111 + [BOARD]0x106/0x107
                                                       │
                                                       ▼
                              VCP-G (제어권 중재) ──[BOARD]──▶ 로컬 조이스틱 무시,
                                                              CAN 조향/속도 적용 + LED/버저
```

**핵심 설계** — 수동 주행 루프는 VCP-G 로컬에서 완결(저지연). D3-G는 인지 결과로 판정·복귀각만 계산해 **CAN으로 제어권(Override)만 지시**. VCP-G가 `0x110 Control Authority`를 보고 USER(로컬 조이스틱) / BOARD(CAN 복귀각)를 중재한다.

## Communication Spec

| 구간 | 규격 | 주요 데이터 |
|---|---|---|
| Camera → AI-G | MIPI CSI-2 | RAW 영상 프레임 |
| AI-G → D3-G(A72) | Ethernet TCP | 좌/우 차선 좌표, 실선(1)/점선(2) 라벨, 위험도(0.0~1.0), 타임스탬프 |
| 조이스틱 → VCP-G | ADC(VRx/VRy) · GPIO(SW) | 조향축, 속도축, 버튼 |
| VCP-G → D3-G(R5) | Classical CAN 2.0 | 방향지시 의도(상향, 0x120) |
| D3-G A72 ↔ R5 | IPC (`/dev/tcc_ipc_micom`) | 제어 명령 패킷: 헤더 + 명령 + CRC |
| D3-G(R5) → VCP-G | Classical CAN 2.0 (11-bit ID) | 제어권·차선상태·조향·속도 |
| VCP-G → Actuators | GPIO · PDM(PWM) · I2C | 방향 핀, PWM 듀티, LED, 버저 |

## CAN Message Table

| 메시지 | CAN ID | 방향 | Data 구조 | 비고 |
|---|---|---|---|---|
| Brake Light | 0x101 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존 |
| Turn Signal | 0x102 | R5→VCP | `[0]` left=0x01, right=0x02 / `[1]` on/off | 기존 |
| Emergency Signal | 0x103 | R5→VCP | `[0]` on=0x01 (경고음 연동) | 기존 |
| Head Light | 0x104 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존 |
| Vehicle Speed | 0x106 | R5→VCP | `[0]` 0~80 (DC 모터 duty %) | 기존 / BOARD 시 복귀 속도 |
| Wheel Angle | 0x107 | R5→VCP | `[0]` 0~127 (서보 각도) | 기존 / BOARD 시 복귀 조향 |
| **Control Authority** | **0x110** | R5→VCP | `[0]` USER=0x01 / BOARD=0x02 | **신규** |
| **Lane Status** | **0x111** | R5→VCP | `[0]` safe/warn/depart / `[1]` solid/dashed | **신규** |
| **Driver Input** | **0x120** | VCP→R5 | `[0]` 방향지시(L/R/off) · (옵션) 속도·조향 | **신규** (상향) |

신규 메시지 의미:
- `0x110 Control Authority` — 현재 제어권 주체. **VCP-G의 조향 소스 중재 기준**(USER=로컬 조이스틱, BOARD=CAN 복귀각), 상태 LED 색상 결정
- `0x111 Lane Status` — 차선 상태와 종류. 실선/점선 경고 차별 표시, 경고음 패턴 분기
- `0x120 Driver Input` — VCP-G→D3-G **상향** 운전자 의도(방향지시). 점선 이탈 시 방향지시 여부로 Override 분기

## Decision State Machine (D3-G A72)

```
USER_CONTROL ──(점선 접근 · 거리<Tin)──────────────────────────▶ WARNING
USER_CONTROL ──(실선 접근 · 거리<Tin · 접근속도>0 · 방향지시 없음)──▶ BOARD_OVERRIDE
BOARD_OVERRIDE ──(거리>Tout · 홀드시간 Th · heading 평행)─────────▶ USER_CONTROL
(차선 접촉/통과) ──────────────────────────────────────────────▶ CRITICAL
```

판정 매트릭스:

| 상태 | 조건 | 동작 | 상태 LED |
|---|---|---|---|
| SAFE | 거리 > Tout | 사용자 제어 유지 | 🟢 초록 (직접주행) |
| WARNING | 거리 < Tin & 점선 쪽 | 경고음만 (점선 허용) | 🟠 주황 |
| OVERRIDE | 거리 < Tin & 실선 쪽 & 방향지시 없음 | 제어권 이양 + 조향 복귀 | 🔴 빨강 |
| CRITICAL | 차선 접촉/통과 | 즉시 감속 + 조향 복귀 | 🔴 빨강 |

- 복귀 조향은 **P 비례 제어**(횡오프셋·heading 오차 기반). 게인·임계값(`Tin`,`Tout`,`Th`,heading 각)은 Phase 4에서 튜닝 확정
- 점선 이탈 + 방향지시(0x120) → 의도적 차선 변경으로 보고 Override 하지 않음

## Development Environment (하이브리드 — 보드 진단 결과 반영)

작업물에 따라 빌드 위치가 다름:

| 작업물 | 위치 | 빌드 |
|---|---|---|
| A72 판정 앱 | **D3-G 보드** (Remote-SSH) | native gcc (보드에 gcc/g++/make 존재, Yocto 재빌드 불필요) |
| R5 펌웨어 | **Windows WSL2** | 텔레칩스 R5 빌드환경 (D02-T01) |
| VCP-G 펌웨어 | **Windows WSL2** | `topst-vcp` BSP — easy-setup + make + FWDN |

- **git**: GitHub origin(`Git-Junsang/LDAR-System`)을 각 환경에 clone. 네트워크 드라이브 공유 워킹트리 금지
- **D3-G 보드**: IP `192.168.0.35`(DHCP), `/dev/tcc_ipc_micom` 존재(A72↔R5 IPC). `/` 파티션은 `sudo resize2fs /dev/mmcblk0p4`로 16G→28G 확장
- **LDAR VCP-G 코드는 `vcp-g/`에** 두고 빌드 시 `~/topst-vcp` 앱 소스에 오버레이. BSP(~76MB)는 repo에 커밋 안 함
- **VCP-G 빌드·플래시(WSL2)**:
  ```bash
  git clone https://github.com/topst-development/FreeRTOS-VCP ~/topst-vcp   # 최초 1회 (branch: develop)
  cd ~/topst-vcp && ./easy-setup_vcp-g.sh                                    # 라이선스 [Tab]→[Enter]
  cd build/tcc70xx/gcc && make                                              # → output/tcc70xx_pflash_boot_2M_ECC.rom
  sudo ~/topst-vcp/tools/fwdn_vcp/fwdn --fwdn ~/topst-vcp/tools/fwdn_vcp/vcp_fwdn.rom \
    -w ~/topst-vcp/build/tcc70xx/gcc/output/tcc70xx_pflash_boot_2M_ECC.rom  # FWDN 모드(스위치+12V/1A)에서
  minicom -D /dev/ttyUSB0 -b 115200 -8                                      # 콘솔 확인
  ```
  Windows→WSL2 USB 전달: `usbipd` (관리자 PowerShell `usbipd attach --wsl --busid <id>`) + CP210x 드라이버
- **CAN 디버깅**: A72에 SocketCAN(`can0`) 없음 → `candump` 불가. **외장 USB-CAN 분석기** 또는 R5/VCP 콘솔 로그로 확인
- 도구: tc-nn-toolkit, tc-nn-app (보유), R5/VCP FreeRTOS toolchain (WSL2)

## Repository Layout

- [ai-g/](ai-g/) — Sensing Zone (정은진): 카메라 캡처, NPU 추론(MobileNet-V2 + U-Net), 후처리, Ethernet TCP 송신
- [d3-g/](d3-g/) — HPC Zone (서준상)
  - `a72/` — 판정 앱: 차선 데이터 수신, 이탈감지, 상태머신, P 복귀각, IPC 송신
  - `r5/` — IPC 수신 ↔ CAN 송수신 (0x110/0x111/0x106/0x107 송신, 0x120 수신)
- [vcp-g/](vcp-g/) — Control Zone (서준상): 조이스틱 ADC, 제어권 중재, 모터·서보·LED·버저 (`can_vcp_ctrl.c` 확장)
- `shared/` — 공용 프로토콜 정의 (CAN ID·페이로드, IPC 패킷) — 단일 소스
- [documents/](documents/) — 텔레칩스 팹리스 교육과정 D01-D10 PDF (참고 자료)
- [LDAR_차선이탈자동복귀시스템_중간발표.pdf](documents/Project_Presentation/LDAR_차선이탈자동복귀시스템_중간발표.pdf) — 중간발표 자료

## Current Status

- ✅ 완료: D01-D02 환경 구성, D02-D03 VCP-G 주변장치(GPIO/ADC/PDM/SPI/I2C), D03-D04 CAN/IPC, D05 Control Zone, D06 AI-G+NPU, 제어 측 아키텍처 확정(아날로그 조이스틱+VCP-G 로컬 중재, 0x110/0x111/0x120, 3색 LED)
- 🟧 진행 중: **인지(AI-G)** UFLD 차선 검출·추론 검증(정은진), **제어(VCP-G/D3-G)** Phase 1부터 구현(서준상)
- ⬜ 예정: 보드 간 통합, 튜닝, 데모
- **모든 자재 준비 완료 — 구현만 남음**

## Roadmap (Phase / Task)

### 인지 (AI-G) — 담당: 정은진
- [ ] MobileNet-V2 + U-Net / UFLD 모델 변환 (ONNX → tc-nn-toolkit)
- [ ] 양자화 + NPU 컴파일, AI-G 배포, tc-nn-app 실행
- [ ] 카메라 → 추론 → 후처리 (좌표 + 실선/점선 라벨), 콘솔 로그 검증
- **Exit**: 실시간 차선 인식 좌표가 콘솔에 정상 출력

### 제어 Phase 1 — VCP-G 수동 주행 (로컬 완결) — 서준상
- [ ] T1.1 조이스틱 ADC 읽기 — VRx(조향)/VRy(속도) 2채널 + SW GPIO, 노이즈 필터·데드존·정규화
- [ ] T1.2 입력 매핑 → DC모터(L298N 방향핀+PWM duty 0~80%), 서보각(0~127) 직접 액추에이션
- [ ] T1.3 방향지시 의도(SW/택트 버튼 2개) → 상향 CAN `0x120` 송신
- [ ] T1.4 D3-G R5: `0x120` 수신 → IPC로 A72 전달
- **Exit**: 조이스틱으로 모형차 수동 주행이 VCP-G 단독으로 동작

### 제어 Phase 2 — 이탈 감지 & 판정·복귀각 (D3-G A72, Mock 기반)
- [ ] T2.1 Mock 차선 데이터 주입기 (실제 AI-G TCP와 동일 구조체, 입력 소스 인터페이스 분리)
- [ ] T2.2 이탈 감지 (횡거리, 접근속도, 실선/점선 판별)
- [ ] T2.3 Decision Matrix 상태머신 (SAFE/WARNING/OVERRIDE/CRITICAL)
- [ ] T2.4 P 비례 복귀각 계산 (횡오프셋·heading 오차, CRITICAL 감속)
- [ ] T2.5 A72→R5 IPC 패킷(제어권·차선상태·복귀각/속도) 송신
- **Exit**: Mock으로 이탈→판정→복귀각 산출이 D3-G 단독 동작 (콘솔 검증)

### 제어 Phase 3 — 오버라이드 CAN & 액추에이션 (제어권 전환)
- [ ] T3.1 CAN 메시지 정의 (0x110/0x111/0x120) + BOARD 시 0x106/0x107 복귀 명령
- [ ] T3.2 D3-G R5: 상태머신 출력 → 0x110/0x111 + (BOARD)0x106/0x107 송신
- [ ] T3.3 VCP-G 제어권 중재 (USER=로컬 조이스틱, BOARD=CAN 조향, 전환 램프) — `can_vcp_ctrl.c` 확장
- [ ] T3.4 상태 표시 — 🟢초록/🟠주황/🔴빨강 LED + 부저(점선 경고음 0x103 연동/실선·CRITICAL 패턴)
- [ ] T3.5 제어권 전환 End-to-End (채터링 없는 핸드오프)
- **Exit**: Mock→D3-G→CAN→VCP-G 오버라이드 흐름 + LED/부저/복귀 액추에이션 동작

### 제어 Phase 4 — AI-G 통합 & 튜닝
- [ ] T4.1 AI-G→D3-G TCP 수신 구현 (패킷 포맷 확정, Mock → 실제 TCP 교체)
- [ ] T4.2 임계값·게인 튜닝 (Tin, Tout, Th, heading 각, P 게인, 복귀 램프)
- [ ] T4.3 엣지 케이스 (곡선·조명 변화, 채터링·오작동 회귀 테스트)
- [ ] T4.4 응답 지연·추론속도 측정 및 최적화
- **Exit**: 시연 5종(정상/실선이탈/점선이탈 무방향/점선이탈+방향지시/제어권 복귀) 안정 통과

### 통합 · 데모 — 공동
- [ ] 카메라→NPU→판정→CAN→액추에이션 전체 흐름 검증
- [ ] (옵션) QT 계기판 App — 제어권 상태·속도 시각화
- [ ] 최종 데모 시연 + 영상 촬영, 발표 자료 정리
- (테스트 트랙·데이터셋은 준비 완료)

## Required Materials (모두 준비 완료)

| 분류 | 항목 | 상태 |
|---|---|---|
| Boards | TOPST AI-G / D3-G / VCP-G, MIPI CSI 카메라, Ethernet 허브·케이블, USB-to-TTL UART | ✅ 보유 |
| Vehicle | 모형 섀시, DC 모터, 서보 모터, L298N, LED, 점퍼선·저항 | ✅ 보유 |
| Input | 아날로그 조이스틱(VRx/VRy/SW), 방향지시 택트 버튼 2개, 상태 LED 3색, 피에조 스피커, 배터리 팩 | ✅ 보유 |
| Env | 실선&점선 차선 트랙(직선+완만 곡선), 차선 학습 데이터셋 | ✅ 준비 완료 |
| Env | tc-nn-toolkit, tc-nn-app, Yocto / FreeRTOS toolchain | ✅ 보유 |

## Notes

- 신규 CAN ID 0x110(Control Authority)·0x111(Lane Status)·0x120(Driver Input, 상향)은 본 프로젝트 추가 — 기존 `can_vcp_ctrl.c` 확장
- 조이스틱은 **아날로그(VRx/VRy/SW)** — VCP-G ADC로 읽음(USB HID 아님). 수동 주행 루프는 VCP-G 로컬 완결
- AI-G → D3-G 패킷 포맷은 추론 모델 출력 텐서 구조 확인 후 확정 (Phase 4) — 그 전까지 Mock 주입기로 단독 개발
- 추론은 AI-G NPU 전담, CPU는 전·후처리·전송만 — 실시간성 확보
- 인지(AI-G) ↔ 판정(D3-G) ↔ 구동(VCP-G) 책임 분리 — Zonal 아키텍처 원칙

## 중간발표 PDF와의 차이 (PDF는 구 설계, README/CLAUDE가 최신)

중간발표 후 하드웨어·아키텍처가 확정되며 일부 슬라이드가 구 설계로 남음:
- **조이스틱** — PDF(슬라이드 11·13·16): "USB HID, D3-G A72 연결, 구매 필요". → 현재: **아날로그(VRx/VRy/SW), VCP-G ADC, 보유**
- **수동 제어/중재 위치** — PDF(슬라이드 9·11): D3-G A72가 사용자 입력 처리·통합. → 현재: **VCP-G 로컬 중재**, D3-G는 판정·복귀각만 계산해 CAN으로 Override 지시
- **CAN 0x120 Driver Input** — PDF에 없음(0x110/0x111만). → 현재: VCP-G→D3-G **상향 0x120** 추가(방향지시 의도)
- **상태 LED 3색** — PDF(슬라이드 12)는 브레이크/방향지시/전조/경고등만. → 현재: 🟢초록/🟠주황/🔴빨강 제어권·차선 상태 LED 추가
- **자재** — PDF(슬라이드 16): 조이스틱·피에조 구매 필요, 트랙 제작·데이터셋 수집 예정. → 현재: **모두 준비 완료**
