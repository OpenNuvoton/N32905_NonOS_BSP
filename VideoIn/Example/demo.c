/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/

#include <stdio.h>
#include "wblib.h"
#include "W55FA93_GPIO.h"
#include "W55FA93_VideoIn.h"
#include "w55fa93_sic.h"
#include "w55fa93_gnand.h"
#include "nvtfat.h"

#include "demo.h"


UINT8 u8PacketFrameBuffer[OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2];		//Keep 720*480*2 RGB565 frame buffer
UINT8 u8PacketFrameBuffer1[OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2];		//Keep 720*480*2 RGB565 frame buffer
UINT8 u8PacketFrameBuffer2[OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2];		//Keep 720*480*2 RGB565 frame buffer
UINT8 u8PlanarFrameBuffer[OPT_ENCODE_WIDTH*OPT_ENCODE_HEIGHT*2];		//Keep PlanarYUV422 frame buffer
UINT8 u8DiffBuf[(640/8)*(480/8)*2];
UINT8 u8OutLumBuf[(640/8)*(480/8)*2];


volatile UINT32 g_u32FrameCount = 0;
BOOL bIsFrameBuffer0=0,  bIsFrameBuffer1=0, bIsFrameBuffer2=0; /* 0 means buffer is clean */
UINT32 u32VideoInIdx = 0;
void VideoIn_InterruptHandler(void)
{
	
	switch(u32VideoInIdx)
	{//Current Frame
		case 0:		
			if(bIsFrameBuffer1==0)
			{/* Change frame buffer 1 if Frame Buffer 1 is clean, Otherwise, do nothing */	
			#ifdef __TV__
				videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
								eVIDEOIN_PACKET,			
								0, 						
								(UINT32)((UINT32)u8PacketFrameBuffer1 ) );		
			#else
								
				videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
								eVIDEOIN_PACKET,			
								0, 					
								(UINT32)((UINT32)u8PacketFrameBuffer1 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );													
				
			 #endif		
			 	bIsFrameBuffer0 = 1; u32VideoInIdx = 1; 
			}
				
				break; 	 				
		case 1:		
			if(bIsFrameBuffer2==0)
			{/* Change frame buffer 2 if Frame Buffer 2 is clean, Otherwise, do nothing */
			#ifdef __TV__
				videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
								eVIDEOIN_PACKET,			
								0, 								
								(UINT32)((UINT32)u8PacketFrameBuffer2 ) );
			#else				
				videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
								eVIDEOIN_PACKET,			
								0, 					
								(UINT32)((UINT32)u8PacketFrameBuffer2 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );					
			#endif						
 				bIsFrameBuffer1 = 1; u32VideoInIdx = 2;
			}	
				break; 	 				
		case 2:		
			if(bIsFrameBuffer0==0)
			{/* Change frame buffer 0 if Frame Buffer 0 is clean, Otherwise, do nothing */
			#ifdef __TV__
				videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
								eVIDEOIN_PACKET,			
								0, 							
								(UINT32)((UINT32)u8PacketFrameBuffer ) );
			#else
				videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
								eVIDEOIN_PACKET,			
								0, 					
								(UINT32)((UINT32)u8PacketFrameBuffer + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );
			#endif
				bIsFrameBuffer2 = 1; u32VideoInIdx = 0; 
			}	
				break; 	 
	}
	g_u32FrameCount = g_u32FrameCount+1;
	//if((g_u32FrameCount%100)==0)
		//sysprintf("F=%d\n", g_u32FrameCount);	
	DrvVideoIn_SetOperationMode(TRUE);	
}
UINT32 VideoIn_GetCurrFrameCount(void)
{
	return g_u32FrameCount;
}
void VideoIn_ClearFrameCount(void)
{
	g_u32FrameCount = 0;
}
void Delay(UINT32 nCount)
{
	volatile UINT32 i;
	for(;nCount!=0;nCount--)
		for(i=0;i<500;i++);
}

void Smpl_DumpPlanarYBuffer(void)
{
	char szAsciiStr[256], suFileName[512];
	volatile UINT32 u32CurrFrame;
	static UINT32 idx = 0;
	INT hFile, nWriteLen;
	UINT32 u32BufAddr =  inp32(REG_YBA0);
	UINT32 u32Size = inp32(REG_UBA0)- inp32(REG_YBA0);
	
	
	/* One shutter, the preview(packet) and encode(planar) pipes will be stop after current frame*/
	u32CurrFrame = g_u32FrameCount; 
	DrvVideoIn_SetOperationMode(TRUE);	
	while((u32CurrFrame-g_u32FrameCount)==0);
	
	sysprintf("Y Buffer = 0x%x\n", inp32(REG_YBA0));	
	sysprintf("U Buffer = 0x%x\n", inp32(REG_UBA0));	
	sysprintf("V Buffer = 0x%x\n", inp32(REG_VBA0));	
	
	idx = idx+1;
	sprintf(szAsciiStr, "C:\\Planar_Yonly_size_%dx%d_%d", OPT_ENCODE_WIDTH, OPT_ENCODE_HEIGHT, idx);
	fsAsciiToUnicode(szAsciiStr, suFileName, TRUE);
	hFile = fsOpenFile(suFileName, NULL, O_CREATE);
	fsWriteFile(hFile, (UINT8 *)u32BufAddr, u32Size, &nWriteLen);	
	fsCloseFile(hFile);
	
	/* Start preview and encode pipe */
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
										TRUE, 						// Engine enable ?
										eVIDEOIN_BOTH_PIPE_ENABLE,		// which Pipes was enable. 											
										0 );							//Useless	
}

void Smpl_DumpPlanarYUV422Buffer(void)
{
	char szAsciiStr[256], suFileName[512];
	volatile UINT32 u32CurrFrame;
	static UINT32 idx = 0;
	INT hFile, nWriteLen;
	UINT32 u32BufAddr =  inp32(REG_YBA0);
	UINT32 u32YSize = inp32(REG_UBA0)- inp32(REG_YBA0);
	UINT32 u32UVSize = inp32(REG_VBA0)- inp32(REG_UBA0);
	
	
	/* One shutter, the preview(packet) and encode(planar) pipes will be stop after current frame*/
	u32CurrFrame = g_u32FrameCount; 
	DrvVideoIn_SetOperationMode(TRUE);	
	while((u32CurrFrame-g_u32FrameCount)==0);
	
	sysprintf("Y Buffer = 0x%x\n", inp32(REG_YBA0));	
	sysprintf("U Buffer = 0x%x\n", inp32(REG_UBA0));	
	sysprintf("V Buffer = 0x%x\n", inp32(REG_VBA0));	
	
	idx = idx+1;
	sprintf(szAsciiStr, "C:\\PlanarYUV422_size_%dx%d_%d.pb", OPT_ENCODE_WIDTH, OPT_ENCODE_HEIGHT, idx);
	fsAsciiToUnicode(szAsciiStr, suFileName, TRUE);
	hFile = fsOpenFile(suFileName, NULL, O_CREATE);
	fsWriteFile(hFile, (UINT8 *)u32BufAddr, (u32YSize+2*u32UVSize), &nWriteLen);	
	fsCloseFile(hFile);
	
	/* Start preview and encode pipe */
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
										TRUE, 						// Engine enable ?
										eVIDEOIN_BOTH_PIPE_ENABLE,		// which Pipes was enable. 											
										0 );							//Useless	
}

IQ_S SensorIQ=0;
IQ_S* pSensorIQ;
int main()
{
	UINT32 u32Item, u32Upll_Clock;
	INT32 g_i32SD0TotalSector;	
#ifdef OPT_UART	
	/*Due to I2C share pins with UART, the SerialIO can not be used*/
	WB_UART_T uart;
	sysUartPort(1);
	uart.uiFreq =sysGetExternalClock()*1000;	//use APB clock
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);
#endif    	
#ifdef __TV__
	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
					162000,			//UINT32 u32PllKHz, 	
					162000,			//UINT32 u32SysKHz,
					162000/4,		//UINT32 u32CpuKHz,
					162000/4,		//UINT32 u32HclkKHz,
					  162000/4);	//UINT32 u32ApbKHz
#else
	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
					192000,			//UINT32 u32PllKHz, 	
					192000,			//UINT32 u32SysKHz,
					192000,		//UINT32 u32CpuKHz,
					192000/2,		//UINT32 u32HclkKHz,
					  192000/4);	//UINT32 u32ApbKHz				  
#endif	
	pSensorIQ = &SensorIQ; 
	
	sysEnableCache(CACHE_WRITE_BACK);
	/* start timer 0 */
	sysSetTimerReferenceClock(TIMER0, sysGetExternalClock()*1000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE); 
	
	/* Initial file system */
	fsInitFileSystem();
	
	/*-----------------------------------------------------------------------*/
	/*  Init SD card                                                       			*/
	/*-----------------------------------------------------------------------*/
	u32Upll_Clock = sysGetPLLOutputKhz(eSYS_UPLL, sysGetExternalClock());
	sicIoctl(SIC_SET_CLOCK, u32Upll_Clock, 0, 0);
	sicOpen();
    	g_i32SD0TotalSector = sicSdOpen0();	/* Total sector or error code */
    	if(g_i32SD0TotalSector < 0)
    	{
    		sysprintf("no SD card or SD card fail\n");
            	sicSdClose0();
	}
	fsAssignDriveNumber('C', DISK_TYPE_SD_MMC, 0, 1);
		
	DBG_PRINTF("================================================================\n");
#ifdef __2ND_PORT__ 	
	DBG_PRINTF("Please use LCD GW9360 for board number: WHS-W55FA93 DB LCD 8							\n");    	
#endif		
#ifdef __3RD_PORT__ 	
	DBG_PRINTF("Please use LCD GAINTPLUS	for board number: NHS-W55FA93-1-IN-1012						\n");    	
#endif	
	DBG_PRINTF("================================================================\n");	    	
	do
	{    	
		DBG_PRINTF("================================================================\n");
		DBG_PRINTF("				VideoIn library demo code								\n");
		DBG_PRINTF(" [1] OV9660 demo 												\n");
		DBG_PRINTF(" [2] OV7670 demo 												\n");
		DBG_PRINTF(" [3] OV7725 demo 												\n");		
		DBG_PRINTF(" [4] WT8861 demo 												\n");
		DBG_PRINTF(" [5] OV99050 demo 											\n");		
		
		DBG_PRINTF(" [8] NT99141 VGA 												\n");		
		DBG_PRINTF(" [9] NT99141 SVGA 											\n");	
		DBG_PRINTF(" [a] NT99141 HD 												\n");	
		DBG_PRINTF(" [b] GC0308 VGA 												\n");	
		DBG_PRINTF(" [c] TVP5150 HVGA 											\n");	
		DBG_PRINTF(" [d] TVP5150 VGA 												\n");	
		DBG_PRINTF(" [e] GM7150 HVGA 												\n");	
		DBG_PRINTF(" [f] FM7150 VGA 												\n");
		DBG_PRINTF(" [g] TW9900 HVGA 												\n");	
		DBG_PRINTF(" [h] TW9900 VGA 												\n");
		DBG_PRINTF(" [D] Dump Planar Y Buf (raw data)									\n");	
		DBG_PRINTF("================================================================\n");
#ifdef OPT_UART
		u32Item = sysGetChar();		
#else
		scanf("%c", &u32Item);			
#endif
		DBG_PRINTF("You selected item %c\n", (char)u32Item);
		switch(u32Item)
		{
			case '1': 	Smpl_OV9660(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 	
			case '2': 	Smpl_OV7670(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 	
			case '3': 	Smpl_OV7725(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 	
	    		case '4': 	Smpl_WT8861(u8PacketFrameBuffer);		break;	//OK	 
	    		case '5': 	register_sensor(&SensorIQ);
	    				Smpl_NT99050(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 		 
	    	
	    		case '8': 	Smpl_NT99141_VGA(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 
	    		case '9': 	sysprintf("Please modify encode dimension to SVGA in file-demo.h. Otherwise, the encode dimension will be VGA \n");
	    				Smpl_NT99141_SVGA(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 	    				
	    		case 'a': 	sysprintf("Please modify encode dimension to HD in file-demo.h. Otherwise, the encode dimension will be VGA\n");
	    				Smpl_NT99141_HD(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 	    			
	    		case 'b': 	
	    				Smpl_GC0308(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 		
	    		case 'c': 	
	    				Smpl_TVP5150_OneField(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		
	    				break; 	
		    	case 'd': 	
	    				Smpl_TVP5150_TwoFields(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		
	    				break;			
	    		case 'e': 	
	    				Smpl_GM7150_OneField(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		
	    				break; 	
		    	case 'f': 	
	    				Smpl_GM7150_TwoFields(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		
	    				break;	
	    		
	    		
	    		case 'g': 	
	    				register_TW9900_sensor(&SensorIQ);	
	    				Smpl_TW9900_VGA(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 	
	    		case 'h':
	    				register_TW9900_sensor(&SensorIQ);	
	    				Smpl_TW9900_VGA_TWO_Field(u8PacketFrameBuffer, u8PacketFrameBuffer1, u8PacketFrameBuffer2);		break; 				
	    							
	    		case 'D':	Smpl_DumpPlanarYBuffer();	break;
	    		case 'C':	Smpl_DumpPlanarYUV422Buffer();	break;
	    		
	    		case 'I':	
	    				if(pSensorIQ->IQ_SetBrightness==NULL)
	    					break;
	    				while(1)
	    				{
	    					static INT16 level = 0;
	    					
		    				pSensorIQ->IQ_SetBrightness(level);
	    					sysDelay(50);
	    					sysprintf("Brightness level =%d\n", level);
	    					level = level+8;	    					
	    					if(level>255)
	    						level = 0;
	    				}
	    				break;
	    		case 'J':	
	    				if(pSensorIQ->IQ_SetHue==NULL)
	    					break;
	    				while(1)
	    				{
	    					static INT16 angle = 0;
	    					pSensorIQ->IQ_SetHue(angle);
	    					sysDelay(50);
	    					sysprintf("Hue Angle =%d\n", angle);
	    					angle = angle+8;	    					
	    					if(angle>255)
	    						angle = 0;
	    				}
	    				pSensorIQ->IQ_SetHue(u32Item);
	    				break;
	    		case 'K':	
	    				if(pSensorIQ->IQ_SetSharpness==NULL)
	    					break;
	    				while(1)
	    				{
	    					static INT16 level = 0;
	    					INT16 regValue;	    					
	    					pSensorIQ->IQ_SetSharpness(level);
	    					sysDelay(50);
	    					regValue = pSensorIQ->IQ_GetSharpness();
	    					sysprintf("Set sharpness level =%d\n", level);
	    					sysprintf("Get sharpness level =%d\n", regValue);
	    					level = level+8;	    					
	    					if(level>255)
	    						level = 0;
	    				}
	    				break;	
	    		case 'L':	
	    				if(pSensorIQ->IQ_SetContrast==NULL)
	    					break;
	    				while(1)
	    				{
	    					static INT16 level = 0;
	    					INT16 regValue;	    					
	    					pSensorIQ->IQ_SetContrast(level);
	    					sysDelay(50);
	    					regValue = pSensorIQ->IQ_GetContrast();
	    					sysprintf("Set Contrast level =%d\n", level);
	    					sysprintf("Get Contrast level =%d\n", regValue);
	    					level = level+8;	    					
	    					if(level>255)
	    						level = 0;
	    				}
	    				break;			
		}
		
	}while((u32Item!= 'q') || (u32Item!= 'Q'));	
    	return 0;
} /* end main */