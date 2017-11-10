/***************************************************************************
 *                                                                         									     *
 * Copyright (c) 2008 Nuvoton Technolog. All rights reserved.              					     *
 *                                                                         									     *
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "wblib.h"
#include "nvtfat.h"
#include "AviLib.h"
#include "W55FA93_reg.h"
#include "W55FA93_Vpost.h"
#include "w55fa93_kpi.h"
#include "USB.h"
#include "Font.h"
#include "nvtloader.h"
#include "emuProduct.h"
#include "W55FA93_adc.h"


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

INT EMU_ADC(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	INT	   	u32Ypos=0;
	char 	Array1[64];
	UINT16 	u16Vol;
	INT 	tmp, u32ExtFreq;
	unsigned short	x=0, y=0;
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
	sprintf(Array1, "[TSC & LVD]");
	u32Xpos = (strlen("[TSC & LVD]")+1)*_FONT_OFFSET_;	
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);
						
	u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;					
	sprintf(Array1, "Quit if (x, y) position <(200,200)");
	DemoFont_PaintA(ptFont,					
						0,					
						u32Ypos,			
						Array1);						
	adc_init();
	adc_open(ADC_TS_4WIRE, 320, 240);	
	u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;

	while(1) 
	{		
		if(adc_read(ADC_NONBLOCK, &x, &y))
		{				
			s_sDemo_Rect.u32StartX = _BORDER_WIDTH_;		/* Avoid border */
			s_sDemo_Rect.u32StartY = u32Ypos;
			s_sDemo_Rect.u32EndX = _LCM_WIDTH_ - _BORDER_WIDTH_ -1;										
			s_sDemo_Rect.u32EndY =u32Ypos + _FONT_RECT_HEIGHT_;
			DemoFont_RectClear(ptFont,
								&s_sDemo_Rect);
		
			sprintf(Array1, "Report Position %d, %d", x, y);			
			DemoFont_PaintA(ptFont,				//S_DEMO_FONT* 	ptFont,
							0,					//UINT32		u32x,
							u32Ypos,			//UINT32		u32y,
							Array1);
			if( (x<200) && (y<200) )				
				break;				
		}
		if((u32Timer0_10ms_Count%100)==0)
		{	
			if(adc_normalread(2, &u16Vol)==Successful)
			{
				sysprintf("Report voltage = %x\n", u16Vol);
				s_sDemo_Rect.u32StartX = _BORDER_WIDTH_;		/* Avoid border */
				s_sDemo_Rect.u32StartY = u32Ypos + _FONT_RECT_HEIGHT_;
				s_sDemo_Rect.u32EndX = _LCM_WIDTH_ - _BORDER_WIDTH_ -1;										
				s_sDemo_Rect.u32EndY =u32Ypos + 2*_FONT_RECT_HEIGHT_;
				DemoFont_RectClear(ptFont,
									&s_sDemo_Rect);
			
				sprintf(Array1, "Report Voltage 0x%x", u16Vol);			
				DemoFont_PaintA(ptFont,				//S_DEMO_FONT* 	ptFont,
								0,					//UINT32		u32x,
								u32Ypos+_FONT_RECT_HEIGHT_,			//UINT32		u32y,
								Array1);
			}
		}
		if(u32Timer0_10ms_Count>=1000)
		{
			if((x==0)&&(y==0))
				return ERR_ADC_TIME_OUT;		
			else
				break;	
		}	
	}	
	sysClearTimerEvent(TIMER0, tmp);
	sysprintf("quit adc test...\n");	
	adc_close();		
	return Successful;											
}