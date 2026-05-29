/****************************************************************************
 *   FileName    : uart_test.h
 *   Description : Demo Application for console function
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall
constitute any express or implied warranty of any kind, including without limitation, any warranty
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright
or other third party intellectual property right. No warranty is made, express or implied,
regarding the information's accuracy, completeness, or performance.
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of
or in connection with this source code or the use in the source code.
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
between Telechips and Company.
*
****************************************************************************/

#ifndef  UART_TEST_HEADER
#define  UART_TEST_HEADER

#define UART_FALSE                      (0UL)
#define UART_TRUE                       (1UL)

#define DMA_BUFF_RECEIVE_LLI            (0x7000000UL) // SRAM1



//#define UART_PASSWORD_VALUE 0x5AFEACE5 // TCC805x
//#define UART_PASSWORD_VALUE (0x8030ACE5) // TCC803x

#define UART_BASE_ADDR		            (0x1B200000UL)
#define UDMA_BASE_ADDR		            (0x1B260000UL)
#define UART_PORTSEL0		            (UART_BASE_ADDR + 0xC0010)  //PORT_SEL0
#define UART_PORTSEL1		            (UART_BASE_ADDR + 0xC0014)  //PORT_SEL1

#define UART_GET_BASE(n)	            (UART_BASE_ADDR + (0x10000 * (n)))
#define UART_WriteReg(p, a, v)	        SAL_WriteReg(v, UART_GET_BASE(p) + (a))
#define UART_ReadReg(p, a)	            SAL_ReadReg(UART_GET_BASE(p) + (a))

#define UDMA_GET_BASE(n)	            (UDMA_BASE_ADDR + (0x10000 * (n)))
#define UDMA_WriteReg(p, a, v)	        SAL_WriteReg(v, UDMA_GET_BASE(p) + (a))
#define UDMA_ReadReg(p, a)	            SAL_ReadReg(UDMA_GET_BASE(p) + (a))


// UART Register Offset
#define OFFSET_DR		                (0x00UL)		// Data register
#define OFFSET_ECR                      (0x04)          // Error Clear Register
#define OFFSET_CR		                (0x30UL)		// Control register
#define OFFSET_IBRD		                (0x24UL)		// Integer Baud rate register
#define OFFSET_FBRD		                (0x28UL)		// Fractional Baud rate register
#define OFFSET_LCRH		                (0x2cUL)		// Line Control register

// UDMA Register Offset
#define OFFSET_SoftBReq                 (0x20)          // Software Burst Request Register
#define OFFSET_Configuration            (0x30)          // Configuration Register
#define OFFSET_CxSrcAddr                (0x100)         // Channel Source Address Registers
#define OFFSET_CxDestAddr               (0x104)         // Channel Destination Address Registers
#define OFFSET_CxLLI                    (0x108)         // Channel Linked List Item Registers
#define OFFSET_CxControl                (0x10C)         // Channel Control Registers
#define OFFSET_CxConfiguration          (0x110)         // Channel Configuration Registers

// UART Control Register Bitset
#define CR_RTSEn                        (1UL << 14UL)       // RTS hardware flow control enable
#define CR_RXE			                (1UL << 9UL)	// Receive enable
#define CR_TXE			                (1UL << 8UL)	// Transmit enable
#define CR_LBE			                (1UL << 7UL)	// Loopback enable
#define CR_UARTEN		                (1UL << 0UL)	// UART enable

// Peripheral SW Reset Register1 (SW_RESET1)
#define SW_RESET1                       (0x1B930010UL)


extern void UdmaLLITest
(
    int                                 ch
);
extern void UartDmaTxTest
(
    void
);
extern void UartDmaRxTest
(
    void
);
extern void UartInterruptTest
(
    void
);
extern void UartPollingTest
(
    void
);
extern void UartLoopBackTest
(
    void
);
extern void UartPortMuxTest
(
    unsigned int                        port
);


enum
{
    UT_CFG_WR_INIT = 0,
    UT_CFG_WR_PW,
    UT_CFG_WR_LOCK,
    UT_CFG_WR_PORTSEL,
    UT_CFG_WR_DONE
};

#endif

