#include <stdio.h>
#include "wblib.h"
#include "W55FA93_ADC.h"


void Smpl_TscPanel_WT_Powerdown_Wakeup(void)
{
	//PFN_ADC_CALLBACK pfnOldCallback;
	UINT32 u32Count=1;

		
	//adc_enableInt(eADC_WT_INT);
	//adc_enableInt(eADC_ADC_INT);

	while(1)
	{/* Power down wake up for test 30 times */										
		adc_setTouchScreen(eADC_TSCREEN_TRIG,	//E_DRVADC_TSC_MODE eTscMode,
							180,						//Delay cycle
							TRUE,					//BOOL bIsPullup,
							FALSE);					//BOOL bMAVFilter
		sysDelay(50);	
		sysprintf("\n");

		sysPowerDown(WE_ADC);

		outp32(0xb0000030, inp32(0xb0000030)| 0x40000000);	//Clear ADC wakeup status										
		sysprintf("Wake up!!! %d \n", u32Count);									
		u32Count = u32Count +1;
	}		
}	
