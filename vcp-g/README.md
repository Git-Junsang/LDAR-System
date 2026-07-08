# VCP-G — Control Zone (구동 · 중재)

조이스틱 수동 주행 루프를 로컬에서 완결하고, D3-G가 CAN `0x110`으로 내리는 **속도 오버라이드**를
받아 **부드럽게 감속/정지**시킨다. **조향은 오버라이드하지 않는다 — 항상 조이스틱.**

```
조이스틱(VRx/VRy/SW) ─ADC/GPIO─▶ VCP-G ─▶ DC모터(L298N) · 서보 · LED · 부저
CAN 0x110 (D3-G R5) ─────────▶ VCP-G ─▶ 적용 상한 슬루 → duty=min(조이스틱, 상한)
```

- `duty = min(조이스틱 요청, 오버라이드 상한)`. 상한은 목표값까지 틱마다 **슬루-레이트**로 이동
  (감속 ≈100%/s, 90%→0% 약 0.9s). STOP은 0% 도달 후 브레이크.
- 상향: 방향지시 버튼 → 녹색 LED 토글 + CAN `0x120` 송신.

---

## 리포 구성 — BSP는 포함하지 않는다

VCP-G 펌웨어는 Telechips **FreeRTOS-VCP BSP**(~100MB, 재배포 대상 아님) 위에서 빌드된다.
리포에는 **우리가 작성한 오버레이 소스만** 둔다.

| 경로 | 내용 |
|---|---|
| [app.ldar.vcp/](app.ldar.vcp/) | VCP-G LDAR 펌웨어 소스 — BSP `sources/app.sample/app.ldar.vcp/`로 얹는다 |
| [app.ldar.vcp/OVERLAY.md](app.ldar.vcp/OVERLAY.md) | BSP에 오버레이하는 방법(통합 지점 3곳) |
| [flash/](flash/) | 플래시 패키지 — `fwdn` + 빌드된 `.rom` + `flash.sh` |

`app.ldar.vcp/` 모듈 (D02-T04~06 패턴, `xxx.h`+`xxx.c`):
`ldar_app`(메인 루프), `joystick_adc`/`joystick_sw`, `motor_dir`/`motor_pwm`, `servo_pwm`,
`turn_signal`/`turn_led`/`turn_can`(방향지시), `override`/`override_can`(속도 오버라이드),
`buzzer`, `pwm_util`. 핀 정의 단일 출처 = `ldar_pins.h`.

---

## 빌드 (외부 Linux 서버 / code-server)

> VCP-G는 **빌드 머신과 플래시 머신이 분리**된다. 여기(서버)선 `.rom`까지만 만들고,
> 플래시·콘솔은 로컬 Windows+WSL2에서 한다.

### 최초 1회 — BSP 클론 + 오버레이

```bash
cd vcp-g
# 1) BSP 클론 (리포에 없음 — 여기서 받는다)
git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp

# 2) Linaro 7.2.1 툴체인 (Makefile 기본 경로 /opt)
cd /tmp && wget https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/arm-eabi/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz
sudo tar xf gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz -C /opt/
sudo apt install whiptail                         # easy-setup용 (또는 -e로 우회)

# 3) BSP 라이선스 동의·초기화
cd topst-vcp && ./easy-setup_vcp-g.sh -e

# 4) 우리 오버레이 소스 얹기
cp -r ../app.ldar.vcp sources/app.sample/app.ldar.vcp
#    + BSP 통합 편집 3곳 (app.ldar.vcp/OVERLAY.md 참조)
```

### 컴파일

```bash
cd topst-vcp/build/tcc70xx/gcc
make MCU_BSP_BUILD_FLAGS_TEST_APP_ADC=1 MCU_BSP_BUILD_FLAGS_TEST_APP_CAN=1
#   → output/tcc70xx_pflash_boot_2M_ECC.rom
#   (PDM/GPIO/GPSB 드라이버는 BSP 기본이 이미 1. ADC·CAN만 켜면 됨)
# 클린: make clean && make ...

# 산출물을 플래시 패키지로 복사 (git 추적 위치)
cp output/tcc70xx_pflash_boot_2M_ECC.rom ../../../../flash/
```

---

## 플래시 · 콘솔 (로컬 Windows + WSL2)

code-server엔 보드 USB가 없으므로, 위에서 만든 `flash/tcc70xx_pflash_boot_2M_ECC.rom`을
git으로 로컬에 전달한 뒤 로컬 WSL2에서 굽는다.

### 준비 (최초 1회)
- Windows: **CP210x 드라이버**(Silicon Labs) + **usbipd-win** (`winget install usbipd`)
- WSL2: `sudo apt install minicom` · `sudo usermod -aG dialout $USER` (재로그인)

### 플래시
1. **FWDN 모드 진입** — 보드 FWDN 스위치를 누른 채 12V/1A 전원 연결, USB-C 연결.
2. 관리자 PowerShell에서 WSL로 USB 넘기기:
   ```
   usbipd list
   usbipd bind   --busid <id>      # 최초 1회
   usbipd attach --wsl --busid <id>
   ```
3. WSL2에서 flash 패키지로 굽기:
   ```bash
   cd vcp-g/flash && ./flash.sh
   #  (내부: fwdn --fwdn vcp_fwdn.rom -w tcc70xx_pflash_boot_2M_ECC.rom)
   ```
4. 전원 분리 → FWDN 스위치 떼고 → 재전원 (Run 모드).

### 콘솔
```bash
minicom -D /dev/ttyUSB0 -b 115200 -8      # 종료 Ctrl+A → Q
```
`/dev/ttyUSB0`이 없으면 `usbipd attach` 여부 + `dmesg | tail` 확인.

---

## 핀맵 (단일 출처 = `app.ldar.vcp/ldar_pins.h`)

`[n]` = 디지털 핀 번호(VCP-G Docs Port Name). 배선이 바뀌면 `ldar_pins.h`만 고친다.

**입력**

| 부품 | 핀 | GPIO/신호 | 헤더 | 설정 |
|---|---|---|---|---|
| 조이스틱 VRx(조향) | A0 | ADC03 | J8D101 #1 | ADC, 중심 2048 고정 |
| 조이스틱 VRy(속도) | A1 | ADC04 | J8D101 #2 | ADC |
| 조이스틱 SW | [7] | GPB01 | J8D102 #1 | **active-low/풀업** → 부저 |
| 턴 버튼 L | [48] | GPA05 | J18D100 | **active-low/풀업** |
| 턴 버튼 R | [49] | GPA04 | J18D100 | **active-low/풀업** |

**출력 — 구동**

| 부품 | 핀 | GPIO | 헤더 | 비고 |
|---|---|---|---|---|
| 모터 IN1 | [5] | GPB10 | J8D102 #3 | L298N 방향 |
| 모터 IN2 | [3] | GPB11 | J8D102 #5 | L298N 방향 |
| 모터 EN(PWM) | [45] | GPA10 | J18D100 #26 | PDM CH0, 듀티 0~90%(오버라이드 시 상한↓) |
| 서보(PWM) | [44] | GPA11 | J18D100 #25 | PDM CH1, 1.0~2.0ms — **조이스틱 전용, 오버라이드 없음** |

**출력 — 표시**

| 부품 | 핀 | GPIO | 비고 |
|---|---|---|---|
| LED 좌/우 녹색 (방향지시) | [42]/[41] | GPA17/GPA18 | 버튼 토글, 1Hz 깜박 |
| LED 좌/우 적색 (오버라이드) | [43]/[40] | GPA16/GPK11 | LIMIT=1Hz 깜박 / STOP=점등 |
| 피에조 부저 | [11] | GPC14 | active-high — SW 누름 또는 오버라이드 중 발음 |

**통신** — CAN0 TX `GPK08`(J5D100 #3), RX `GPK01`(#4)
**전원** — 3.3V→J8D100 #4(조이스틱·로직), 5V→서보 별도, 모터→별도 배터리(L298N), GND 공통

> ⚠️ **모든 버튼·SW는 active-low** (핀→버튼→GND, 내부 풀업). 풀다운/active-high면 영원히 안 눌림.

---

## CAN 0x110 수신 (Speed Override)

VCP-G가 표준ID RANGE 필터(0x101~0x200)→RXFIFO1로 받아 메인 루프에서
`CAN_CheckNewRxMessage`/`CAN_GetNewRxMessage`로 드레인한다 (`override_can.c`).

- `[0]` mode: `0x00` RELEASE / `0x01` LIMIT / `0x02` STOP · `[1]` 한계속도(km/h, LIMIT만 유효)
- 수신 → `Override_SetLimit/SetStop/Release` → `override.c`가 적용 상한을 슬루 → 부드러운 감속.
- 적색 LED(LIMIT 깜박/STOP 점등) + 부저(오버라이드 중)로 상태 표시.

CAN 배선(2노드 버스): 두 트랜시버를 `CAN_H─CAN_H`, `CAN_L─CAN_L`, GND 공통, **종단저항 120Ω 양 끝**.
A72엔 `candump`가 없으니 확인은 VCP/R5 콘솔 로그나 외장 USB-CAN 분석기로.

---

## 모듈 추가 방법

`app.ldar.vcp/` 안에서 D02-T04~06 패턴(`xxx.h`+`xxx.c`)으로:

1. 모듈 파일 쌍 생성 (예: `buzzer.h/c`).
2. `app.ldar.vcp/rules.mk`에 `SRCS += xxx.c` 등록.
3. `ldar_app.c`의 `LDAR_Run()`에 `Xxx_Init()` + 루프 내 `Xxx_Step()` 추가.
4. 새 핀은 **`ldar_pins.h`에만** `#define LDAR_PIN_XXX GPIO_GPx(n)` 추가.
5. 새 BSP 드라이버 카테고리는 빌드 시 `MCU_BSP_BUILD_FLAGS_TEST_APP_XXX=1`.
