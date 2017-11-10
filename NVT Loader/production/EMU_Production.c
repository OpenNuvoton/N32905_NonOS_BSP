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
#include "Font.h"
#include "emuProduct.h"

__align(32) static S_DEMO_FONT s_sDemo_Font;
__align(32) UINT16 FrameBuffer[_LCM_WIDTH_*_LCM_HEIGHT_];

void Timer0_DelayTick(UINT32 u32Tick)
{
	volatile unsigned int btime, etime;
	btime = sysGetTicks(TIMER0);
	while (1)
	{	/* Using timer 1 for time out */		
		etime = sysGetTicks(TIMER0);
		if ((etime - btime) >= u32Tick)		
			break;		
	}
}

#define ITEM_DDR	0
#define ITEM_USBD	1
#define ITEM_ADC	2
#define ITEM_KPI	3
#define ITEM_SPU	4
#define ITEM_VPOST	5
#define ITEM_NAND	6
#define ITEM_SD		7
#define ITEM_USBH	8
#define ITEM_NONE	(ITEM_USBH+1)
static UINT32 au32Result[ITEM_NONE]={0}; 


typedef struct 
{
	char* ucItem;
	UINT32 u32ErrorCode;
}ITEM_INFO_T;

ITEM_INFO_T ItemInfo[ITEM_NONE]={"DDR:",0,
									"USBD:",0,
									"ADC:",0,	
									"KPI:",0,
									"SPU:",0,
									"VPOST:",0,
									"NAND:",0,
									"SD:",0,
									"USBH:",0				
									};
void EMU_MassProduction(void)
{
	//UINT16 u16Color[]={0xF800, 0x07E0, 0x001F, 0xF81F};
	//UINT32 u32Idx;
	//EMU_InitTask();
	INT32 	i32Idx, i32ErrCode;
	char Array1[64];
	UINT32 u32KpiReport;
	UINT32 u32Ypos;
	

	i32ErrCode = EMU_InitSdCard();									
	EMU_InitFont(&s_sDemo_Font,
				(UINT32)FB_ADDR);				
	kpi_init();
	kpi_open(3); // use nIRQ0 as external interrupt source
	u32KpiReport = kpi_read(KPI_NONBLOCK);
	sysprintf("Key scan code = 0x%x\n", u32KpiReport);			
	if(u32KpiReport == 0x91)
	//if(1)
	{//HOME | UP | ENTER 
		Font_ClrFrameBuffer((UINT32)FB_ADDR);				

	/*	
		EMU_AIC(&s_sDemo_Font,
						(UINT32)FrameBuffer);	
							
		EMU_InitTimer0(&s_sDemo_Font,
						(UINT32)FrameBuffer);	
																
		EMU_InitTimer1(&s_sDemo_Font,
						(UINT32)FrameBuffer);	
	*/												
		for(i32Idx = 0; i32Idx < ITEM_NONE ;i32Idx=i32Idx+1)
			au32Result[i32Idx]=0x1;
						
		DemoFont_ChangeFontColor(&s_sDemo_Font,
											0x07F0);
	
	#ifdef _EMU_SDCard_		
		if(i32ErrCode==Successful)						
			au32Result[ITEM_SD] = EMU_SDCard(&s_sDemo_Font,
					(UINT32)FB_ADDR);	
		else
			au32Result[ITEM_SD]	= i32ErrCode;		
	#endif
					
	#ifdef _EMU_ADC_															
		au32Result[ITEM_ADC] = EMU_ADC(&s_sDemo_Font,
					(UINT32)FB_ADDR);	
	#endif				
	#ifdef _EMU_DDR_
		au32Result[ITEM_DDR] = EMU_Memory(&s_sDemo_Font,
						(UINT32)FB_ADDR);
	#endif				
	#ifdef _EMU_USBD_		
		au32Result[ITEM_USBD] = EMU_USBD(&s_sDemo_Font,
					(UINT32)FB_ADDR);
	#endif				
	#ifdef _EMU_SPU_																						
		au32Result[ITEM_KPI] = EMU_KPI(&s_sDemo_Font,
					(UINT32)FB_ADDR);					
				
	#endif				
	#ifdef _EMU_SPU_	
		au32Result[ITEM_SPU] = EMU_SPU(&s_sDemo_Font,
					(UINT32)FB_ADDR);
	#endif				
	#ifdef _EMU_VPOST_		
		au32Result[ITEM_VPOST] = EMU_VPOST(&s_sDemo_Font,
					(UINT32)FB_ADDR);						
	#endif 				
	#ifdef _EMU_NandFlash_		
		au32Result[ITEM_NAND] = EMU_NandFlash(&s_sDemo_Font,
					(UINT32)FB_ADDR);
	#endif		
	#ifdef _EMU_USBH_	
		au32Result[ITEM_USBH] = EMU_USBH(&s_sDemo_Font,
					(UINT32)FB_ADDR);
	#endif 
		
		u32Ypos = 0;			
		Font_ClrFrameBuffer((UINT32)FB_ADDR);
		Draw_InitialBorder(&s_sDemo_Font);	
		Draw_ItemBorder(&s_sDemo_Font);
		sprintf(Array1, "FA93 Production Test Code");
		DemoFont_PaintA(&s_sDemo_Font,					
						0,					
						u32Ypos,			
						Array1);
		u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;	
						
		for(i32Idx = 0; i32Idx < ITEM_NONE; i32Idx=i32Idx+1)
		{
			DemoFont_ChangeFontColor(&s_sDemo_Font,
											0x001F);	
			if(au32Result[i32Idx]!=Successful)
			{
				sprintf(Array1, ItemInfo[i32Idx].ucItem);									
				DemoFont_PaintA(&s_sDemo_Font,					
							4,					
							u32Ypos,			
							Array1);
				if((au32Result[i32Idx]==ERR_ADC_TIME_OUT) || 
					(au32Result[i32Idx]==ERR_KPI_TIME_OUT) || 
					(au32Result[i32Idx]==ERR_USBD_TIME_OUT) ||
					(au32Result[i32Idx]==ERR_USBH_TIME_OUT))
				{			
					DemoFont_ChangeFontColor(&s_sDemo_Font,
												0xF800);							
					sprintf(Array1, "Time Out");													
				}
				else if (au32Result[i32Idx] == 0x1)
				{//Default value
					DemoFont_ChangeFontColor(&s_sDemo_Font,
												0xF800);							
					sprintf(Array1, "Not to Test");	
				}
				else
				{
					DemoFont_ChangeFontColor(&s_sDemo_Font,
												0xF800);							
					sprintf(Array1, "FAIL");	
				}
				DemoFont_PaintA(&s_sDemo_Font,					
							70,								/* Max 6 char(VPOST:) = (6+1)*_FONT_OFFSET_ */ 		
							u32Ypos,			
							Array1);										
			}
			else
			{
				sprintf(Array1, ItemInfo[i32Idx].ucItem);									
				DemoFont_PaintA(&s_sDemo_Font,					
							4,					
							u32Ypos,			
							Array1);
				DemoFont_ChangeFontColor(&s_sDemo_Font,
											0x07F0);	
				sprintf(Array1, "PASS");																	
				DemoFont_PaintA(&s_sDemo_Font,					
							70,					
							u32Ypos,			
							Array1);									
			}
			u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;	
		}	
		u32KpiReport = kpi_read(KPI_BLOCK);																																																														
	}		
						
}	
UINT32 EMU_ReadKey(void)
{
	int result;
	result = kpi_read(KPI_NONBLOCK);	
	return result;
}	
