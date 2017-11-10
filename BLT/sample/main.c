#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"

#include "w55fa93_reg.h"
#include "DrvBLT.h"
//#include "DrvAIC.h"
#include "w55fa93_vpost.h"
#include "FI.h"

// Pattern for VPOST Buffer
__align(32) UINT8 Vpost_Pattern[]= {
//#include "river_480x272_rgb565.dat"
#include "roof_320x240_RGB565.dat"
};

// Pattern for Source Buffer
__align(32) UINT8 src_pattern[] = {
	#include "img.dat"		//96*129 ARGB888
};

__align(32) UINT8 SRC_IMAGE_BUFF[VPOST_WIDTH*VPOST_HEIGHT*2];	// Reserved buffer for Src Pattern
__align(32) UINT8 SRC_DEST_BUFF[VPOST_WIDTH*VPOST_HEIGHT*2];	// Reserved buffer for Destination Pattern
__align(32) UINT8 SRC_VPOST_BUFF[VPOST_WIDTH*VPOST_HEIGHT*2];	// Reserved buffer for VPOST Pattern


UINT32 SRC_IMAGE_ADDR= (UINT32)&SRC_IMAGE_BUFF[0];
UINT32 SRC_DEST_ADDR = (UINT32)&SRC_DEST_BUFF[0];
UINT32 SRC_DEST2_ADDR = (UINT32)&SRC_VPOST_BUFF[0];
UINT32 VPOST_DISPLAY_ADDR = (UINT32)&SRC_DEST_BUFF[0];


int main()
{
	LCDFORMATEX lcdFormat;	                 
	

	outp32(0xb0000204, inp32(0xb0000204) | 0x01010020);		// enable EDMA 0 ~ 5
	outp32(0xb0000208, inp32(0xb0000208) | 0x00000300);		// enable Timer	
                  
	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
	lcdFormat.nScreenWidth = VPOST_WIDTH;
	lcdFormat.nScreenHeight = VPOST_HEIGHT;	

	vpostLCMInit(&lcdFormat, (UINT32*)SRC_DEST_ADDR);
	memcpy((UINT8*)SRC_DEST2_ADDR, (UINT8 *)Vpost_Pattern, sizeof(Vpost_Pattern));
	memcpy((UINT8*)SRC_IMAGE_ADDR, (UINT8 *)src_pattern, sizeof(src_pattern));	
	

	DrvBLT_Open();
	
	DrvBLT_SetSrcFormat(eDRVBLT_SRC_RGB565);
	DrvBLT_SetDisplayFormat(eDRVBLT_SRC_RGB565);	
     
    do {
	    sysprintf("BitBlt copy demo..\n"); 
		BlitOpTest();
		sysprintf("Press any key to continue\n"); 
		sysGetChar();
		sysprintf("BitBlt ARGB alpha blending demo..\n"); 
	    BlitAlpahTest(); 
	    sysprintf("Press any key to continue\n"); 
		sysGetChar(); 
	} while(1);
	
	while(1);
	return 0;
}