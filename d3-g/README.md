# D3-G — HPC Zone (판정 · 명령)

AI-G가 인식한 **교통표지판**을 받아 **속도제한/정지를 판정**하고, VCP-G에 **속도 오버라이드를
CAN으로 명령**한다. Linux를 도는 **A72**(판정)와 FreeRTOS를 도는 **R5**(IPC↔CAN)로 나뉜다.

```
AI-G ──Ethernet TCP(표지판 JSON)──▶ ldar_decision.py (A72)
                                     │ 표지판 → 속도 명령 매핑 (SIGN_TO_CMD)
                                     │ ldar_can.py → IPC (/dev/tcc_ipc_micom)
                                     ▼
                               R5 (하향 브리지) ──CAN 0x110──▶ VCP-G
                                 Speed Override: [0]=mode, [1]=한계속도(km/h)
```

---

## 구성

| 경로 | 환경 | 내용 |
|---|---|---|
| [a72/](a72/) | D3-G 보드 (Python, Linux) | 판정·명령 앱 — 표지판→속도 매핑, IPC 송신 |
| [r5/](r5/) | WSL2 (R5 BSP 오버레이) | R5 LDAR 모듈 — CAN↔IPC 브리지 (상향 0x120, 하향 0x110) |
| [shared/](shared/) | 공용 | A72↔R5 IPC 헤더 `ldar_ipc_proto.h` |

> **R5 BSP는 리포에 포함하지 않는다.** R5 펌웨어는 Windows WSL2의 Telechips R5 빌드환경
> (TCC805x BSP)에서 빌드한다. 여기(code-server)선 빌드하지 않는다. 리포의 `r5/`는 BSP
> `sources/app.sample/app.ldar.bridge/`로 얹는 **오버레이 모듈**이다.

---

## A72 판정 앱 (보드에서 직접)

```bash
cd a72
python3 ldar_decision.py --source mock --dry-run   # AI-G 없이 매핑 검증 (콘솔만)
python3 ldar_decision.py --source mock             # 실제 IPC 송신 (sudo 필요)
python3 ldar_decision.py --source tcp --port 9999  # 실제 AI-G TCP 수신
```

`/dev/tcc_ipc_micom` 접근은 root 필요(`sudo`). 파일별 역할·튜닝은 [a72/README.md](a72/README.md).

### 판정 매핑 (`SIGN_TO_CMD`)

| 표지판 | 명령 | CAN 0x110 `[mode,km/h]` | VCP-G 동작 |
|---|---|---|---|
| speed_30 | LIMIT 30 | `[1,30]` | 듀티 상한 30%까지 부드럽게 감속 |
| speed_60 | LIMIT 60 | `[1,60]` | 듀티 상한 60%까지 부드럽게 감속 |
| stop / no_entry | STOP | `[2,0]` | 0%까지 부드럽게 감속 후 정지 |
| clear (제한구역 종료) | RELEASE | `[0,0]` | 상한 해제 |
| none (표지판 없음) | — | (송신 안 함) | 직전 명령 유지 |

- 신뢰도 `conf < CONF_THRESH`(0.5) 검출은 무시. 같은 명령이 `CONFIRM`(2) 프레임 연속이어야 전환(깜박임 방지).
- 새 표지판은 직전 명령을 덮어씀. `none`은 직전 명령을 **유지**. 임계값은 `CFG` 딕셔너리에서 튜닝.

---

## IPC (A72 ↔ R5)

`/dev/tcc_ipc_micom` 경유. 교육용 IPC 패킷(SYNC·CMD·LENGTH·DATA·CRC16)으로 CAN 프레임을 래핑.

- **하향** — A72가 `(canID, data)`를 교육용 CAN 데모 명령(CMD1=0x05)으로 IPC write → R5가 CAN 송신.
  → `a72/ldar_can.py`
- **상향** — R5가 0x120 CAN 수신 → `LdarIpcUpstream_t`로 A72 전달.
  → `r5/sources/app.ldar.bridge/ldar_bridge.c`, `shared/ldar_ipc_proto.h`

### R5 하향 브리지 올리기 — 택일 (둘 다 켜면 CAN 중복 송신)

**방법 A (간단): 교육용 IPC-CAN 데모 활성화** — `ldar_can.py`가 보내는 IPC 패킷은 Telechips
교육용 CAN 데모 명령(CMD1=0x05)이다. R5 micom 교육 펌웨어에 이 명령을 받아 `(canID,data)`를
그대로 CAN으로 쏘는 핸들러가 들어있어, 그 펌웨어가 돌면 **커스텀 R5 코드 0**으로 하향이 된다.

**방법 B (명시적 제어): `r5/.../ldar_downstream.c`** — 직접 파싱·송신을 통제할 때. WSL2 R5 BSP에서:
1. `r5/sources/app.ldar.bridge/`를 BSP `sources/app.sample/app.ldar.bridge/`로 배치.
2. `app.sample/rules.mk`가 이 폴더 `rules.mk`를 include하는지 확인.
3. R5 `app.base/main.c`의 `AppTaskCreate()`에서 `CAN_Init()` 뒤에 `LdarDownstream_Init();` 호출 추가.
4. `ldar_downstream.c`의 `extern int Tcc_Ipc_Recv(...)` 플레이스홀더를 **실제 BSP IPC 수신 API**로 교체.
5. 빌드 → D3-G 플래시. (상향 0x120→IPC는 `ldar_bridge.c`에 이미 있음.)

---

## AI-G → D3-G TCP 연동 — 포맷 shim ⚠️

실제 AI-G 빌드는 프레임당 JSON 한 줄을 `192.168.0.100:9999`에서 송신:
```json
{"boxes":[{"cls":3,"score":0.92,"xmin":..,"ymin":..,"xmax":..,"ymax":..}]}
```
그런데 `ldar_decision.py`는 `{"ts":.., "sign":"speed_30", "conf":0.92}`(문자열 sign)를 기대한다 →
**shim에서 `cls(int)→sign(str)` 매핑**(AI-G `dataset/data.yaml` 클래스 순서), `score→conf`,
top-box 선택을 해야 한다. score 스케일(0~1 vs 0~100)을 라이브 출력으로 확인 후 `CONF_THRESH` 조정.

---

## 보드 셋업 메모

- **접속** — Ethernet DHCP(예 `192.168.0.35`), `/dev/tcc_ipc_micom` 존재.
- **rootfs 확장**(용량 부족 시, D01-T06) — `/dev/mmcblk0p4`를 `parted` rescue→`resizepart 4`→
  `resize2fs`로 확장. **중간에 깨지면 접속 불능**이 될 수 있으니 재부팅 전 신중히.
- **CAN 디버깅** — A72에 SocketCAN(`can0`) **없음** → `candump` 불가. 외장 USB-CAN 분석기 또는
  R5/VCP 콘솔 로그로 확인.
