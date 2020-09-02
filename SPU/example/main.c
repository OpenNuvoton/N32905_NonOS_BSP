/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    N329xx series SPU demo code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wblib.h"
#include "SONG.h"
#include "DrvSPU.h"
#include "SPU.h"

#define LENGTH		194540	// The length of song in unit of 16-bit

static UINT16 u16IntCount = 2;
static UINT32 u32FragSize;
static volatile UINT8 bPlaying = TRUE;

int playCallBack(UINT8 * pu8Buffer)
{	
	UINT32 u32Offset = 0;
	UINT32 len = sizeof(SPU_SOURCE);
	
	u32Offset = ( u32FragSize / 2) * u16IntCount;
	if (u32Offset >= len)		// Reach the end of the song, restart from beginning
	{		
		u16IntCount = 2;
		u32Offset = 0;
		bPlaying = FALSE;
		return TRUE;
	}	
	
	memcpy(pu8Buffer, &SPU_SOURCE[u32Offset], u32FragSize/2);
	
	u16IntCount++;
	
	return FALSE;
}


int main(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;	

	sysEnableCache(I_D_CACHE);
	sysprintf("Start Playing...\n");	

	spuOpen(eDRVSPU_FREQ_8000);
	
	spuIoctl(SPU_IOCTL_SET_VOLUME, 0x3f, 0x3f);
	spuIoctl(SPU_IOCTL_GET_FRAG_SIZE, (UINT32)&u32FragSize, 0);
	
	spuEqOpen(eDRVSPU_EQBAND_2, eDRVSPU_EQGAIN_P7DB);
	spuDacOn(1);
    spuStartPlay((PFN_DRVSPU_CB_FUNC *) playCallBack, (UINT8 *)SPU_SOURCE);		

   	while(bPlaying == TRUE);	
	
    spuEqClose();	
    spuClose();
    	
    while(1);	
		
	return(0);
}