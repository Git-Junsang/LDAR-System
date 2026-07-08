# app.ldar.vcp 오버레이 방법

이 폴더는 Telechips **FreeRTOS-VCP BSP**에 얹는 VCP-G LDAR 펌웨어 오버레이다. BSP는 리포에
없으니 빌드 시 `git clone https://github.com/topst-development/FreeRTOS-VCP topst-vcp` 후 아래대로
통합한다. 전체 빌드·플래시 절차는 [../README.md](../README.md).

## 1) 소스 배치
이 폴더를 BSP 트리로 복사:
```bash
cp -r vcp-g/app.ldar.vcp  <BSP>/sources/app.sample/app.ldar.vcp
```

## 2) BSP 통합 편집 (3곳)

### (a) `sources/app.sample/rules.mk` — 앱 등록 (파일 끝)
```make
# LDAR VCP-G (always on)
include $(MCU_BSP_APP_SAMPLE_PATH)/app.ldar.vcp/rules.mk
```
> `app.ldar.vcp/rules.mk`가 `-DMCU_BSP_SUPPORT_APP_LDAR_VCP=1`과 SRCS 목록을 스스로 정의한다.

### (b) `sources/app.sample/app.base/main.c` — 진입 훅
include 부:
```c
#if ( MCU_BSP_SUPPORT_APP_LDAR_VCP == 1 )
    #include "ldar_app.h"
#endif
```
태스크/메인 부(다른 앱 루프 대신):
```c
#if ( MCU_BSP_SUPPORT_APP_LDAR_VCP == 1 )
    LDAR_Run();  // never returns
#endif
```

### (c) 빌드 시 드라이버 플래그 — ADC · CAN 켜기
GPIO/GPSB/PDM는 BSP 기본이 이미 ON. LDAR가 쓰는 ADC(조이스틱)·CAN(오버라이드)만 추가:
```bash
cd <BSP>/build/tcc70xx/gcc
make MCU_BSP_BUILD_FLAGS_TEST_APP_ADC=1 MCU_BSP_BUILD_FLAGS_TEST_APP_CAN=1
#  → output/tcc70xx_pflash_boot_2M_ECC.rom
```

## 모듈 구성
`ldar_app`(메인 루프) · `joystick_adc`/`joystick_sw` · `motor_dir`/`motor_pwm` · `servo_pwm` ·
`turn_signal`/`turn_led`/`turn_can`(방향지시) · `override`/`override_can`(속도 오버라이드) ·
`buzzer` · `pwm_util`. 핀 정의 단일 출처 = `ldar_pins.h`.
