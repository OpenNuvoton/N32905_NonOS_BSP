#include <stdio.h>
#include "w55fa93_reg.h"
#include "blt.h"
#include "DrvBLT.h"
#include "FI.h"

#define RECT_TEST_STEP_SIZE 1

void ClearFrameBuffer(void)
{
	// Fill Frame Buffer with Black color
	 FillFrameBuffer(VPOST_DISPLAY_ADDR, VPOST_WIDTH, VPOST_HEIGHT, gDisplayFormat, 0x00, 0x00, 0x00, 0xff);
}


void FillFrameBuffer(UINT32 u32BufAddr, UINT32 Width, UINT32 Height, E_DRVBLT_DISPLAY_FORMAT eDestFmt,UINT8 u8R, UINT8 u8G, UINT8 u8B, UINT8 u8A)
{
    S_FI_FILLOP s_ClearOP;
    
    DrvBLT_SetRevealAlpha(eDRVBLT_NO_EFFECTIVE);
       
    s_ClearOP.sRect.i16Xmin = 0;
    s_ClearOP.sRect.i16Ymin = 0;  
    s_ClearOP.sRect.i16Xmax = Width;
    s_ClearOP.sRect.i16Ymax = Height;      
    
    s_ClearOP.sARGB8.u8Blue=u8B;
    s_ClearOP.sARGB8.u8Green=u8G;
    s_ClearOP.sARGB8.u8Red=u8R;   
    s_ClearOP.sARGB8.u8Alpha=u8A;            
    
    s_ClearOP.u32FBAddr = u32BufAddr;
    //s_ClearOP.i32Stride = Width;
    s_ClearOP.i32Stride = GetDestRowByte(eDestFmt,Width);
    s_ClearOP.eDisplayFmt = eDestFmt;
    s_ClearOP.i32Blend = 0;
    
    SI_Fill(s_ClearOP);
     
}

void PrintMessage(void)
{
}

void CompareDataforFill(void)
{
 
}


void FillOpTest(void)
{
#if 1
	CompareDataforFill();
#endif
    // No blend
    ClearFrameBuffer();
   
}















