// SPDX-License-Identifier: Apache-2.0

#include "override.h"

/* 상한 슬루(부드러운 감속/복구) — 메인 루프 20ms 틱 기준.
 *   DOWN 2%/틱 ≈ 100%/s → 90%→30% 약 0.6s, 90%→0%(정지) 약 0.9s
 *   UP   4%/틱           → 해제 시 상한 복구는 조금 더 빠르게 */
#define OVR_CAP_FULL        (100U)  /* 상한 없음 = 풀스케일 */
#define OVR_CAP_SLEW_DOWN   (2U)    /* 상한 낮출 때 %/틱 — 부드러운 강제 감속 */
#define OVR_CAP_SLEW_UP     (4U)    /* 상한 올릴 때 %/틱 — 오버라이드 해제 */

static OverrideMode_t s_mode;
static uint8_t        s_targetCap;    /* 명령된 목표 상한 */
static uint8_t        s_appliedCap;   /* 슬루되어 실제 적용 중인 상한 */

void Override_Init(void)
{
    s_mode       = OVR_MODE_NONE;
    s_targetCap  = OVR_CAP_FULL;
    s_appliedCap = OVR_CAP_FULL;
}

void Override_SetLimit(uint8_t capPct)
{
    if (capPct > OVR_CAP_FULL) {
        capPct = OVR_CAP_FULL;
    }
    s_mode      = OVR_MODE_LIMIT;
    s_targetCap = capPct;
}

void Override_SetStop(void)
{
    s_mode      = OVR_MODE_STOP;
    s_targetCap = 0U;
}

void Override_Release(void)
{
    s_mode      = OVR_MODE_NONE;
    s_targetCap = OVR_CAP_FULL;
}

OverrideMode_t Override_GetMode(void)
{
    return s_mode;
}

uint8_t Override_GetCap(void)
{
    return s_appliedCap;
}

/* 적용 상한을 목표 상한으로 한 틱만큼 이동(슬루-레이트 제한). */
static void Override_SlewCap(void)
{
    if (s_appliedCap < s_targetCap) {
        uint8_t room = (uint8_t)(s_targetCap - s_appliedCap);
        s_appliedCap = (uint8_t)(s_appliedCap +
                       ((room < OVR_CAP_SLEW_UP) ? room : OVR_CAP_SLEW_UP));
    } else if (s_appliedCap > s_targetCap) {
        uint8_t room = (uint8_t)(s_appliedCap - s_targetCap);
        s_appliedCap = (uint8_t)(s_appliedCap -
                       ((room < OVR_CAP_SLEW_DOWN) ? room : OVR_CAP_SLEW_DOWN));
    } else {
        /* 이미 목표 도달 — 유지 */
    }
}

uint8_t Override_ApplyDuty(uint8_t requestedDuty)
{
    Override_SlewCap();
    return (requestedDuty < s_appliedCap) ? requestedDuty : s_appliedCap;
}
