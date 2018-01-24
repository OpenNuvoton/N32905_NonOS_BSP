/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
#include "wblib.h"

#include "w55fa93_reg.h"
#include "w55fa93_sic.h"
#include "nvtfat.h"

__align (32) UINT8 g_ram0[512*16*16*16];
__align (32) UINT8 g_ram1[512*16*16*16];

UINT32 u32SecCnt;
UINT32 u32backup[10];

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

    /* initialize FMI (Flash memory interface controller) */

    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);      /* clock from PLL */

    sicOpen();

    if (sicSdOpen0()<=0)
    {
        sysprintf("Error in initializing SD card !! \n");
        sicSdClose0();

        //--- Example code to enable/disable SD0 Card Detect feature.
        sysprintf("SD0 card detect feature is DISABLED by default.\n");
        sysprintf("The card status will always be INSERTED if card detect feature disabled.\n");
        sicIoctl(SIC_GET_CARD_STATUS, (INT32)&ii, 0, 0);
        sysprintf("Check SD0 card status ..... is %s\n", ii ? "INSERTED" : "REMOVED");

        sysprintf("Now, try to enable SD0 card detect feature ... Done!\n");
        sicIoctl(SIC_SET_CARD_DETECT, TRUE, 0, 0);  // MUST call sicIoctl() BEFORE sicSdOpen0()
        sicSdOpen0();

        ii = FALSE;
        sysprintf("Wait SD0 card insert ... \n");
        while (ii == FALSE)
        {
            sicIoctl(SIC_GET_CARD_STATUS, (INT32)&ii, 0, 0);
        }
        sysprintf("Card inserted !!\n");
        sicSdClose0();

        sysprintf("Now, Try to disable SD0 card detect feature ... Done!\n");
        sicIoctl(SIC_SET_CARD_DETECT, FALSE, 0, 0); // MUST call sicIoctl() BEFORE sicSdOpen0()
        sicSdOpen0();

        sysprintf("=== THE END ===\n");
        while(1);
    }
    else
    {
        sysprintf("SD total sector is %4x !! --\n", pDisk_SD0->uTotalSectorN);
    }

    for(ii=0; ii<512*16*16*16; ii++)
        g_ram0[ii] = rand();
//      g_ram0[ii] = ii*(ii/512 + ii);

    while(1)
    {
        u32backup[0] = g_ram0[0];
        for (jj = 1000; jj < pDisk_SD0->uTotalSectorN -0x100; jj+=u32SecCnt)
        {
            u32SecCnt = rand()&(0xFFF);
            if (u32SecCnt==0)
                u32SecCnt = 1;

            sicSdWrite0(jj, u32SecCnt, (UINT32)g_ram0);

            memset(g_ram1, 0x11, u32SecCnt);
            sicSdRead0(jj, u32SecCnt, (UINT32)g_ram1);
            if(memcmp(g_ram0, g_ram1, u32SecCnt*512) != 0)
            {
                sysprintf("data compare ERROR at sector No. !! -- %4x \n", jj);
                while(1);
            }
            sysprintf("data compare OK at sector No. !! -- %4x \n", jj);
        }
    }
}
