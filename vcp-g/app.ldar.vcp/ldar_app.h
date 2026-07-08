// SPDX-License-Identifier: Apache-2.0

#ifndef LDAR_APP_H
#define LDAR_APP_H

/* Phase 1 cooperative loop: init GPIO modules, poll inputs, drive motor direction.
 * Called from Main_StartTask after AppTaskCreate(). Never returns. */
void LDAR_Run(void);

#endif
