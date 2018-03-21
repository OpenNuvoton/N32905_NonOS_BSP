/***************************************************************************
 *                                                                         *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include "wblib.h"

#include "W55FA93_GPIO.h"
#include "W55FA93_VideoIn.h"
#include "demo.h"
#include "jpegcodec.h"
#include "usbd.h"
#include "videoclass.h"

IQ_S SensorIQ=0;
IQ_S* pSensorIQ;	
BOOL volatile gbMT99050 = FALSE;
BOOL volatile gOneField = FALSE;

int main()
{
	/*Due to I2C share pins with UART, the SerialIO can not be used*/
	WB_UART_T uart;

#ifdef __UVC_VIN__	
	UINT32 u32Item;
#endif	
	uart.uiFreq =sysGetExternalClock()*1000;	//use APB clock
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);

	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
					192000,			//UINT32 u32PllKHz, 	
					192000,			//UINT32 u32SysKHz,
					192000,			//UINT32 u32CpuKHz,
					 96000,			//UINT32 u32HclkKHz,
					 48000);		//UINT32 u32ApbKHz		
							  
	sysEnableCache(CACHE_WRITE_BACK);
	
#ifdef __UVC_VIN__	

	do
	{    	
		DBG_PRINTF("================================================================\n");
		DBG_PRINTF("				VideoIn library demo code						\n");
		DBG_PRINTF(" [1] OV7670 demo 												\n");
		DBG_PRINTF(" [2] OV7725 demo 												\n");		
		DBG_PRINTF(" [3] NT99050 demo 												\n");			
		DBG_PRINTF(" [4] GC0308 demo 												\n");			
		DBG_PRINTF(" [5] TVP5150 two fields demo 									\n");			
#ifdef __MJPEG__		
		DBG_PRINTF(" [6] TVP5150 one field demo (MJPEG format Only - Encode Upscale)\n");	
#endif				
		DBG_PRINTF(" [7] GM7150 two fields demo 									\n");					
#ifdef __MJPEG__
		DBG_PRINTF(" [8] GM7150 one field demo (MJPEG format Only - Encode Upscale)	\n");			
#endif					

		DBG_PRINTF(" [9] NT99141 demo	\n");			
		
		DBG_PRINTF("================================================================\n");

		u32Item = sysGetChar();	
		if ( (u32Item >='1') && (u32Item <= '9') )
		   break;
    }while (1);
	
	
	u32PacketFrameBuffer0 = (UINT32) u8FrameBuffer0 + BITSTREAM_OFFSET;
	u32PacketFrameBuffer1 = (UINT32) u8FrameBuffer1 + BITSTREAM_OFFSET;
	u32BitstreamBuffer0 = (UINT32) u8FrameBuffer0;
	u32BitstreamBuffer1 = (UINT32) u8FrameBuffer1;
	
    if ( u32Item ==  '1' )	
	   Smpl_OV7670((PUINT8)u32PacketFrameBuffer0,(PUINT8)u32PacketFrameBuffer1);		 	
	else if(u32Item ==  '2' )	
	   Smpl_OV7725((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1);   
	else if(u32Item ==  '3' )	
	{
		register_sensor(&SensorIQ);			
		gbMT99050 = TRUE;
		Smpl_NT99050((PUINT8)u32PacketFrameBuffer0,(PUINT8)u32PacketFrameBuffer1);	
		uvcPuInfo.PU_BACKLIGHT_COMPENSATION_MIN = 0;
		uvcPuInfo.PU_BACKLIGHT_COMPENSATION_MAX = 0;
		uvcPuInfo.PU_BACKLIGHT_COMPENSATION_DEF = 0;
		uvcPuInfo.PU_BRIGHTNESS_MIN = 0;
		uvcPuInfo.PU_BRIGHTNESS_MAX = 255;
		uvcPuInfo.PU_BRIGHTNESS_DEF = SensorIQ.IQ_GetBrightness();
		uvcPuInfo.PU_CONTRAST_MIN = 0;
		uvcPuInfo.PU_CONTRAST_MAX = 255;
		uvcPuInfo.PU_CONTRAST_DEF = SensorIQ.IQ_GetContrast();
		uvcPuInfo.PU_HUE_MIN = 0;
		uvcPuInfo.PU_HUE_MAX = 255;
		uvcPuInfo.PU_HUE_DEF = SensorIQ.IQ_GetHue();
		uvcPuInfo.PU_SATURATION_MIN = 0;
		uvcPuInfo.PU_SATURATION_MAX = 0;
		uvcPuInfo.PU_SATURATION_DEF = 0;
		uvcPuInfo.PU_SHARPNESS_MIN = 0;
		uvcPuInfo.PU_SHARPNESS_MAX =255;
		uvcPuInfo.PU_SHARPNESS_DEF = SensorIQ.IQ_GetSharpness();
		uvcPuInfo.PU_GAMMA_MIN = 0;
		uvcPuInfo.PU_GAMMA_MAX = 0;
		uvcPuInfo.PU_GAMMA_DEF = 0;
		uvcPuInfo.PU_POWER_LINE_FREQUENCY_MIN = 0;
		uvcPuInfo.PU_POWER_LINE_FREQUENCY_MAX = 0;
		uvcPuInfo.PU_POWER_LINE_FREQUENCY_DEF = 0; 			
	}   
	else if(u32Item ==  '4' )	
	 	Smpl_GC0308((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1);   
	else if(u32Item ==  '5' )	
	 	Smpl_TVP5150_TwoFields((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1);
    else if(u32Item ==  '9' )	 
	 	Smpl_NT99141_VGA((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1);		
#ifdef __MJPEG__	 	
	else if(u32Item ==  '6' )	
	{
	 	gOneField = TRUE;
	 	Smpl_TVP5150_OneField((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1);	 	
	}
#endif	 	
	 else if(u32Item ==  '7' )	
	 	Smpl_GM7150_TwoFields((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1); 	 
#ifdef __MJPEG__	
	 else if(u32Item ==  '8' )	
	 {
	 	gOneField = TRUE;	 
	 	Smpl_GM7150_OneField((PUINT8)u32PacketFrameBuffer0, (PUINT8)u32PacketFrameBuffer1);		
	 }
#endif		 	
	jpegOpen ();    
#endif		
	uvc_main();		
	while(1);
    return 0;
} /* end main */