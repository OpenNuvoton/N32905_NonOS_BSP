//#include <platform.h>
#include "w55fa93_reg.h"
#include "blt.h"
#include "DrvBLT.h"
#include "FI.h"

#define NULL	0

void WaitOpCompleted(void)
{
//return;
#if 1
	// This may be error because CPU may check status 
	if ((inp32(BLTINTCR) & 0x02) == 0)
	{
		//while((inp32(BLTINTCR) & BLT_INTS) ==0); 
		while((inp32(SET2DA) & BLIT_EN) != 0);		
		outp32(BLTINTCR, inp32(BLTINTCR)); 	
	}
#else
		while((inp32(SET2DA) & BLIT_EN) != 0); 
#endif
	
}

void SI_FlushBlit(void)
{
    while (DrvBLT_GetBusyStatus() !=0 );    
}

void SI_2DBlit(S_FI_BLITOP sBlitOp)
{
    S_DRVBLT_DEST_FB sNewDestFB;
    UINT32 u32BytePerPixel;    
       
    SI_FlushBlit();
    
    DrvBLT_SetFillOP(FALSE);    
    
    if (sBlitOp.sFISrcImage.psARGB8 != 0)
	    DrvBLT_SetColorPalette(0, 256, sBlitOp.sFISrcImage.psARGB8);
	    
#if 1
	// Src Offset is 16.16 formt
	sBlitOp.sFISrcImage.sDrvSrcImage.i32XOffset = sBlitOp.sFISrcImage.sDrvSrcImage.i32XOffset << 16;		
	sBlitOp.sFISrcImage.sDrvSrcImage.i32YOffset = sBlitOp.sFISrcImage.sDrvSrcImage.i32YOffset << 16;	
#endif		    
    DrvBLT_SetSrcImage(sBlitOp.sFISrcImage.sDrvSrcImage);    

	// Remove OFFSETDX and OFFSETDY
	if (inp32(DFMT) == 1)
		u32BytePerPixel = 4;
	else
		u32BytePerPixel	= 2;
		
    sNewDestFB.i32Stride       = sBlitOp.sDestFB.i32Stride;  
    sNewDestFB.i16Width        = sBlitOp.sDestFB.i16Width; 
    sNewDestFB.i16Height       = sBlitOp.sDestFB.i16Height; 

#if STRIDE_FOR_BYTE
	// Change for rowbyte stride
    sNewDestFB.u32FrameBufAddr = sBlitOp.sDestFB.u32FrameBufAddr + sBlitOp.sDestFB.i32YOffset * sBlitOp.sDestFB.i32Stride 
    							 +  sBlitOp.sDestFB.i32XOffset *u32BytePerPixel;
#else
    sNewDestFB.u32FrameBufAddr = sBlitOp.sDestFB.u32FrameBufAddr + sBlitOp.sDestFB.i32YOffset * sBlitOp.sDestFB.i32Stride*u32BytePerPixel 
    							 +  sBlitOp.sDestFB.i32XOffset *u32BytePerPixel; 
#endif    							    
    sNewDestFB.i32XOffset      = 0; 
    sNewDestFB.i32YOffset      = 0;  
    
    DrvBLT_SetDestFrameBuf(sNewDestFB);
    
    if (sBlitOp.psTransform != NULL)
    {
        DrvBLT_SetTransformMatrix(sBlitOp.psTransform->sMatrix);    
      	DrvBLT_SetSrcFormat(sBlitOp.psTransform->eSrcFmt);
      	DrvBLT_SetDisplayFormat(sBlitOp.psTransform->eDestFmt);  	
      	DrvBLT_SetTransformFlag(sBlitOp.psTransform->i32Flag);
      	DrvBLT_SetColorMultiplier(sBlitOp.psTransform->sColorMultiplier);  
      	DrvBLT_SetColorOffset(sBlitOp.psTransform->sColorOffset);   		
      	DrvBLT_SetFillStyle(sBlitOp.psTransform->eFilStyle);
    }  	
    else
    	defaultSetting();

    DrvBLT_Trigger(); 
    
   	WaitOpCompleted(); 	
    
}


void SI_Fill(S_FI_FILLOP sFillOp)
{
    S_DRVBLT_DEST_FB sDestFB;
    S_DRVBLT_ARGB8   sARGB8;
    UINT32 u32BytePerPixel;
    
    
    SI_FlushBlit();
    
    DrvBLT_SetFillOP(TRUE);
    
    sARGB8.u8Blue   = sFillOp.sARGB8.u8Blue;
    sARGB8.u8Green  = sFillOp.sARGB8.u8Green;
    sARGB8.u8Red    = sFillOp.sARGB8.u8Red; 
    sARGB8.u8Alpha  = sFillOp.sARGB8.u8Alpha;             
    DrvBLT_SetARGBFillColor(sARGB8);
    
	if (inp32(DFMT) == 1)
		u32BytePerPixel = 4;
	else
		u32BytePerPixel	= 2;
		
    sDestFB.i32Stride       = sFillOp.i32Stride;  
    sDestFB.i16Width        = sFillOp.sRect.i16Xmax - sFillOp.sRect.i16Xmin; 
    sDestFB.i16Height       = sFillOp.sRect.i16Ymax - sFillOp.sRect.i16Ymin; 

#if STRIDE_FOR_BYTE
	// Change for rowByte stride
    sDestFB.u32FrameBufAddr = sFillOp.u32FBAddr + sFillOp.sRect.i16Ymin*sFillOp.i32Stride +  sFillOp.sRect.i16Xmin *u32BytePerPixel; 
#else
    sDestFB.u32FrameBufAddr = sFillOp.u32FBAddr + sFillOp.sRect.i16Ymin*sFillOp.i32Stride*u32BytePerPixel +  sFillOp.sRect.i16Xmin *u32BytePerPixel;    
#endif    
    sDestFB.i32XOffset      = 0; 
    sDestFB.i32YOffset      = 0;     
    
    DrvBLT_SetDestFrameBuf(sDestFB);
    
    DrvBLT_SetDisplayFormat(sFillOp.eDisplayFmt);
    
    DrvBLT_SetFillAlpha(sFillOp.i32Blend);
    
    DrvBLT_Trigger();
    
    WaitOpCompleted();
}


















