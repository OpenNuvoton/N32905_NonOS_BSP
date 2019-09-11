#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_vpost.h"
#include "AviLib.h"
#include "nvtloader.h"
#include "emuProduct.h"

#define MEMORY_ACCESS_LEN	(2*1024*1024)
/*
	Only test the DDR connection 
*/
INT MemoryAccess(void)
{
	PUINT8 	pu8DstAddrTmp, pu8ReadAddrTmp;
	PUINT8 	pu8DstAddr, pu8ReadAddr;
	PUINT8 	pu8DstAddrIdx, pu8ReadAddrIdx; 
	INT 	t0; 
	
	pu8DstAddrTmp = malloc(MEMORY_ACCESS_LEN+0x1F);	/* Allocate 4MB then initialize the buffer for writing*/
	pu8DstAddr = (PUINT8)( ((UINT32)pu8DstAddrTmp +0x1F)& ~0x1F );
	printf("pu8DstAddrTmp = 0x%x\n", (UINT32)pu8DstAddrTmp);
	printf("pu8DstAddr = 0x%x\n", (UINT32)pu8DstAddr);
	if(pu8DstAddr == NULL)
	{
		sysprintf("%s\n","Out of message");	
		return ERR_OUT_MEMORY;
	}	
	pu8ReadAddrTmp = malloc(MEMORY_ACCESS_LEN+0x1F);/* Allocate 4MB then initialize the buffer for reading*/
	pu8ReadAddr = (PUINT8)( ((UINT32)pu8ReadAddrTmp +0x1F)& ~0x1F );
	printf("pu8ReadAddrTmp = 0x%x\n", (UINT32)pu8ReadAddrTmp);
	printf("pu8ReadAddr = 0x%x\n", (UINT32)pu8ReadAddr);

	if(pu8ReadAddr == NULL)
	{
		sysprintf("%s\n","Out of message");	
		free(pu8DstAddrTmp);
		return ERR_OUT_MEMORY;
	}	
	pu8DstAddrIdx = pu8DstAddr;
	for(t0=0; t0<(MEMORY_ACCESS_LEN); t0=t0+1)
	{
		*pu8DstAddrIdx = (t0 & 0xFF)+ ((t0>>8)& 0xFF)+((t0>>16)& 0xFF); 
		pu8DstAddrIdx++;
	}
	
	pu8ReadAddrIdx = pu8ReadAddr;
	for(t0=0; t0<(MEMORY_ACCESS_LEN); t0=t0+1)
	{
		*pu8ReadAddrIdx = (t0 & 0xFF)+ ((t0>>8)& 0xFF)+((t0>>16)& 0xFF); 
		pu8ReadAddrIdx++;
	}
	t0 = memcmp(pu8DstAddr, pu8ReadAddr, MEMORY_ACCESS_LEN);
	free(pu8DstAddrTmp);
	free(pu8ReadAddrTmp);
	if(t0==0)
		return Successful;
	else
		return ERR_SDRAM_FAIL;
}
/* Test 2MB */
INT EMU_Memory(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	UINT32 	u32Ypos=0; 
	char 	Array1[64];
	INT 	nStatus;
	
	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);
	sprintf(Array1, "[DDR]");
	DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
	nStatus = MemoryAccess();
	u32Ypos = 10*_FONT_RECT_HEIGHT_;
	if(nStatus==Successful)					
		sprintf(Array1, "SDRAM Access Pass");
	else
		sprintf(Array1, "SDRAM Access fail: Error Code 0x%x", nStatus);
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);	
							
	return nStatus;
}