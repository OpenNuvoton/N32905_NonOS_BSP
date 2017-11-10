#include "Font.h"
void Draw_InitialBorder(S_DEMO_FONT* ptFont)
{	
	S_DEMO_RECT s_sDemo_Rect;
	UINT16 u16FontColor; 
	
	u16FontColor	= DemoFont_GetFontColor(ptFont);
	DemoFont_ChangeFontColor(ptFont, 
								0xFFE0);
	s_sDemo_Rect.u32StartX =0;
	s_sDemo_Rect.u32StartY = 0;
	s_sDemo_Rect.u32EndX = _LCM_WIDTH_-1,
	s_sDemo_Rect.u32EndY = _LCM_HEIGHT_-1;			
	DemoFont_Border(ptFont,
					&s_sDemo_Rect,
						_BORDER_WIDTH_);
	DemoFont_ChangeFontColor(ptFont, 
								u16FontColor);						
}					
void Draw_ItemBorder(S_DEMO_FONT* ptFont)
{	
	S_DEMO_RECT s_sDemo_Rect;
	UINT16 u16FontColor; 
	
	u16FontColor	= DemoFont_GetFontColor(ptFont);
	DemoFont_ChangeFontColor(ptFont, 
								0xFFE0);
	s_sDemo_Rect.u32StartX =0;
	s_sDemo_Rect.u32StartY = 0;
	s_sDemo_Rect.u32EndX = _LCM_WIDTH_-1,
	s_sDemo_Rect.u32EndY = _FONT_RECT_HEIGHT_-1;			
	DemoFont_Border(ptFont,
					&s_sDemo_Rect,
						_BORDER_WIDTH_);	
	DemoFont_ChangeFontColor(ptFont, 
								u16FontColor);					
}