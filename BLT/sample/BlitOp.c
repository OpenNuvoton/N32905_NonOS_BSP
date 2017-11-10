#include "w55fa93_reg.h"
#include "blt.h"
#include "DrvBLT.h"
#include "FI.h"
#include "palette.h"

#define	 NULL	0
extern UINT8 Vpost_Pattern[];

void CopySrcImageToFB(void)
{
	BlitCopyImageToFB(SRC_IMAGE_ADDR, 0, 0, VPOST_WIDTH, VPOST_HEIGHT, gDisplayFormat);
}

void CopyDestImageToFB(void)
{
	//DrvBLT_SetDisplayFormat(gDisplayFormat);
	DrvBLT_SetSrcFormat(eDRVBLT_SRC_RGB565);
	BlitCopyImageToFB(SRC_DEST2_ADDR, 0, 0, VPOST_WIDTH, VPOST_HEIGHT, gDisplayFormat);
}

//void BlitCopySrcImageToFB(UINT32 u32Xoffset, UINT32 u32Yoffset, UINT32 Width, UINT32 Height, E_DRVBLT_DISPLAY_FORMAT eDestFmt)
void BlitCopyImageToFB(UINT32 u32SrcAddr, UINT32 u32Xoffset, UINT32 u32Yoffset, UINT32 Width, UINT32 Height, E_DRVBLT_DISPLAY_FORMAT eDestFmt)
{
    S_FI_BLITOP sBlitOpPara;
    
    DrvBLT_SetRevealAlpha(eDRVBLT_NO_EFFECTIVE);
    
    sBlitOpPara.sFISrcImage.psARGB8 = default_pallete;
    sBlitOpPara.sFISrcImage.sDrvSrcImage.u32SrcImageAddr = u32SrcAddr;
    //sBlitOpPara.sFISrcImage.sDrvSrcImage.i32Stride = VPOST_WIDTH; 
    sBlitOpPara.sFISrcImage.sDrvSrcImage.i32Stride = GetSrcRowByte(inp32(SFMT), VPOST_WIDTH);  
    sBlitOpPara.sFISrcImage.sDrvSrcImage.i32XOffset = u32Xoffset;  
    sBlitOpPara.sFISrcImage.sDrvSrcImage.i32YOffset = u32Yoffset;       
    sBlitOpPara.sFISrcImage.sDrvSrcImage.i16Width = Width;  
    sBlitOpPara.sFISrcImage.sDrvSrcImage.i16Height = Height;  
    
    sBlitOpPara.sDestFB.u32FrameBufAddr = SRC_DEST_ADDR;
    sBlitOpPara.sDestFB.i32XOffset = u32Xoffset;  
    sBlitOpPara.sDestFB.i32YOffset = u32Yoffset;   
    //sBlitOpPara.sDestFB.i32Stride = VPOST_WIDTH;   
    sBlitOpPara.sDestFB.i32Stride = GetDestRowByte(inp32(DFMT), VPOST_WIDTH);            
    sBlitOpPara.sDestFB.i16Width = Width;  
    sBlitOpPara.sDestFB.i16Height = Height;       
           
    sBlitOpPara.psTransform = NULL;           

    
    SI_2DBlit(sBlitOpPara);
     
}


void BlitOpCopy(S_FI_BLITTRANSFORMATION* psTran, UINT32 step)
{
    S_FI_BLITOP sBlitOpPara;    
    int i,j,k,evenline=1,startX;
    INT32  i32Blend=0;
    UINT32 u32SrcFmt, u32DestFmt;
   
    DrvBLT_SetSrcFormat(eDRVBLT_SRC_RGB565);
    
    sBlitOpPara.sFISrcImage.psARGB8 = default_pallete;
    sBlitOpPara.sFISrcImage.sDrvSrcImage.u32SrcImageAddr = (UINT32)&Vpost_Pattern[0];
    sBlitOpPara.sDestFB.u32FrameBufAddr = SRC_DEST_ADDR;    
    sBlitOpPara.psTransform = psTran;     
    
    if (psTran != 0)
        i32Blend=1;
        
    if (psTran != 0)
    {   // Change for new Format
        u32SrcFmt = psTran->eSrcFmt;
        u32DestFmt = psTran->eDestFmt;    
    }
    else
    {   // Keep Origianl Format
        u32SrcFmt = inp32(SFMT);
        u32DestFmt = inp32(DFMT);
    }
            
#if 0                    
    for(i=MIN_RECT_TEST_SIZE;i<=MAX_RECT_TEST_SIZE;i=i+step)
#else
	i = 64;
#endif    
    {
        if (i32Blend)
            CopyDestImageToFB();
        else
            ClearFrameBuffer();
        
        for(j=0;(j+i)<=VPOST_HEIGHT;j=j+i)
        {
            if (evenline)
            {
                startX=0;
                evenline =0;
            }                
            else
            {
                startX=i;     
                evenline =1;                
            }                
            
            for(k=startX;(k+i)<=VPOST_WIDTH;k=k+i*2)
            {
                //sBlitOpPara.sFISrcImage.sDrvSrcImage.i32Stride = VPOST_WIDTH;  
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i32Stride = GetSrcRowByte(u32SrcFmt,VPOST_WIDTH);  
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i32XOffset = k;  
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i32YOffset = j;       
#if 0                
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i16Width = i;  
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i16Height = i;
#else
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i16Width = VPOST_WIDTH;  
                sBlitOpPara.sFISrcImage.sDrvSrcImage.i16Height = VPOST_HEIGHT;
#endif                
                
                sBlitOpPara.sDestFB.i32XOffset = k;  
                sBlitOpPara.sDestFB.i32YOffset = j;   
                //sBlitOpPara.sDestFB.i32Stride = VPOST_WIDTH;         
                sBlitOpPara.sDestFB.i32Stride = GetDestRowByte(u32DestFmt,VPOST_WIDTH);
                sBlitOpPara.sDestFB.i16Width = i;  
                sBlitOpPara.sDestFB.i16Height = i;                 
                
                SI_2DBlit(sBlitOpPara); 
            }
        }   
        //BLT_WriteFile("BlitCase1",i, SRC_DEST_ADDR);	   
    }
}


void BlitOpTest(void)
{
    // Only Copy Src Image to Destination FB
    ClearFrameBuffer();
#if 0    
    CopyDestImageToFB();
#else    
	BlitOpCopy(0, 16);
    //DrvBLT_SetSrcFormat(eDRVBLT_SRC_RGB565);
	//BlitCopyImageToFB(SRC_DEST2_ADDR, 50, 30, VPOST_WIDTH, VPOST_HEIGHT, gDisplayFormat);
#endif	
      
    
}























