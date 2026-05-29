/*
***************************************************************************************************
*
*   FileName : key.h
*
*   Copyright (c) Telechips Inc.
*
*   Description :
*
*
***************************************************************************************************
*
*   TCC Version 1.0
*
*   This source code contains confidential information of Telechips.
*
*   Any unauthorized use without a written permission of Telechips including not limited to
*   re-distribution in source or binary form is strictly prohibited.
*
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty of
*   merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
*   or other third party intellectual property right. No warranty is made, express or implied,
*   regarding the information's accuracy,completeness, or performance.
*
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between
*   Telechips and Company.
*   This source code is provided "AS IS" and nothing contained in this source code shall constitute
*   any express or implied warranty of any kind, including without limitation, any warranty
*   (of merchantability, fitness for a particular purpose or non-infringement of any patent,
*   copyright or other third party intellectual property right. No warranty is made, express or
*   implied, regarding the information's accuracy, completeness, or performance.
*   In no event shall Telechips be liable for any claim, damages or other liability arising from,
*   out of or in connection with this source code or the use in the source code.
*   This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
*   between Telechips and Company.
*
***************************************************************************************************
*/

#ifndef KEY_APP_H
#define KEY_APP_H

/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/


#define KEY_MANY_PUSHKEY                (0x0)
#define KEY_ONLY_ROTARY                 (0x1)

#define KEY_DEVICE_ST_NO_OPEN           (0)
#define KEY_DEVICE_ST_OPENED            (1)
#define KEY_DEVICE_ST_ENABLED           (2)
#define KEY_DEVICE_ST_DISABLED          (3)


#define KEY_APP_TASK_STK_SIZE           (256)

#define KEY_STAT_RELEASED               (0)
#define KEY_STAT_PRESSED                (1)

#define KEY_TIME_LIMIT                  (30)


/*
***************************************************************************************************
*                                         MODULE INTERFACES
***************************************************************************************************
*/
/*
***************************************************************************************************
*                                          KeyAppCreate
*
*
*
*
* Notes
*
***************************************************************************************************
*/

void KEY_AppCreate(void);

#endif /* _KEY_APP_H_ */

