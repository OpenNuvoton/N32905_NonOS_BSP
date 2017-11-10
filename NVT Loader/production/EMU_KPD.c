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
#include "w55fa93_kpi.h"
#include "W55FA93_reg.h"
#include "Font.h"
#include "emuProduct.h" 

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

INT EMU_KPI(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	//UINT32 	u32BlockSize, u32FreeSize, u32DiskSize;
	UINT32 	u32Ypos=0, u32ExtFreq; 
	INT	   	result=0, tmp;
	char 	Array1[64];
	S_DEMO_RECT s_sDemo_Rect;

	ptTmpFont = ptFont;
	sysSetLocalInterrupt(ENABLE_IRQ);	
	u32Timer0_10ms_Count =0;	
	u32ExtFreq = sysGetExternalClock();
	sysSetTimerReferenceClock (TIMER0, u32ExtFreq*1000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);
	tmp = sysSetTimerEvent(TIMER0, 1, (PVOID)Timer0_Callback);	// 1 ticks = 0.01s call back 
	
	
	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);
	sprintf(Array1, "[KPI]");
	u32Xpos = (strlen("[KPI]")+1)*_FONT_OFFSET_;
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);
	sprintf(Array1, "Key scan code (Quit if Pressing [HOME] key)" );
	u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);						
	u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;
	while(result != HOME_KEY)
	{
		result = kpi_read(KPI_NONBLOCK);				
		if(result!=0)
		{
			s_sDemo_Rect.u32StartX = 10;		/* Avoid board */
			s_sDemo_Rect.u32StartY = u32Ypos;
			s_sDemo_Rect.u32EndX = s_sDemo_Rect.u32StartX + 
										4*_FONT_RECT_WIDTH_;	/* Clear 4 character = 0x?? */
			s_sDemo_Rect.u32EndY =u32Ypos + _FONT_RECT_HEIGHT_;
			DemoFont_RectClear(ptFont,
								&s_sDemo_Rect);
			sprintf(Array1, "0x%x",result);									
			DemoFont_PaintA(ptFont,					
							4,					
							u32Ypos,			
							Array1);
		}
		if(u32Timer0_10ms_Count>=1000)
		{
			if(result==0)
				return ERR_KPI_TIME_OUT;							
			else
				break;
		}	
	}	
	sysClearTimerEvent(TIMER0, tmp);
	sysprintf("quit KPI test...\n");	
	return Successful;												
}						