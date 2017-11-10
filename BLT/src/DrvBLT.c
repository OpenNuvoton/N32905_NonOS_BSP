//--------------------------------------------------------------
//
// Copyright (c) Nuvoton Technology Corp. All rights reserved.
//
//--------------------------------------------------------------

#include "w55fa93_reg.h"
#include "blt.h"
#include "DrvBLT.h"
//#include "DrvAIC.h"

static PFN_DRVBLT_CALLBACK  g_pfnBltCallback =0;
/*
#define	E_SUCCESS	0
#define	DRVBLT_VERSION_NUM	1

#define	ELEMENTA REG_ELEMENTA
#define	ELEMENTB REG_ELEMENTB
#define	ELEMENTC REG_ELEMENTC
#define	ELEMENTD REG_ELEMENTD
#define	SFMT	 REG_SFMT
#define	DFMT	 REG_DFMT
#define	BLTINTCR	REG_BLTINTCR
#define	MLTB		REG_MLTB
#define	MLTG		REG_MLTG
#define	MLTR		REG_MLTR
#define	MLTA		REG_MLTA

#define	SADDR		REG_SADDR
#define	SSTRIDE		REG_SSTRIDE
#define	OFFSETSX	REG_OFFSETSX
#define	OFFSETSY	REG_OFFSETSY
#define	SWIDTH		REG_SWIDTH
#define	SHEIGHT		REG_SHEIGHT
#define	DADDR		REG_DADDR
#define	DSTRIDE		REG_DSTRIDE
#define	DWIDTH		REG_DWIDTH
#define	DHEIGHT		REG_DHEIGHT
#define FILLARGB	REG_FILLARGB
#define SET2DA		REG_SET2DA
#define	PALETTE	    REG_PALETTE
#define TR_RGB565	0xFFFF
#define	TRCOLOR		REG_TRCOLOR

*/





UINT32
DrvBLT_Open(void)
{
	// 1.Check I/O pins. If I/O pins are used by other IPs, return error code.
	// 2.Enable IP¡¦s clock
		outp32(REG_AHBCLK, inp32(REG_AHBCLK) | 0x01010020);		// enable HCLK4, HCLK3 and BLT clock
		//outp32(0xb0000208, inp32(0xb0000208) | 0x00000300);		// enable Timer		
	// 3.Reset IP
	outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) | BLTRST);
	outp32(REG_AHBIPRST, inp32(REG_AHBIPRST) & ~BLTRST);
	// 4.Configure IP according to inputted arguments.
	// 5.Enable IP I/O pins
	// 6.Return 0 to present success
	return E_SUCCESS;
}	// DrvBLT_Open


void DrvBLT_Close(void)
{
	// 1.Disable IP I/O pins
	// 2.Disable IP¡¦s clock
	//outp32(AHBCLK, inp32(AHBCLK) & ~GE2D_CKE);
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~0x00000020);		// disable BLT clock
}	// DrvBLT_Close

//	Set	Transform Matrix | a c |
//						 | b d |
void DrvBLT_SetTransformMatrix(
	S_DRVBLT_MATRIX sMatrix		// [in] Transformation Matrix 
)
{
	outp32(ELEMENTA, sMatrix.a);
	outp32(ELEMENTB, sMatrix.b);
	outp32(ELEMENTC, sMatrix.c);
	outp32(ELEMENTD, sMatrix.d);	
}

// Get Transform Matrix
void DrvBLT_GetTransformMatrix(
	S_DRVBLT_MATRIX* psMatrix		// [out] Transformation Matrix 
)
{
	psMatrix->a = inp32(ELEMENTA);
	psMatrix->b = inp32(ELEMENTB);
	psMatrix->c = inp32(ELEMENTC);
	psMatrix->d = inp32(ELEMENTD);	
}

//	Set Pixel Format of Source Image
ERRCODE 
DrvBLT_SetSrcFormat(
	E_DRVBLT_BMPIXEL_FORMAT eSrcFmt	// [in] Source Image Format 
)
{
    if (eSrcFmt & ~0x3F)
        return E_DRVBLT_FALSE_INPUT;
        
	outp32(SFMT, eSrcFmt);
	return 	E_SUCCESS;	
}

// Get Pixel Format of Source Image
E_DRVBLT_BMPIXEL_FORMAT 
DrvBLT_GetSrcFormat(void)
{

	return inp32(SFMT) & 0x3F;
}

//	Set Pixel Format of Display 
ERRCODE 
DrvBLT_SetDisplayFormat(
	E_DRVBLT_DISPLAY_FORMAT eDisplayFmt	// [in] Display Format 
)
{
    if (eDisplayFmt & ~0x07)
        return E_DRVBLT_FALSE_INPUT;
        
	outp32(DFMT, eDisplayFmt);        
	return 	E_SUCCESS;	
}

//	Get Pixel Format of Display 
E_DRVBLT_DISPLAY_FORMAT 
DrvBLT_GetDisplayFormat(void)
{
	return inp32(DFMT) & 0x07;
}

//	Enable BitBlt Interrupt
void DrvBLT_EnableInt(void)
{
	outp32(BLTINTCR, inp32(BLTINTCR) | BLT_INTE);
}

//	Disable BitBlt Interrupt
void DrvBLT_DisableInt(void)
{
	outp32(BLTINTCR, inp32(BLTINTCR) & ~BLT_INTE);
}

//	Get Interrupt enable/disable status
BOOL 
DrvBLT_IsIntEnabled(void)
{
	return inp32(BLTINTCR) & BLT_INTE;
}

//	Clear BitBlt Interrupt status
void DrvBLT_ClearInt(void)
{
	outp32(BLTINTCR,  inp32(BLTINTCR) | BLT_INTS);
}

//	Polling BitBlt Interrupt status
BOOL 
DrvBLT_PollInt(void)
{
	return inp32(BLTINTCR & BLT_INTS);
}


// Set A/R/G/B Color Multiplier
void DrvBLT_SetColorMultiplier(
	S_DRVBLT_ARGB16 sARGB16	// [in] ARGB Multiplier 
)
{
/*
	if ((sARGB16.i16Blue & 0xFFFF0000) || (sARGB16.i16Green & 0xFFFF0000) 
		|| (sARGB16.i16Red & 0xFFFF0000) || (sARGB16.i16Alpha & 0xFFFF0000))
        return E_DRVBLT_FALSE_INPUT;	
*/        
	outp32(MLTB, (inp32(MLTB)  & 0xFFFF0000 ) | (sARGB16.i16Blue  & 0x0FFFF));
	outp32(MLTG, (inp32(MLTG)  & 0xFFFF0000 ) | (sARGB16.i16Green & 0x0FFFF));
	outp32(MLTR, (inp32(MLTR)  & 0xFFFF0000 ) | (sARGB16.i16Red   & 0x0FFFF));	        
	outp32(MLTA, (inp32(MLTA)  & 0xFFFF0000 ) | (sARGB16.i16Alpha & 0x0FFFF));
}

// Get A/R/G/B Color Multiplier
void DrvBLT_GetColorMultiplier(
	S_DRVBLT_ARGB16* psARGB16	// [out] ARGB Multiplier 
)
{
    psARGB16->i16Blue 	= inp32(MLTB) & 0xFFFF;
    psARGB16->i16Green 	= inp32(MLTG) & 0xFFFF;
    psARGB16->i16Red 	= inp32(MLTR) & 0xFFFF;
    psARGB16->i16Alpha 	= inp32(MLTA) & 0xFFFF;    
    
}

// Set A/R/G/B Color Offset
void DrvBLT_SetColorOffset(
	S_DRVBLT_ARGB16 sARGB16	// [in] ARGB offset 
)
{

	outp32(MLTB, (inp32(MLTB)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Blue  & 0x0FFFF) << 16));
	outp32(MLTG, (inp32(MLTG)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Green & 0x0FFFF) << 16));
	outp32(MLTR, (inp32(MLTR)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Red   & 0x0FFFF) << 16));	        
	outp32(MLTA, (inp32(MLTA)  & 0x0000FFFF ) | ((UINT32)(sARGB16.i16Alpha & 0x0FFFF) << 16));
      
}

// Get A/R/G/B Color Offset
void DrvBLT_GetColorOffset(
	S_DRVBLT_ARGB16* psARGB16	// [out] ARGB offset 
)
{

	psARGB16->i16Blue 	= inp32(MLTB) >> 16;
	psARGB16->i16Green 	= inp32(MLTG) >> 16;
	psARGB16->i16Red 	= inp32(MLTR) >> 16;
	psARGB16->i16Alpha 	= inp32(MLTA) >> 16;
}

// Set Source Image
void DrvBLT_SetSrcImage(
	S_DRVBLT_SRC_IMAGE sSrcImage	// [in] Source Image Setting
)
{

	outp32(SADDR,    sSrcImage.u32SrcImageAddr);	// Set Source Image Start Addr
	outp32(SSTRIDE,  sSrcImage.i32Stride);			// Set Source Image Stride	
	outp32(OFFSETSX, sSrcImage.i32XOffset);			// Set X offset into the source to start rendering from
	outp32(OFFSETSY, sSrcImage.i32YOffset);			// Set Y offset into the source to start rendering from
	outp32(SWIDTH,   sSrcImage.i16Width);			// Set width of source image
	outp32(SHEIGHT,  sSrcImage.i16Height);			// Set height of source image
}

// Set Destination Frame Buffer
void DrvBLT_SetDestFrameBuf(
	S_DRVBLT_DEST_FB sFrameBuf	// [in] Frame Buffer Setting
)
{
	outp32(DADDR,    sFrameBuf.u32FrameBufAddr);	// Set Frame Buffer Start Addr
	//outp32(OFFSETDX, sFrameBuf.i32XOffset);			// X offset into the frame buffer in pixels
	//outp32(OFFSETDY, sFrameBuf.i32YOffset);			// Y offset into the frame buffer in pixels
	outp32(DSTRIDE,  sFrameBuf.i32Stride);			// Set Frame Buffer Stride
	outp32(DWIDTH,   sFrameBuf.i16Width);			// Set width of Frame Buffer
	outp32(DHEIGHT,  sFrameBuf.i16Height);			// Set height of Frame Buffer
}

//	Set ARGB color for Fill Operation
void DrvBLT_SetARGBFillColor(
	S_DRVBLT_ARGB8 sARGB8	// [in] ARGB value for fill operation
)
{
	UINT32 u32ARGB;
	
	u32ARGB = ((UINT32)sARGB8.u8Alpha << 24) | ((UINT32)sARGB8.u8Red << 16) 
			  | ((UINT32)sARGB8.u8Green << 8) | sARGB8.u8Blue;  
	outp32(FILLARGB, u32ARGB);			  
}

//	Get ARGB color for Fill Operation
void DrvBLT_GetARGBFillColor(
	S_DRVBLT_ARGB8* psARGB8	// [out] ARGB value for fill operation
)
{
	UINT32 u32ARGB;
	
	u32ARGB = inp32(FILLARGB);
	psARGB8->u8Alpha 	= (u32ARGB >> 24) & 0xFF;
	psARGB8->u8Red 		= (u32ARGB >> 16) & 0xFF;
	psARGB8->u8Green 	= (u32ARGB >> 8) & 0xFF;
	psARGB8->u8Blue 	= u32ARGB & 0xFF;	
}

// 	Get BitBlt engine busy(rendering) status
BOOL 
DrvBLT_GetBusyStatus(void)
{
	return inp32(SET2DA) & BLIT_EN;
}

// Set Fill Blending bit
void DrvBLT_SetFillAlpha(BOOL bEnable)
{
	outp32(SET2DA, (inp32(SET2DA) & ~FILL_BLEND) | (bEnable << 2));
}

// Get Fill Blending bit
BOOL 
DrvBLT_GetFillAlpha(void)
{
	return (inp32(SET2DA) & FILL_BLEND) >> 2;
}


// 	Set Transform Flag
void DrvBLT_SetTransformFlag(
	UINT32 u32TransFlag			// [in] A combination of the enum E_DRVBLT_TRANSFORM_FLAG
)
{
	outp32(SET2DA, (inp32(SET2DA) & ~TRANS_FLAG) | (u32TransFlag << 4));
}

// 	Get Transform Flag
UINT32 
DrvBLT_GetTransformFlag(void)
{
	return (inp32(SET2DA)  & TRANS_FLAG) >> 4;
}

// Set Palette Endian
void DrvBLT_SetPaletteEndian(
	E_DRVBLT_PALETTE_ORDER eEndian	// [in] Palette Endian Type
)
{
	outp32(SET2DA, (inp32(SET2DA) & ~L_ENDIAN) | (eEndian << 1));
}

// Get Palette Endian
E_DRVBLT_PALETTE_ORDER 
DrvBLT_GetPaletteEndian(void)
{
	return (inp32(SET2DA) & L_ENDIAN) >> 1;
}

// Set the Color Palette used in BitBlt
// The format pointer by pu32ARGB are [31:24] -> Alpha, [23:16] -> Red, [15:8] -> Green, [7:0] -> Blue
void DrvBLT_SetColorPalette(
	UINT32 u32PaletteInx, 		// [in] Color Palette Start index
	UINT32 u32Num, 				// [in] Color Palette number to set
	S_DRVBLT_ARGB8* psARGB			// [in] pointer for Color palette from u32PaletteInx
)
{
	UINT32 u32PaletteStartInx;
	UINT32 u32ARGB;
	int i;
	
	u32PaletteStartInx = PALETTE + u32PaletteInx;
	for(i=0;i<u32Num;i++)
	{
		u32ARGB = ((UINT32)(psARGB-> u8Alpha) << 24) | ((UINT32)(psARGB-> u8Red) << 16) | 
				  ((UINT32)(psARGB-> u8Green) << 8) | psARGB-> u8Blue; 
		outp32(u32PaletteStartInx+i*4, u32ARGB);
		psARGB++;
	}
}

// Enable/Disable Fill Operation
void DrvBLT_SetFillOP(
	E_DRVBLT_FILLOP eOP		// [in] Enable/Disable FillOP
)
{
	outp32(SET2DA,(inp32(SET2DA) &  ~FILL_OP) | (eOP<<11));
}

// Get Fill or Blit Operation 
BOOL 
DrvBLT_GetFillOP(void)
{
    return (inp32(SET2DA) & FILL_OP) >> 11;
}

// Set Fill stype 
void DrvBLT_SetFillStyle(
	E_DRVBLT_FILL_STYLE eStyle		// [in] Fill Style for Operation
)
{
	outp32(SET2DA,(inp32(SET2DA) &  ~FILL_STYLE) | (eStyle <<8));
}

// Get Fill stype 
E_DRVBLT_FILL_STYLE 
DrvBLT_GetFillStyle(void)
{
	return (inp32(SET2DA) & FILL_STYLE) >> 8;
}

// Enable/Disable Pre-multiply with alpha
void DrvBLT_SetRevealAlpha(
	E_DRVBLT_REVEAL_ALPHA eAlpha		// [in] need / no need un-multiply alpha on source image
)
{
	outp32(SET2DA,(inp32(SET2DA) &  ~S_ALPHA) | (eAlpha << 3));
}

// Get Pre-multiply status
BOOL 
DrvBLT_GetRevealAlpha(void)
{
	return (inp32(SET2DA) & S_ALPHA) >> 3;
}


//	Trigger BitBlt engine to render image
void DrvBLT_Trigger(void)
{
	outp32(SET2DA,inp32(SET2DA) | BLIT_EN);
}

// BitBlt ISR
static void DrvBLT_ISR(void)
{
	if (g_pfnBltCallback != 0)
		(*g_pfnBltCallback)();
	outp32(BLTINTCR,  inp32(BLTINTCR) | BLT_INTS);
}
/*
ERRCODE  
DrvBLT_InstallCallBack(
	PFN_DRVBLT_CALLBACK pfncallback,    
	PFN_DRVBLT_CALLBACK *pfnOldcallback  	
)
{
	
	if (*pfnOldcallback != 0)
		*pfnOldcallback = g_pfnBltCallback;
		
	g_pfnBltCallback = pfncallback;
	DrvAIC_InstallISR(eDRVAIC_INT_LEVEL7 ,eDRVAIC_INT_BLT, (PVOID)DrvBLT_ISR, 0);
    DrvAIC_EnableInt(eDRVAIC_INT_BLT);
    DrvAIC_SetIntType(eDRVAIC_INT_BLT, eDRVAIC_HIGH_LEVEL);	
    
    return E_SUCCESS;
}
*/
void DrvBLT_SetRGB565TransparentColor(
	UINT16 u16RGB565	// [in] RGB565 Transparent Color
)
{
	outp32(TRCOLOR, u16RGB565 & TR_RGB565);
	    
}

UINT16 
DrvBLT_GetRGB565TransparentColor(void)
{
	return inp32(TRCOLOR) & TR_RGB565;
}


void DrvBLT_SetRGB565TransparentCtl(
BOOL bEnable
)
{
    outp32(SET2DA, (inp32(SET2DA) & ~0x80) | (bEnable << 7));
}

BOOL 
DrvBLT_GetRGB565TransparentCtl(void)
{
    return (inp32(SET2DA) & 0x80)>>7;
}

UINT32 
DrvBLT_GetVersion(void)
{
	return DRVBLT_VERSION_NUM;
}





















