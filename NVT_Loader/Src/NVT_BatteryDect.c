/**************************************************************************//**
 * @file     NVT_BatteryDect.c
 * @brief    Detection system working voltage
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

/****************************************************************************
* FILENAME
*   adc.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   Battery detection
*
* DATA STRUCTURES
*   None
*
* FUNCTIONS
*
* HISTORY
*
* REMARK
*   None
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wblib.h"

#include "nvtloader.h"
#include "W55FA93_adc.h"
#include "RTC.h"

void ForceShutdown(void)
{
	/* RTC Initialize */
	RTC_Init();	
	RTC_Ioctl(0, RTC_IOC_SET_POWER_OFF, 0, 0);
	while(1);	/* Forever loop */
}
/*
	There are 2 resistors form voltage divider in demo board. One is 100K Ohm, anothe is 200K Ohm
	The ratio is 1/3. 	 
	

*/
void BatteryDetection(BOOL bIsExtraPower)
{
	float voltage, step, tmp;
	UINT16 u16Vol;
	step = 3.3/1024.;  						/* Assume the reference votage is 3.3 V. 10bits ADC */
		
	if(adc_normalread(2, &u16Vol)==Successful)
	{
		UINT32 u32Dec, u32Fraction=3;
		tmp = step*u16Vol*3;				/* Ratio is 1/3 */
		voltage = tmp;
		sysprintf("Battery value = %x\n", u16Vol);	
		sysprintf("Battery Voltage = ");			
		u32Dec = tmp;	
		sysprintf("%d.",u32Dec); 	
		tmp = tmp - u32Dec;
		while((tmp!=0.) && (u32Fraction!=0))
		{		
			tmp = tmp*10.;	
			u32Dec = tmp;	
			sysprintf("%d",u32Dec); 	
			tmp = tmp - u32Dec;
			u32Fraction = u32Fraction-1;
		}
		sysprintf("\n"); 
		if(bIsExtraPower==FALSE)
		{//Withput extra power form USB or power cable
			if(voltage<LOW_BATTERY_LEVEL)
			{
				sysprintf("Low Battery ...., please change the battery\n");
				sysprintf("Forcce shutdown\n");
				ForceShutdown();				
			}
		}
	}	
}