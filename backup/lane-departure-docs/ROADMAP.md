# ROADMAP — 개발 현황 · 단계 · 자재

> [README](../README.md)의 진행 현황 분리본.

## 현황 요약
- ✅ **환경/기초** — D01–D06 (WSL/Yocto/D3-G, VCP-G 주변장치, CAN/IPC, Control Zone, AI-G+NPU 기초), 제어 측 아키텍처 확정
- ✅ **제어 Phase 1 (VCP-G 수동주행)** — 조이스틱 ADC·모터·서보·방향지시 토글·부저 펌웨어 동작
- ✅ **제어 Phase 2 (D3-G 판정앱)** — Mock으로 상태머신·P복귀각 검증 완료
- 🟧 **인지(AI-G)** — YOLOv8-seg fine-tune 차선 검출 (정은진, UFLD에서 전환)
- 🟧 **제어 Phase 3 (오버라이드 통합)** — R5 하향(IPC→CAN) + VCP-G CAN 수신 남음
- ⬜ Phase 4 통합·튜닝, 데모

## 인지 (AI-G) — 정은진

**접근 = YOLOv8-seg fine-tune** (UFLD에서 전환). 데모는 **고정 환경**(검정 배경,
흰 실선/점선, 직선 3m, 같은 조명·PiCam)이라 그 환경에 오버핏시키는 전략.
→ 전환 근거·대안 비교는 아래 "모델 결정" 참조.

### A1 — 데이터 수집 ([ai-g/data_pipeline/](../ai-g/data_pipeline/)) ✅코드
- [x] PiCam 촬영 `capture.py` (picamera2, OpenCV 폴백) + 프레임추출 `extract_frames.py`
- [ ] 마운트 확정(정면·아래 15~30° 틸트·고정) 후 시나리오 5종 촬영
      (center/drift_left/drift_right/weave/heading) — 목표 200~300장
- [ ] 흰색 임계값으로 마스크 자동 생성 → 손 교정(반자동 라벨링)

### A2 — 학습 & 변환
- [ ] yolov8s-seg pretrained → fine-tune (클래스 `solid_L/solid_R/dashed_C`)
- [ ] ONNX export → tc-nn-toolkit 양자화(같은 트랙 프레임으로 INT8 calibration)
- [ ] NPU 컴파일 → AI-G 배포, tc-nn-app 실행

### A3 — 추론 & 송신
- [ ] PiCam → NPU 추론 → 후처리: seg 마스크 → 차선 중심선 좌표 + 실선/점선 라벨
- [ ] AI-G → D3-G TCP 송신 (좌/우 좌표, 실선(1)/점선(2), 위험도, 타임스탬프)
- **Exit**: 실시간 차선 좌표 + 실선/점선 콘솔 출력 → D3-G 수신

### 모델 결정 (UFLD → YOLOv8-seg)
- **UFLD 탈락** — ⓐ pretrained가 실도로·전방시점 분포라 검정배경·RC 저각 구도에서 도메인/구도 갭 → 재학습 필수 ⓑ 선 종류(실선/점선) 미출력 ⓒ column-anchor reshape tc-nn-toolkit 컴파일 검증 부담
- **YOLOv8-seg 채택** — ⓐ **yolov8s가 이미 NPU에서 동작**([ai_model/](../ai-g/ai_model/)) + Telechips D06-T06/D07-T02~T03 Yolo 튜토리얼 → 호환성 리스크 최소 ⓑ 실선/점선을 **클래스로 직접 학습** → 프로토콜 충족 ⓒ 검정배경이라 라벨링 쉬움
- **고전 CV** = 무학습 fallback (검정 고대비엔 견고하나 NPU 미사용) — 데모 안전망

## 제어 (VCP-G + D3-G) — 서준상

### Phase 1 — VCP-G 수동 주행 (로컬 완결) ✅
- [x] T1.1 조이스틱 ADC — VRx/VRy 2채널 + SW, 고정중심 2048·데드존·정규화 (조이스틱 VCC 3.3V 확정)
- [x] T1.2 입력 매핑 → DC모터(L298N 방향+PWM 0~90%), 서보(0~127, center 63 좌우대칭)
- [x] T1.3 방향지시 택트 버튼 2개(토글, active-low) → 녹색 LED + 상향 CAN `0x120`
- [x] 조이스틱 SW → 피에조 부저, 방향지시/오버라이드 표시 LED 배선·구동
- [~] T1.4 R5 `0x120` 수신 → IPC (ldar_bridge.c 작성됨, 보드 통합 검증 남음)

### Phase 2 — 이탈 감지 & 판정 (D3-G A72, Mock) ✅
- [x] T2.1 Mock 차선 주입기 (입력 소스 인터페이스 분리 — Mock/TCP 교체)
- [x] T2.2 이탈 감지 (횡거리, 실선/점선 판별)
- [x] T2.3 Decision Matrix 상태머신 (SAFE/WARNING/OVERRIDE/CRITICAL)
- [x] T2.4 P 비례 복귀각 (횡오프셋·heading, CRITICAL 감속)
- [x] T2.5 A72 → IPC 명령 송신 (0x110/0x111/0x107/0x106) — [a72/ldar_can.py](../d3-g/a72/ldar_can.py)

### Phase 3 — 오버라이드 CAN & 액추에이션 🟧
- [ ] T3.1 R5 하향 경로 — A72 IPC 프레임 → CAN TX (교육 CAN demo 또는 ldar_bridge 확장)
- [ ] T3.2 VCP-G CAN 수신 — 0x110/0x111/0x106/0x107 핸들러 (`override.c`가 현재 적색 LED·부저 스텁)
- [ ] T3.3 VCP-G 제어권 중재 (USER=조이스틱, BOARD=CAN 조향, 전환 램프)
- [ ] T3.4 상태 LED(🟢🟠🔴) + 부저 패턴 (점선 경고/실선·CRITICAL)
- [ ] T3.5 제어권 전환 E2E (채터링 없는 핸드오프)
- **Exit**: Mock→D3-G→CAN→VCP-G 오버라이드 + LED/부저/복귀 액추에이션

### Phase 4 — AI-G 통합 & 튜닝 ⬜
- [ ] T4.1 AI-G→D3-G TCP 수신 (패킷 포맷 확정, Mock→실제 교체)
- [ ] T4.2 임계값·게인 튜닝 (Tin/Tout/Th/heading/P게인/복귀램프)
- [ ] T4.3 엣지 케이스 (곡선·조명, 채터링 회귀)
- [ ] T4.4 응답 지연·추론속도 측정·최적화
- **Exit**: 시연 5종(정상/실선이탈/점선이탈 무방향/점선이탈+방향지시/복귀) 안정 통과

### 통합 · 데모 — 공동
- [ ] 카메라→NPU→판정→CAN→액추에이션 전체 흐름 검증
- [ ] (옵션) QT 계기판 App — 제어권·속도 시각화
- [ ] 최종 데모 시연 + 영상, 발표 자료

## 필요 자재 — 모두 준비 완료
| 분류 | 항목 |
|---|---|
| Boards | TOPST AI-G / D3-G / VCP-G, MIPI CSI 카메라, Ethernet 허브·케이블, USB-to-TTL UART |
| Vehicle | 모형 섀시, DC 모터, 서보, L298N, LED, 점퍼·저항, 모터 전용 배터리팩 |
| Input | 아날로그 조이스틱(VRx/VRy/SW), 택트 버튼 2개, 상태 LED 3색, 피에조 부저 |
| Env | 실선&점선 트랙(직선+완만곡선), 차선 데이터셋, tc-nn-toolkit/app, Yocto/FreeRTOS toolchain |

## 중간발표 PDF와의 차이 (PDF는 구 설계)
- **조이스틱** — PDF: USB HID, D3-G 연결, 구매 필요 → 현재: **아날로그(VRx/VRy/SW), VCP-G ADC, 3.3V 보유**
- **수동 제어/중재 위치** — PDF: D3-G A72가 입력 처리 → 현재: **VCP-G 로컬 중재**, D3-G는 판정·복귀각만 CAN으로 지시
- **CAN 0x120** — PDF에 없음 → 현재: VCP-G→D3-G **상향 0x120**(방향지시 의도) 추가
- **상태 LED 3색** — PDF는 브레이크/방향지시/전조/경고만 → 현재: 🟢🟠🔴 제어권·차선 상태 LED 추가
- **자재** — PDF: 조이스틱·피에조 구매·트랙 제작 예정 → 현재: **모두 준비 완료**
