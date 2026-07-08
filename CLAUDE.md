# CLAUDE.md — 에이전트 작업 지침

> **정의의 단일 출처는 [README.md](README.md) + 각 보드 폴더 README**
> ([ai-g](ai-g/README.md) · [d3-g](d3-g/README.md) · [vcp-g](vcp-g/README.md)).
> 이 문서는 작업 방식 가이드일 뿐, 스펙이 충돌하면 README/코드가 기준. 상세는 여기 중복하지 말고 링크로.

## 프로젝트 한 줄
조이스틱 수동 주행 중 카메라가 교통표지판(속도제한 30/60·정지·진입금지)을 인식하면 D3-G가
판정해 VCP-G에 CAN으로 속도제한/정지를 명령(Override)하는 SDV 축소 구현. **조향은 항상 운전자,
개입은 속도뿐.** 3-Zonal: **AI-G**(인지) → **D3-G**(판정, A72+R5) → **VCP-G**(구동·중재).
(코드네임 LDAR는 구 "Lane Departure Auto-Recovery"에서 유지된 이름일 뿐 — **차선 인식은 폐기**,
현재 주제는 표지판 속도 오버라이드로 고정.)

## 어디서 무엇을 빌드/검증하나
- **VCP-G 펌웨어** — 이 환경(code-server)에서 빌드. **BSP는 리포에 없다** → `vcp-g/`에서
  `git clone .../FreeRTOS-VCP topst-vcp` 후 `app.ldar.vcp/`를 오버레이해 빌드
  (`build/tcc70xx/gcc && make MCU_BSP_BUILD_FLAGS_TEST_APP_ADC=1 MCU_BSP_BUILD_FLAGS_TEST_APP_CAN=1`
  → `output/tcc70xx_pflash_boot_2M_ECC.rom`). 툴체인 `/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi`.
  절차·통합 지점은 [vcp-g/README.md](vcp-g/README.md) + [app.ldar.vcp/OVERLAY.md](vcp-g/app.ldar.vcp/OVERLAY.md).
  **플래시·콘솔은 사용자 로컬 WSL2** — 여기선 못 함. 코드 수정 후 `make` 컴파일 검증까지가 내 몫.
- **D3-G A72 판정 앱** — Python. `cd d3-g/a72 && python3 ldar_decision.py --source mock --dry-run`로 로직 검증.
- **R5 펌웨어** — Windows WSL2(텔레칩스 R5 빌드환경). 여기선 빌드 안 함. 오버레이 = `d3-g/r5/`.
- **AI-G 인지** — **YOLOv8 표지판 검출**(클래스 stop/no_entry/speed_60/speed_30). 파이프라인
  [ai-g/data_pipeline/](ai-g/data_pipeline/)(`vcap`/`uyvy2img.py` 촬영), 학습→NPU [ai-g/ai_model/](ai-g/ai_model/).
  이 환경에선 `python3 -m py_compile`로 문법 검증까지. 상세 [ai-g/README.md](ai-g/README.md).

## 리포 작업 규칙
- **BSP는 커밋 안 함** — VCP-G `topst-vcp/`, R5 `D3G-R5/`는 `.gitignore`. 우리 오버레이 소스만 추적.
- **VCP-G LDAR 코드** = [vcp-g/app.ldar.vcp/](vcp-g/app.ldar.vcp/) (빌드 시 BSP `sources/app.sample/`로 오버레이).
- **핀 매핑 단일 출처 = [ldar_pins.h](vcp-g/app.ldar.vcp/ldar_pins.h)**. 배선 바뀌면 이 파일만.
- **D3-G 하향 CAN** = [a72/ldar_can.py](d3-g/a72/ldar_can.py), 판정 = [a72/ldar_decision.py](d3-g/a72/ldar_decision.py).
- **AI-G 데이터(`ai-g/data_pipeline/data/`, `ai-g/ai_model/dataset/` 영상·프레임·라벨)는 git 추적 제외**(용량) — 코드만 커밋.

## 하드웨어 함정 (반복해서 물린 것들)
- **모든 버튼·조이스틱 SW는 active-low** — 핀→버튼→GND, 내부 **풀업**, 눌림=0. 풀다운/active-high로 두면 영원히 0.
- **조이스틱 VCC는 3.3V** — 5V에 물리면 중립이 ~2.5V(raw 3100)로 뜨고 한쪽이 ADC 3.3V에서 클리핑됨.
- **DC 모터는 오픈루프 PWM** — 듀티가 곧 평균전압이라 속도·토크 동반. 속도만 줄이려면 듀티 상한↓(현 90%), 토크 유지는 불가.
- **A72에 SocketCAN 없음** → `candump` 불가. CAN은 외장 USB-CAN 분석기 또는 R5/VCP 콘솔 로그로 본다.
- 콘솔 로그가 갑자기 다 죽고 DC 모터만 살면 보통 **3.3V 로직 레일 브라운아웃**(배선/단락) 의심.
- **AI-G 학습 황금률** — 고정 데모 환경 오버핏 전략이라 **학습 데이터 = 데모 셋업이 100% 동일**해야 함(같은 PiCam·마운트·해상도·표지판세트·조명). 마운트 먼저 고정하고 그 카메라로만 촬영. 폰 촬영·중간 재장착 금지.
- **AI-G 카메라 = MIPI CSI-2 15핀(OV5647 PiCam)** — 추론(tcnnapp)이 V4L2 `/dev/video2`로 MIPI를 잡음. **USB 웹캠은 AI-G 스펙에 미지원** → 추론·촬영은 MIPI PiCam으로, USB 웹캠은 PC 리허설용만. picamera2는 텔레칩스라 안 됨(V4L2 경로). 스펙: N-Dolphin A53 Quad·NPU 8TOPS·RAM **2GB**(입력 해상도 과하게 X).

## 현재 상태 (요약 — 상세는 README.md "개발 현황")
- ✅ VCP-G 수동주행(조이스틱·모터·서보·방향지시 토글·부저)
- ✅ VCP-G 속도 오버라이드 — CAN 0x110 수신 → 부드러운 감속/정지(`override.c`+`override_can.c`, 컴파일·ROM 검증), D3-G 판정앱(표지판→명령, Mock 검증)
- 🟧 인지(AI-G) — YOLOv8 표지판 검출 학습·보드 탑재 진행
- 🟧 통합 — R5 하향(IPC→CAN 0x110 송신) + AI-G→D3-G TCP 연동 남음
