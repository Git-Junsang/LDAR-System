# AI-G 데이터 파이프라인 — 촬영 · 프레임추출

YOLOv8 **표지판 검출** fine-tune 학습 데이터를 만든다. 검출 클래스 4종
(`Stop / No Entry / Speed_Limit_60 / Speed_Limit_30`, 순서는 `ai_model/dataset/data.yaml` 고정).
데모는 **고정 환경**(같은 조명·카메라·표지판 세트)이라 그 환경에 오버핏시키는 게 목표.

## ⚠️ 황금률 — 추론 셋업 그대로 촬영

학습 영상 = 데모 셋업이 **100% 같아야** 한다. 하나라도 다르면 학습이 무용지물.
1. **마운트 먼저 확정** — PiCam 정면, 나사로 단단히 고정. 촬영~데모 사이 떼지 말 것.
2. **같은 카메라(PiCam)로만 촬영** — 폰 금지.
3. **해상도는 NPU 추론 입력(640×640 letterbox)에 맞춤** — `--width/--height`.
4. 배경·조명 고정. **변화시킬 건 표지판 종류·거리·각도(화면 내 위치·크기)뿐.**

## ⚠️ AI-G 보드 카메라 환경 (실측 확인)

AI-G(N-Dolphin, A53 Quad, NPU 8TOPS, **RAM 2GB**, Yocto)에서 직접 확인:
- 센서 = **OV5647**(RasPi Cam v1.3, MIPI CSI-2 15핀). 추론 `tcnnapp`이 V4L2 **`/dev/video2`**로 잡음.
- 파이프라인: ov5647(bayer) → mipi_csi2 → tcc-isp → **UYVY 1288×956** → `/dev/video2` (링크 전부 ENABLED·IMMUTABLE).
- **보드 도구 전무**: python3·gcc·ffmpeg·ImageMagick 없음, gstreamer 쓸 엘리먼트 없음, **v4l2-ctl 하나뿐**
  (게다가 `--stream-to`는 multiplanar 버퍼를 파일에 안 써줌 = 0바이트).
- **USB 웹캠 미지원**(스펙에 USB 호스트/UVC 없음) → 촬영은 MIPI OV5647로.
- **저장은 `/home/root`(eMMC)** — `/tmp`는 tmpfs(RAM)라 큰 캡처 금지.

→ 그래서 보드 촬영은 **정적 캡처 바이너리 `vcap`**(의존성 0)으로. 인코더가 없으니
**보드는 UYVY raw 만 저장 → PC로 옮겨 `uyvy2img.py`로 jpg 변환·축소**.

## 1) 촬영

### AI-G 보드에서 (실제 학습 데이터) — `vcap` (정적 바이너리)

빌드(코드서버에서, 이미 빌드된 `vcap` 있으면 생략):
```bash
aarch64-linux-gnu-gcc -O2 -static vcap.c -o vcap   # 의존성 없는 aarch64 정적 바이너리
```
`vcap`을 보드로 옮긴 뒤(scp/USB) **표지판 종류별로** 촬영:
```sh
# ./vcap <outdir> <prefix> <save_count> [skip] [device] [W] [H]
# skip=5 → 30fps에서 매 6프레임 중 1장(~5fps), 처음 10프레임은 AE 안정용 자동 버림
./vcap /home/root/data/speed_30  speed_30  60 5
./vcap /home/root/data/speed_60  speed_60  60 5
./vcap /home/root/data/stop      stop      60 5
./vcap /home/root/data/no_entry  no_entry  60 5
```
각 표지판을 **다양한 거리·각도·조명**으로. → `/home/root/data/<class>/<class>_NNNNN.uyvy` (UYVY 1288×956 raw).
그 뒤 **폴더째 PC로 전송**(scp/USB) → 아래 2)로 변환.

### PC에서 (USB웹캠 리허설용) — `capture.py`

cv2 있는 PC에서 파이프라인 검증용. 표지판을 보여주며 클립을 녹화:
```bash
# 표지판별 10초 클립 (여러 번 반복해 거리·각도 다양화)
python3 capture.py --scenario speed_30 --duration 10
python3 capture.py --scenario stop     --duration 10
# Enter 누를 때까지: --duration 0   /  영상 없이 프레임 바로 저장: --mode frames --fps 5 --max-frames 200
```
→ `data/raw/<class>/<class>_<타임스탬프>_NN.mp4`

## 2) 변환/추출 (PC에서)

### 보드 UYVY raw → jpg — `uyvy2img.py` ★ 본 경로
```bash
python3 uyvy2img.py data/raw_uyvy --out data/frames --src-size 1288x956 --scale 640x640 --dedup 4.0
```
### PC mp4 클립 → jpg — `extract_frames.py` (USB웹캠 리허설용)
```bash
python3 extract_frames.py data/raw --out data/frames --fps 2 --dedup 4.0
```
→ `data/frames/*.jpg`

**목표 장수**: 데모 최소 클래스당 ~100장, 안정 권장 **100~200장/클래스**. 장수보다
**4클래스 × 거리·각도·조명 커버**가 중요.

## 3) 다음 단계 (이 디렉터리 밖)

- **라벨링** — 클래스 4종 **바운딩박스**(detection). 라벨 .txt의 정수 인덱스 = `data.yaml` 순서 고정.
- **학습·변환·배포** — yolov8s pretrained fine-tune → ONNX export → tc-nn-toolkit 양자화·컴파일 → NPU 배포.
  전 과정은 [../ai_model/README.md](../ai_model/README.md).

## 데이터는 git 추적 제외

`data/`(영상·프레임·라벨)는 용량이 커 커밋하지 않는다 — 코드만 추적.
