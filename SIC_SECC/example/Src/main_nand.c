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

#define DBG_PRINTF	printf
//#define DBG_PRINTF	sysprintf
//#define DBG_PRINTF(...)

__align (32) UINT8 g_ram0[512*16*16];
__align (32) UINT8 g_ram1[512*16*16];
UINT32 u32SecCnt;
UINT32 u32backup[10];

int volatile gTotalSectors_SD = 0, gTotalSectors_SM = 0;

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
NDRV_T _nandDiskDriver1 = 
{
	nandInit1,
	nandpread1,
	nandpwrite1,
	nand_is_page_dirty1,
	nand_is_valid_block1,
	nand_ioctl,
	nand_block_erase1,
	nand_chip_erase1,
	0
};

NDISK_T *ptMassNDisk;
NDISK_T MassNDisk;

void initPLL(void)
{
}


int count = 0;
int STATUS;

void Test()
{
	DBG_PRINTF("test -> %d\n", ++count);
}

void Test1()
{
	DBG_PRINTF("Hello World!\n");
}

void Test2()
{
	DBG_PRINTF("timer 1 test\n");
}


void DemoAPI_Timer0(void)
{
	//	unsigned int volatile i;
	volatile unsigned int btime, etime, tmp, tsec;
	volatile UINT32 u32TimeOut = 0;
	UINT32 u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock()*1000;
	DBG_PRINTF("Timer 0 Test...\n");	
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq); //External Crystal
	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);			/* 100 ticks/per sec ==> 1tick/10ms */
	
	tmp = sysSetTimerEvent(TIMER0, 100, (PVOID)Test);	/* 100 ticks = 1s call back */
	DBG_PRINTF("No. of Event [%d]\n", tmp);

	tmp = sysSetTimerEvent(TIMER0, 300, (PVOID)Test1);	/* 300 ticks/per sec */
	DBG_PRINTF("No. of Event [%d]\n", tmp);	
	sysSetLocalInterrupt(ENABLE_IRQ);

	btime = sysGetTicks(TIMER0);
	tsec = 0;
	tmp = btime;
	while (1)
	{			
		etime = sysGetTicks(TIMER0);
		if ((etime - btime) >= 100)
		{
			DBG_PRINTF("tick = %d\n", ++tsec);			
			btime = etime;
			u32TimeOut = u32TimeOut +1;
			if(u32TimeOut==10)			/* 10s Time out  */
				break;			
		}
	}
	DBG_PRINTF("Finish timer 0 testing...\n");
	sysStopTimer(TIMER0);
}

int main(void)
{
	int ii, jj;
	int nSectorPerPage;	 
	 
//	DrvSIO_printf("TEST %d\n",count);
	
	initPLL();
	
	sysSetTimerReferenceClock(TIMER0, 12000000); 			//External Crystal
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);			/* 100 ticks/per sec ==> 1tick/10ms */
	sysSetLocalInterrupt(ENABLE_IRQ);	

	/* initialize FMI (Flash memory interface controller) */
    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);  /* clock from PLL */
    sicOpen();

	// NAND erase
	#if 0
		nandInit1(&MassNDisk);
		nand_chip_erase1();
		
		for (ii=0; ii< pSM1->uBlockPerFlash; ii++)
		{
			if (fmiCheckInvalidBlock(pSM1, ii))
				printf("invald block No. %4d \n", ii);	
		}

		while(1);
		
	#endif	

	ptMassNDisk = (NDISK_T*)&MassNDisk;
	if ( GNAND_InitNAND(&_nandDiskDriver0, (NDISK_T *)ptMassNDisk, TRUE))
//	if ( GNAND_InitNAND(&_nandDiskDriver1, (NDISK_T *)ptMassNDisk, TRUE))	
	{
		printf("GNAND init fail !!\n");					
		return 1;
	}				

	nSectorPerPage = ptMassNDisk->nPageSize / 512;
   	//gTotalSectors_SM = ptMassNDisk->nZone * (ptMassNDisk->nLBPerZone-1) * ptMassNDisk->nPagePerBlock * nSectorPerPage - ptMassNDisk->nStartBlock;
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
				printf("data compare ERROR at sector No. !! -- %4x \n", jj);
	        	while(1);
			}			        	
			printf("data compare OK at sector No. !! -- %4x \n", jj);					
		}			
	}
}


