#include <stdio.h>
#include "w55fa93_reg.h"
#include "blt.h"
#include "DrvBLT.h"
#include "FI.h"

UINT32	gDisplayFormat = eDRVBLT_DEST_RGB565;

 
void defaultSetting(void)
{
	outp32(MLTA, 0x00000100);
	outp32(MLTR, 0x00000100);
	outp32(MLTG, 0x00000100);
	outp32(MLTB, 0x00000100);

	outp32(ELEMENTA, 0x00010000);	
	outp32(ELEMENTB, 0x00000000);
	outp32(ELEMENTC, 0x00000000);
	outp32(ELEMENTD, 0x00010000);	
}

UINT32 GetSrcRowByte(E_DRVBLT_BMPIXEL_FORMAT eSrcFmt, UINT32 u32pixelwidth)
{
#if (STRIDE_FOR_BYTE ==0)
	return u32pixelwidth;
#else
    UINT32 u32Byte;
    
    switch (eSrcFmt)
    {
        case eDRVBLT_SRC_ARGB8888:
            return u32pixelwidth* 4;
        case eDRVBLT_SRC_RGB565:
            return u32pixelwidth* 2;
        case eDRVBLT_SRC_1BPP:
            u32Byte = u32pixelwidth /8;
            break;            
        case eDRVBLT_SRC_2BPP:
            u32Byte = u32pixelwidth * 2 /8;
            break;
        case eDRVBLT_SRC_4BPP:
            u32Byte = u32pixelwidth * 4 /8;
            break;
        case eDRVBLT_SRC_8BPP:
            u32Byte = u32pixelwidth;
            break;                                               
    }
    
    return (u32Byte+3)/4*4;    
#endif    
}


UINT32 GetDestRowByte(E_DRVBLT_DISPLAY_FORMAT eDestFmt, UINT32 u32pixelwidth)
{
#if (STRIDE_FOR_BYTE ==0)
	return u32pixelwidth;
#else
    if (eDestFmt == eDRVBLT_DEST_ARGB8888)
        return u32pixelwidth * 4;
    else    
        return u32pixelwidth * 2;    
#endif        
}


















