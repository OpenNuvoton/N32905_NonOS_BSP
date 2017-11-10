#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "w55fa93_reg.h"
#include "wblib.h"
#include "w55fa93_sic.h"
#include "w55fa93_gnand.h"
#include "nvtfat.h"
#include "Font.h"
#include "writer.h"


extern S_DEMO_FONT s_sDemo_Font;

int font_x=0, font_y=16;
UINT32 u32SkipX;


#if 1
#define dbgprintf sysprintf
#else
#define dbgprintf(...)
#endif




char pstrDisp[26][32] = {
 {"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"},
 {"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"},
 {"CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"},
 {"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"},
 {"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
 {"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"},
 {"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"},
 {"HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH"},
 {"IIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"},
 {"JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ"},
 {"KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK"},
 {"LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"},
 {"MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"},
 {"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN"},
 {"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"},
 {"PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP"},
 {"QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"},
 {"RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"},
 {"SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS"},
 {"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"},
 {"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"},
 {"VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"},
 {"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"},
 {"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
 {"YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY"},
 {"ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"}
 };





/**********************************/
int main()
{
    DateTime_T ltime;

   int j, Next_Font_Height, i;
   UINT32 wait_ticks, no, dispno, dispcolor;

    //--- Reset SIC engine to make sure it under normal status.
    outp32(REG_AHBCLK, inp32(REG_AHBCLK) | (SIC_CKE | NAND_CKE | SD_CKE));
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) | SICRST);     // SIC engine reset is avtive
    outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) & ~SICRST);    // SIC engine reset is no active. Reset completed.

    /* enable cache */
    sysDisableCache();
    sysInvalidCache();
    sysEnableCache(CACHE_WRITE_BACK);

    /* configure Timer0 for FMI library */
    sysSetTimerReferenceClock(TIMER0, 12000000);
    sysStartTimer(TIMER0, 100, PERIODIC_MODE);

    sysprintf("=====> W55FA93 NandWriter (v%d.%d) Begin [%d] <=====\n", MAJOR_VERSION_NUM, MINOR_VERSION_NUM, sysGetTicks(0));

    ltime.year = 2011;
    ltime.mon  = 10;
    ltime.day  = 31;
    ltime.hour = 8;
    ltime.min  = 40;
    ltime.sec  = 0;
    sysSetLocalTime(ltime);

    Draw_Init();
    font_y = g_Font_Height;
    Next_Font_Height = g_Font_Height-6;
    
    i= 0;
    while (1)
    {
        no=i%26;
        dispno = i % 3;
        if (dispno == 0 )
           dispcolor = COLOR_RGB16_RED;
        else if ( dispno == 1 )
           dispcolor = COLOR_RGB16_GREEN;
        else
           dispcolor = COLOR_RGB16_BLUE;      
		DemoFont_ChangeFontColor(&s_sDemo_Font, dispcolor);
		for (j= 0;j<8; j++)
		{
			DemoFont_PaintA(&s_sDemo_Font,				
				font_x,								
				font_y+ j*g_Font_Height,						
	    		pstrDisp[no]);	
	    }	    	
		wait_ticks = sysGetTicks(TIMER0);
		while( 1 )
		{
			if((sysGetTicks(TIMER0) - wait_ticks) > 200)
				break;    	
	    }
        DemoFont_ChangeFontColor(&s_sDemo_Font, COLOR_RGB16_BLACK);
		for (j= 0;j<8; j++)
		{
			DemoFont_PaintA(&s_sDemo_Font,				
				font_x,								
				font_y+ j*g_Font_Height,						
	    		pstrDisp[no]);	
	    }	    		    
	    i++;
    }	    
    u32SkipX = 14;


    while(1);
}
