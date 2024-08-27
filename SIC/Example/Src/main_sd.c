/**************************************************************************//**
 * @file     main_sd.c
 * @brief    The main file for SIC/SD demo code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
#include "wblib.h"

#include "W55FA93_reg.h"
#include "W55FA93_SIC.h"
#include "NVTFAT.h"

#if defined (__GNUC__)
    UINT8 g_ram0[512*16*16] __attribute__((aligned (32)));
    UINT8 g_ram1[512*16*16] __attribute__((aligned (32)));
#else
    __align (32) UINT8 g_ram0[512*16*16];
    __align (32) UINT8 g_ram1[512*16*16];
#endif

UINT32 u32SecCnt;

extern PDISK_T *pDisk_SD0;

/*-----------------------------------------------------------------------------
 * Initial UART.
 *---------------------------------------------------------------------------*/
void init_UART()
{
    UINT32 u32ExtFreq;
    WB_UART_T uart;

    u32ExtFreq = sysGetExternalClock()*1000;
    sysUartPort(1);
    uart.uiFreq = u32ExtFreq;   //use APB clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    uart.uart_no = WB_UART_1;
    sysInitializeUART(&uart);
}


int main(void)
{
    int ii, jj;

    init_UART();

    sysprintf("\n=====> W55FA93 Non-OS SIC/SD Driver Sample Code <=====\n");

    //--- Initial system clock
    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);      /* clock from PLL */

    //--- Enable AHB clock for SIC/SD/NAND, interrupt ISR, DMA, and FMI engineer
    sicOpen();

    if (sicSdOpen0()<=0)
    {
        sysprintf("Error in initializing SD card !! \n");
    }
    else
    {
        sysprintf("SD total sector is 0x%X !! \n", pDisk_SD0->uTotalSectorN);

        for(ii=0; ii<512*16*16; ii++)
            g_ram0[ii] = rand();

        for (jj = 1000; jj < pDisk_SD0->uTotalSectorN -0x100; jj+=u32SecCnt)
        {
            u32SecCnt = rand()&(0xFF);
            if (u32SecCnt==0)
                u32SecCnt = 1;

            sicSdWrite0(jj, u32SecCnt, (UINT32)g_ram0);

            memset(g_ram1, 0x11, u32SecCnt);
            sicSdRead0(jj, u32SecCnt, (UINT32)g_ram1);
            if(memcmp(g_ram0, g_ram1, u32SecCnt*512) != 0)
            {
                sysprintf("data compare ERROR at sector 0x%X, sector count = 0x%X \n", jj, u32SecCnt);
                break;
            }
            sysprintf("data compare OK at sector 0x%X, sector count = 0x%X \n", jj, u32SecCnt);
        }
    }

    sicSdClose0();
    sysprintf("\n===== THE END =====\n");
    return 0;
}
