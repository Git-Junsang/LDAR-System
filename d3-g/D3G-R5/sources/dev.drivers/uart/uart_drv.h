/*
***************************************************************************************************
*
*   FileName : uart_drv.h
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

#ifndef UART_DRV_HEADER
#define UART_DRV_HEADER

// UART Settings
#define UART_BUFF_SIZE                  (0x100UL)   // 256

#define UART_MODE_TX                    (0UL)
#define UART_MODE_RX                    (1UL)

#define UART_PORT_CFG_MAX               (47U)
#define UART_PORT_TBL_SIZE              (UART_PORT_CFG_MAX - 5UL)

// UDMA Address
#define UDMA_ADDRESS_OFFSET             (0UL)
#define UDMA_ADDRESS_UNIT               (UDMA_BUFF_SIZE + UDMA_BUFF_MARGIN)
#define UDMA_ADDRESS_UNIT_CH_RX(ch)     ((uint32)UDMA_ADDRESS_OFFSET + ((UDMA_ADDRESS_UNIT * 2UL) * (ch)))
#define UDMA_ADDRESS_UNIT_CH_TX(ch)     (((uint32)UDMA_ADDRESS_OFFSET + ((UDMA_ADDRESS_UNIT * 2UL) * (ch))) + UDMA_ADDRESS_UNIT)

// channel number : 0, 1, 2, 3
// baudrate : 9600, 14400, 19200, 38400, 57600, 115200, 230400, 460800, 921600 etc.
// flow control. Enable : 1, Disable : 0
// mode : UART_POLLING_MODE, UART_INTR_MODE, UART_DMA_MODE

#define TCC_GPNONE                      (0xFFFFUL)
#define TCC_GPSD0(x)                    (TCC_GPNONE)
#define TCC_GPSD1(x)                    (TCC_GPNONE)
#define TCC_GPSD2(x)                    (TCC_GPNONE)

// UART FIFO buffer size
#define UART_TX_FIFO_SIZE               (8UL)
#define UART_RX_FIFO_SIZE               (12UL)

// UART Base address
#define UART_GET_BASE(n)                (MCU_BSP_UART_BASE + (0x10000UL * (n)))
#define UARTPORTCFG                     (MCU_BSP_UART_BASE + 0xC0010UL)  //PORT_SEL0

// UART Port CFG Base address
#define SNOR_UARTCFG_VAL_ADDR(n)        (MCU_BSP_SNOR_BASE + 0x1100 + (0x4 * (n)))

// Raw Interrupt Status Register (RIS) Fields
// Masked Interrupt Status Register (MIS) Fields
// Interrupt Clear Register (ICR) Fields
#define UART_INT_OEIS                   (1UL << 10)   // Overrun error interrupt
#define UART_INT_BEIS                   (1UL << 9)    // Break error interrupt
#define UART_INT_PEIS                   (1UL << 8)    // Parity error interrupt
#define UART_INT_FEIS                   (1UL << 7)    // Framing error interrupt
#define UART_INT_RTIS                   (1UL << 6)    // Receive timeout interrupt
#define UART_INT_TXIS                   (1UL << 5)    // Transmit interrupt
#define UART_INT_RXIS                   (1UL << 4)    // Receive interrupt

// DMA Control Register (DMACR) Fields
#define UART_DMACR_DMAONERR             (1UL << 2)    // DMA on error
#define UART_DMACR_TXDMAE               (1UL << 1)    // Transmit DMA enable
#define UART_DMACR_RXDMAE               (1UL << 0)    // Receive DMA enable

// UART Interrupt mode
typedef struct UART_INTERRUPT_DATA
{
    uint8 *                             iXmitBuf;
    sint32                              iHead;
    sint32                              iTail;
    sint32                              iSize;
} UartInterruptData_t;

typedef struct UART_BOARD_PORT
{
    uint32                              bPortCfg;   // Config port ID
    uint32                              bPortTx;    // UT_TXD GPIO
    uint32                              bPortRx;    // UT_RXD GPIO
    uint32                              bPortRts;   // UT_RTS GPIO
    uint32                              bPortCts;   // UT_CTS GPIO
    uint32                              bPortFs;    // UART function select
} UartBoardPort_t;

typedef struct UART_STATUS
{
    boolean                             sIsProbed;
    uint32                              sBase;      // UART Controller base address
    uint8                               sCh;        // UART Channel
    uint8                               sOpMode;    // Operation Mode
    uint8                               sCtsRts;    // CTS and RTS
    uint8                               sWordLength;    // Word Length
    uint8                               s2StopBit;   // 1: two stop bits are transmitted
    uint8                               sParity;    // 0:disable, 1:enable
    UartBoardPort_t                     sPort;      // GPIO Port Infomation
    UDMAInformation_t                   sRxDma;     // Rx DMA
    UDMAInformation_t                   sTxDma;     // Tx DMA
    UartInterruptData_t                 sRxIntr;    // Rx Interrupt
    UartInterruptData_t                 sTxIntr;    // Tx Interrupt
} UartStatus_t;

#endif /* _PLATFORM_TCC_UART_H_ */

