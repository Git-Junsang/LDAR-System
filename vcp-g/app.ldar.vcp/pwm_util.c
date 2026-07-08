// SPDX-License-Identifier: Apache-2.0

#include "pwm_util.h"

#include <pdm.h>
#include <sal_api.h>
#include <debug.h>

void PwmUtil_Apply(uint32_t channel, uint32_t portCh, uint32_t periodNs, uint32_t dutyNs)
{
    PDMModeConfig_t cfg;
    uint32_t        wait = 0U;

    cfg.mcPortNumber      = portCh;
    cfg.mcOperationMode   = PDM_OUTPUT_MODE_PHASE_1;
    cfg.mcInversedSignal  = 0U;
    cfg.mcOutSignalInIdle = 0U;
    cfg.mcLoopCount       = 0U;
    cfg.mcOutputCtrl      = 0U;
    cfg.mcPeriodNanoSec1  = periodNs;
    cfg.mcDutyNanoSec1    = dutyNs;
    cfg.mcPeriodNanoSec2  = 0U;
    cfg.mcDutyNanoSec2    = 0U;

    (void)PDM_Disable(channel, PMM_ON);
    while (PDM_GetChannelStatus(channel) != 0U) {
        (void)SAL_TaskSleep(1);
        wait++;
        if (wait > 100U) {
            mcu_printf("\n[PWM] disable timeout ch=%d", (int)channel);
            return;
        }
    }

    if (PDM_SetConfig(channel, &cfg) != SAL_RET_SUCCESS) {
        mcu_printf("\n[PWM] SetConfig fail ch=%d", (int)channel);
        return;
    }
    if (PDM_Enable(channel, PMM_ON) != SAL_RET_SUCCESS) {
        mcu_printf("\n[PWM] Enable fail ch=%d", (int)channel);
    }
}
