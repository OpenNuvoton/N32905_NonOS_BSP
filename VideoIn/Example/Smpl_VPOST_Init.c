
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
#include "W55fa93_vpost.h"

void InitVPOST(UINT8* pu8FrameBuffer)
{		
	LCDFORMATEX lcdFormat;	
	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
	lcdFormat.nScreenWidth = 480;
	lcdFormat.nScreenHeight = 272;	  
	vpostLCMInit(&lcdFormat, (UINT32*)pu8FrameBuffer);
}	