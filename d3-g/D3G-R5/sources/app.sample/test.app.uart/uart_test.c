/****************************************************************************
 *   FileName    : uart_test.c
 *   Description :
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

#include "string.h"
#include "sal_com.h"
#include <app_cfg.h>

#include <debug.h>
#include <reg_phys.h>
#include <uart.h>
#include <uart_test.h>
#include <bsp.h>

#define MAX_BUFFER_SIZE (32UL)
static uint8 gTxBuf[MAX_BUFFER_SIZE];
static uint8 gRxBuf[MAX_BUFFER_SIZE];


static void delay1us
(
    uint32                              n
)
{
    uint32  i;
    uint32  val;

    val = n * 500UL;
    for (i = 0UL; i < val; i++)
    {
         BSP_NOP_DELAY();
    }
}

// IP Test
char dma_buff_send_lli[5]={'A', 'B', 'C', 'D', 'E'};
unsigned char *dma_buff_receive_lli = (unsigned char *)DMA_BUFF_RECEIVE_LLI; // char type should be used for writeb/readb

struct lli {
	unsigned int                        src;
	unsigned int                        *det;
	unsigned int                        next_lli;
	unsigned int                        con;
};

struct lli lli_test[4] = {
    {
        (unsigned int)&dma_buff_send_lli[1],
        (unsigned int *)(DMA_BUFF_RECEIVE_LLI+1),
        0UL,
        0x90012001UL
    },
    {
        (unsigned int)&dma_buff_send_lli[2],
        (unsigned int *)DMA_BUFF_RECEIVE_LLI+2,
		0UL,
		0x90012001UL
    },
    {
        (unsigned int)&dma_buff_send_lli[3],
        (unsigned int *)DMA_BUFF_RECEIVE_LLI+3,
		0UL,
		0x90012001UL
    },
    {
        (unsigned int)&dma_buff_send_lli[4],
        (unsigned int *)DMA_BUFF_RECEIVE_LLI+4,
		0UL,
		0x90012001UL
    }
};

// UDMA Linked List Item Test
void UdmaLLITest
(
    int                                 ch
)
{
    uint32  i;
//    char result_lli[5] = {0,};

    lli_test[0].next_lli = (unsigned int)&lli_test[1];
    lli_test[1].next_lli = (unsigned int)&lli_test[2];
    lli_test[2].next_lli = (unsigned int)&lli_test[3];

    for(i = 0 ; i < 5 ; i++)
    {
        SAL_WriteReg(0x0, dma_buff_receive_lli+i);
    }

	// udma enable
    UDMA_WriteReg(ch, OFFSET_Configuration, 0x1);

    UDMA_WriteReg(ch, OFFSET_CxSrcAddr, (unsigned int)&dma_buff_send_lli);
    UDMA_WriteReg(ch, OFFSET_CxDestAddr, (unsigned int*)dma_buff_receive_lli);
    UDMA_WriteReg(ch, OFFSET_CxLLI, (unsigned int)lli_test);

    // protect(Privileged mode), destination increment, destination burst(8), source burst(8), transfer size(1)
    UDMA_WriteReg(ch, OFFSET_CxControl, 0x9C012001);

    // enable (1), src peripheral(0x3), flow control(Memory-to-memory, 0x0), IE(1), ITC(1)
    UDMA_WriteReg(ch, OFFSET_CxConfiguration, 0xc007);

    // udma burst request for ch2
    UDMA_WriteReg(ch, OFFSET_SoftBReq, 0x8);

    delay1us(4000);

//    for(i = 0 ; i < 5 ; i++)
//    {
//        result_lli[i] = SAL_ReadReg(dma_buff_receive_lli+i);
//    }

    // udma disable
    UDMA_WriteReg(ch, OFFSET_Configuration, 0x0);
}

static uint32 Uart_Reg_Read
(
    uint32                              uiCh,
    uint32                              uiAddr
)
{
    uint32  uiRet;
    uint32  uiRegAddr;

    uiRegAddr = UART_BASE_ADDR + (0x10000UL * (uiCh)) + uiAddr;
    uiRet = SAL_ReadReg(uiRegAddr);
    return uiRet;
}


static void Uart_Reg_Write
(
    uint32                              uiCh,
    uint32                              uiAddr,
    uint32                              uiSetValue
)
{
    uint32  uiRegAddr;

    uiRegAddr = UART_BASE_ADDR + (0x10000UL * (uiCh)) + uiAddr;
    SAL_WriteReg(uiSetValue, uiRegAddr);
    return;
}


// Test

void UartDmaTxTest
(
    void
)
{
    uint32  i;
    uint8   ch;
    uint8   portSel;

    ch = UART_CH0;
    portSel = 46U;

    (void)UART_Open(ch, 10UL, 115200UL, UART_DMA_MODE, UART_CTSRTS_OFF, portSel,
                    WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, (GICIsrFunc)&UART_ISR);

    (void)SAL_MemSet(gTxBuf, 0, MAX_BUFFER_SIZE);

    // Write temporary data
    for(i = 0UL; i < MAX_BUFFER_SIZE; i++)
    {
        gTxBuf[i] = (uint8)'A' + (uint8)(i & 0xFFUL);
    }

    (void)UART_Write(ch, gTxBuf, MAX_BUFFER_SIZE);
}

void UartDmaRxTest
(
    void
)
{
    uint8   ch;
    uint8   portSel;

    ch = UART_CH0;
    portSel = 46U;

    (void)UART_Close(ch);
    (void)UART_Open(ch, 10UL, 115200UL, UART_DMA_MODE, UART_CTSRTS_OFF, portSel,
                    WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, (GICIsrFunc)&UART_ISR);

    (void)SAL_MemSet(gRxBuf, 0, MAX_BUFFER_SIZE);

    /* STOP */
    /* Type the text in the console to test UDMA Rx */

    (void)UART_Read(ch, gRxBuf, MAX_BUFFER_SIZE);
}

void UartInterruptTest
(
    void
)
{
    sint32  size;
    sint32  i;
    uint8   ch;
    uint8   portSel;

    size = 0;
    ch = UART_CH0;
    portSel = 46U;

    (void)UART_Open(ch, 10UL, 115200UL, UART_INTR_MODE, UART_CTSRTS_OFF, portSel,
                    WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, (GICIsrFunc)&UART_ISR);

    while(1)
    {
/**** UART Interrupt Rx Test ****/

        size = UART_Read(ch, gRxBuf, MAX_BUFFER_SIZE);

        if(size == 0)
        {
            continue;
        }
        else if (size < 0)
        {
            break;
        }
        else
        {
            for(i = 0; i < size; i ++)
            {
                gTxBuf[i] = gRxBuf[i];
            }

            if(gTxBuf[0] == (uint8)'q') //input 'Q' to exit
            {
                UART_Close(ch);
                break;
            }

/**** UART Interrupt Tx Test ****/

            (void)UART_Write(ch, gTxBuf, (uint32)size);

            (void)SAL_MemSet(gTxBuf, 0, MAX_BUFFER_SIZE);
            (void)SAL_MemSet(gRxBuf, 0, MAX_BUFFER_SIZE);

            delay1us(1000);
        }
    }
}


void UartPollingTest
(
    void
)
{
    //uint32 ch = 0;
    sint32  data;
    sint8   getc_err;
    uint32  val;
    uint8   ch;
    uint8   portSel;

    data        = 0;
    getc_err    = 0;
    val         = 0UL;
    ch          = UART_CH0;
    portSel     = 46U;

    (void)UART_Open(ch, GIC_PRIORITY_NO_MEAN, 115200UL, UART_POLLING_MODE, UART_CTSRTS_OFF, portSel,
                    WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, NULL_PTR);

    while(1)
    {
        data = UART_GetChar(ch, (sint32)0, (sint8 *)&getc_err);
        if(data >= 0)
        {
            val = (uint32)data;
            (void)UART_PutChar(ch, (uint8)(val & 0xFFUL));
        }

        if(data == (sint32)'q') //input 'Q' to exit
        {
            UART_Close(ch);
            break;
        }

        delay1us(1000);
    }
#if 0 // fixed ch value

    switch(ch)
    {
        case 0:
            while(1)
            {
                data = UART_GetChar(UART_DEBUG_CH, 0, (sint8 *)&getc_err);
                UART_PutChar(UART_DEBUG_CH, data);

                if(data == 'q') //input 'Q' to exit
                {
                    break;
                }

                delay1us(1000);
            }
            break;

        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            while(1)
            {
                data = UART_GetChar(UART_DEBUG_CH, 0, (sint8 *)&getc_err);
                UART_PutChar(UART_DEBUG_CH, data);

                if(gTxBuf[0] == 'q') //input 'Q' to exit
                {
                    break;
                }
                 delay1us(1000);
            }
            break;

        default:
            break;
    }
#endif
}



// UART Loopback sequence
// Uart baudrate -> UARTEN -> write Tx data -> UART RXE -> UART TXE
void UartLoopBackTest
(
    void
)
{
    uint32  ch;
    uint32  i;
    uint32  max_ch;
    uint32  max_data;
    uint32  wdata;
    uint32  rdata;
    uint32  cr_data;

    max_ch   = 4UL;
    max_data = 30UL;
    wdata    = 0;
    rdata    = 0;
    cr_data  = 0;

    for (ch = 0UL; ch < max_ch; ch++)
    {
        // Set baudrate 115200
        Uart_Reg_Write(ch, OFFSET_IBRD, 0x1AUL);
        Uart_Reg_Write(ch, OFFSET_FBRD, 0x2UL);
        // Set word length is 8bits and Enables FIFOs
        Uart_Reg_Write(ch, OFFSET_LCRH, 0x70UL);

        // UART Enable
        cr_data = CR_UARTEN;
        Uart_Reg_Write(ch, OFFSET_CR, cr_data);

        // Loopback enable
        cr_data |= CR_LBE;
        Uart_Reg_Write(ch, OFFSET_CR, cr_data);

        // Write data
        for(i = 0; i < max_data; i++)
        {
            wdata = (uint32)'A' + (uint32)(i & 0xFFUL);

            // Write Data
            Uart_Reg_Write(ch, OFFSET_DR, wdata);

            // Receive Enable
            cr_data |= CR_RXE;
            Uart_Reg_Write(ch, OFFSET_CR, cr_data);

            // Transmit Enable
            cr_data |= CR_TXE;
            Uart_Reg_Write(ch, OFFSET_CR, cr_data);

            delay1us(1000);

            // Read Data
            rdata = Uart_Reg_Read(ch, OFFSET_DR);

            if (wdata != rdata)
            {
                break;
            }

            wdata = 0;
            rdata = 0;
        }
    }
}


/* TEST METHOD
    uint32 port = 1.2.3.4.5;
    while(1)
    {
        UartPortMuxTest(port);
    }
*/

void UartPortMuxTest
(
    unsigned int                        port
)
{
    uint32  ch = 0;
    uint32  max_ch = 5;
    uint8   wdata = 0;
    uint8   rdata=0;
    uint32  cr_data;
    int     ret;

    for (ch = 0; ch <= max_ch; ch++)
    {
        // Reset UART Software Reset
        SAL_WriteReg(0x3FFFFFFF, SW_RESET1);

        // Error Clear
        UART_WriteReg(ch, OFFSET_ECR, 0x0);

        // Initialize the UART Port Select
        SAL_WriteReg(0x3F3F3F3F, UART_PORTSEL0);
        SAL_WriteReg(0x3F3F, UART_PORTSEL1);

        // Initialize Control Register
        UART_WriteReg(ch, OFFSET_CR, 0x0);

        ret = UART_Open(ch, GIC_PRIORITY_NO_MEAN, 115200UL, UART_POLLING_MODE, UART_CTSRTS_OFF, port,
                        WORD_LEN_8, ENABLE_FIFO, TWO_STOP_BIT_OFF, PARITY_SPACE, (GICIsrFunc)&UART_ISR);

        if (ret >= 0)
        {
            // Write data
            wdata = 'A';

            // Disable RTSEn
            cr_data = UART_ReadReg(ch, OFFSET_CR);
            cr_data &= ~CR_RTSEn;
            UART_WriteReg(ch, OFFSET_CR, cr_data);

            // Write Data
            UART_WriteReg(ch, OFFSET_DR, wdata);

            delay1us(1000);

            // Read Data
            rdata = UART_ReadReg(ch, OFFSET_DR);

            if (wdata != rdata)
            {
                // Write Data
                UART_WriteReg(ch, OFFSET_DR, wdata);

                delay1us(1000);
                // enable RTSEn
                cr_data = UART_ReadReg(ch, OFFSET_CR);
                cr_data |= CR_RTSEn;
                UART_WriteReg(ch, OFFSET_CR, cr_data);

                delay1us(1000);

                // Read Data
                rdata = UART_ReadReg(ch, OFFSET_DR);

                if (wdata != rdata)
                {
                    // TEST FAIL!!
                    // rdata doesn't exist even though the RTS flag is enabled.
                    delay1us(1);
                }
                else
                {
                    // TEST SUCCESS !!
                    // Disable UART for the next test
                    UART_WriteReg(ch, OFFSET_CR, 0x0);
                }
            }
            else
            {
                // TEST FAIL!!
                // rdata exists even though the RTS flag is disabled.
                delay1us(1);
            }

            wdata = 0;
            rdata = 0;
        }
        else
        {
            // TEST FAIL!!
            // port fail
            delay1us(1);
        }
    }
}

