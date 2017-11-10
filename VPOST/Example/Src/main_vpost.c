/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
//#include "WBChipDef.h"
//#include "usbd.h"

#include "w55fa93_vpost.h"
#include "w55fa93_reg.h"

__align (32) UINT8 g_ram0[512*16*16];
__align (32) UINT8 g_ram1[512*16*16];
UINT32 u32SecCnt;
UINT32 u32backup[10];


__align(32) UINT8 Vpost_Frame[]=
{
//	#include "roof_800x600_rgb565.dat"		// for SVGA size test
	#include "sea_800x480_rgb565.dat"		
//	#include "roof_800x480_rgb565.dat"			
//	#include "roof_720x480_rgb565.dat"		// for D1 size test
//	#include "lake_720x480_rgb565.dat"		// for D1 size test
//	#include "mountain_640x480_rgb565.dat"	// for VGA size test	
//	#include "river_480x272_rgb565.dat"
//	#include "roof_320x240_rgb565.dat"	
//	#include "roof_320x240_yuv422.dat"	
//	#include "roof_320x240_rgbx888.dat"		
};


LCDFORMATEX lcdFormat;

int volatile gTotalSectors_SD = 0, gTotalSectors_SM = 0;


void initPLL(void)
{
}


int count = 0;
int STATUS;

int main(void)
{
//	InitUART();
//	g_SetupTOK =0;
//	g_SetupPKT=0;
//	g_OutTOK = 0;
//	g_SetupInTOK=0;
//	g_TXD=0;
//	g_RXD = 0;
	 
//	DrvSIO_printf("TEST %d\n",count);

	
	initPLL();
	
	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
//	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;
//	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGBx888;

	lcdFormat.nScreenWidth = 800;
	lcdFormat.nScreenHeight = 600;

//	lcdFormat.nScreenWidth = 640;
//	lcdFormat.nScreenHeight = 480;
	
//	lcdFormat.nScreenWidth = 720;
//	lcdFormat.nScreenHeight = 480;
	
//	lcdFormat.nScreenWidth = 480;
//	lcdFormat.nScreenHeight = 272;
//	lcdFormat.nScreenWidth = 320;
//	lcdFormat.nScreenHeight = 240;
	
	  
	vpostLCMInit(&lcdFormat, (UINT32*)Vpost_Frame);
	while(1);

}


