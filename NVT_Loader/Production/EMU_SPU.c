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
#include "SONG.h"
#include "DrvSPU.h"
#include "spu.h"
#include "Font.h"
#include "emuProduct.h" 

static UINT16 u16IntCount = 2;
static UINT32 u32FragSize;
static volatile UINT8 bPlaying = TRUE;

int playCallBack(UINT8 * pu8Buffer)
{	
	UINT32 u32Offset = 0;
	UINT32 len = sizeof(SPU_SOURCE);
	
	u32Offset = ( u32FragSize / 2) * u16IntCount;
	if (u32Offset >= len)		// Reach the end of the song, restart from beginning
	{		
		u16IntCount = 2;
		u32Offset = 0;
		bPlaying = FALSE;
		return TRUE;
	}	
	
	memcpy(pu8Buffer, &SPU_SOURCE[u32Offset], u32FragSize/2);
	
	u16IntCount++;
	
	return FALSE;
}



INT EMU_SPU(
	S_DEMO_FONT* 	ptFont,
	UINT32 			u32FrameBufAddr	
)	
{
	INT32 idx;
	
	UINT32 u32Ypos=0, u32Volume; 
	char 	Array1[64];
	S_DEMO_RECT s_sDemo_Rect;
	
	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);
	sprintf(Array1, "[SPU]");
	DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
						
	u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;					
	//for(idx=0x0; idx <0x3F; idx=idx+9) 
	for(idx=0x2; idx <=0x3F; idx=idx+0x1e) 
	{//Volume form 0 to 0x3F
		sprintf(Array1, "Playback SPU...");
		DemoFont_PaintA(ptFont,				//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
		#if 0
		for(u32Volume=0; u32Volume<=idx; u32Volume= u32Volume+9)  
		{
			s_sDemo_Rect.u32StartX = _BORDER_WIDTH_+u32Volume;
			s_sDemo_Rect.u32StartY = u32Ypos + _FONT_RECT_HEIGHT_;
			s_sDemo_Rect.u32EndX = _BORDER_WIDTH_ + 2 +u32Volume,
			s_sDemo_Rect.u32EndY =u32Ypos + 2*_FONT_RECT_HEIGHT_;		
			DemoFont_Rect(ptFont,
							&s_sDemo_Rect);		
		}
		#else
		s_sDemo_Rect.u32StartX = _BORDER_WIDTH_;
		s_sDemo_Rect.u32StartY = u32Ypos + _FONT_RECT_HEIGHT_;
		s_sDemo_Rect.u32EndX = _BORDER_WIDTH_ + idx,
		s_sDemo_Rect.u32EndY =u32Ypos + 2*_FONT_RECT_HEIGHT_;		
		DemoFont_Rect(ptFont,
						&s_sDemo_Rect);			
		#endif	
		spuOpen(eDRVSPU_FREQ_8000);
		spuIoctl(SPU_IOCTL_SET_VOLUME, idx, idx);
		//spuIoctl(SPU_IOCTL_SET_VOLUME, 0x3f, 0x3f);
		spuIoctl(SPU_IOCTL_GET_FRAG_SIZE, (UINT32)&u32FragSize, 0);
		
		spuEqOpen(eDRVSPU_EQBAND_2, eDRVSPU_EQGAIN_P7DB);
		spuDacOn(1);
	    spuStartPlay((PFN_DRVSPU_CB_FUNC *) playCallBack, (UINT8 *)SPU_SOURCE);		

	   	while(bPlaying == TRUE);	
		
	    spuEqClose();	
	    spuClose();
		bPlaying = TRUE; 
	}   
	return Successful;
}	 	