/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wblib.h"
#include "nvtfat.h"
#include "W55FA93_reg.h"
#include "USB.h"
#include "Font.h"
#include "emuProduct.h"
/*                                                                          									    
  Test read write file to USBH
  Create U:\SRC\0.jpg, 1.jpg and 2.jpg	
  
  
*/
typedef struct tagFileInfo
{
	PUINT8 pu8Addr;
	UINT32 u32Len;	
}FILE_INFO_T;
//static FILE_INFO_T tFileInfo[10];

static UINT32 u32Timer0_10ms_Count=0;
static UINT32 u32Xpos;
static S_DEMO_FONT*	ptTmpFont;
static void Timer0_Callback(void)
{
	u32Timer0_10ms_Count++;
	if((u32Timer0_10ms_Count%100)==0)
	{
		DemoFont_PaintA(ptTmpFont,					
						u32Xpos,					
						0,			
						"*");
		u32Xpos = u32Xpos +	_FONT_OFFSET_;				
	}	
}

INT USBH_Access(void)
{		
	CHAR    szFileName[32];
	CHAR    suFileName[128];	
	UINT32 	u32Idx;
	PUINT8 	pu8DstAddr, pu8ReadAddr;
	PUINT8 	pu8DstAddrTmp; 
	INT		hFile, nStatus, nReadBytes, nWriteBytes;
	INT 	t0, t1, nLen; 
	
	pu8DstAddr = malloc(3*128*1024) ;	/* Allocate 3*128 KB then initialize the buffer for writing*/
	if(pu8DstAddr == NULL)
	{
		sprintf("%s\n","Out of message");	
		return ERR_OUT_MEMORY;
	}	
	pu8ReadAddr = malloc(3*128*1024);	/* Allocate 3*128 KB then initialize the buffer for reading*/
	if(pu8ReadAddr == NULL)
	{
		sprintf("%s\n","Out of message");	
		return ERR_OUT_MEMORY;
	}	
	pu8DstAddrTmp = pu8DstAddr;
	for(t0=0; t0<(3*128*1024); t0=t0+1)
	{
		*pu8DstAddrTmp = (t0 & 0xFF)+ ((t0>>8)& 0xFF)+((t0>>16)& 0xFF); 
		pu8DstAddrTmp++;
	}
	pu8DstAddrTmp = pu8DstAddr;
	
	t0 = sysGetTicks(TIMER0);					
	sysprintf("Read Begain tick = %d\n", t0);
	/* Write Performenc test */
	t0 = sysGetTicks(TIMER0);					
	sysprintf("Write Begain tick = %d\n", t0);
	for(u32Idx=0; u32Idx<=2; u32Idx=u32Idx+1)	
	{
		sprintf(szFileName, "U:\\%d.jpg", u32Idx);			
		fsAsciiToUnicode(szFileName, suFileName, 1);
		/* Open */ 
		hFile = fsOpenFile(suFileName, NULL, O_CREATE);
		if (hFile > 0)
			;
		else
		{
			sysprintf("Failed to open file: %s (%x)\n", szFileName, hFile);
			return hFile;
		}
		/* Read from SDRAM */ 
		nStatus = fsWriteFile(hFile, pu8DstAddrTmp+(u32Idx*128*1024), 128*1024, &nWriteBytes);
		if (nStatus < 0) 
		{
			sysprintf("Warning found: %d bytes\n", nWriteBytes);		
			return nStatus;
		}			
		fsCloseFile(hFile);		
	}
	t1 = sysGetTicks(TIMER0);			
	sysprintf("Write End tick = %d\n", t1);
	sysprintf("Write Take  ticks = %d\n", (t1-t0));
	
	
	/* Read Performenc test */
	for(u32Idx=0; u32Idx<=2; u32Idx=u32Idx+1)	
	{
		sprintf(szFileName, "U:\\%d.jpg", u32Idx);			
		fsAsciiToUnicode(szFileName, suFileName, 1);
		/* Open */ 
		hFile = fsOpenFile(suFileName, NULL, O_RDONLY);
		if (hFile > 0)
			;
		else
		{
			sysprintf("Failed to open file: %s (%x)\n", szFileName, hFile);
			return hFile;
		}
		nLen = 0;
		/* Read to SDRAM */ 
		pu8DstAddrTmp = pu8ReadAddr+(u32Idx*128*1024);
		while(1)
		{	
			nStatus = fsReadFile(hFile, pu8DstAddrTmp, 4096, &nReadBytes);
			if((nReadBytes<4096) || (nReadBytes==0))
			{		
				nLen = nLen+nReadBytes;						
				break;				
			}
			if (nStatus < 0) 
			{
				sysprintf("Warning found: %d bytes\n", nReadBytes);		
				return nStatus;
			}
				
			nLen = nLen+nReadBytes;
			pu8DstAddrTmp = pu8DstAddrTmp + nReadBytes;
		}	
		fsCloseFile(hFile);		
	}
	t1 = sysGetTicks(TIMER0);		
	sysprintf("Read End tick = %d\n", t1);
	sysprintf("Read Take  ticks = %d\n", (t1-t0));
	
	for(u32Idx=0; u32Idx<=2; u32Idx=u32Idx+1)	
	{		
		sprintf(szFileName, "U:\\%d.dat", u32Idx);			
		fsAsciiToUnicode(szFileName, suFileName, 1);
		fsDeleteFile(suFileName, NULL);				
	}
	free(pu8DstAddr);
	free(pu8ReadAddr);
		
	return Successful;
}


INT EMU_USBH(
	S_DEMO_FONT* 	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	UINT32 	u32ExtFreq, t0;
	UINT32 	u32BlockSize, u32FreeSize, u32DiskSize, u32Ypos=0; 
	INT	   	tmp, nStatus;
	char 	Array1[64];
	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);
    outp32(REG_APBCLK, 0xFFFFFFFF);
	ptTmpFont = ptFont;
	u32Timer0_10ms_Count =0;
	u32ExtFreq = sysGetExternalClock();
	sysSetLocalInterrupt(ENABLE_FIQ_IRQ);
	
	u32ExtFreq = sysGetExternalClock();
	sysSetTimerReferenceClock (TIMER0, u32ExtFreq*1000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	tmp = sysSetTimerEvent(TIMER0, 1, (PVOID)Timer0_Callback);	// 1 ticks = 0.01s call back 		

	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);
	sprintf(Array1, "[USBH]");
	u32Xpos = (strlen("[USBH]")+1)*_FONT_OFFSET_;	
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);
	
	
	USB_PortDisable(FALSE, TRUE);
	USB_PortInit(HOST_LIKE_PORT0);
	fsAssignDriveNumber('U', DISK_TYPE_USB_DEVICE, 0, 1);		/* Remember to initialize file system in advance*/ 
	
	InitUsbSystem();       								
	UMAS_InitUmasDriver();
	
	t0 = sysGetTicks(TIMER0);
	while (sysGetTicks(TIMER0) - t0 < 300)
		;
		
	nStatus = fsDiskFreeSpace('U', &u32BlockSize, &u32FreeSize, &u32DiskSize);
	if ( nStatus == 0)
	{
		sysprintf("U Disk size = %d KB, Free speace = %d KB\n", u32DiskSize, u32FreeSize);
		sprintf(Array1, "DISK Size = %d KB. Free Size %d KB", u32DiskSize, u32FreeSize);
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);
	}	
	else
	{	
		sysprintf("fsDiskFreeSpace failed!!\n");
		sprintf(Array1, "Get disk information fail");
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);								
		goto Usbh_Err;
	}	
		
	nStatus = USBH_Access();	
	if(nStatus==Successful)
	{
		sprintf(Array1, "USBH access pass");
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,				
						0,					
						u32Ypos,			
						Array1);
	}
	else
	{
				
		sprintf(Array1, "USBH access fail");
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,				
						0,				
						u32Ypos,		
						Array1);									
	}						
Usbh_Err:	
	sysClearTimerEvent(TIMER0, tmp);
	UMAS_RemoveUmasDriver();
    	DeInitUsbSystem();	
   	return nStatus;	 
}