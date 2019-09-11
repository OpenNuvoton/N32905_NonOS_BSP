#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_vpost.h"
#include "AviLib.h"
#include "nvtfat.h"
#include "nvtloader.h"
#include "w55fa93_kpi.h"
#include "w55fa93_gpio.h"
#include "font.h"
#include "emuProduct.h" 

LCDFORMATEX LCDInfo;
	

void EMU_InitVPost(PUINT16 fb)
{
	static BOOL bIsInitVpost=FALSE;
	if(bIsInitVpost==FALSE)
	{
		bIsInitVpost = TRUE;
		LCDInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;	
		LCDInfo.nScreenWidth = _LCM_WIDTH_;	
		LCDInfo.nScreenHeight = _LCM_HEIGHT_;
		vpostLCMInit(&LCDInfo, (UINT32*)fb);
	}	
}
UINT32 u32Pattern[8] = {	0xFFFFFFFF,	//White			
							0xF800F800,	//R
							0x07E007E0,	//G
							0x001F001F,	//B			  	
							0xF81FF81F,	//Red + Blue ==> Pink
							0x07FF07FF,	//Green + Blue ==> Cyan
							0xFFE0FFE0,	//Red + Green ==> Yellow							
							0x00000000,	//Black	
						};

INT EMU_VPOST(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)	
{
	UINT32 	u32Idx, u32LcmW, u32LcmH; 
	PUINT32 pu32FrameBuf;
	UINT32 	u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock()*1000;	
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq); 		//External Crystal	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);          /* 100 ticks/per sec ==> 1tick/10ms */
	sysSetLocalInterrupt(ENABLE_IRQ);

	for(u32Idx=0; u32Idx<sizeof(u32Pattern)/sizeof(u32Pattern[0]); u32Idx=u32Idx+1)
	{
		pu32FrameBuf = (PUINT32)u32FrameBufAddr;
		for(u32LcmH=0;u32LcmH<_LCM_HEIGHT_; u32LcmH=u32LcmH+1)
		{
			for(u32LcmW=0;u32LcmW<_LCM_WIDTH_/2; u32LcmW=u32LcmW+1)
			{
				*pu32FrameBuf++ = u32Pattern[u32Idx];
			}
		}	
		Timer0_DelayTick(50);		/* Delay 50 tick = 500ms */							
	}
	return Successful;
}
