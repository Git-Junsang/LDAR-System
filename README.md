# LDAR — Lane Departure Auto-Recovery System

**조이스틱 수동 주행 + AI 차선이탈 자동 복귀 SDV 축소 구현**

조이스틱으로 수동 주행하다가 차선 이탈이 감지되면 시스템이 제어권을 가져와(Override) 원래 차선으로 자동 복귀한다. 상용차의 LDWS/LKAS 기능을 Telechips Zonal 아키텍처 기반 모형 차량에서 재현하여, **카메라 → NPU 추론 → 판정 → CAN 제어 → 액추에이션** 을 End-to-End로 구현하는 것을 목표로 한다.

- 팀: **서준상**(전자전기 — 제어 측), **정은진**(소프트웨어 — 인지 측)
- 학기: 2026년 1학기 중앙대 고급프로젝트 (TOPST Advanced Project)
- 중간발표 자료: [LDAR_차선이탈자동복귀시스템_중간발표.pdf](documents/Project_Presentation/LDAR_차선이탈자동복귀시스템_중간발표.pdf)

---

## 시스템 구성 (3-Zonal)

| Zone    | Board              | 역할　　　　　　　　　　　　　　　　　　　　　　　　　　　　　| 하드웨어　　　　　　　　　　　　　　　　　　　| 담당　 |
| ---------| --------------------| ---------------------------------------------------------------| -----------------------------------------------| --------|
| Sensing | **AI-G**           | Perception — 카메라 영상 → NPU 추론 → 차선 좌표 추출　　　　　| A53 Quad + NPU 8TOPS, 2GB LPDDR4X, MIPI CSI-2 | 정은진 |
| HPC     | **D3-G** (TCC8050) | Decision — 차선 데이터 기반 이탈 판정·복귀각 계산　　　　　　 | A72(Linux, 판정) + R5(FreeRTOS, IPC↔CAN)　　　| 서준상 |
| Control | **VCP-G**          | Actuation — 조이스틱 수동주행 + CAN 수신 → 모터·서보·LED·버저 | MCU + FreeRTOS, ADC/GPIO/PDM/I2C　　　　　　　| 서준상 |

### 데이터 흐름

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

**핵심 설계** — 수동 주행 루프는 VCP-G 로컬에서 완결(저지연), D3-G는 인지 결과로 판정·복귀각만 계산해 **CAN으로 제어권(Override)만 지시**. VCP-G가 `0x110 Control Authority`를 보고 USER(로컬 조이스틱) / BOARD(CAN 복귀각)를 중재한다.

---

## 통신 규격

| 구간 | 규격 | 주요 데이터 |
|---|---|---|
| Camera → AI-G | MIPI CSI-2 | RAW 영상 프레임 |
| AI-G → D3-G(A72) | Ethernet TCP | 좌/우 차선 좌표, 실선(1)/점선(2) 라벨, 위험도(0.0~1.0), 타임스탬프 |
| 조이스틱 → VCP-G | ADC(VRx/VRy) · GPIO(SW) | 조향축, 속도축, 버튼 |
| VCP-G → D3-G(R5) | Classical CAN 2.0 | 방향지시 의도(상향, 0x120) |
| D3-G A72 ↔ R5 | IPC (`/dev/tcc_ipc_micom`) | 제어 명령 패킷: 헤더 + 명령 + CRC |
| D3-G(R5) → VCP-G | Classical CAN 2.0 (11-bit ID) | 제어권·차선상태·조향·속도 |
| VCP-G → Actuators | GPIO · PDM(PWM) · I2C | 방향 핀, PWM 듀티, LED, 버저 |

### CAN 메시지 테이블

| 메시지 | CAN ID | 방향 | Data 구조 | 비고 |
|---|---|---|---|---|
| Brake Light | 0x101 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존 |
| Turn Signal | 0x102 | R5→VCP | `[0]` left=0x01, right=0x02 / `[1]` on/off | 기존 |
| Emergency Signal | 0x103 | R5→VCP | `[0]` on=0x01 (경고음 연동) | 기존 |
| Head Light | 0x104 | R5→VCP | `[0]` on=0x01 / off=0x02 | 기존 |
| Vehicle Speed | 0x106 | R5→VCP | `[0]` 0~80 (DC 모터 duty %) | 기존 / BOARD 시 복귀 속도 |
| Wheel Angle | 0x107 | R5→VCP | `[0]` 0~127 (서보 각도) | 기존 / BOARD 시 복귀 조향 |
| **Control Authority** | **0x110** | R5→VCP | `[0]` USER=0x01 / BOARD=0x02 | **신규** — 제어권 주체 |
| **Lane Status** | **0x111** | R5→VCP | `[0]` safe/warn/depart · `[1]` solid/dashed | **신규** — 차선 상태·종류 |
| **Driver Input** | **0x120** | VCP→R5 | `[0]` 방향지시(L/R/off) · (옵션) 속도·조향 | **신규** — 상향 운전자 의도 |

---

## Decision State Machine (D3-G A72)

```
USER_CONTROL ──(점선 접근 · 거리<Tin)──────────────────────────▶ WARNING
USER_CONTROL ──(실선 접근 · 거리<Tin · 접근속도>0 · 방향지시 없음)──▶ BOARD_OVERRIDE
BOARD_OVERRIDE ──(거리>Tout · 홀드시간 Th · heading 평행)─────────▶ USER_CONTROL
(차선 접촉/통과) ──────────────────────────────────────────────▶ CRITICAL
```

| 상태 | 조건 | 동작 | 상태 LED |
|---|---|---|---|
| SAFE | 거리 > Tout | 사용자 제어 유지 | 🟢 초록 (직접주행) |
| WARNING | 거리 < Tin & 점선 쪽 | 경고음만 (점선 허용) | 🟠 주황 |
| OVERRIDE | 거리 < Tin & 실선 쪽 & 방향지시 없음 | 제어권 이양 + 조향 복귀 | 🔴 빨강 |
| CRITICAL | 차선 접촉/통과 | 즉시 감속 + 조향 복귀 | 🔴 빨강 |

- 복귀 조향은 **P 비례 제어**(횡오프셋·heading 오차 기반). 게인과 임계값(`Tin`,`Tout`,`Th`,heading 각)은 Phase 4(튜닝)에서 확정
- **점선 이탈 + 방향지시(0x120)** → 의도적 차선 변경으로 보고 Override 하지 않음

---

## 디렉터리 구조

```
LDAR-System/
├── ai-g/                              # Sensing Zone (정은진)
│   ├── ai-g app/                      #   AI-G 보드 런타임 바이너리 (motrex_app, tcnnapp)
│   ├── ai_model/                      #   변환된 NPU 모델 (yolov8s.bin, yolov8s_extracted.onnx)
│   └── qt/                            #   QT 계기판 자산 (test.obj)
│
├── d3-g/                              # HPC Zone (서준상)
│   ├── a72/                           #   A72 판정 앱 (ldar_listener.c + Makefile) — IPC로 R5에 송신
│   ├── d3-g app/                      #   A72↔R5 IPC 파이썬 라이브러리·예제
│   │   ├── IPC_Example.py
│   │   └── Library/IPC_Library.py
│   ├── D3G-R5/                        #   R5 BSP 풀트리 (edu-motrex 기반, 외부 git)
│   │   ├── sources/                   #     app.drivers / app.sample / core / dev.drivers / os / sal
│   │   ├── scripts/debug/
│   │   └── tools/                     #     cangaroo, fwdn_v8, tcmktool, FWUG.zip
│   ├── r5/                            #   R5 LDAR 모듈 (D3G-R5 BSP의 app.sample에 오버레이될 소스)
│   │   └── sources/app.ldar.bridge/   #     ldar_bridge.{c,h} + rules.mk — IPC↔CAN 브리지
│   └── shared/                        #   A72↔R5 공용 헤더 (ldar_ipc_proto.h)
│
├── vcp-g/                             # Control Zone (서준상)
│   ├── flash/                         #   플래시 패키지 — 이 폴더만 있으면 ./flash.sh 한 줄로 굽기
│   │   ├── fwdn                       #     Linux fwdn 바이너리
│   │   ├── vcp_fwdn.rom               #     FWDN 1단 로더
│   │   ├── tcc70xx_pflash_boot_2M_ECC.rom  #  최신 빌드 산출물 (git 추적)
│   │   └── flash.sh                   #     원-라이너 래퍼 (sudo 자동 요청)
│   └── topst-vcp/                     #   VCP-G BSP (FreeRTOS-VCP) — BSP 자체는 untracked, LDAR 소스·산출물만 git에 -f로 추가
│       ├── sources/app.sample/
│       │   ├── app.ldar.vcp/          #     LDAR 모듈: joystick_adc/sw, motor_dir/pwm, servo_pwm, turn_can/led/signal,
│       │   │                          #              ldar_app, ldar_pins, pwm_util, rules.mk
│       │   ├── app.base/main.c        #     LDAR_Run() 호출 패치 (MCU_BSP_SUPPORT_APP_LDAR_VCP 가드)
│       │   └── rules.mk               #     빌드 플래그(MCU_BSP_BUILD_FLAGS_TEST_APP_*) ON/OFF
│       ├── build/tcc70xx/gcc/         #     빌드 산출물 위치 — output/tcc70xx_pflash_boot_2M_ECC.rom
│       └── tools/fwdn_vcp/            #     원본 fwdn 툴 (Linux/Windows 둘 다) — 실 사용은 vcp-g/flash/ 쪽
│
├── documents/                         # 참고 자료
│   ├── tutorials/                     #   텔레칩스 팹리스 교육과정 D01–D10 + 종합본 PDF, VCP-G Docs.pdf
│   ├── d3g_references/                #   TCC805x MCU BSP API Specification PDF 모음 (GPIO/ADC/CAN/IPC/PMIO/SAL 등)
│   └── Project_Presentation/          #   LDAR 중간발표 PDF
│
├── CLAUDE.md                          # 작업 지침 (README가 정의의 단일 출처)
└── README.md
```

- **공용 프로토콜 헤더 위치**: `d3-g/shared/ldar_ipc_proto.h` (A72↔R5 IPC 패킷 정의)
- **R5 BSP는 두 군데로 분리**: 풀트리는 `d3-g/D3G-R5/`(외부 git), LDAR 전용 추가 모듈은 `d3-g/r5/sources/app.ldar.bridge/` — 빌드 시 BSP 트리에 오버레이
- **VCP-G도 두 군데로 분리**: BSP는 `vcp-g/topst-vcp/`(untracked), LDAR 모듈은 그 안의 `sources/app.sample/app.ldar.vcp/`에 직접 둠. 플래시 산출물·도구는 `vcp-g/flash/`로 모아 git 추적

---

## 개발 환경 (하이브리드 — 보드 진단 결과 반영)

작업물에 따라 빌드 위치가 다르다:

| 작업물 | 작성·빌드 | 플래시·콘솔 |
|---|---|---|
| **A72 판정 앱** | D3-G 보드 (VS Code Remote-SSH, native gcc) | 동일 (보드에서 직접 실행) |
| **R5 펌웨어** | Windows WSL2 (텔레칩스 R5 빌드환경, D02-T01) | Windows WSL2 (보드 USB 직결) |
| **VCP-G 펌웨어** | **외부 Linux 서버(code-server)** — `topst-vcp` BSP, `vcp-g/topst-vcp/`에서 직접 작업 | **사용자 로컬 Windows + WSL2** (`usbipd` USB 전달 + `fwdn` + `minicom`) |

- **VCP-G는 빌드 머신과 플래시 머신이 분리됨** — code-server는 보드 USB가 없으므로 `.rom` 산출물을 git으로 로컬에 전달한 뒤 로컬 WSL2에서 플래시한다.
- **git**: GitHub origin(`Git-Junsang/LDAR-System`)을 각 환경에 clone. 네트워크 드라이브 공유 워킹트리 금지
- **D3-G 보드**: IP `192.168.0.35`(DHCP, 공유기 IP 고정 권장), `/dev/tcc_ipc_micom` 존재(A72↔R5 IPC). `/` 파티션은 `sudo resize2fs /dev/mmcblk0p4`로 16G→28G 확장
- **LDAR VCP-G 코드**는 BSP 트리 안에 직접 둔다 — `vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/`. BSP(~76MB) 자체는 git에 커밋하지 않음 (`vcp-g/.gitkeep`만 추적). LDAR 파일과 `.rom` 산출물은 `git add -f`로 명시 추가

### VCP-G — 사전 준비 (최초 1회)

#### 서버(code-server) 쪽 — 빌드 환경

```bash
# 1) BSP clone (이미 있으면 skip)
cd <repo>/vcp-g
git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp

# 2) Linaro 7.2.1 툴체인 — Makefile이 /opt 경로를 기본값으로 가짐
cd /tmp
wget https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/arm-eabi/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz
sudo tar xf gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz -C /opt/
# 다른 위치 사용 시: MCU_BSP_TOOLCHAIN_PATH=/your/path make

# 3) easy-setup용 whiptail (또는 -e 플래그로 우회)
sudo apt install whiptail

# 4) easy-setup (라이선스 동의 + 빌드 환경 초기화)
cd <repo>/vcp-g/topst-vcp && ./easy-setup_vcp-g.sh -e
```

> 서버에는 `fwdn`/`minicom` 불필요 — 플래시·콘솔은 사용자 로컬에서.

#### 사용자 로컬(Windows + WSL2) 쪽 — 플래시 환경

```bash
# WSL2 안에서
# 1) repo clone + BSP clone (fwdn 바이너리 사용 목적)
git clone https://github.com/Git-Junsang/LDAR-System ~/LDAR-System
cd ~/LDAR-System/vcp-g
git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp

# 2) 보조 도구
sudo apt install minicom
sudo usermod -aG dialout $USER   # /dev/ttyUSB0 권한 (재로그인 필요)
```

Windows 쪽:
- **CP210x USB-Serial 드라이버** 설치 (Silicon Labs)
- **usbipd-win** 설치 (`winget install usbipd`) — Windows USB 장치를 WSL2로 전달
- 보드 USB 연결 후 관리자 PowerShell:
  ```
  usbipd list                          # busid 확인
  usbipd bind --busid <id>             # 최초 1회
  usbipd attach --wsl --busid <id>     # WSL2에 attach (재연결 때마다)
  ```

### VCP-G — 코드 추가 방법 (서버에서)

LDAR 모듈은 `vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/` 안에서 작업한다 (BSP 트리 직접 편집, overlay 단계 없음).

새 주변장치/기능을 추가할 때:

1. **모듈 파일 쌍 생성** — PDF D02-T04~T06 패턴(`xxx.h` + `xxx.c`):
   ```
   vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/
   ├── ldar_pins.h        # 모든 GPIO/ADC/PDM 핀 매크로 한 곳에 집중
   ├── ldar_app.h / .c    # 진입점 LDAR_Run() — 폴링 루프
   ├── joystick_sw.h/c    # 모듈 예: 조이스틱 SW 버튼
   ├── motor_dir.h/c      # 모듈 예: L298N 모터 방향핀
   ├── turn_signal.h/c    # 모듈 예: 방향지시 택트 버튼
   └── rules.mk           # SRCS 등록 + 빌드 플래그
   ```

2. **`app.ldar.vcp/rules.mk`에 소스 등록**:
   ```makefile
   SRCS += new_module.c
   ```

3. **`ldar_app.c`의 `LDAR_Run()`에 init/step 호출 추가**:
   ```c
   NewModule_Init();
   while (1) {
       NewModule_Step();
       (void)SAL_TaskSleep(20);
   }
   ```

4. **새 핀이 필요하면 `ldar_pins.h`에 `#define LDAR_PIN_XXX GPIO_GPB(n)` 추가** — 모든 핀 매핑을 한 파일에 모은다 (배선 변경 시 이 파일만 수정).

5. **새 BSP 드라이버 카테고리(ADC/PDM/CAN 등)가 필요하면** `vcp-g/topst-vcp/sources/app.sample/rules.mk`의 해당 `MCU_BSP_BUILD_FLAGS_TEST_APP_XXX` 빌드 플래그를 ON(`?= 1`)으로 켜거나, `build/tcc70xx/gcc/Makefile` 상단에서 ON.

6. **별도 main.c 수정은 보통 불필요** — `app.sample/app.base/main.c`의 `Main_StartTask`가 이미 `MCU_BSP_SUPPORT_APP_LDAR_VCP` 가드로 `LDAR_Run()`을 호출하도록 패치돼 있음. `app.ldar.vcp/rules.mk`가 이 매크로를 `-D`로 정의함.

### VCP-G — 빌드 (서버에서)

```bash
cd vcp-g/topst-vcp/build/tcc70xx/gcc
make                       # → output/tcc70xx_pflash_boot_2M_ECC.rom
# 깨끗하게 다시: make clean && make
```

### VCP-G — 산출물 git 전달 (서버 → 로컬)

서버:
```bash
cd <repo>
# BSP 트리는 untracked이므로 LDAR 파일과 산출물을 명시 add (.gitignore의 build/, *.bin 회피)
git add -f vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/
git add -f vcp-g/topst-vcp/sources/app.sample/rules.mk
git add -f vcp-g/topst-vcp/sources/app.sample/app.base/main.c
git add -f vcp-g/topst-vcp/build/tcc70xx/gcc/output/tcc70xx_pflash_boot_2M_ECC.rom
git commit -m "vcp-g: <변경 요약>"
git push
```

로컬 WSL2:
```bash
cd ~/LDAR-System && git pull
```

### VCP-G — 플래시 (로컬 WSL2에서)

1. **보드를 FWDN 모드로 진입**
   - 보드 위 **FWDN 스위치를 누른 채** 12V/1A 어댑터 전원 연결
   - USB-C 케이블 연결 → Windows에서 `usbipd attach --wsl --busid <id>`

2. **fwdn 실행** (WSL2에서, sudo 필요):
   ```bash
   cd ~/LDAR-System/vcp-g/topst-vcp
   sudo tools/fwdn_vcp/fwdn \
       --fwdn tools/fwdn_vcp/vcp_fwdn.rom \
       -w    build/tcc70xx/gcc/output/tcc70xx_pflash_boot_2M_ECC.rom
   ```

3. **플래시 완료 후**: 전원 분리 → FWDN 스위치 떼고 → 다시 전원 연결 (Run 모드 부팅).

### VCP-G — 콘솔 확인 (로컬 WSL2에서)

```bash
minicom -D /dev/ttyUSB0 -b 115200 -8
# 종료: Ctrl+A → Q
```

`/dev/ttyUSB0`이 안 보이면 Windows에서 usbipd attach 여부와 `dmesg | tail`로 장치 enumerate 확인.

### CAN 디버깅

A72에 SocketCAN(`can0`)이 **없음** → D3-G에서 `candump` 불가. CAN은 R5/VCP가 전담하므로 **외장 USB-CAN 분석기**를 버스에 연결하거나 R5/VCP 콘솔 로그로 확인.

---

## 개발 현황

### ✅ 완료

- [x] D01-D02 환경 구성 (WSL/Ubuntu, Yocto, D3-G)
- [x] D02-D03 VCP-G 주변장치 (GPIO/ADC/PDM/SPI/I2C)
- [x] D03-D04 CAN / IPC 통신 (A72·R5·VCP-G, IPC APP 통합)
- [x] D05 Control Zone 구성 / 부분 통합
- [x] D06 AI-G 환경 구성 + NPU 모델링 기초
- [x] 제어 측 아키텍처 확정 — 아날로그 조이스틱(VCP-G ADC) + VCP-G 로컬 중재, 3색 상태 LED, 0x110/0x111/0x120 메시지 정의

### 🟧 진행 중

- 🟧 **인지(AI-G)** — UFLD 차선 검출 모델 탑재·추론 검증 (정은진)
- 🟧 **제어(VCP-G/D3-G)** — 아래 Phase 1부터 구현 (서준상)

---

### 인지 (AI-G) — 담당: 정은진

- [x] MobileNet-V2 + U-Net / UFLD 모델 변환 (ONNX → tc-nn-toolkit)
- [ ] 양자화 + NPU 컴파일 (tc-nn-toolkit)
- [ ] AI-G 보드 모델 배포, tc-nn-app 런타임 실행
- [ ] 카메라 입력 → 추론 → 후처리 (좌표 + 실선/점선 라벨 출력)
- [ ] 추론 결과 터미널 로그 검증
- **Exit**: 실시간 차선 인식 좌표가 콘솔에 정상 출력

---

### 제어 (VCP-G + D3-G) — 담당: 서준상

#### Phase 1 — VCP-G 수동 주행 (로컬 완결)
- [ ] T1.1 조이스틱 ADC 읽기 — VRx(조향)/VRy(속도) 2채널 + SW GPIO, 노이즈 필터·데드존·정규화
- [ ] T1.2 입력 매핑 → DC모터(L298N 방향핀+PWM duty 0~80%), 서보각(0~127) 직접 액추에이션
- [ ] T1.3 방향지시 의도(SW/택트 버튼 2개) → 상향 CAN `0x120` 송신
- [ ] T1.4 D3-G R5: `0x120` 수신 → IPC로 A72 전달
- **Exit**: 조이스틱으로 모형차 수동 주행이 VCP-G 단독으로 동작

#### Phase 2 — 이탈 감지 & 판정·복귀각 (D3-G A72, Mock 기반)
- [ ] T2.1 Mock 차선 데이터 주입기 (실제 AI-G TCP와 동일 구조체, 입력 소스 인터페이스 분리)
- [ ] T2.2 이탈 감지 (횡거리, 접근속도, 실선/점선 판별)
- [ ] T2.3 Decision Matrix 상태머신 (SAFE/WARNING/OVERRIDE/CRITICAL)
- [ ] T2.4 P 비례 복귀각 계산 (횡오프셋·heading 오차, CRITICAL 감속)
- [ ] T2.5 A72→R5 IPC 패킷(제어권·차선상태·복귀각/속도) 송신
- **Exit**: Mock으로 이탈→판정→복귀각 산출이 D3-G 단독 동작 (콘솔 검증)

#### Phase 3 — 오버라이드 CAN & 액추에이션 (제어권 전환)
- [ ] T3.1 CAN 메시지 정의 (0x110/0x111/0x120) + BOARD 시 0x106/0x107 복귀 명령
- [ ] T3.2 D3-G R5: 상태머신 출력 → 0x110/0x111 + (BOARD)0x106/0x107 송신
- [ ] T3.3 VCP-G 제어권 중재 (USER=로컬 조이스틱, BOARD=CAN 조향, 전환 램프)
- [ ] T3.4 상태 표시 — 🟢초록/🟠주황/🔴빨강 LED + 부저(점선 경고음/실선·CRITICAL 패턴)
- [ ] T3.5 제어권 전환 End-to-End (채터링 없는 핸드오프)
- **Exit**: Mock→D3-G→CAN→VCP-G 오버라이드 흐름 + LED/부저/복귀 액추에이션 동작

#### Phase 4 — AI-G 통합 & 튜닝
- [ ] T4.1 AI-G→D3-G TCP 수신 구현 (패킷 포맷 확정, Mock → 실제 TCP 교체)
- [ ] T4.2 임계값·게인 튜닝 (Tin, Tout, Th, heading 각, P 게인, 복귀 램프)
- [ ] T4.3 엣지 케이스 (곡선·조명 변화, 채터링·오작동 회귀 테스트)
- [ ] T4.4 응답 지연·추론속도 측정 및 최적화
- **Exit**: 시연 5종(정상/실선이탈/점선이탈 무방향/점선이탈+방향지시/제어권 복귀) 안정 통과

---

### 통합 · 데모 — 공동

- [ ] 테스트 트랙 제작 (실선+점선, 직선+완만 곡선)
- [ ] 카메라→NPU→판정→CAN→액추에이션 전체 흐름 검증
- [ ] (옵션) QT 계기판 App — 제어권 상태·속도 시각화
- [ ] 최종 데모 시연 + 영상 촬영
- [ ] 최종 발표 자료 정리

---

## 필요 자재

> **모든 자재 준비 완료 — 구현만 남음.**

| 분류 | 항목 | 상태 |
|---|---|---|
| Boards | TOPST AI-G / D3-G / VCP-G, MIPI CSI 카메라, Ethernet 허브·케이블, USB-to-TTL UART | ✅ 보유 |
| Vehicle | 모형 섀시, DC 모터, 서보 모터, L298N, LED, 점퍼선·저항 | ✅ 보유 |
| Input | 아날로그 조이스틱(VRx/VRy/SW) — HID 불필요 | ✅ 보유 |
| Input | 방향지시용 택트 버튼 2개 (좌/우 구분) | ✅ 보유 |
| Input | 상태 LED 3색(초록/주황/빨강), 피에조 스피커(버저) | ✅ 보유 |
| Input | 모터 전용 배터리 팩 | ✅ 보유 |
| Env | 실선&점선 차선 트랙 (직선+완만 곡선) | ✅ 준비 완료 |
| Env | 차선 학습 데이터셋 | ✅ 준비 완료 |
| Env | tc-nn-toolkit, tc-nn-app, Yocto / FreeRTOS toolchain | ✅ 보유 |

---

## 참고 자료

- [LDAR 중간발표 PDF](documents/Project_Presentation/LDAR_차선이탈자동복귀시스템_중간발표.pdf) — 모든 정의의 출처
- [documents/](documents/) — 텔레칩스 팹리스 교육과정 D01-D10 PDF (CAN/IPC/VCP/AI-G 구현 참고)
- 재사용 교육 코드: VCP-G CAN Task(`can_vcp_ctrl.c`, D04-T04~05), VCP ADC/GPIO/PDM(D02-T04~06), R5 CAN(D04-T01~02), IPC APP(D04-T06)
