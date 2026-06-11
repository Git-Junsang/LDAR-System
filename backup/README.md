# backup/ — 구(舊) 차선이탈 자동복귀(LDAR) 자료

프로젝트가 **차선이탈 자동복귀** → **표지판 인식 속도/정지 오버라이드**로 전환되면서
더 이상 쓰지 않는 구 설계 자료를 여기 보관한다. (git 이력 `72d173a` 이전에도 전부 남아 있음 —
이 폴더는 작업트리에서 바로 참고하기 위한 사본/이관본.)

## 무엇이 바뀌었나
| 구분 | 구 설계 (이 폴더) | 신 설계 (리포 본체) |
|---|---|---|
| 인지(AI-G) | 차선 좌표·실선/점선 검출 | **교통표지판** 검출(YOLOv8) |
| 판정(D3-G) | 이탈 판정 + P 비례 복귀각 | 표지판 → **속도제한 30/60 · 정지** 결정 |
| 명령(CAN) | 0x110 제어권·0x111 차선상태·0x107 조향·0x106 속도 | **0x110 Speed Override**(mode + 한계속도) |
| 구동(VCP-G) | 조향 오버라이드(좌/우 복귀) | **속도 상한/정지 오버라이드**(부드러운 감속) |

차선은 더 이상 인식·판정하지 않는다. 조이스틱 수동주행은 그대로 유지되고, 표지판을 만나면
속도만 강제로(부드럽게) 제한되거나 정지한다.

## 폴더 내용
| 경로 | 원래 위치 | 비고 |
|---|---|---|
| `lane-departure-docs/README.md` | `README.md` | 구 최상위 README(차선이탈) |
| `lane-departure-docs/PROTOCOL.md` | `docs/PROTOCOL.md` | 구 CAN/판정/핀맵 |
| `lane-departure-docs/ROADMAP.md` | `docs/ROADMAP.md` | 구 로드맵 |
| `lane-departure-docs/a72-README.md` | `d3-g/a72/README.md` | 구 D3-G 판정앱 설명 |
| `d3-g-lane-decision/ldar_decision.py` | `d3-g/a72/ldar_decision.py` | 차선 상태머신 + P 복귀각 |
| `d3-g-lane-decision/ldar_can.py` | `d3-g/a72/ldar_can.py` | 구 하향 CAN(제어권/차선상태/조향/속도) |
| `vcp-g-lane/override.{c,h}` | `app.ldar.vcp/override.{c,h}` | 구 조향 복귀 오버라이드(OVR_LEFT/RIGHT) |
| `vcp-g-lane/turn_led.{c,h}` | `app.ldar.vcp/turn_led.{c,h}` | 구 적색 LED = 복귀 방향 표시 |

> 참고용일 뿐 빌드에 포함하지 않는다. 재사용(조이스틱/모터/서보/방향지시/부저 등 수동주행
> 펌웨어, AI-G 촬영 파이프라인)은 리포 본체에 그대로 남아 있다.
