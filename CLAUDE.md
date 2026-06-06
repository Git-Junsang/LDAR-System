# CLAUDE.md — 에이전트 작업 지침

> **정의의 단일 출처는 [README.md](README.md) + [docs/](docs/)**. 이 문서는 작업 방식 가이드일 뿐,
> 스펙이 충돌하면 README/docs/코드가 기준. 상세는 여기 중복하지 말고 링크로 참조.

## 프로젝트 한 줄
조이스틱 수동 주행 중 차선 이탈을 AI가 감지하면 D3-G가 판정해 VCP-G에 CAN으로 복귀 조향을
명령(Override)하는 SDV 축소 구현. 3-Zonal: **AI-G**(인지) → **D3-G**(판정, A72+R5) → **VCP-G**(구동·중재).
전체 그림·데이터 흐름은 [README.md](README.md), 스펙은 [docs/PROTOCOL.md](docs/PROTOCOL.md).

## 어디서 무엇을 빌드/검증하나
- **VCP-G 펌웨어** — 이 환경(code-server)에서 빌드한다. `cd vcp-g/topst-vcp/build/tcc70xx/gcc && make`
  → `output/tcc70xx_pflash_boot_2M_ECC.rom`. 툴체인은 `/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi`(Makefile 기본).
  **플래시·콘솔은 사용자 로컬 WSL2** — 여기선 못 함. 코드 수정 후엔 `make`로 컴파일 검증까지가 내 몫.
- **D3-G A72 판정 앱** — Python. `cd d3-g/a72 && python3 ldar_decision.py --source mock --dry-run`로 로직 검증.
- **R5 펌웨어** — Windows WSL2(텔레칩스 R5 빌드환경). 여기선 빌드 안 함.
- 빌드/플래시/모듈추가 상세는 [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

## 리포 작업 규칙
- **VCP-G LDAR 코드**는 BSP 트리 안 `vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/`에 직접. BSP(~76MB)는
  커밋 안 함 → LDAR 파일·`.rom`은 `git add -f`로 명시 추가(자세한 목록 DEVELOPMENT.md).
- **핀 매핑 단일 출처 = [ldar_pins.h](vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/ldar_pins.h)**. 배선 바뀌면 이 파일만.
- **D3-G 하향 CAN** = [a72/ldar_can.py](d3-g/a72/ldar_can.py), 판정 = [a72/ldar_decision.py](d3-g/a72/ldar_decision.py).
- `d3-g/reference/`는 유사 프로젝트 원본(참고용, 빌드 무관) — import/include 하지 말 것.

## 하드웨어 함정 (반복해서 물린 것들)
- **모든 버튼·조이스틱 SW는 active-low** — 핀→버튼→GND, 내부 **풀업**, 눌림=0. 풀다운/active-high로 두면 영원히 0.
- **조이스틱 VCC는 3.3V** — 5V에 물리면 중립이 ~2.5V(raw 3100)로 뜨고 한쪽이 ADC 3.3V에서 클리핑됨.
- **DC 모터는 오픈루프 PWM** — 듀티가 곧 평균전압이라 속도·토크 동반. 속도만 줄이려면 듀티 상한↓(현 90%), 토크 유지는 불가.
- **A72에 SocketCAN 없음** → `candump` 불가. CAN은 외장 USB-CAN 분석기 또는 R5/VCP 콘솔 로그로 본다.
- 콘솔 로그가 갑자기 다 죽고 DC 모터만 살면 보통 **3.3V 로직 레일 브라운아웃**(배선/단락) 의심.

## 현재 상태 (요약 — 상세는 docs/ROADMAP.md)
- ✅ VCP-G 수동주행(조이스틱·모터·서보·방향지시 토글·부저), D3-G 판정앱(Mock 검증)
- 🟧 인지(AI-G) 추론, 제어 Phase 3 통합(R5 하향 IPC→CAN + VCP-G CAN 수신; 현재 `override.c`는 적색 LED·부저 스텁)
