// SPDX-License-Identifier: Apache-2.0

#include "joystick_adc.h"
#include "ldar_pins.h"

#include <adc.h>
#include <sal_api.h>
#include <debug.h>

/* 12-bit ADC: 0..4095. 기계적 중립은 조이스틱마다 달라 부팅 시 자동 캘리브레이션.
 * 한 축 조작 시 다른 축이 같이 움직이는 현상 완화 위해 데드존 크게.
 * VRy는 INVERT — raw 작음(위) = vry 양수 = FWD
 * VRx도 INVERT — raw 작음(왼쪽) = vrx 양수 = 좌측 조향 */
#define ADC_RAW_MAX      (4095)
#define ADC_DEADZONE_VRX (600)
#define ADC_DEADZONE_VRY (300)
#define NORM_RANGE       (127)

#define CAL_SAMPLES      (16)    /* 부팅 시 평균 낼 샘플 수 */
#define CAL_SAMPLE_MS    (5)     /* 샘플 간 sleep */

static int32_t s_centerX = 2048;
static int32_t s_centerY = 2048;

static uint32_t ReadRaw(uint8_t ch)
{
    return (uint32_t)ADC_Read(ch, LDAR_ADC_MODULE, 0) & 0xFFFU;
}

void JoystickAdc_Init(void)
{
    ADC_Init(LDAR_ADC_TYPE, LDAR_ADC_MODULE);

    /* 부팅 시 조이스틱이 손 떼진 상태(중립)임을 가정하고 중심값 측정. */
    int32_t sumX = 0;
    int32_t sumY = 0;
    for (uint32_t i = 0U; i < CAL_SAMPLES; i++) {
        sumX += (int32_t)ReadRaw(LDAR_ADC_CH_VRX);
        sumY += (int32_t)ReadRaw(LDAR_ADC_CH_VRY);
        (void)SAL_TaskSleep(CAL_SAMPLE_MS);
    }
    s_centerX = sumX / (int32_t)CAL_SAMPLES;
    s_centerY = sumY / (int32_t)CAL_SAMPLES;
    mcu_printf("\n[JOY] calibrated center: vrx=%d vry=%d (raw 0..4095)",
               (int)s_centerX, (int)s_centerY);
}

/* 비대칭 정규화: 양 측 풀스윙이 다른 경우(캘리 비대칭) 양 끝 ±NORM_RANGE 보장.
 * invert=1이면 마지막에 부호 뒤집어 raw 작음을 양수로 매핑. */
static int16_t Normalize(uint32_t raw, int32_t center, int32_t deadzone, int8_t invert)
{
    int32_t diff = (int32_t)raw - center;
    if ((diff > -deadzone) && (diff < deadzone)) {
        return 0;
    }
    int32_t span = (diff > 0) ? (ADC_RAW_MAX - center) : center;
    if (span <= 0) { span = 1; }
    int32_t scaled = (diff * NORM_RANGE) / span;
    if (scaled >  NORM_RANGE) { scaled =  NORM_RANGE; }
    if (scaled < -NORM_RANGE) { scaled = -NORM_RANGE; }
    if (invert) { scaled = -scaled; }
    return (int16_t)scaled;
}

JoystickAxes_t JoystickAdc_Read(void)
{
    JoystickAxes_t a;
    a.raw_vrx = (uint16_t)ReadRaw(LDAR_ADC_CH_VRX);
    a.raw_vry = (uint16_t)ReadRaw(LDAR_ADC_CH_VRY);
    a.vrx = Normalize((uint32_t)a.raw_vrx, s_centerX, ADC_DEADZONE_VRX, 1);
    a.vry = Normalize((uint32_t)a.raw_vry, s_centerY, ADC_DEADZONE_VRY, 1);
    return a;
}
