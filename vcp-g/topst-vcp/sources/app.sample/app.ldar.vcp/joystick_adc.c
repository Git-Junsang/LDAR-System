// SPDX-License-Identifier: Apache-2.0

#include "joystick_adc.h"
#include "ldar_pins.h"

#include <adc.h>

/* 단순·안정 조이스틱 읽기 — 고정 중심 2048, 부팅 자동보정 없음.
 *  자동보정은 콜드부팅 과도값을 중심으로 잡아 정지 시 후진하는 문제가 있어 제거.
 *  - 채널당 ADC_Read 1회 (연속 더블리드 금지 — 축 커플링 원인이었음)
 *  - 데드존으로 정지 시 vrx=vry=0 보장 (스틱 중립 오프셋·미세 떨림 흡수)
 *  - INVERT: raw 작을수록 + (VRy 위=+=FWD, VRx 왼쪽=+=좌측 조향) */
#define ADC_CENTER    (2048)
#define NORM_RANGE    (127)
#define DEADZONE_RAW  (100)    /* 정지 시 0 보장. 정지에서 떨면 ↑, 둔하면 ↓ */

static int16_t Normalize(uint32_t raw)
{
    int32_t diff = (int32_t)ADC_CENTER - (int32_t)raw;   /* invert: raw<center → + */
    if ((diff > -DEADZONE_RAW) && (diff < DEADZONE_RAW)) {
        return 0;
    }
    int32_t v = (diff * NORM_RANGE) / ADC_CENTER;
    if (v >  NORM_RANGE) { v =  NORM_RANGE; }
    if (v < -NORM_RANGE) { v = -NORM_RANGE; }
    return (int16_t)v;
}

void JoystickAdc_Init(void)
{
    ADC_Init(LDAR_ADC_TYPE, LDAR_ADC_MODULE);
}

JoystickAxes_t JoystickAdc_Read(void)
{
    JoystickAxes_t a;
    a.raw_vrx = (uint16_t)(ADC_Read(LDAR_ADC_CH_VRX, LDAR_ADC_MODULE, 0) & 0xFFFU);
    a.raw_vry = (uint16_t)(ADC_Read(LDAR_ADC_CH_VRY, LDAR_ADC_MODULE, 0) & 0xFFFU);
    a.vrx = Normalize(a.raw_vrx);
    a.vry = Normalize(a.raw_vry);
    return a;
}
