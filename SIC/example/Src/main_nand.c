/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wblib.h"
#include "wbtypes.h"

#include "w55fa93_reg.h"
#include "w55fa93_sic.h"
#include "w55fa93_GNAND.h"

#define DBG_PRINTF	sysprintf
//#define DBG_PRINTF(...)

__align (32) UINT8 g_ram0[512*16*16];
__align (32) UINT8 g_ram1[512*16*16];
UINT32 u32SecCnt;
UINT32 u32backup[10];

int volatile gTotalSectors_SM = 0;

NDRV_T _nandDiskDriver0 =
{
	nandInit0,
	nandpread0,
	nandpwrite0,
	nand_is_page_dirty0,
	nand_is_valid_block0,
	nand_ioctl,
	nand_block_erase0,
	nand_chip_erase0,
	0
};

NDISK_T *ptMassNDisk;
NDISK_T MassNDisk;

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
	int nSectorPerPage;

    init_UART();

	sysSetTimerReferenceClock(TIMER0, 12000000); 			//External Crystal
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);				/* 100 ticks/per sec ==> 1tick/10ms */
	sysSetLocalInterrupt(ENABLE_IRQ);

	/* initialize FMI (Flash memory interface controller) */
    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);  /* clock from PLL */
    sicOpen();

	ptMassNDisk = (NDISK_T*)&MassNDisk;
	if ( GNAND_InitNAND(&_nandDiskDriver0, (NDISK_T *)ptMassNDisk, TRUE))
	{
		sysprintf("GNAND init fail !!\n");
		return 1;
	}

	nSectorPerPage = ptMassNDisk->nPageSize / 512;
   	gTotalSectors_SM = ptMassNDisk->nZone * (ptMassNDisk->nLBPerZone-1) * ptMassNDisk->nPagePerBlock * nSectorPerPage;

    if (gTotalSectors_SM < 0)
    {
        GNAND_UnMountNandDisk(ptMassNDisk);
        fmiSMClose(0);
        while(1);
    }

	for(ii=0; ii<512*16*16; ii++)
		g_ram0[ii] = rand();

	while(1)
	{
		u32backup[0] = g_ram0[0];
		for (jj = 3000; jj < 50000; jj+=u32SecCnt)
		{
			u32SecCnt = rand()&(0xFF);
			if (u32SecCnt==0)
				u32SecCnt = 1;

			GNAND_write(ptMassNDisk, jj, u32SecCnt, g_ram0);

		    memset(g_ram1, 0x11, u32SecCnt);
			GNAND_read(ptMassNDisk, jj, u32SecCnt, g_ram1);
	        if(memcmp(g_ram0, g_ram1, u32SecCnt*512) != 0)
	        {
				sysprintf("data compare ERROR at sector No. !! -- %4x \n", jj);
	        	while(1);
			}
			sysprintf("data compare OK at sector No. !! -- %4x \n", jj);
		}
	}
}
