/*
***************************************************************************************************
*
*   FileName : console.h
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

#ifndef CONSOLE_H
#define CONSOLE_H

#define NUL_KEY                         (0x00UL)
#define BACK_SPACE                      (0x08UL)    // Back space
#define DEL_KEY                         (0x7FUL)    // Delete
#define ESC_KEY                         (0x1BUL)    // ESC
#define NAK_KEY                         (0x15UL)
#define SPACE_BAR                       (0x20UL)
#define LBRACKET                        (0x5BUL)
#define RBRACKET                        (0x5DUL)
#define UPPERCASE_A                     (0x41UL)    // ^ (up)
#define UPPERCASE_B                     (0x42UL)    // v (down)
#define UPPERCASE_C                     (0x43UL)    // > (right)
#define UPPERCASE_D                     (0x44UL)    // < (left)
#define ARRIAGE_RETURN                  (uint8)('\r')
#define LINE_FEED                       (uint8)('\n')  // Enter
#define CRLF                            ("\r\n")

#define CSL_TASK_STK_SIZE               (512UL)

#define CSL_CMD_BUFF_SIZE               (64UL)
#define CSL_CMD_PROMPT_SIZE             (2UL)
#if defined(AUTOMOTIVE_IVI_CLUSTER_DEMO_EN)
#define CSL_CMD_NUM_MAX                 (34UL)
#else
#define CSL_CMD_NUM_MAX                 (33UL)
#endif

#define CSL_ARGUMENT_MAX                (10UL)
#define CSL_SPACE_1COUNT                (1UL)
#define CSL_HISTORY_COUNT               (10UL)

#define CSL_LOG_NUM_MAX                 (3UL)

#define CMD_ENABLE                      (1UL)
#define CMD_DISABLE                     (0UL)

#ifndef ON
#define ON                              (1UL)
#define OFF                             (0UL)
#endif

typedef void                            (*CmdLogFunc) (void *pArgv[]);
typedef struct ConsoleLogFunc
{
    CmdLogFunc                          clFunc;
} ConsoleLogFunc_t;

enum
{
    CSL_NOINPUT,
    CSL_INPUTING,
    CSL_EXECUTE
};

typedef void (CmdFuncType)(uint8 argc, void *argv[]);

// Console Command List
typedef struct ConsoleCmdList
{
    uint32                              clEnable;
    const uint8                         *clName;
    CmdFuncType                         *clFunc;

} ConsoleCmdList_t;

// Console History Control
typedef struct CSLHistroyControl
{
    uint8                               hcIndex;
    uint8                               hcFlag;
    uint32                              hcSize;
    uint8                               hcBuf[CSL_CMD_BUFF_SIZE];

} CSLHistroyControl_t;

// Console History Manager
typedef struct CSLHistoryManager
{
    uint32                              hmPtr;
    CSLHistroyControl_t                 hmList[CSL_HISTORY_COUNT];

} CSLHistoryManager_t;

/*
***************************************************************************************************
*                                         EXTERN FUNCTION
***************************************************************************************************
*/

extern void CreateConsoleTask(void);

#endif

