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
#include "w55fa93_sic.h"
#include "emuProduct.h"


static NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};

static NDISK_T ptNDisk;


INT EMU_InitNandFlash(void)
{	
	UINT32 	u32ExtFreq;
	UINT32 	u32TotalSize;
	
	
	u32ExtFreq = sysGetExternalClock();
	fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);
	fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);
	
	sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);  /* clock from PLL */   
	sicOpen();

	if(GNAND_InitNAND(&_nandDiskDriver0, &ptNDisk, TRUE) < 0) 
	{
		sysprintf("GNAND_InitNAND error\n");
		return ERR_GNAND_INIT_FAIL;
	}	
	if(GNAND_MountNandDisk(&ptNDisk) < 0) 
	{
		sysprintf("GNAND_MountNandDisk error\n");
		return ERR_GNAND_MOUNT_FAIL;
	}

	fsSetVolumeLabel('C', "NAND1-1\n", strlen("NAND1-1"));
    fsSetVolumeLabel('D', "NAND1-2\n", strlen("NAND1-2"));
	
	u32TotalSize = ptNDisk.nZone* ptNDisk.nLBPerZone*ptNDisk.nPagePerBlock*ptNDisk.nPageSize;
	sysprintf("Total Disk Size %d\n", u32TotalSize);	
	
	/* Not to format NAND flash*/
	/*
	if ((fsDiskFreeSpace('C', &block_size, &free_size, &disk_size) < 0) || 
	    (fsDiskFreeSpace('D', &block_size, &free_size, &disk_size) < 0)) 			    
	{   
		sysprintf("unknow disk type\n");	
		return ERR_UNKNOW_DISK;		  
	}
	*/
	
	return Successful;
}
INT NandDiskInformation(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	UINT32 	u32BlockSize, u32FreeSize, u32DiskSize; 
	char 	Array1[64];
	INT 	nStatus, u32Ypos=0;

	nStatus = fsDiskFreeSpace('C', &u32BlockSize, &u32FreeSize, &u32DiskSize);
	if ( nStatus == 0)
	{
		sysprintf("C Disk size = %d KB, Free speace = %d KB\n", u32DiskSize, u32FreeSize);
		sprintf(Array1, "DISK Size = %d KB. Free Size %d KB", u32DiskSize, u32FreeSize);
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,				//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
	}	
	else
	{
		sysprintf("fsDiskFreeSpace failed!!\n");
		sprintf(Array1, "Get C disk information fail");
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
		goto NANDInfo_Err;
	}	
	
	nStatus = fsDiskFreeSpace('D', &u32BlockSize, &u32FreeSize, &u32DiskSize);
	if ( nStatus == 0)
	{
		sysprintf("D Disk size = %d KB, Free speace = %d KB\n", u32DiskSize, u32FreeSize);
		sprintf(Array1, "DISK Size = %d KB. Free Size %d KB", u32DiskSize, u32FreeSize);
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,				//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
	}	
	else
	{
		sysprintf("fsDiskFreeSpace failed!!\n");
		sprintf(Array1, "Get D disk information fail");
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
		goto NANDInfo_Err;
	}	

	return Successful;	
NANDInfo_Err:
	return ERR_UNKNOW_DISK;
		
}		

INT NandFlashAccess(void)
{
	CHAR    szFileName[32];
	CHAR    suFileName[128];	
	UINT32 	u32Idx;
	PUINT8 	pu8DstAddr, pu8ReadAddr;
	PUINT8 	pu8DstAddrTmp; 
	INT		hFile, nStatus, nReadBytes, nWriteBytes;
	INT 	t0, t1, nLen, nDriver; 
	
	pu8DstAddr = malloc(3*128*1024);	/* Allocate 3*128 KB then initialize the buffer for writing*/
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
	for(nDriver='C'; nDriver<='D'; nDriver=nDriver+1)
	{
		for(u32Idx=0; u32Idx<=2; u32Idx=u32Idx+1)	
		{
			sprintf(szFileName, "%c:\\%d.dat", nDriver, u32Idx);			
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
			//sysprintf("Index: %d \n", u32Idx);
			sprintf(szFileName, "%c:\\%d.dat", nDriver, u32Idx);			
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
			sprintf(szFileName, "%c:\\%d.dat", nDriver, u32Idx);			
			fsAsciiToUnicode(szFileName, suFileName, 1);
			fsDeleteFile(suFileName, NULL);				
		}
	}
	free(pu8DstAddr);
	free(pu8ReadAddr);
	return Successful;							
}
INT EMU_NandFlash(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	UINT32 	u32Ypos=0; 
	INT	   	nStatus;
	char 	Array1[64];
	
	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);
	sprintf(Array1, "[NAND]");
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);	
	nStatus = EMU_InitNandFlash();			
	if(nStatus==Successful)
	{
		sysprintf("NAND Init successful\n");	
		nStatus = NandDiskInformation(ptFont, u32FrameBufAddr);
		if ( nStatus != Successful)
		{
			DemoFont_ChangeFontColor(ptFont, 
										0xF800);											
			u32Ypos = 10*_FONT_RECT_HEIGHT_;
			sprintf(Array1, "NAND Info fail: Err code = 0x%x\n", nStatus);
			DemoFont_PaintA(ptFont,				
							0,					
							u32Ypos,			
							Array1);
			DemoFont_ChangeFontColor(ptFont, 
										0x001F);						
			nStatus = ERR_NAND_INFO_FAIL; 
			goto E_NAND_FINISH;	
		}					
	}
	else
	{
		DemoFont_ChangeFontColor(ptFont, 
										0xF800);		
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		sprintf(Array1, "NAND Init fail: Err code = 0x%x\n", nStatus);
		DemoFont_PaintA(ptFont,				
						0,					
						u32Ypos,			
						Array1);
		DemoFont_ChangeFontColor(ptFont, 
									0x001F);				
		nStatus = ERR_NAND_INIT_FAIL;
		goto E_NAND_FINISH; 											
	}	
		
	nStatus = NandFlashAccess();	
	if(nStatus==Successful)
	{
		sprintf(Array1, "NAND access pass");
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);
		nStatus = Successful;						
	}
	else
	{
		DemoFont_ChangeFontColor(ptFont, 
									0xF800);	
		sprintf(Array1, "NAND access fail");
		u32Ypos = 10*_FONT_RECT_HEIGHT_;
		DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);
		DemoFont_ChangeFontColor(ptFont, 
									0x001F);					
		nStatus = ERR_NAND_ACCESS_FAIL;		 						
	}	
E_NAND_FINISH:
   	GNAND_UnMountNandDisk(&ptNDisk);
    	fmiSMClose(0);
	sicClose();
    	return nStatus;
}