# ROADMAP — 개발 현황 · 단계 · 자재

> [README](../README.md)의 진행 현황 분리본. (구 차선이탈 로드맵은 [backup/](../backup/).)

## 현황 요약
- ✅ **환경/기초** — D01–D06 (WSL/Yocto/D3-G, VCP-G 주변장치, CAN/IPC, Control Zone, AI-G+NPU 기초), 제어 측 아키텍처 확정
- ✅ **제어 Phase 1 (VCP-G 수동주행)** — 조이스틱 ADC·모터·서보·방향지시 토글·부저 펌웨어 동작
- ✅ **제어 Phase 2 (D3-G 판정앱)** — 표지판→속도 명령 매핑 Mock 검증 완료
- ✅ **제어 Phase 3 (VCP-G 속도 오버라이드 수신)** — CAN 0x110 수신 → 부드러운 감속/정지 (컴파일·ROM 검증)
- 🟧 **인지(AI-G)** — YOLOv8 표지판 검출 학습·보드 탑재 진행 (정은진)
- 🟧 **통합** — R5 하향(IPC→CAN 0x110 송신) + AI-G→D3-G TCP 연동 남음
- ⬜ Phase 4 통합·튜닝, 데모

## 인지 (AI-G) — 정은진

**접근 = YOLOv8 표지판 검출**. 데모는 **고정 환경**(같은 조명·PiCam·트랙·표지판 세트)이라
그 환경에 오버핏시키는 전략. 검출할 클래스 = `speed_30 / speed_60 / stop / no_entry`.

### A1 — 데이터 수집 ([ai-g/data_pipeline/](../ai-g/data_pipeline/)) ✅코드
- [x] PiCam 촬영 `capture.py` (V4L2 경로) + 프레임추출 `extract_frames.py`
- [ ] 마운트 확정(정면·고정) 후 표지판별 촬영 (각 클래스 다양한 거리·각도·조명) — 클래스당 100~200장
- [ ] 바운딩박스 라벨링 (표지판 4종)

### A2 — 학습 & 변환
- [ ] yolov8 pretrained → fine-tune (클래스 `speed_30/speed_60/stop/no_entry`)
- [ ] ONNX export → tc-nn-toolkit 양자화(같은 환경 프레임으로 INT8 calibration)
- [ ] NPU 컴파일 → AI-G 배포, tc-nn-app 실행

### A3 — 추론 & 송신
- [ ] PiCam → NPU 추론 → 후처리: 최상위 표지판 클래스 + 신뢰도 추출
- [ ] AI-G → D3-G TCP 송신 (표지판 클래스, 신뢰도, 타임스탬프)
- **Exit**: 실시간 표지판 클래스 콘솔 출력 → D3-G 수신

### 모델 결정 (YOLOv8 detection)
- **YOLOv8 채택** — ⓐ **yolov8s가 이미 NPU에서 동작**([ai_model/](../ai-g/ai_model/)) + Telechips D06-T06/D07-T02~T03 Yolo 튜토리얼 → 호환성 리스크 최소 ⓑ 표지판은 **바운딩박스 검출**로 충분(세그멘테이션 불필요) → 라벨링·후처리 간단 ⓒ 고정 데모 환경 오버핏으로 소량 데이터로도 충분
- **고전 CV(템플릿/색상)** = 무학습 fallback — 데모 안전망

## 제어 (VCP-G + D3-G) — 서준상

### Phase 1 — VCP-G 수동 주행 (로컬 완결) ✅
- [x] T1.1 조이스틱 ADC — VRx/VRy 2채널 + SW, 고정중심 2048·데드존·정규화 (조이스틱 VCC 3.3V 확정)
- [x] T1.2 입력 매핑 → DC모터(L298N 방향+PWM 0~90%), 서보(0~127, center 63 좌우대칭)
- [x] T1.3 방향지시 택트 버튼 2개(토글, active-low) → 녹색 LED + 상향 CAN `0x120`
- [x] 조이스틱 SW → 피에조 부저, 표시 LED 배선·구동
- [~] T1.4 R5 `0x120` 수신 → IPC (ldar_bridge.c 작성됨, 보드 통합 검증 남음)

### Phase 2 — 표지판 판정 (D3-G A72, Mock) ✅
- [x] T2.1 Mock 표지판 주입기 (입력 소스 인터페이스 분리 — Mock/TCP 교체)
- [x] T2.2 표지판 → 속도 명령 매핑 (`SIGN_TO_CMD`)
- [x] T2.3 신뢰도 임계 + CONFIRM 디바운스 (검출 깜박임 방지)
- [x] T2.4 A72 → IPC 명령 송신 (0x110 Speed Override) — [a72/ldar_can.py](../d3-g/a72/ldar_can.py)

### Phase 3 — 속도 오버라이드 CAN & 액추에이션 (VCP-G) ✅
- [x] T3.1 VCP-G CAN 수신 — 0x110 핸들러 ([override_can.c](../vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/override_can.c), RXFIFO1 폴링)
- [x] T3.2 속도 오버라이드 모듈 — LIMIT/STOP/RELEASE + 적용 상한 슬루(부드러운 감속) ([override.c](../vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/override.c))
- [x] T3.3 메인 루프 통합 — `duty=min(조이스틱, 상한)`, STOP 0% 도달 시 브레이크, 조향은 조이스틱 유지
- [x] T3.4 상태 LED(적색 LIMIT 깜박/STOP 점등) + 부저(오버라이드 중) + 컴파일·ROM 검증
- [~] T3.5 R5 하향 경로 — A72 IPC(CMD1=0x05) → CAN TX. 코드 작성+호스트 상호운용 검증 ([r5/.../ldar_downstream.c](../d3-g/r5/sources/app.ldar.bridge/ldar_downstream.c)). WSL2 R5 빌드 + `Tcc_Ipc_Recv` 실제 API 교체 남음
- **Exit**: Mock→D3-G→CAN→VCP-G 속도제한/정지 + LED/부저/감속 액추에이션

### Phase 4 — AI-G 통합 & 튜닝 ⬜
- [ ] T4.1 AI-G→D3-G TCP 수신 (표지판 패킷 포맷 확정, Mock→실제 교체)
- [ ] T4.2 슬루레이트·임계값 튜닝 (감속 부드러움, CONF_THRESH/CONFIRM, 정지거리)
- [ ] T4.3 엣지 케이스 (오검출·조명, 표지판 연속 등장, 정지 후 재출발)
- [ ] T4.4 추론속도·표지판 인식→감속 응답지연 측정·최적화
- **Exit**: 시연 5종(정상주행/30제한/60제한/정지/해제) 안정 통과

### 통합 · 데모 — 공동
- [ ] 카메라→NPU→판정→CAN→액추에이션 전체 흐름 검증
- [ ] (옵션) QT 계기판 App — 현재 속도 상한·오버라이드 상태 시각화
- [ ] 최종 데모 시연 + 영상, 발표 자료

## 필요 자재 — 모두 준비 완료
| 분류 | 항목 |
|---|---|
| Boards | TOPST AI-G / D3-G / VCP-G, MIPI CSI 카메라, Ethernet 허브·케이블, USB-to-TTL UART |
| Vehicle | 모형 섀시, DC 모터, 서보, L298N, LED, 점퍼·저항, 모터 전용 배터리팩 |
| Input | 아날로그 조이스틱(VRx/VRy/SW), 택트 버튼 2개, 상태 LED 3색, 피에조 부저 |
| Env | 교통표지판 세트(속도제한 30/60·정지·진입금지), 트랙, 표지판 데이터셋, tc-nn-toolkit/app, Yocto/FreeRTOS toolchain |

## 방향 전환 메모 (차선이탈 → 표지판)
- **인지** — 차선 좌표/실선·점선 검출 → **표지판 4종 검출**. 차선은 인식하지 않음.
- **판정** — 이탈 상태머신 + P 복귀각 → **표지판 → 속도제한/정지** 단순 매핑.
- **CAN** — 0x110 제어권·0x111 차선·0x107 조향·0x106 속도 → **0x110 Speed Override**(mode + 한계속도) 단일 메시지.
- **구동** — 조향 오버라이드(좌/우 복귀) → **속도 상한/정지**(부드러운 감속). 조향은 항상 운전자.
- 구 설계 코드·문서는 [backup/](../backup/)에 보관.
