# AI-G 표지판 검출 — 학습 → ONNX → NPU → 배포

YOLOv8s 4클래스(`0 Stop / 1 No Entry / 2 Speed_Limit_60 / 3 Speed_Limit_30`) 표지판
검출기를 학습해 TOPST **AI-G(N-Dolphin, Enlight NPU 8TOPS)**에 올리는 전 과정.
정본 근거 = `documents/tutorials/D06-T06-AI모델적용(Yolo).pdf` + `d3-g/reference` 코드.

> **클래스 순서(0~3)는 라벨 .txt가 고정**한다. 이 정수 인덱스가 그대로 NPU→D3-G 전선 위의
> `cls` 값이 된다. **절대 재정렬 금지.** (`dataset/data.yaml`이 단일 출처)

## 3개 환경 — 무엇을 어디서

| 단계 | 환경 | 비고 |
|---|---|---|
| 1. 학습 (fine-tune) | **GPU PC** (ultralytics) | CPU도 되지만 yolov8s@640는 ~하루/100ep. GPU 권장 |
| 2. ONNX export (헤드 잘라내기) | GPU PC 또는 이 code-server | 가벼움. `export_onnx.py` |
| 3. NPU 변환·양자화·컴파일 | **WSL Ubuntu 22.04 + tc-nn-toolkit** | Enlight SDK. GPU 불필요, 전용 환경 필요 |
| 4. net.so 빌드 | WSL (aarch64 크로스 gcc) | 모델별 후처리 라이브러리 |
| 5. 배포·실행 | AI-G 보드 (Ethernet 192.168.0.100) | `tcnnapp` |
| 6. D3-G 연동 | D3-G A72 | TCP 9999, **포맷 변환 shim 필요** |

입력 해상도는 **640×640(letterbox)**. (`data_pipeline/README`의 640×384는 폐기된 차선(UFLD)용.)

---

## 1) 학습 — GPU PC

```bash
# (PC) 가상환경 + 의존성
python3 -m venv venv && . venv/bin/activate
pip install ultralytics onnx onnxslim        # GPU PC면 CUDA torch가 자동 설치됨

# 데이터셋 + 스크립트를 PC로 (둘 중 하나)
#   git clone <this repo>            # dataset/ 은 git 제외이므로 별도 복사
#   또는 ai-g/ai_model/ 폴더째 scp/rsync
# dataset/data.yaml 의 path: 를 PC의 실제 절대경로로 수정

cd ai_model
python3 train.py --epochs 120 --imgsz 640 --batch 16 --device 0   # --device 0 = 첫 GPU
# 결과: runs/detect/signs_yolov8s/weights/best.pt  (+ test mAP 출력)
```
- `yolov8s` 고정 — NPU에서 이미 검증된 아키텍처라 컴파일 리스크 최소.
- 좌우반전 aug는 끔(표지판은 거울상이 다른 의미). `train.py` 참고.
- **데이터 주의(데모 안정성):** 현재 dataset은 GTSRB/Roboflow(유럽 표지판) 공개셋이다.
  프로젝트 황금률은 "데모와 100% 동일한 PiCam·환경으로 직접 촬영". 데모 표지판이
  공개셋과 다르면 실차 인식률이 떨어질 수 있다 → 데모 셋업으로 추가 촬영·재학습 권장
  (`data_pipeline/`의 `vcap`→`uyvy2img.py` 경로, 단 라벨은 bbox 4종).

## 2) ONNX export — 헤드를 정확히 잘라낸다 ★ 이전 시도가 틀린 지점

Enlight `converter.py`는 **일반 풀헤드 ONNX가 아니라**, DFL/decode/concat 직전에서 자른
**6-출력 그래프**(scale별 cls·box conv)를 받는다. 스톡 `yolov8s_extracted.onnx`는 cls 분기가
**80채널(COCO)** — 커스텀은 **4채널**이어야 한다.

```bash
cd ai_model
python3 export_onnx.py --weights runs/detect/signs_yolov8s/weights/best.pt
# -> yolov8s_signs_extracted.onnx
# [verify] 가 cv3.*=[1,4,*,*], cv2.*=[1,64,*,*] 를 확인. cv3가 80이면 즉시 중단(COCO 헤드).
```
출력 6개(순서 = `--output-order cl`): `cv3.0(4ch,80²) cv2.0(64ch,80²) cv3.1(4ch,40²) cv2.1(64ch,40²) cv3.2(4ch,20²) cv2.2(64ch,20²)`.

## 3) NPU 변환·양자화·컴파일 — WSL Ubuntu 22.04 + tc-nn-toolkit

```bash
# (최초 1회) tc-nn-toolkit 설치: topst-downloads .../Education/Motrex/tc-nn-toolkit.zip
#   unzip → chmod 755 * → python3.8 venv → requirements.txt
#   + torch==1.12.0+cpu/torchvision==0.13.0+cpu/torchaudio==0.12.0, netron
#   + 크로스 gcc gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu 를 PATH 에
cd ~/Work/tc-nn-toolkit && . venv/bin/activate

# 캘리브레이션 이미지 폴더 준비(./calib_signs): 데모 셋업으로 찍은 실제 표지판 프레임 수십 장 (640 리사이즈).
#   COCO 샘플 금지 — INT8 스케일이 표지판에 맞아야 함.

# STAGE 1: ONNX -> .enlight  (※ 스톡과 다른 곳: --num-class 80 → 4, --dataset-root 실제 폴더)
python ./EnlightSDK/converter.py ./input_networks/yolov8s_signs_extracted.onnx \
  --type obj --add-detection-post-process <yolov8-v8-reference.bin> \
  --dataset Custom --dataset-root ./calib_signs \
  --output ./output_networks/yolov8s_signs.enlight \
  --enable-track --mean 0 0 0 --std 1 1 1 \
  --num-class 4 --yolo-version v8 --enable-letterbox --dfl-reg-max 16 \
  --output-order cl --num-images 1

# STAGE 2: 양자화 (FP32 -> INT8)
python ./EnlightSDK/quantizer.py ./output_networks/yolov8s_signs.enlight \
  --output ./output_networks/yolov8s_signs_quantized.enlight

# STAGE 3: 컴파일 (-> output_code/yolov8s_signs_quantized/: quantized_network.bin, npu_cmd.bin, network.h, post_process.c, oe_network.*)
python ./EnlightSDK/compiler.py ./output_networks/yolov8s_signs_quantized.enlight \
  --th-iou 0.5 --th-conf 0.5            # 작은 표지판이면 --th-conf 낮춰볼 것

# STAGE 4(권장): 시뮬레이터로 보드 전에 정확도 확인 — 검출 표의 cls 가 {0,1,2,3} 인지
python ./EnlightSDK/enlight_sim.py ./output_networks/yolov8s_signs_quantized.enlight \
  --inputs sample/sign.png --th-iou 0.5 --th-conf 0.5 --enable-letterbox
```
> **`--add-detection-post-process` 주의:** 이 폴더의 스톡 `yolov8s.bin`은 80-class COCO 산출물.
> 이 플래그가 .bin에서 클래스 수를 가져온다면 `--num-class 4`와 충돌한다. 툴킷 자체의 v8
> 레퍼런스 bin을 쓰거나, 플래그가 `--yolo-version v8 + --num-class 4`에서 헤드를 유도하는지 확인.
> (미해결 항목 — converter `--help`/`yolo_detector.c`로 확인)

## 4) net.so 빌드 — WSL

```bash
cd build_network
cp -r ../output_code/yolov8s_signs_quantized/ ./ -ar
rm -rf network.h post_process.c          # ★ 안 지우면 이전 모델의 디코드가 박힘(실패원인)
ln -s yolov8s_signs_quantized/network.h
ln -s yolov8s_signs_quantized/post_process.c
make                                      # -> net.so
cp net.so yolov8s_signs_quantized/
```

## 5) 배포 + 실행 — 보드

```bash
# 모델 "폴더 전체"를 보드로 (단일 .bin 만 복사하면 로드 실패 — 이전 실패원인)
scp -r yolov8s_signs_quantized root@192.168.0.100:/home/root/    # pw: root
#   PC NIC 는 192.168.0.8/24 고정

# (보드) CSI 카메라 + DSI 디스플레이 연결 후 부팅, 그 다음:
tcnnapp -n yolov8s_signs_quantized -p /dev/video2
```
배포 폴더 필수 구성: `quantized_network.bin` + `npu_cmd.bin` + `net.so` (+ network.h, oe_network.*, oact_entry_table.py). **클래스 수·임계값은 런타임 인자가 아니라 컴파일에 박혀 있음.** 보드에는 라벨/.names 파일 없음 — cls→이름 매핑은 소비자(D3-G)에서.

## 6) D3-G 연동 — 포맷 변환 shim 필요 ⚠️

이 프로젝트 커스텀 AI-G 빌드(`d3-g/reference` NnAppMain)는 **TCP 서버 `192.168.0.100:9999`**,
프레임당 JSON 한 줄을 송신:
```json
{"boxes":[{"cls":3,"score":0.92,"xmin":..,"ymin":..,"xmax":..,"ymax":..}]}
```
그런데 `d3-g/a72/ldar_decision.py`는 `{"ts":..,"sign":"speed_30","conf":0.92}`(문자열 sign)를 기대 →
**불일치.** shim에서 `cls(int)→sign(str)`(data.yaml 순서로 매핑), `score→conf`, top-box 선택을 해야 함.
+ score 스케일(0~1 vs 0~100) 라이브 출력으로 확인 후 `CONF_THRESH` 신뢰.
+ 보드의 tcnnapp가 이 커스텀(TCP) 빌드인지, 스톡(/dev/overlay, TCP 없음) 빌드인지 먼저 확인.

---

## 이전 시도가 실패한 지점 — 체크리스트
1. **ONNX cls 분기가 4가 아니라 80** (스톡 onnx 그대로 컴파일/COCO 헤드 export) → 가장 유력한 근본원인. `export_onnx.py [verify]`로 차단.
2. **converter `--num-class` 80 방치** (튜토리얼 예시 그대로).
3. **`--add-detection-post-process`에 80-class 스톡 .bin** → num-class 4 무력화 가능.
4. **단일 .bin만 복사** — `npu_cmd.bin`+`net.so` 누락 → 로드 실패.
5. **net.so 재빌드 누락**(rm+symlink+make 생략) → 이전 모델 디코드.
6. **캘리브레이션 셋 부적절**(placeholder/비표지판) → INT8가 표지판을 노이즈로 양자화.
7. **해상도 불일치**(학습/export/.bin 입력이 제각각) → 스케일러가 왜곡 프레임 공급.
8. **전선 포맷/score 스케일**(5·6번) → NPU가 맞아도 D3-G가 못 받음(계약 실패가 모델 실패로 보임).

## 파일
- `train.py` — yolov8s 4클래스 fine-tune
- `export_onnx.py` — 풀헤드 export → 6-출력 추출 + 4채널 검증
- `dataset/data.yaml` — 4클래스 정의(순서 고정)
- `yolov8s_extracted.onnx`, `yolov8s.bin` — **스톡(80-class) 참고용**. 커스텀 컴파일에 그대로 쓰지 말 것.
