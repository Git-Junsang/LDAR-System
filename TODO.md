# 지금 해야 할 일 — LDAR (표지판 인식 속도 오버라이드)

> 작성 2026-06-12. 상세 스펙은 [docs/PROTOCOL.md](docs/PROTOCOL.md), 단계는 [docs/ROADMAP.md](docs/ROADMAP.md).
> 이 문서는 **현재 시점에서 남은 실행 항목**만 모은 작업 체크리스트.

## 한 줄 현황
**VCP-G 플래시 완료 ✅.** 다음 목표 = **D3-G 하향 경로 연결 → Mock으로 끝까지(E2E) 시연** → 그 후 AI-G 통합.

## 파이프라인 — 어디까지 됐나
```
① AI-G    카메라 → YOLOv8 표지판 → 라벨            ⬜ 미구현 (모델 학습·탑재)
② AI-G→D3-G  TCP로 라벨 전송                       ⬜ 미구현 (받는 쪽 TcpSource는 준비됨)
③ D3-G A72   라벨 → 30/60/정지 명령 매핑            ✅ 코드+Mock검증
④ A72→R5     IPC 송신(ldar_can.py)                ✅ 코드
⑤ R5→CAN     IPC 파싱 → CAN 0x110 송신             🟧 코드 작성+호스트검증, **보드 빌드/배선 남음**
⑥ VCP-G      CAN 0x110 수신 → 부드러운 감속/정지     ✅ 코드+ROM+**플래시 완료**
```
→ **막힌 곳은 ⑤뿐.** ⑤만 연결하면 AI-G 없이 `③Mock → ④ → ⑤ → ⑥` 전체가 돈다.

---

# A. 즉시 (제어 — 서준상): 하향 경로 + Mock E2E
**이걸 끝내면 AI-G 없이도 "표지판 명령 → 부드러운 감속/정지" 시연 가능.**

## A-1. CAN 배선 (D3-G ↔ VCP-G) — 2노드 버스
- [ ] 각 보드 CAN 커넥터에 **트랜시버** 연결 (VCP-G: J5D100 = TX0/GPK08, RX0/GPK01, 3.3V, GND)
- [ ] 두 트랜시버를 **CAN_H ─ CAN_H**, **CAN_L ─ CAN_L** 로 연결, **GND 공통**
- [ ] **종단저항 120Ω 양 끝** (트랜시버 모듈 내장이면 둘 다 켜서 병렬 60Ω)
- ⚠️ AI-G는 CAN 아님 — **Ethernet(LAN)** 으로 D3-G에 붙음. CAN은 D3-G↔VCP-G 둘뿐.

## A-2. R5 하향 브리지 올리기 (방법 A 먼저, 안 되면 B)
A72 IPC 패킷이 **교육용 CAN 데모 명령(CMD1=0x05)** 형식이라 두 길이 있음. **택일**(둘 다 켜면 CAN 중복 송신).

### 방법 A (권장·간단): R5 교육용 IPC-CAN 데모 활성화
**배경** — A72의 `ldar_can.py`가 보내는 IPC 패킷은 텔레칩스 **교육용 CAN 데모 명령(CMD1=0x05)** 형식이다.
R5 micom 교육 펌웨어에는 이 명령을 받아 `(canID, data)`를 그대로 CAN으로 쏘는 핸들러가 들어 있다
(D02 IPC·CAN 강의의 micom 샘플 — `reference/ipc-example/IPC_Example.py`가 바로 그걸 쓰는 A72측 도구).
즉 **R5에 그 교육 펌웨어가 돌면 커스텀 R5 코드 0**으로 하향이 된다. (유사 프로젝트 `demo_rc_ver.py`도 이 경로.)

**A-2A-① 이미 되는지 먼저 확인** (A-1 배선 후 — 새로 빌드하기 전에!)
- [ ] D3-G A72에서 **0x110 한 프레임** 쏘기 (우리 라이브러리 사용, root 필요):
  ```bash
  cd d3-g/a72
  sudo python3 -c "import ldar_can as C; c=C.LdarCan(); c.limit(60); c.close()"   # 0x110 [01 3C] = LIMIT 60
  ```
- [ ] **VCP-G 콘솔에 `[OVR] RX 0x110 LIMIT 60%`** + 모터 듀티 감소가 뜨면 → **R5 교육 핸들러 이미 활성 = 방법 A 끝, 추가 작업 0.** 바로 A-3로.
  - 정지 테스트: `... c.stop()` / 해제: `... c.release()`
  - (전체 순환은 `sudo python3 ldar_decision.py --source mock`)

**A-2A-② 안 뜨면 R5 micom에 교육용 IPC-CAN 데모 펌웨어를 올린다** (WSL2 R5 BSP)
- [ ] R5 BSP에서 **IPC 서비스 + CAN + 교육용 CAN demo** 활성 빌드 (VCP-G의 `MCU_BSP_SUPPORT_CAN_DEMO`에 대응하는 D3-G R5측 IPC-CAN 교육 앱 플래그 ON) + `CAN_Init()` 도는지 확인
- [ ] 상향 0x120도 함께 쓰려면 같은 빌드에 `ldar_bridge.c`(`LdarBridge_Init()`)도 포함
- [ ] 빌드 → D3-G 플래시 → 위 A-2A-① 재확인
- [ ] 정확한 빌드 플래그·파일명은 **D02(IPC·CAN) 강의 micom 샘플** 기준 (참고 도구: `reference/ipc-example/IPC_Example.py`, 강의자료 `~/Education/d3-g app/`)

**주의**
- 방법 A와 방법 B(아래)는 **택일** — 둘 다 켜면 같은 IPC를 두 번 처리해 CAN 중복 송신
- IPC 장치 = `/dev/tcc_ipc_micom`, 패킷 내 channel_bitmask=`0x01` → **CAN ch0**(VCP-G와 동일)
- 장점: 커스텀 R5 코드 0 / 단점: BSP 핸들러 의존(블랙박스), R5 콘솔 로그가 방법 B(`[LDAR-DS]`)보다 적을 수 있음
- ⚠️ `reference/`는 빌드에 import/include 금지 — `IPC_Example.py`는 **단독 실행 테스트 도구로만** 참고(위 확인은 우리 `ldar_can.py` 사용)

### 방법 B (명시적 제어): 작성해 둔 코드 사용
[d3-g/r5/.../ldar_downstream.c](d3-g/r5/sources/app.ldar.bridge/ldar_downstream.c) — 직접 파싱·송신을 통제할 때.
- [ ] `ldar_downstream.{c,h}`를 R5 BSP `sources/app.sample/app.ldar.bridge/`에 배치
- [ ] `app.sample/rules.mk`가 이 폴더 `rules.mk`를 include하는지 확인 (`ldar_downstream.c` 등록은 완료)
- [ ] R5 `app.base/main.c`의 `AppTaskCreate()`에서 `CAN_Init()` 뒤에 **`LdarDownstream_Init();`** 호출 추가
- [ ] `ldar_downstream.c`의 `extern int Tcc_Ipc_Recv(...)` 플레이스홀더를 **R5 BSP 실제 IPC 수신 API**로 교체 (상향 `ldar_bridge.c`의 `Tcc_Ipc_Send`와 짝)
- [ ] `LDAR_DS_IPC_CHANNEL`(현 1)을 BSP micom IPC 채널 번호에 맞춤

**공통 — 빌드/플래시 (WSL2 R5 환경, 여기 아님)**
- [ ] R5 BSP `make` → R5 .rom → D3-G 플래시
- ⚠️ R5 펌웨어는 **사용자 WSL2(텔레칩스 R5 빌드환경)**에서만 빌드됨

## A-3. Mock E2E 검증
- [ ] D3-G A72: `cd d3-g/a72 && sudo python3 ldar_decision.py --source mock`  ← **`--dry-run` 빼기!**(실제 IPC 송신)
- [ ] **R5 콘솔**에 `[LDAR-DS] IPC -> CAN 0x110 ...` 뜨는지 확인
- [ ] **VCP-G 콘솔**에 `[OVR] RX 0x110 LIMIT 60% / STOP / RELEASE` + 모터 듀티 부드럽게 감소 확인
- [ ] Mock 순환(`none→60→30→stop→clear`)대로 적색 LED(LIMIT 깜박/STOP 점등)·부저 동작 확인
- 🔧 CAN 안 보이면: 종단저항·트랜시버·H/L 결선부터. (A72엔 `candump` 없음 → 콘솔 로그나 USB-CAN 분석기로)

## A-4. 튜닝 (선택, 시연 보며)
- [ ] 감속 부드러움 — [override.c](vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/override.c) `OVR_CAP_SLEW_DOWN/UP`
- [ ] 표지판 명령 안정성 — [ldar_decision.py](d3-g/a72/ldar_decision.py) `CFG`의 `CONF_THRESH`/`CONFIRM`

---

# B. 병행 (인지 — 정은진): AI-G YOLOv8 표지판
**A와 독립.** 모델이 준비되면 마지막에 D3-G에 연결.

## B-1. 데이터 수집 — [ai-g/data_pipeline/](ai-g/data_pipeline/)
- [ ] 마운트 **먼저 고정**(정면·고정), 그 PiCam으로만 촬영 (폰·중간 재장착 금지)
- [ ] 표지판 4종(`speed_30/speed_60/stop/no_entry`) 다양한 거리·각도·조명 — 클래스당 100~200장
- [ ] 바운딩박스 라벨링
- ⚠️ **학습 데이터 = 데모 셋업 100% 동일**(같은 PiCam·해상도·표지판세트·조명)

## B-2. 학습 · 변환 · NPU
- [ ] yolov8 pretrained → fine-tune (위 4클래스)
- [ ] ONNX export → tc-nn-toolkit INT8 양자화(같은 환경 프레임 calibration)
- [ ] NPU 컴파일 → AI-G 배포, tc-nn-app 실행

## B-3. 추론 → 송신
- [ ] 추론 후처리: 최상위 표지판 클래스 + 신뢰도 추출
- [ ] AI-G → D3-G **TCP 송신**: 줄단위 `{"ts":.., "sign":"speed_30", "conf":0.9}\n`
  - ⚠️ 이 JSON 포맷·클래스명은 **임시 약속** — D3-G [ldar_decision.py](d3-g/a72/ldar_decision.py)의 `Detection`/`SIGN_TO_CMD`와 맞추기

---

# C. 마지막: 전체 통합 — 공동
- [ ] AI-G→D3-G 실연결: `python3 ldar_decision.py --source tcp --port 9999` (Mock 대신 실제 표지판)
- [ ] 카메라→NPU→판정→CAN→감속 전체 흐름 검증
- [ ] 시연 5종(정상주행 / 30제한 / 60제한 / 정지 / 해제) 안정 통과
- [ ] (옵션) QT 계기판 — 속도상한·오버라이드 상태 시각화
- [ ] 최종 데모 영상 + 발표 자료

---

## 이미 끝난 것 (다시 안 해도 됨)
- ✅ VCP-G 수동주행 펌웨어(조이스틱·모터·서보·방향지시·부저)
- ✅ VCP-G 속도 오버라이드: `override.c`(슬루 감속) + `override_can.c`(0x110 수신) — 컴파일·ROM·**플래시 완료**
- ✅ D3-G 판정 매핑 `ldar_decision.py`(표지판→명령) — Mock dry-run 검증
- ✅ D3-G 하향 CAN `ldar_can.py`(limit/stop/release) + R5 `ldar_downstream.c` 코드 — 호스트 상호운용 테스트 통과(파서·CRC16·오프셋 확실)
- ✅ 문서(README/CLAUDE/PROTOCOL/ROADMAP) 새 컨셉 반영, 구 차선 자료 [backup/](backup/)

## 하드웨어 함정 (CLAUDE.md 반복 주의)
- 모든 버튼·조이스틱 SW = **active-low**(풀업, 눌림=0)
- 조이스틱 VCC = **3.3V**(5V 금지)
- DC 모터 = 오픈루프 PWM(듀티=평균전압) → 속도 상한이 곧 오버라이드 수단
- A72엔 **SocketCAN 없음** → `candump` 불가, 콘솔/USB-CAN로 확인
- 콘솔 다 죽고 모터만 살면 **3.3V 레일 브라운아웃**(배선/단락) 의심

## 핵심 파일 빠른참조
| 영역 | 파일 |
|---|---|
| VCP-G 수신·감속 | `app.ldar.vcp/override_can.c`, `override.c`, `ldar_app.c` |
| VCP-G 핀·ID | `app.ldar.vcp/ldar_pins.h` (0x110, CAN ch0) |
| D3-G 판정 | `d3-g/a72/ldar_decision.py` |
| D3-G 하향 CAN | `d3-g/a72/ldar_can.py` |
| R5 하향 브리지 | `d3-g/r5/sources/app.ldar.bridge/ldar_downstream.c` |
| AI-G 촬영 | `ai-g/data_pipeline/capture.py`, `extract_frames.py` |
