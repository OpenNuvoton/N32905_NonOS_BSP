#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_vpost.h"
#include "AviLib.h"
#include "nvtfat.h"
#include "nvtloader.h"
#include "w55fa93_kpi.h"
#include "w55fa93_gpio.h"
#include "w55fa93_sic.h"
#include "emuProduct.h"

INT32 EMU_InitSdCard(void)
{	
	UINT32 u32PllOutKHz, u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock();
	fsAssignDriveNumber('X', DISK_TYPE_SD_MMC, 0, 1);
	u32PllOutKHz = sysGetPLLOutputKhz(eSYS_UPLL, u32ExtFreq);
	 
	sicIoctl(SIC_SET_CLOCK, u32PllOutKHz, 0, 0);	
	sicOpen();	
	if (sicSdOpen0()<=0)
	{
		DBG_PRINTF("Error in initializing SD card !! \n");						
		return ERR_SDCARD_ACCESS_FAIL;
	}		
	return Successful;
}	

#define SDCARD_ACCESS_LEN	(1024*1024)		//It must multiple of "SDCARD_OPEN_FILE"
#define SDCARD_OPEN_FILE	(4)

INT SDCardAccess(void)
{
	CHAR    szFileName[32];
	CHAR    suFileName[128];	
	UINT32 	u32Idx;
	PUINT8 	pu8DstAddr, pu8ReadAddr;
	PUINT8 	pu8DstAddrTmp; 
	INT		hFile, nStatus, nReadBytes, nWriteBytes;
	INT 	t0, t1, nLen; 
	
	pu8DstAddr = malloc(SDCARD_ACCESS_LEN);	/* Allocate buffer for writing*/
	if(pu8DstAddr == NULL)
	{
		sprintf("%s\n","Out of message");	
		return ERR_OUT_MEMORY;
	}	
	pu8ReadAddr = malloc(SDCARD_ACCESS_LEN);/* Allocate buffer for reading*/
	if(pu8ReadAddr == NULL)
	{
		sprintf("%s\n","Out of message");	
		free(pu8DstAddr);
		return ERR_OUT_MEMORY;
	}	
	pu8DstAddrTmp = pu8DstAddr;
	for(t0=0; t0<(SDCARD_ACCESS_LEN); t0=t0+1)
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
	for(u32Idx=0; u32Idx<SDCARD_OPEN_FILE; u32Idx=u32Idx+1)	
	{
		sprintf(szFileName, "X:\\%d.dat", u32Idx);			
		fsAsciiToUnicode(szFileName, suFileName, 1);
		/* Open */ 
		hFile = fsOpenFile(suFileName, NULL, O_CREATE);
		if (hFile > 0)
			;
		else
		{
			sysprintf("Failed to open file: %s (%x)\n", szFileName, hFile);
			free(pu8DstAddr);
			free(pu8ReadAddr);
			return hFile;
		}
		/* Write from SD */ 
		printf("Write to SD : pu8DstAddrTmp = 0x%x\n",((UINT32)pu8DstAddrTmp+(SDCARD_ACCESS_LEN/SDCARD_OPEN_FILE)*u32Idx));
		nStatus = fsWriteFile(hFile, 
								pu8DstAddrTmp+(SDCARD_ACCESS_LEN/SDCARD_OPEN_FILE)*u32Idx, 
								(SDCARD_ACCESS_LEN/SDCARD_OPEN_FILE), 
								&nWriteBytes);
		if (nStatus < 0) 
		{
			sysprintf("Warning found: %d bytes\n", nWriteBytes);	
			free(pu8DstAddr);
			free(pu8ReadAddr);	
			return nStatus;
		}			
		fsCloseFile(hFile);		
	}
	t1 = sysGetTicks(TIMER0);			
	sysprintf("Write End tick = %d\n", t1);
	sysprintf("Write Take  ticks = %d\n", (t1-t0));
	
	
	/* Read Performenc test */
	for(u32Idx=0; u32Idx<SDCARD_OPEN_FILE; u32Idx=u32Idx+1)	
	{
		//sysprintf("Index: %d \n", u32Idx);
		sprintf(szFileName, "X:\\%d.dat", u32Idx);			
		fsAsciiToUnicode(szFileName, suFileName, 1);
		/* Open */ 
		hFile = fsOpenFile(suFileName, NULL, O_RDONLY);
		if (hFile > 0)
			;
		else
		{
			sysprintf("Failed to open file: %s (%x)\n", szFileName, hFile);
			free(pu8DstAddr);
			free(pu8ReadAddr);
			return hFile;
		}
		nLen = 0;
		/* Read to SDRAM */ 
		pu8DstAddrTmp = pu8ReadAddr+(SDCARD_ACCESS_LEN/SDCARD_OPEN_FILE)*u32Idx;
		printf("pu8DstAddrTmp = 0x%x\n",(UINT32)pu8DstAddrTmp);
		while(1)
		{	
			nStatus = fsReadFile(hFile, 
									pu8DstAddrTmp, 
									4096, 
									&nReadBytes);
			if((nReadBytes<4096) || (nReadBytes==0))
			{		
				nLen = nLen+nReadBytes;						
				break;				
			}
			if (nStatus < 0) 
			{
				sysprintf("Warning found: %d bytes\n", nReadBytes);	
				free(pu8DstAddr);
				free(pu8ReadAddr);	
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
		//sysprintf("Index: %d \n", u32Idx);
		sprintf(szFileName, "X:\\%d.dat", u32Idx);			
		fsAsciiToUnicode(szFileName, suFileName, 1);
		fsDeleteFile(suFileName, NULL);				
	}
	nStatus = memcmp(pu8DstAddr, 
						pu8ReadAddr, 
						SDCARD_ACCESS_LEN);
	if(nStatus!=0)
	{
		free(pu8DstAddr);
		free(pu8ReadAddr);
		return ERR_SDCARD_ACCESS_FAIL;					
	}		
	free(pu8DstAddr);
	free(pu8ReadAddr);

	return Successful;							
}
INT EMU_SDCard(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	UINT32 	u32BlockSize, u32FreeSize, u32DiskSize, u32Ypos=0; 
	INT	   	nStatus;
	char 	Array1[64];

	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);

	sprintf(Array1, "[SD Card]");
	DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);	
	nStatus = fsDiskFreeSpace('X', &u32BlockSize, &u32FreeSize, &u32DiskSize);
	if ( nStatus == 0)
	{
		sysprintf("X Disk size = %d KB, Free speace = %d KB\n", u32DiskSize, u32FreeSize);
		sprintf(Array1, "DISK Size = %d KB. Free Size %d KB", u32DiskSize, u32FreeSize);
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,				//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
	}	
	else
	{
		DemoFont_ChangeFontColor(ptFont, 
									0xF800);	
		sysprintf("fsDiskFreeSpace failed!!\n");
		sprintf(Array1, "Get disk information fail");
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
		DemoFont_ChangeFontColor(ptFont, 
									0x001F);							
		goto Sdcard_Err;
	}	
	nStatus = SDCardAccess();	
	if(nStatus==Successful)
	{
		sprintf(Array1, "SD card access pass");
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
	}
	else
	{	
		DemoFont_ChangeFontColor(ptFont, 
									0xF800);	
		sprintf(Array1, "SD card access fail");
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);	
		DemoFont_ChangeFontColor(ptFont, 
									0x001F);								
	}		
Sdcard_Err:	
   	return nStatus;	 
}