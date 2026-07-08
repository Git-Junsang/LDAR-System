# AI-G — Sensing Zone (인지)

PiCam 영상을 NPU로 추론해 **교통표지판 4종을 검출**하고, 클래스+신뢰도를 D3-G에 **TCP로 송신**한다.
접근 = **YOLOv8 검출**. 데모는 **고정 환경**(같은 조명·PiCam·트랙·표지판 세트)이라 그 환경에
오버핏시키는 전략.

**클래스(순서 고정, `ai_model/dataset/data.yaml`이 단일 출처):**
`0 Stop / 1 No Entry / 2 Speed_Limit_60 / 3 Speed_Limit_30`

> 이 정수 인덱스가 그대로 NPU→D3-G 전선 위의 `cls` 값이 된다. **절대 재정렬 금지.**

---

## 보드 (N-Dolphin) — 실측 확인

- SoC: **A53 Quad + Enlight NPU 8TOPS**, RAM **2GB**(입력 해상도 과하게 X), Yocto Linux.
- 카메라: **OV5647**(RasPi Cam v1.3, MIPI CSI-2 15핀). 추론 `tcnnapp`이 V4L2 **`/dev/video2`**로 잡음.
  파이프라인: ov5647(bayer)→mipi_csi2→tcc-isp→**UYVY 1288×956**→`/dev/video2`.
- **USB 웹캠 미지원**(스펙에 USB 호스트/UVC 없음) → 추론·촬영은 MIPI PiCam으로. picamera2 안 됨(V4L2 경로).
- 접속: UART 시리얼 **115200/None**, login `root`/`root`. Ethernet(예 **192.168.0.100**, PC NIC 192.168.0.8/24 고정).
- 보드 도구 전무(python3·gcc·ffmpeg 없음, v4l2-ctl 하나뿐) → 촬영은 정적 바이너리 `vcap`으로.

---

## 워크플로

| 단계 | 폴더/문서 | 환경 |
|---|---|---|
| 1. 데이터 촬영·프레임추출 | [data_pipeline/README.md](data_pipeline/README.md) | AI-G 보드(vcap) + PC(uyvy2img) |
| 2. 라벨링 (bbox 4클래스) | (data_pipeline 밖) | PC |
| 3. 학습 → ONNX → NPU 변환·양자화·컴파일 | [ai_model/README.md](ai_model/README.md) | GPU PC → WSL(tc-nn-toolkit) |
| 4. 배포·추론 | `ai-g app/tcnnapp` | AI-G 보드 |
| 5. D3-G 연동 (TCP shim) | [../d3-g/README.md](../d3-g/README.md) | D3-G A72 |

- **`data_pipeline/`** — PiCam 촬영(`vcap`)·프레임추출(`uyvy2img.py`/`extract_frames.py`).
- **`ai_model/`** — `train.py`(yolov8s fine-tune) · `export_onnx.py`(6-출력 추출+4채널 검증) · `dataset/data.yaml`.
- **`ai-g app/`** — 보드 NPU 추론 런타임(`tcnnapp`/`motrex_app`). 배포 폴더 필수 구성: `quantized_network.bin`+`npu_cmd.bin`+`net.so`.

입력 해상도는 **640×640(letterbox)**.

---

## TCP 송신 포맷 (AI-G → D3-G)

커스텀 AI-G 빌드는 **TCP 서버 `192.168.0.100:9999`**, 프레임당 JSON 한 줄:
```json
{"boxes":[{"cls":3,"score":0.92,"xmin":..,"ymin":..,"xmax":..,"ymax":..}]}
```
D3-G `ldar_decision.py`는 `{"ts":.., "sign":"speed_30", "conf":0.92}`를 기대 → **shim으로 변환**
(`cls`→`sign` by data.yaml, `score`→`conf`, top-box 선택). 상세 [../d3-g/README.md](../d3-g/README.md).

---

## ⚠️ 학습 황금률

학습 데이터 = **데모 셋업 100% 동일**(같은 PiCam·마운트·해상도·표지판세트·조명). 마운트 먼저
고정하고 그 카메라로만 촬영. 폰 촬영·중간 재장착 금지.

- 현재 `dataset/`은 공개셋(GTSRB/Roboflow, 유럽 표지판) 기반 → 데모 표지판이 다르면 인식률↓.
  데모 셋업으로 추가 촬영·재학습 권장.
- 좌우반전 aug는 끈다(표지판은 거울상이 다른 의미).

---

## 현황

- [x] 데이터 파이프라인 코드(`vcap`/`uyvy2img.py`/`capture.py`/`extract_frames.py`)
- [x] 학습·export·NPU 변환 스크립트(`train.py`/`export_onnx.py`) + 스톡 yolov8s NPU 검증
- [ ] 데모 셋업 촬영·라벨링(클래스당 100~200장) → fine-tune
- [ ] NPU 컴파일·보드 배포(`tcnnapp`) → 실시간 검출
- [ ] AI-G → D3-G TCP 송신 + shim 연동

> **데이터(`data_pipeline/data/`, `ai_model/dataset/`의 영상·프레임·라벨)는 git 추적 제외**(용량) — 코드만 커밋.
