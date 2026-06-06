# DEVELOPMENT — 개발 환경 · 빌드 · 플래시

> [README](../README.md)의 개발 환경 분리본. 작업물에 따라 빌드 위치가 다르다.

| 작업물 | 작성·빌드 | 플래시·콘솔 |
|---|---|---|
| **A72 판정 앱** (Python) | D3-G 보드 (Remote-SSH) | 동일 — 보드에서 `python3` 직접 실행 |
| **R5 펌웨어** | Windows WSL2 (텔레칩스 R5 빌드환경, D02-T01) | Windows WSL2 (보드 USB 직결) |
| **VCP-G 펌웨어** | **외부 Linux 서버(code-server)** — `vcp-g/topst-vcp/`에서 직접 작업 | **로컬 Windows + WSL2** (`usbipd` + `fwdn` + `minicom`) |

- **VCP-G는 빌드 머신과 플래시 머신이 분리** — code-server엔 보드 USB가 없으므로 `.rom` 산출물을 git으로 로컬에 전달 후 로컬 WSL2에서 플래시.
- **git**: GitHub origin(`Git-Junsang/LDAR-System`)을 각 환경에 clone. 네트워크 드라이브 공유 워킹트리 금지.
- **D3-G 보드**: IP `192.168.0.35`(DHCP), `/dev/tcc_ipc_micom` 존재. `/` 파티션 `sudo resize2fs /dev/mmcblk0p4`로 16G→28G 확장.
- **LDAR VCP-G 코드는 BSP 트리 안에 직접** — `vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/`. BSP(~76MB)는 커밋 안 함(`vcp-g/.gitkeep`만 추적), LDAR 파일·`.rom`은 `git add -f`로 명시 추가.

## VCP-G — 사전 준비 (최초 1회)

### 서버(code-server) — 빌드
```bash
cd <repo>/vcp-g
git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp      # 이미 있으면 skip
# Linaro 7.2.1 툴체인 (Makefile 기본 경로 /opt) — 다른 위치면 MCU_BSP_TOOLCHAIN_PATH=... make
cd /tmp && wget https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/arm-eabi/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz
sudo tar xf gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz -C /opt/
sudo apt install whiptail                                                  # easy-setup용 (또는 -e로 우회)
cd <repo>/vcp-g/topst-vcp && ./easy-setup_vcp-g.sh -e                       # 라이선스 동의 + 초기화
```
> 서버엔 `fwdn`/`minicom` 불필요.

### 로컬(Windows + WSL2) — 플래시
```bash
# WSL2
git clone https://github.com/Git-Junsang/LDAR-System ~/LDAR-System
cd ~/LDAR-System/vcp-g && git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp  # fwdn 바이너리용
sudo apt install minicom
sudo usermod -aG dialout $USER     # /dev/ttyUSB0 권한 (재로그인)
```
Windows 쪽: **CP210x 드라이버**(Silicon Labs) + **usbipd-win**(`winget install usbipd`).
보드 USB 연결 후 관리자 PowerShell:
```
usbipd list                          # busid 확인
usbipd bind   --busid <id>           # 최초 1회
usbipd attach --wsl --busid <id>     # 재연결마다
```

## VCP-G — 빌드 (서버)
```bash
cd vcp-g/topst-vcp/build/tcc70xx/gcc
make                       # → output/tcc70xx_pflash_boot_2M_ECC.rom
# 클린: make clean && make
```

## VCP-G — 산출물 git 전달 (서버 → 로컬)
```bash
git add -f vcp-g/topst-vcp/sources/app.sample/app.ldar.vcp/
git add -f vcp-g/topst-vcp/sources/app.sample/rules.mk
git add -f vcp-g/topst-vcp/sources/app.sample/app.base/main.c
# .rom 은 정식 추적 위치(vcp-g/flash/)로 복사 후 add — flash.sh가 이걸 굽는다
cp vcp-g/topst-vcp/build/tcc70xx/gcc/output/tcc70xx_pflash_boot_2M_ECC.rom vcp-g/flash/
git add vcp-g/flash/tcc70xx_pflash_boot_2M_ECC.rom
git commit -m "vcp-g: <요약>" && git push
```
로컬: `cd ~/LDAR-System && git pull`

## VCP-G — 플래시 (로컬 WSL2)
1. **FWDN 모드 진입** — 보드 FWDN 스위치 누른 채 12V/1A 전원 연결, USB-C 연결 → `usbipd attach`.
2. **fwdn** (sudo):
   ```bash
   cd ~/LDAR-System/vcp-g/topst-vcp
   sudo tools/fwdn_vcp/fwdn --fwdn tools/fwdn_vcp/vcp_fwdn.rom \
       -w build/tcc70xx/gcc/output/tcc70xx_pflash_boot_2M_ECC.rom
   ```
   (또는 `vcp-g/flash/flash.sh` 원-라이너)
3. 전원 분리 → FWDN 스위치 떼고 → 재전원 (Run 모드).

## VCP-G — 콘솔 (로컬 WSL2)
```bash
minicom -D /dev/ttyUSB0 -b 115200 -8     # 종료 Ctrl+A → Q
```
`/dev/ttyUSB0` 없으면 `usbipd attach` 여부 + `dmesg | tail` 확인.

## VCP-G — 모듈 추가 방법 (서버)
`app.ldar.vcp/` 안에서 PDF D02-T04~06 패턴(`xxx.h`+`xxx.c`)으로:
1. 모듈 파일 쌍 생성 (예: `buzzer.h/c`).
2. `app.ldar.vcp/rules.mk`에 `SRCS += xxx.c` 등록.
3. `ldar_app.c`의 `LDAR_Run()`에 `Xxx_Init()` + 루프 내 `Xxx_Step()` 추가.
4. 새 핀은 **`ldar_pins.h`에만** `#define LDAR_PIN_XXX GPIO_GPx(n)` 추가 (배선 변경 시 이 파일만).
5. 새 BSP 드라이버 카테고리는 `app.sample/rules.mk`의 `MCU_BSP_BUILD_FLAGS_TEST_APP_XXX` ON.
6. `main.c`는 보통 수정 불필요 — `MCU_BSP_SUPPORT_APP_LDAR_VCP` 가드로 이미 `LDAR_Run()` 호출.

## D3-G A72 판정 앱 (보드에서 직접)
```bash
cd d3-g/a72
python3 ldar_decision.py --source mock --dry-run   # AI-G 없이 상태머신 검증 (콘솔만)
python3 ldar_decision.py --source mock             # 실제 IPC 송신 (sudo 필요)
python3 ldar_decision.py --source tcp --port 9999  # 실제 AI-G 수신 (Phase 4)
```
상세: [a72/README.md](../d3-g/a72/README.md).

## CAN 디버깅
A72에 SocketCAN(`can0`) **없음** → `candump` 불가. CAN은 R5/VCP 전담 → **외장 USB-CAN 분석기** 또는 R5/VCP 콘솔 로그로 확인.
