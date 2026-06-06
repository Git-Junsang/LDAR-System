# d3-g/reference — 참고 자료 (유사 프로젝트 원본)

유사 프로젝트(자율주행 RC) 및 텔레칩스 교육 예제에서 가져온 **참고용 원본**.
이 프로젝트 빌드와는 무관하며(아무 데서도 import/include 하지 않음), 적응해서 쓸 코드의
출처일 뿐이다. 실제 적응본은 [`../a72/`](../a72/)에 있다.

> ⚠️ **파일명과 실제 내용이 뒤섞여 있다** (복사 과정에서 확장자·이름이 엉킴).
> 이름은 원본 보존을 위해 그대로 두고, 아래 표에 **실제 내용**을 명시한다.

## 파일별 실제 내용

### `src/` — 소스 원본
| 파일명 | 실제 내용 | 용도 |
|---|---|---|
| `NnAppMain.h` | **NnAppMain.c** (AI 앱 본체) | AI-G NPU 비전앱 main — 카메라·추론·디스플레이 파이프라인 |
| `can_vcp_ctrl.c` | **NnAppMain.h** (AI 앱 헤더) | `param_info_t` / `app_context_t` 정의 |
| `can_vcp_ctrl.h` | **can_vcp_ctrl.c** (R5 제어 구현) | R5/MCU가 CAN 수신 → 모터·서보·LED·LCD 액추에이션 |
| `demo_last.py` | **can_vcp_ctrl.h** (위 구현의 헤더) | `VCP_IO` enum(0x101~0x107), 핀/PWM 정의 |
| `demo_rc_ver.py` | A72 자율주행 파이썬 *(이름 일치)* | 차선검출+조향/속도 제어, IPC로 CAN 송신 — **a72 적응 시 주 참고** |

### `bin/` — 프리빌트 실행파일 (aarch64 ELF)
| 파일명 | 실제 내용 |
|---|---|
| `tcnnapp` | NPU 앱 러너 (ELF) |
| `test_dding.py` | **ELF 바이너리** (OpenCV+NPU 비전앱, `.py`는 잘못된 확장자) |

### `ipc-example/` — 교육용 IPC 예제
| 파일 | 용도 |
|---|---|
| `IPC_Example.py` | `/dev/tcc_ipc_micom`로 CAN 프레임 송수신하는 CLI 예제 |
| `Library/IPC_Library.py` | IPC 패킷(CRC16) transport — **a72/Library 의 출처** |

## 이 프로젝트에서 무엇을 재사용했나
- `ipc-example/Library/IPC_Library.py` → [`../a72/Library/IPC_Library.py`](../a72/Library/IPC_Library.py) 그대로 복사 (검증된 transport).
- `src/demo_rc_ver.py`의 IPC→CAN 송신 방식 + 조향 매핑 개념 → [`../a72/ldar_can.py`](../a72/ldar_can.py), [`../a72/ldar_decision.py`](../a72/ldar_decision.py)로 재구성.
  - 단, **차선 검출(OpenCV)은 버림** — LDAR에선 인지가 AI-G 담당, D3-G는 TCP로 좌표만 받아 판정.
- `src/can_vcp_ctrl.*`(R5 액추에이터 제어)는 VCP-G 로컬 펌웨어로 대체됨 — CAN ID(0x106/0x107) 의미만 참고.

필요하면 위 매핑대로 **정확한 이름으로 정정**해 드릴 수 있다(전부 참고용이라 rename 안전).
