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
	
#ifdef __LCD_800x600__
	#include "roof_800x600_rgb565.dat"		// for SVGA size test
#endif

#ifdef __LCD_800x480__
	#include "sea_800x480_rgb565.dat"		
//	#include "roof_800x480_rgb565.dat"				
#endif

#ifdef __LCD_720x480__
	#include "lake_720x480_rgb565.dat"		// for D1 size test
//	#include "roof_720x480_rgb565.dat"		// for D1 size test
#endif

#ifdef __LCD_640x480__
    #include "mountain_640x480_rgb565.dat"	// for VGA size test	
#endif

#ifdef __LCD_480x272__
	#include "river_480x272_rgb565.dat"
#endif

#ifdef __LCD_320x240__	
	#include "roof_320x240_rgb565.dat"	
//	#include "roof_320x240_yuv422.dat"	
//	#include "roof_320x240_rgbx888.dat"		
#endif
};

LCDFORMATEX lcdFormat;

int volatile gTotalSectors_SD = 0, gTotalSectors_SM = 0;

void initUART(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock();
	
	/* Please run the path if you want to use normal speed UART */
	sysUartPort(1);
	uart.uiFreq = u32ExtFreq*1000;
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);
}

void initPLL(void)
{
	UINT32 u32ExtFreq;
	UINT32 u32PllOutKHz;	
	
#ifdef __TV_OUTPUT__
	sysSetSystemClock(eSYS_UPLL, 	    //E_SYS_SRC_CLK eSrcClk,	
						189000,		    //UINT32 u32PllKHz, 	
						189000,		    //UINT32 u32SysKHz,
						189000,		    //UINT32 u32CpuKHz,
						189000/2,		//UINT32 u32HclkKHz,
    					189000/4);	    //UINT32 u32ApbKHz
#endif					
    sysprintf("If TV output is selcted, PLL must set to times of 27 MHz.\n");
    
    
//#endif
			
	u32ExtFreq = sysGetExternalClock();
	u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
						  	
    sysprintf("PLL out frequency %d KHz\n", u32PllOutKHz);
}


int count = 0;
int STATUS;

int main(void)
{
    initUART();
	initPLL();
	
	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
//	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;
//	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_RGBx888;

	vpostLCMInit(&lcdFormat, (UINT32*)Vpost_Frame);
    sysprintf("LCD initialized successfully.\n");	
	while(1);

}


