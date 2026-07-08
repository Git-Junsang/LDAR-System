
# LDAR-System

[English](README.md) | **한국어**

---

**교통표지판 인식 기반 속도 오버라이드 SDV — Telechips 3-Zonal 축소 구현**

---

## 1. 배경

상용차는 전방 카메라로 교통표지판을 읽고 지능형 속도보조(ISA)가 규정 속도를 넘지 않게 지킨다. LDAR-System은 그 동작을 Telechips Zonal 모형차에서 재현한다. 운전자가 조이스틱으로 수동 조향하는 동안, 카메라가 교통표지판(속도제한 30/60·정지·진입금지)을 인식하면 시스템이 **속도를 부드럽게 강제로 낮추거나 정지**시킨다.

- 전 과정이 엣지에서 돈다: **카메라 → NPU 추론 → 판정 → CAN 제어 → 액추에이션**, End-to-End.
- **조향은 항상 운전자 — 개입은 속도뿐.** 수동 주행 루프는 로컬에서 저지연으로 완결되고, 오버라이드는 속도 상한만 낮춘다.
- 3-Zonal 구조: **AI-G**(인지) → **D3-G**(판정, A72 + R5) → **VCP-G**(구동·중재).

> 코드네임 **`LDAR`** 은 초기 주제 "Lane Departure Auto-Recovery"(차선이탈 자동복귀)에서 유지된 이름일 뿐이다. **차선 인식은 폐기**되었고, 현재 주제는 **교통표지판 속도 오버라이드**로 고정되어 있다.

---

## 2. 시스템 구성

```
조이스틱(VRx/VRy/SW) ─ADC/GPIO─▶ VCP-G ─────▶ DC모터 · 서보   (로컬, 저지연)
                                    └─CAN 0x120(방향지시, 상향)─▶ D3-G R5 ─IPC─▶ A72
Camera ─MIPI CSI-2─▶ AI-G ─Ethernet TCP─▶ D3-G A72   (표지판 검출 → 속도 판정)
                                            │ IPC
                                            ▼
                       D3-G R5 ─CAN 0x110─▶ Speed Override (mode + 한계속도)
                                            ▼
                       VCP-G ──▶ 한계속도까지 부드럽게 감속/정지 (조향은 조이스틱 유지)
```

| Zone    | Board                    | 역할                                                                             | 하드웨어                                  |
| ------- | ------------------------ | -------------------------------------------------------------------------------- | ----------------------------------------- |
| Sensing | **AI-G**           | PiCam → NPU 추론(YOLOv8) →**교통표지판 검출**(클래스+신뢰도) → TCP 송신 | A53 Quad + Enlight NPU 8TOPS, MIPI CSI-2  |
| HPC     | **D3-G** (TCC8050) | 표지판 →**속도제한/정지 판정** → CAN 명령                                | A72(Linux, 판정) + R5(FreeRTOS, IPC↔CAN) |
| Control | **VCP-G**          | 조이스틱 수동주행 +**속도 오버라이드 중재** + 모터·서보·LED·부저        | MCU + FreeRTOS, ADC/GPIO/PDM/I2C          |

**핵심 설계** — 수동 주행 루프는 VCP-G 로컬에서 완결(저지연). D3-G는 표지판→속도만 판정해 **CAN 0x110(Speed Override)으로 속도 상한/정지만 지시**한다. VCP-G는 그 명령을 받아 적용 상한을 틱마다 슬루-레이트로 이동시켜 **급변 없이 부드럽게** 감속/정지한다. 상한 아래에서는 조이스틱이 그대로 통과하고, **조향은 어떤 경우에도 운전자**가 쥔다.

---

## 3. 아키텍처

### 3.1 인지 모델 — AI-G (YOLOv8)

**YOLOv8s 표지판 검출기**, 4클래스. 데모는 **고정 환경**(같은 조명·PiCam·트랙·표지판 세트)이라 그 환경에 의도적으로 오버핏시키는 전략이다.

**클래스(순서 고정, `ai_model/dataset/data.yaml`이 단일 출처):**

```
0 Stop   /   1 No Entry   /   2 Speed_Limit_60   /   3 Speed_Limit_30
```

> 이 정수 인덱스가 그대로 NPU → D3-G 전선 위의 `cls` 값이 된다. **절대 재정렬 금지.**

| 항목            | 사양                                                                                             |
| --------------- | ------------------------------------------------------------------------------------------------ |
| 보드(N-Dolphin) | A53 Quad +**Enlight NPU 8TOPS**, RAM **2GB**(입력 해상도 과하게 X), Yocto Linux      |
| 카메라          | **OV5647**(RasPi Cam v1.3, MIPI CSI-2 15핀) → V4L2 `/dev/video2`, UYVY 1288×956        |
| 모델            | YOLOv8s fine-tune, 입력**640×640(letterbox)**, NPU용 INT8 양자화                          |
| 툴체인          | Ultralytics(학습) → ONNX 6-출력 추출 → tc-nn-toolkit(Enlight 변환/양자화/컴파일) →`tcnnapp` |
| 출력            | TCP 서버`192.168.0.100:9999`, 프레임당 JSON 한 줄                                              |

**TCP 송신 포맷 (AI-G → D3-G)** — 프레임당 JSON 한 줄:

```json
{"boxes":[{"cls":3,"score":0.92,"xmin":..,"ymin":..,"xmax":..,"ymax":..}]}
```

D3-G `ldar_decision.py`는 `{"ts":.., "sign":"speed_30", "conf":0.92}`(문자열 `sign`)를 기대하므로, **shim이 `cls(int) → sign(str)`**(`data.yaml` 순서), `score → conf`, top-box 선택을 처리한다. 상세 [d3-g/README.md](d3-g/README.md).

> ⚠️ **학습 황금률** — 학습 데이터 = **데모 셋업 100% 동일**(같은 PiCam·마운트·해상도·표지판세트·조명). 마운트 먼저 고정하고 그 카메라로만 촬영. 폰 촬영·중간 재장착 금지. AI-G 스펙상 USB 웹캠은 미지원(MIPI PiCam 전용) — 웹캠은 PC 리허설용만.

### 3.2 제어 · 통신 아키텍처

**동작 시나리오 (표지판 → 속도)**

| 표지판          | D3-G 판정 | CAN 0x110`[mode, km/h]` | VCP-G 동작                                               |
| --------------- | --------- | ------------------------- | -------------------------------------------------------- |
| 속도제한 30     | LIMIT 30  | `[0x01, 30]`            | 듀티 상한 30%까지 부드럽게 감속, 그 아래선 조이스틱 자유 |
| 속도제한 60     | LIMIT 60  | `[0x01, 60]`            | 듀티 상한 60%까지 부드럽게 감속                          |
| 정지 / 진입금지 | STOP      | `[0x02, 0]`             | 0%까지 부드럽게 감속 후 브레이크 정지                    |
| (제한구역 종료) | RELEASE   | `[0x00, 0]`             | 상한 해제 — 조이스틱 풀스로틀 복귀                      |
| (표지판 없음)   | —        | (송신 안 함)              | 직전 명령 유지                                           |

> 한계속도(km/h)는 모형차 듀티%에 1:1 대응(30 → 30%, 60 → 60%). 로컬 조이스틱 풀스로틀 듀티 상한은 펌웨어에서 90%(`MOTOR_DUTY_CAP_PCT`).

**오버라이드 동작** — `duty = min(조이스틱 요청, 오버라이드 상한)`. 적용 상한은 목표값까지 틱마다 **슬루-레이트**로 이동(감속 ≈ 100%/s, 90% → 0% 약 0.9s). STOP은 0% 도달 후 브레이크. 적색 LED(LIMIT=1Hz 깜박 / STOP=점등)와 부저로 오버라이드 상태를 표시한다.

**CAN 메시지 테이블** — 11-bit CAN ID, 채널 0. 하향(R5 → VCP)은 판정 결과, 상향(VCP → R5)은 운전자 의도.

| 메시지                    | CAN ID                | 방향      | Data                                   | 비고                              |
| ------------------------- | --------------------- | --------- | -------------------------------------- | --------------------------------- |
| **Speed Override**  | **0x110**       | R5 → VCP | `[0]` mode · `[1]` 한계속도(km/h) | **표지판 속도 명령 (핵심)** |
| **Driver Input**    | **0x120**       | VCP → R5 | `[0]` 방향지시(0 off / 1 L / 2 R)    | 상향 운전자 의도                  |
| Brake / Turn / Head Light | 0x101 / 0x102 / 0x104 | R5 → VCP | 교육용 기존 메시지                     | 미사용                            |

- **0x110 mode**: `0x00` RELEASE(상한 해제) / `0x01` LIMIT(상한 제한) / `0x02` STOP(정지). `[1]` 한계속도는 LIMIT일 때만 유효 — VCP-G가 듀티% 상한으로 1:1 적용.

**통신 경로 요약**

| 구간               | 인터페이스                   | 주요 데이터                                          |
| ------------------ | ---------------------------- | ---------------------------------------------------- |
| Camera → AI-G     | MIPI CSI-2                   | RAW 영상 (OV5647 → UYVY`/dev/video2`)             |
| AI-G → D3-G(A72)  | Ethernet TCP                 | 표지판 클래스, 신뢰도, 바운딩박스                    |
| 조이스틱 → VCP-G  | ADC(VRx/VRy) · GPIO(SW)     | 조향 · 속도 · 버튼                                 |
| VCP-G → D3-G(R5)  | Classical CAN 2.0 (11-bit)   | 방향지시 의도 (상향,**0x120**)                 |
| D3-G A72 ↔ R5     | IPC (`/dev/tcc_ipc_micom`) | 교육용 IPC 패킷(SYNC·CMD·LEN·DATA·CRC16)         |
| D3-G(R5) → VCP-G  | Classical CAN 2.0 (11-bit)   | **속도 오버라이드 (0x110)** — mode + 한계속도 |
| VCP-G → Actuators | GPIO · PDM(PWM) · I2C      | 방향핀 · PWM · LED · 부저                         |

**모듈 맵**

| 그룹         | 모듈                                                                                                                                                                                                                                         |
| ------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| VCP-G 펌웨어 | `ldar_app`(메인 루프), `joystick_adc` / `joystick_sw`, `motor_dir` / `motor_pwm`, `servo_pwm`, `turn_signal` / `turn_led` / `turn_can`, `override` / `override_can`, `buzzer`, `pwm_util`; 핀 정의 `ldar_pins.h` |
| D3-G A72     | `ldar_decision.py`(판정 앱), `ldar_can.py`(하향 0x110 IPC), `Library/IPC_Library.py`(CRC16 IPC transport)                                                                                                                              |
| D3-G R5      | `ldar_bridge.c`(상향 0x120 CAN→IPC), `ldar_downstream.c`(IPC→CAN 0x110), `shared/ldar_ipc_proto.h`                                                                                                                                   |

> 상세 규격은 각 보드 README: [핀맵 · CAN 수신](vcp-g/README.md) · [판정 · IPC](d3-g/README.md) · [TCP 포맷](ai-g/README.md).

---

## 4. 디렉토리 구조

```
LDAR-System/
├── ai-g/                     # Sensing Zone: 표지판 검출 · NPU · TCP 송신
│   ├── README.md             #   ★ AI-G 셋업 (보드 · 카메라 · 학습 · 배포)
│   ├── data_pipeline/        #   학습 데이터 촬영 / 프레임추출 (vcap / uyvy2img / capture)
│   ├── ai_model/             #   YOLOv8 학습 → ONNX → NPU 변환 (train.py / export_onnx.py / data.yaml)
│   └── ai-g app/             #   보드 NPU 추론 런타임 (tcnnapp / motrex_app)
│
├── d3-g/                     # HPC Zone: 판정 · 명령
│   ├── README.md             #   ★ D3-G 셋업 (A72 앱 · R5 오버레이 · IPC/CAN)
│   ├── a72/                  #   판정 앱 (Python) — ldar_decision.py, ldar_can.py
│   ├── r5/                   #   R5 LDAR 오버레이 모듈 (CAN↔IPC 브리지, 하향)
│   └── shared/               #   A72↔R5 공용 IPC 헤더 (ldar_ipc_proto.h)
│
├── vcp-g/                    # Control Zone: 구동 · 중재
│   ├── README.md             #   ★ VCP-G 셋업 (BSP 오버레이 빌드 · 플래시 · 핀맵)
│   ├── app.ldar.vcp/         #   VCP-G LDAR 펌웨어 (BSP에 오버레이하는 우리 소스)
│   └── flash/                #   플래시 패키지 (fwdn + .rom + flash.sh)
│
├── documents/                # 발표 · 보고서 · 튜토리얼 PDF · BSP-API 스펙 (참고 자료)
└── README.md                 # (이 문서) 프로젝트 정의의 단일 출처
```

> **BSP는 커밋하지 않는다.** VCP-G / R5 펌웨어는 Telechips BSP 위에 우리 소스를 오버레이해 빌드한다(각 폴더 "빌드" 참조). 리포에는 우리가 작성한 파일만 둔다. 데이터(`ai-g/data_pipeline/data/`, `ai-g/ai_model/dataset/`의 영상·프레임·라벨)는 용량 때문에 git 추적 제외 — 코드만.

---

## 5. 시작하기

### 요구 환경

| 환경          | 용도                                                                       |
| ------------- | -------------------------------------------------------------------------- |
| code-server   | VCP-G 펌웨어 빌드(BSP 오버레이 →`.rom`), D3-G A72 Python 판정 로직 검증 |
| GPU PC        | YOLOv8 학습(ultralytics), ONNX export                                      |
| WSL2 (Ubuntu) | tc-nn-toolkit(NPU 변환/양자화/컴파일), R5 BSP 빌드, VCP-G 플래시 · 콘솔   |
| 보드          | AI-G(Ethernet 192.168.0.100), D3-G(A72 Linux + R5), VCP-G(MCU)             |

### 빠른 시작 — 하드웨어 없이 판정 로직

```bash
cd d3-g/a72
python3 ldar_decision.py --source mock --dry-run   # 표지판 시나리오 순환, 콘솔만
```

### AI-G — 데이터 파이프라인 → 학습 → NPU 배포

```bash
# 1) 보드에서 촬영 (정적 바이너리, 의존성 0) — 표지판 클래스별 UYVY raw
./vcap /home/root/data/speed_30 speed_30 60 5        # <outdir> <prefix> <count> <skip>
# 2) PC에서 변환: UYVY raw → jpg (640 리사이즈)
python3 uyvy2img.py data/raw_uyvy --out data/frames --src-size 1288x956 --scale 640x640 --dedup 4.0
# 3) 라벨링(bbox, 4클래스) → GPU PC에서 학습
cd ai-g/ai_model
python3 train.py --epochs 120 --imgsz 640 --batch 16 --device 0
# 4) ONNX export (6-출력 추출; [verify]가 cv3.*=4ch 확인, 80ch면 중단)
python3 export_onnx.py --weights runs/detect/signs_yolov8s/weights/best.pt
# 5) tc-nn-toolkit(WSL): converter → quantizer → compiler → net.so   (ai-g/ai_model/README.md)
# 6) 모델 폴더 전체를 보드로 배포한 뒤:
tcnnapp -n yolov8s_signs_quantized -p /dev/video2
```

전 과정: [ai-g/README.md](ai-g/README.md) · [ai-g/data_pipeline/README.md](ai-g/data_pipeline/README.md) · [ai-g/ai_model/README.md](ai-g/ai_model/README.md).

### D3-G — 판정 앱 · 하향 브리지

```bash
cd d3-g/a72
python3 ldar_decision.py --source mock --dry-run     # 매핑 검증, 콘솔만
python3 ldar_decision.py --source mock               # 실제 IPC 송신 (sudo 필요)
python3 ldar_decision.py --source tcp --port 9999    # 실제 AI-G TCP 수신
```

`/dev/tcc_ipc_micom` 접근은 root 필요. R5 하향 브리지(IPC → CAN 0x110)는 WSL2 Telechips R5 BSP에서 빌드한다 — `r5/sources/app.ldar.bridge/`를 오버레이, `LdarDownstream_Init()` 호출 추가, IPC 수신 플레이스홀더를 실제 BSP API로 교체. 상세 [d3-g/README.md](d3-g/README.md).

### VCP-G — BSP 오버레이 빌드 · 플래시 · 콘솔

```bash
# --- code-server에서 빌드 ---
cd vcp-g
git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp    # BSP (리포에 없음)
cd topst-vcp && ./easy-setup_vcp-g.sh -e
cp -r ../app.ldar.vcp sources/app.sample/app.ldar.vcp                    # + 통합 편집 3곳 (OVERLAY.md)
cd build/tcc70xx/gcc
make MCU_BSP_BUILD_FLAGS_TEST_APP_ADC=1 MCU_BSP_BUILD_FLAGS_TEST_APP_CAN=1
#   → output/tcc70xx_pflash_boot_2M_ECC.rom
cp output/tcc70xx_pflash_boot_2M_ECC.rom ../../../../flash/

# --- 로컬 Windows + WSL2에서 플래시 · 콘솔 ---
cd vcp-g/flash && ./flash.sh          # fwdn --fwdn vcp_fwdn.rom -w tcc70xx_pflash_boot_2M_ECC.rom
minicom -D /dev/ttyUSB0 -b 115200 -8  # 콘솔 (종료: Ctrl+A → Q)
```

툴체인 `/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi`. 핀맵 단일 출처는 [vcp-g/app.ldar.vcp/ldar_pins.h](vcp-g/app.ldar.vcp/ldar_pins.h) — 배선이 바뀌면 이 파일만 고친다. 전체 절차·오버레이 지점: [vcp-g/README.md](vcp-g/README.md) + [vcp-g/app.ldar.vcp/OVERLAY.md](vcp-g/app.ldar.vcp/OVERLAY.md).

---

## 6. 개발 현황

**현재 상태 — VCP-G 수동주행·속도 오버라이드 검증 완료, 인지·통합 진행 중.** VCP-G 수동주행(조이스틱·모터·서보·방향지시·부저)이 실제 펌웨어로 동작하고, 속도 오버라이드(CAN 0x110 → 부드러운 감속/정지)는 컴파일·ROM·플래시 검증됐다. D3-G 판정앱은 표지판→속도 매핑에 신뢰도 임계·CONFIRM 디바운스를 더해 Mock 검증됐고, 하향 CAN 경로(`ldar_can.py` + R5 `ldar_downstream.c`)는 호스트 상호운용 검증됐다. 남은 것은 AI-G 인지(NPU 위 YOLOv8), 실기의 R5 IPC → CAN 0x110 브리지, AI-G → D3-G TCP 연동.

### 완료

- [X] **환경/기초** — Yocto/D3-G, VCP-G 주변장치, CAN/IPC, Control Zone, AI-G + NPU 기초
- [X] **VCP-G 수동주행** — 조이스틱 ADC · DC모터(L298N) · 서보 · 방향지시 토글 · 부저 (펌웨어 동작)
- [X] **D3-G 판정앱** — 표지판→속도 매핑(`SIGN_TO_CMD`) + 신뢰도 임계 + CONFIRM 디바운스 (Mock 검증)
- [X] **VCP-G 속도 오버라이드** — CAN 0x110 수신 → 적용 상한 슬루(부드러운 감속/정지) (컴파일 · ROM · 플래시 검증)
- [X] **D3-G 하향 CAN** — `ldar_can.py`(limit/stop/release) + R5 `ldar_downstream.c` (호스트 상호운용 검증)

### 진행 중

- [ ] **인지(AI-G)** — YOLOv8 표지판 검출 학습(4클래스) · ONNX → NPU 변환 · 보드 탑재
- [ ] **R5 하향 브리지** — A72 IPC → CAN 0x110 송신 (WSL2 R5 빌드 + 실제 BSP IPC 수신 API 교체)
- [ ] **AI-G → D3-G TCP 연동** — 표지판 패킷 shim(`cls → sign`) + Mock → 실제 교체

### 통합 · 데모

- [ ] 카메라 → NPU → 판정 → CAN → 액추에이션 전체 흐름 (E2E)
- [ ] 슬루레이트 · 신뢰도 임계 튜닝, 엣지 케이스(오검출, 정지 후 재출발)
- [ ] 시연 5종(정상주행 / 30제한 / 60제한 / 정지 / 해제) 안정 통과
- [ ] (옵션) Qt 계기판 App — 속도 상한 · 오버라이드 상태 시각화
- [ ] 최종 데모 영상 + 발표 자료

---

## 7. 기타

### 산출물

| 경로                                                      | 내용                                                                              |
| --------------------------------------------------------- | --------------------------------------------------------------------------------- |
| `ai-g/ai_model/yolov8s.bin`, `yolov8s_extracted.onnx` | **스톡(80-class COCO) 참고용** — 4클래스 커스텀 컴파일에 그대로 쓰지 말 것 |
| `vcp-g/flash/tcc70xx_pflash_boot_2M_ECC.rom`            | 빌드된 VCP-G 펌웨어 이미지(플래시 패키지)                                         |

### 문서

| 경로                          | 내용                                                                                                 |
| ----------------------------- | ---------------------------------------------------------------------------------------------------- |
| `documents/tutorials/`      | 텔레칩스 팹리스교육 과정(D01~D10) — Yocto/D3-G, VCP GPIO/ADC/PDM, CAN, AI모델(YOLO), SensingZone 등 |
| `documents/d3g_references/` | TCC805x MCU BSP-API 스펙 PDF(ADC, CAN, GPIO, IPC, PDM, ...) + Getting Started / User Guide           |
| `documents/`                | 최종보고서(docx), 중간발표(pdf)                                                                      |

### 하드웨어 함정 (반복해서 물린 것들)

- **모든 버튼 · 조이스틱 SW는 active-low** — 핀 → 버튼 → GND, 내부 풀업, 눌림 = 0. 풀다운/active-high면 영원히 0.
- **조이스틱 VCC는 3.3V** — 5V에 물리면 중립이 raw ~3100으로 뜨고 한쪽 축이 ADC 3.3V에서 클리핑.
- **DC 모터는 오픈루프 PWM** — 듀티가 곧 평균전압이라 속도 · 토크 동반. 속도 상한이 곧 오버라이드 수단.
- **A72에 SocketCAN 없음** → `candump` 불가. CAN은 외장 USB-CAN 분석기 또는 R5/VCP 콘솔 로그로 확인.
- 콘솔이 다 죽고 DC 모터만 살면 보통 **3.3V 로직 레일 브라운아웃**(배선/단락) 의심.
- **AI-G 학습 황금률** — 고정 데모 환경 오버핏 전략 → 학습 데이터 = 데모 셋업 100% 동일(같은 PiCam·마운트·해상도·표지판세트·조명).

### 데이터 주의

`ai-g/data_pipeline/data/`, `ai-g/ai_model/dataset/`(영상·프레임·라벨)는 용량 때문에 git 추적 제외 — 코드만. 현재 `dataset/`은 공개셋(GTSRB/Roboflow, 유럽 표지판) 기반이므로, 안정적 인식을 위해 데모 셋업으로 재촬영·재학습 권장.

---

## 8. 참고 자료

- [Telechips TOPST](https://topst.ai/) — TCC8050(D3-G) / TCC70xx(VCP-G) Zonal 플랫폼 · 팹리스교육 과정
- [FreeRTOS-VCP BSP](https://github.com/topst-development/FreeRTOS-VCP) — VCP-G 펌웨어 베이스(오버레이, 커밋 제외)
- [Ultralytics YOLOv8](https://github.com/ultralytics/ultralytics) — 표지판 검출기 베이스 아키텍처
- [GTSRB](https://benchmark.ini.rub.de/gtsrb_news.html) — German Traffic Sign Recognition Benchmark(공개셋 참고)
