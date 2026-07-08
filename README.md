# LDAR-System

**English** | [한국어](README.ko.md)

---

**Traffic-sign speed-override SDV — a scaled Telechips 3-Zonal implementation**

---

## 1. Background

On a production car, a forward camera reads traffic signs and an Intelligent Speed Assist (ISA) function keeps the vehicle within the posted limit. LDAR-System reproduces that behavior on a small Telechips Zonal model car: while the driver steers manually with a joystick, a camera detects traffic signs (speed limit 30/60, stop, no-entry) and the system **smoothly forces the speed down or brings the car to a stop**.

- The full chain runs on the edge: **camera → NPU inference → decision → CAN control → actuation**, end-to-end.
- **Steering is always the driver's — the only intervention is speed.** The manual driving loop stays local and low-latency; the override only ever lowers the speed ceiling.
- Architecture is 3-Zonal: **AI-G** (perception) → **D3-G** (decision, A72 + R5) → **VCP-G** (drive & arbitration).

> The codename **`LDAR`** is a leftover from the original topic "Lane Departure Auto-Recovery." **Lane detection has been dropped**; the current, fixed topic is **traffic-sign speed override**.

---

## 2. System Overview

```
Joystick (VRx/VRy/SW) ─ADC/GPIO─▶ VCP-G ─────▶ DC motor · servo   (local, low-latency)
                                    └─CAN 0x120 (turn signal, upstream)─▶ D3-G R5 ─IPC─▶ A72
Camera ─MIPI CSI-2─▶ AI-G ─Ethernet TCP─▶ D3-G A72   (sign detection → speed decision)
                                            │ IPC
                                            ▼
                       D3-G R5 ─CAN 0x110─▶ Speed Override (mode + limit speed)
                                            ▼
                       VCP-G ──▶ smooth decel / stop up to the limit (joystick keeps steering)
```

| Zone    | Board                    | Role                                                                                            | Hardware                                        |
| ------- | ------------------------ | ----------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| Sensing | **AI-G**           | PiCam → NPU inference (YOLOv8) →**traffic-sign detection** (class + confidence) → TCP  | A53 Quad + Enlight NPU 8TOPS, MIPI CSI-2        |
| HPC     | **D3-G** (TCC8050) | sign →**speed-limit / stop decision** → CAN command                                     | A72 (Linux, decision) + R5 (FreeRTOS, IPC↔CAN) |
| Control | **VCP-G**          | joystick manual driving +**speed-override arbitration** + motor · servo · LED · buzzer | MCU + FreeRTOS, ADC/GPIO/PDM/I2C                |

**Core design** — the manual driving loop is completed locally on VCP-G (low latency). D3-G decides *only* sign → speed and issues **CAN 0x110 (Speed Override) — a speed ceiling / stop only**. VCP-G receives that command and slews its applied ceiling per tick with a slew-rate limit, so it decelerates/stops **smoothly, without jerk**. Below the ceiling the joystick passes through untouched, and **steering is the driver's in every case**.

---

## 3. Architecture

### 3.1 Perception Model — AI-G (YOLOv8)

**YOLOv8s traffic-sign detector**, 4 classes. The demo runs in a **fixed environment** (same lighting, PiCam, track, sign set), so the strategy is to deliberately overfit to that environment.

**Classes (order is frozen; `ai_model/dataset/data.yaml` is the single source of truth):**

```
0 Stop   /   1 No Entry   /   2 Speed_Limit_60   /   3 Speed_Limit_30
```

> This integer index *is* the `cls` value carried over the NPU → D3-G wire. **Never reorder it.**

| Item              | Spec                                                                                                               |
| ----------------- | ------------------------------------------------------------------------------------------------------------------ |
| Board (N-Dolphin) | A53 Quad +**Enlight NPU 8TOPS**, RAM **2GB** (don't over-size the input), Yocto Linux                  |
| Camera            | **OV5647** (RasPi Cam v1.3, MIPI CSI-2 15-pin) → V4L2 `/dev/video2`, UYVY 1288×956                       |
| Model             | YOLOv8s fine-tune, input**640×640 (letterbox)**, INT8-quantized for the NPU                                 |
| Toolchain         | Ultralytics (train) → ONNX 6-output extract → tc-nn-toolkit (Enlight convert / quantize / compile) →`tcnnapp` |
| Output            | TCP server`192.168.0.100:9999`, one JSON line per frame                                                          |

**TCP output format (AI-G → D3-G)** — one JSON line per frame:

```json
{"boxes":[{"cls":3,"score":0.92,"xmin":..,"ymin":..,"xmax":..,"ymax":..}]}
```

D3-G's `ldar_decision.py` expects `{"ts":.., "sign":"speed_30", "conf":0.92}` (string `sign`), so a **shim converts `cls(int) → sign(str)`** (by `data.yaml` order), `score → conf`, and picks the top box. See [d3-g/README.md](d3-g/README.md).

> ⚠️ **Training golden rule** — training data must be **100% identical to the demo setup** (same PiCam, mount, resolution, sign set, lighting). Fix the mount first and shoot only through that camera. No phone captures, no re-mounting mid-way. USB webcams are unsupported by the AI-G spec (MIPI PiCam only); a webcam is for PC rehearsal only.

### 3.2 Control & Communication Architecture

**Operation scenario (sign → speed)**

| Sign                 | D3-G decision | CAN 0x110`[mode, km/h]` | VCP-G behavior                                                   |
| -------------------- | ------------- | ------------------------- | ---------------------------------------------------------------- |
| Speed limit 30       | LIMIT 30      | `[0x01, 30]`            | slew duty ceiling smoothly down to 30%; joystick free below that |
| Speed limit 60       | LIMIT 60      | `[0x01, 60]`            | slew duty ceiling smoothly down to 60%                           |
| Stop / No entry      | STOP          | `[0x02, 0]`             | slew down to 0% then brake to a stop                             |
| (limit-zone cleared) | RELEASE       | `[0x00, 0]`             | ceiling released — joystick returns to full throttle            |
| (no sign)            | —            | (not sent)                | hold the previous command                                        |

> Limit speed (km/h) maps 1:1 to model-car duty % (30 → 30%, 60 → 60%). The local joystick full-throttle duty ceiling is 90% in firmware (`MOTOR_DUTY_CAP_PCT`).

**Override behavior** — `duty = min(joystick request, override ceiling)`. The applied ceiling moves toward its target per tick with a **slew-rate limit** (decel ≈ 100%/s, so 90% → 0% ≈ 0.9s). STOP brakes after reaching 0%. A red LED (LIMIT = 1Hz blink / STOP = solid) plus the buzzer indicate the override state.

**CAN message table** — 11-bit CAN ID, channel 0. Downstream (R5 → VCP) is the decision result; upstream (VCP → R5) is driver intent.

| Message                   | CAN ID                | Direction | Data                                       | Note                                    |
| ------------------------- | --------------------- | --------- | ------------------------------------------ | --------------------------------------- |
| **Speed Override**  | **0x110**       | R5 → VCP | `[0]` mode · `[1]` limit speed (km/h) | **the sign-speed command (core)** |
| **Driver Input**    | **0x120**       | VCP → R5 | `[0]` turn signal (0 off / 1 L / 2 R)    | upstream driver intent                  |
| Brake / Turn / Head Light | 0x101 / 0x102 / 0x104 | R5 → VCP | education-course legacy messages           | unused                                  |

- **0x110 mode**: `0x00` RELEASE (ceiling released) / `0x01` LIMIT (ceiling capped) / `0x02` STOP. `[1]` is valid only for LIMIT — VCP-G applies it 1:1 as a duty-% ceiling.

**Communication path summary**

| Segment            | Interface                    | Payload                                            |
| ------------------ | ---------------------------- | -------------------------------------------------- |
| Camera → AI-G     | MIPI CSI-2                   | RAW video (OV5647 → UYVY`/dev/video2`)          |
| AI-G → D3-G (A72) | Ethernet TCP                 | sign class, confidence, bounding box               |
| Joystick → VCP-G  | ADC (VRx/VRy) · GPIO (SW)   | steering · speed · button                        |
| VCP-G → D3-G (R5) | Classical CAN 2.0 (11-bit)   | turn-signal intent (upstream,**0x120**)      |
| D3-G A72 ↔ R5     | IPC (`/dev/tcc_ipc_micom`) | education IPC packet (SYNC·CMD·LEN·DATA·CRC16) |
| D3-G (R5) → VCP-G | Classical CAN 2.0 (11-bit)   | **Speed Override (0x110)** — mode + limit   |
| VCP-G → Actuators | GPIO · PDM (PWM) · I2C     | direction pins · PWM · LED · buzzer             |

**Module map**

| Group          | Modules                                                                                                                                                                                                                                       |
| -------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| VCP-G firmware | `ldar_app` (main loop), `joystick_adc` / `joystick_sw`, `motor_dir` / `motor_pwm`, `servo_pwm`, `turn_signal` / `turn_led` / `turn_can`, `override` / `override_can`, `buzzer`, `pwm_util`; pins in `ldar_pins.h` |
| D3-G A72       | `ldar_decision.py` (decision app), `ldar_can.py` (downstream 0x110 over IPC), `Library/IPC_Library.py` (CRC16 IPC transport)                                                                                                            |
| D3-G R5        | `ldar_bridge.c` (upstream 0x120 CAN→IPC), `ldar_downstream.c` (IPC→CAN 0x110), `shared/ldar_ipc_proto.h`                                                                                                                              |

> Detailed specs live in each board README: [pin map · CAN receive](vcp-g/README.md) · [decision · IPC](d3-g/README.md) · [TCP format](ai-g/README.md).

---

## 4. Directory Structure

```
LDAR-System/
├── ai-g/                     # Sensing Zone: sign detection · NPU · TCP send
│   ├── README.md             #   ★ AI-G setup (board · camera · training · deploy)
│   ├── data_pipeline/        #   training-data capture / frame extract (vcap / uyvy2img / capture)
│   ├── ai_model/             #   YOLOv8 train → ONNX → NPU convert (train.py / export_onnx.py / data.yaml)
│   └── ai-g app/             #   on-board NPU inference runtime (tcnnapp / motrex_app)
│
├── d3-g/                     # HPC Zone: decision · command
│   ├── README.md             #   ★ D3-G setup (A72 app · R5 overlay · IPC/CAN)
│   ├── a72/                  #   decision app (Python) — ldar_decision.py, ldar_can.py
│   ├── r5/                   #   R5 LDAR overlay module (CAN↔IPC bridge, downstream)
│   └── shared/               #   A72↔R5 shared IPC header (ldar_ipc_proto.h)
│
├── vcp-g/                    # Control Zone: drive · arbitration
│   ├── README.md             #   ★ VCP-G setup (BSP-overlay build · flash · pin map)
│   ├── app.ldar.vcp/         #   VCP-G LDAR firmware (our source, overlaid onto the BSP)
│   └── flash/                #   flash package (fwdn + .rom + flash.sh)
│
├── documents/                # presentation · report · tutorial PDFs · BSP-API specs (reference)
└── README.md                 # (this document) single source of the project definition
```

> **The BSP is not committed.** VCP-G / R5 firmware is built by overlaying our source onto the Telechips BSP (see each folder's "Build"). The repo keeps only the files we wrote. Data (`ai-g/data_pipeline/data/`, `ai-g/ai_model/dataset/` video/frames/labels) is git-ignored for size — code only.

---

## 5. Getting Started

### Requirements

| Environment   | Used for                                                                            |
| ------------- | ----------------------------------------------------------------------------------- |
| code-server   | VCP-G firmware build (BSP overlay →`.rom`), D3-G A72 Python decision-logic check |
| GPU PC        | YOLOv8 training (ultralytics), ONNX export                                          |
| WSL2 (Ubuntu) | tc-nn-toolkit (NPU convert/quantize/compile), R5 BSP build, VCP-G flash & console   |
| Boards        | AI-G (Ethernet 192.168.0.100), D3-G (A72 Linux + R5), VCP-G (MCU)                   |

### Quick Start — decision logic, no hardware

```bash
cd d3-g/a72
python3 ldar_decision.py --source mock --dry-run   # cycle sign scenarios, console only
```

### AI-G — data pipeline → training → NPU deploy

```bash
# 1) Capture on the board (static binary, zero deps) — UYVY raw per sign class
./vcap /home/root/data/speed_30 speed_30 60 5        # <outdir> <prefix> <count> <skip>
# 2) Convert on PC: UYVY raw → jpg (resize to 640)
python3 uyvy2img.py data/raw_uyvy --out data/frames --src-size 1288x956 --scale 640x640 --dedup 4.0
# 3) Label (bbox, 4 classes) → train on GPU PC
cd ai-g/ai_model
python3 train.py --epochs 120 --imgsz 640 --batch 16 --device 0
# 4) Export ONNX (6-output extract; [verify] must show cv3.*=4ch, not 80ch)
python3 export_onnx.py --weights runs/detect/signs_yolov8s/weights/best.pt
# 5) tc-nn-toolkit (WSL): converter → quantizer → compiler → net.so   (see ai-g/ai_model/README.md)
# 6) Deploy the whole model folder to the board, then:
tcnnapp -n yolov8s_signs_quantized -p /dev/video2
```

Full walkthrough: [ai-g/README.md](ai-g/README.md) · [ai-g/data_pipeline/README.md](ai-g/data_pipeline/README.md) · [ai-g/ai_model/README.md](ai-g/ai_model/README.md).

### D3-G — decision app & downstream bridge

```bash
cd d3-g/a72
python3 ldar_decision.py --source mock --dry-run     # mapping check, console only
python3 ldar_decision.py --source mock               # real IPC send (sudo required)
python3 ldar_decision.py --source tcp --port 9999    # receive real AI-G over TCP
```

`/dev/tcc_ipc_micom` access needs root. The R5 downstream bridge (IPC → CAN 0x110) is built in WSL2 on the Telechips R5 BSP — overlay `r5/sources/app.ldar.bridge/`, call `LdarDownstream_Init()`, and replace the IPC-recv placeholder with the real BSP API. See [d3-g/README.md](d3-g/README.md).

### VCP-G — BSP-overlay build, flash, console

```bash
# --- build on code-server ---
cd vcp-g
git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp    # BSP (not in repo)
cd topst-vcp && ./easy-setup_vcp-g.sh -e
cp -r ../app.ldar.vcp sources/app.sample/app.ldar.vcp                    # + 3 integration edits (OVERLAY.md)
cd build/tcc70xx/gcc
make MCU_BSP_BUILD_FLAGS_TEST_APP_ADC=1 MCU_BSP_BUILD_FLAGS_TEST_APP_CAN=1
#   → output/tcc70xx_pflash_boot_2M_ECC.rom
cp output/tcc70xx_pflash_boot_2M_ECC.rom ../../../../flash/

# --- flash & console on local Windows + WSL2 ---
cd vcp-g/flash && ./flash.sh          # fwdn --fwdn vcp_fwdn.rom -w tcc70xx_pflash_boot_2M_ECC.rom
minicom -D /dev/ttyUSB0 -b 115200 -8  # console (exit: Ctrl+A → Q)
```

Toolchain `/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi`. The pin map single source is [vcp-g/app.ldar.vcp/ldar_pins.h](vcp-g/app.ldar.vcp/ldar_pins.h) — change wiring there only. Full procedure and overlay points: [vcp-g/README.md](vcp-g/README.md) + [vcp-g/app.ldar.vcp/OVERLAY.md](vcp-g/app.ldar.vcp/OVERLAY.md).

---

## 6. Development Status

**Current state — manual driving and speed override verified on VCP-G; perception and integration in progress.** VCP-G manual driving (joystick · motor · servo · turn signal · buzzer) runs on real firmware, and speed override (CAN 0x110 → smooth decel/stop) is compile/ROM/flash-verified. The D3-G decision app maps sign → speed with a confidence threshold and CONFIRM debounce (Mock-verified), and the downstream CAN path (`ldar_can.py` + R5 `ldar_downstream.c`) is host-interop-verified. Remaining work is AI-G perception (YOLOv8 on the NPU), the R5 IPC → CAN 0x110 bridge on real hardware, and the AI-G → D3-G TCP link.

### Done

- [X] **Environment / basics** — Yocto/D3-G, VCP-G peripherals, CAN/IPC, Control Zone, AI-G + NPU basics
- [X] **VCP-G manual driving** — joystick ADC · DC motor (L298N) · servo · turn-signal toggle · buzzer (firmware runs)
- [X] **D3-G decision app** — sign → speed mapping (`SIGN_TO_CMD`) + confidence threshold + CONFIRM debounce (Mock-verified)
- [X] **VCP-G speed override** — CAN 0x110 receive → applied-ceiling slew (smooth decel/stop) (compile · ROM · flash-verified)
- [X] **D3-G downstream CAN** — `ldar_can.py` (limit/stop/release) + R5 `ldar_downstream.c` (host-interop-verified)

### In progress

- [ ] **Perception (AI-G)** — YOLOv8 sign detection training (4 classes) · ONNX → NPU convert · on-board deploy
- [ ] **R5 downstream bridge** — A72 IPC → CAN 0x110 send (WSL2 R5 build + real BSP IPC-recv API swap)
- [ ] **AI-G → D3-G TCP link** — sign-packet shim (`cls → sign`) + Mock → real swap

### Integration · demo

- [ ] Full flow camera → NPU → decision → CAN → actuation (E2E)
- [ ] Slew-rate / confidence-threshold tuning, edge cases (misdetection, restart after stop)
- [ ] Five demo scenarios (normal / limit 30 / limit 60 / stop / release) pass stably
- [ ] (optional) Qt cluster app — visualize speed ceiling & override state
- [ ] Final demo video + presentation deck

---

## 7. Extras

### Artifacts

| Path                                                      | Contents                                                                                            |
| --------------------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| `ai-g/ai_model/yolov8s.bin`, `yolov8s_extracted.onnx` | **stock (80-class COCO) reference only** — do not compile as-is for the 4-class custom model |
| `vcp-g/flash/tcc70xx_pflash_boot_2M_ECC.rom`            | built VCP-G firmware image (flash package)                                                          |

### Documents

| Path                          | Contents                                                                                                            |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| `documents/tutorials/`      | Telechips fabless-education course (D01~D10), incl. Yocto/D3-G, VCP GPIO/ADC/PDM, CAN, AI-model (YOLO), SensingZone |
| `documents/d3g_references/` | TCC805x MCU BSP-API specification PDFs (ADC, CAN, GPIO, IPC, PDM, ...) + Getting Started / User Guide               |
| `documents/`                | Final report (docx), mid-term presentation (pdf)                                                                    |

### Hardware gotchas (things we got bitten by repeatedly)

- **All buttons / joystick SW are active-low** — pin → button → GND, internal pull-up, pressed = 0. Pull-down / active-high stays 0 forever.
- **Joystick VCC is 3.3V** — on 5V, neutral floats to raw ~3100 and one axis clips at ADC 3.3V.
- **The DC motor is open-loop PWM** — duty *is* the average voltage, so speed and torque move together. The speed ceiling is the override lever.
- **No SocketCAN on the A72** → `candump` unavailable. Observe CAN via an external USB-CAN analyzer or the R5/VCP console log.
- If the console dies but the DC motor still runs, suspect a **3.3V logic-rail brownout** (wiring/short).
- **AI-G golden rule** — fixed-demo overfit strategy → training data must equal the demo setup 100% (same PiCam, mount, resolution, sign set, lighting).

### Data note

`ai-g/data_pipeline/data/` and `ai-g/ai_model/dataset/` (video / frames / labels) are git-ignored for size — code only. The current `dataset/` is built from public sets (GTSRB/Roboflow, European signs); re-shoot with the demo setup for reliable recognition.

---

## 8. References

- [Telechips TOPST](https://topst.ai/) — TCC8050 (D3-G) / TCC70xx (VCP-G) Zonal platform & fabless-education course
- [FreeRTOS-VCP BSP](https://github.com/topst-development/FreeRTOS-VCP) — VCP-G firmware base (overlaid, not committed)
- [Ultralytics YOLOv8](https://github.com/ultralytics/ultralytics) — traffic-sign detector base architecture
- [GTSRB](https://benchmark.ini.rub.de/gtsrb_news.html) — German Traffic Sign Recognition Benchmark (public-set reference)
