/****************************************************************
 *                                                             *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved. *
 *                                                              *
 ****************************************************************/
 
#ifndef __DRVBLT_H__
#define __DRVBLT_H__

#include "wbtypes.h"
#include "wblib.h"
#include "wbio.h"
// #include header file
//#include "Platform.h"
//#include "System/SysInfra.h"

#define	ERRCODE	int

#ifdef  __cplusplus
extern "C"
{
#endif

// #define Constant
#define DRVBLT_MAJOR_NUM 3
#define DRVBLT_MINOR_NUM 60
#define DRVBLT_BUILD_NUM 1

// Version define with SysInfra
//_SYSINFRA_VERSION_DEF(DRVBLT, DRVBLT_MAJOR_NUM, DRVBLT_MINOR_NUM, DRVBLT_BUILD_NUM);

#define	E_DRVBLT_FALSE_INPUT	-1
// E_DRVBLT_FALSE_INPUT		Non-support parameter
//_SYSINFRA_ERRCODE_DEF (DRVBLT, FALSE_INPUT, TRUE, MODULE_ID_DRVBLT, 1)

// For interrupt CallBack Function
typedef void (*PFN_DRVBLT_CALLBACK)(void);

typedef enum
{
    eDRVBLT_DISABLE,      
    eDRVBLT_ENABLE     
} E_DRVBLT_FILLOP;

typedef enum
{
    eDRVBLT_EFFECTIVE,      
    eDRVBLT_NO_EFFECTIVE     
} E_DRVBLT_REVEAL_ALPHA;

typedef enum
{
    eDRVBLT_NONTRANSPARENCY,     
    eDRVBLT_HASTRANSPARENCY=1,     
    eDRVBLT_HASCOLORTRANSFORM=2, 
    eDRVBLT_HASALPHAONLY=4           
} E_DRVBLT_TRANSFORM_FLAG;

typedef enum
{
    eDRVBLT_SRC_ARGB8888=1,     
    eDRVBLT_SRC_RGB565, 
    eDRVBLT_SRC_1BPP=4,             
    eDRVBLT_SRC_2BPP=8,    
    eDRVBLT_SRC_4BPP=16,     
    eDRVBLT_SRC_8BPP=32
} E_DRVBLT_BMPIXEL_FORMAT;

typedef enum
{
    eDRVBLT_DEST_ARGB8888=1,    
    eDRVBLT_DEST_RGB565, 
    eDRVBLT_DEST_RGB555=4         
} E_DRVBLT_DISPLAY_FORMAT;

typedef enum
{
    eDRVBLT_CLIP_TO_EDGE=1,    
    eDRVBLT_NOTSMOOTH,    
    eDRVBLT_NONE_FILL=4
} E_DRVBLT_FILL_STYLE;

typedef enum
{
    eDRVBLT_BIG_ENDIAN,     
    eDRVBLT_LITTLE_ENDIAN  
} E_DRVBLT_PALETTE_ORDER;


typedef struct {
    INT32 a;
    INT32 b;
    INT32 c;
    INT32 d;
} S_DRVBLT_MATRIX;

typedef struct {
    INT16  i16Blue;
    INT16  i16Green;
    INT16  i16Red;
    INT16  i16Alpha;
} S_DRVBLT_ARGB16;

typedef struct {
    INT16   i16Xmin;
    INT16   i16Xmax;
    INT16   i16Ymin;
    INT16   i16Ymax;
} S_DRVBLT_RECT;

typedef struct {
    UINT8   u8Blue;
    UINT8   u8Green;
    UINT8   u8Red;
    UINT8   u8Alpha;
} S_DRVBLT_ARGB8;

typedef struct {
//    S_DRVBLT_ARGB8*  pSARGB8;
    UINT32              u32SrcImageAddr;
    INT32               i32Stride;    
    INT32               i32XOffset;
    INT32               i32YOffset;
    INT16               i16Width;
    INT16               i16Height;   
} S_DRVBLT_SRC_IMAGE;    

typedef struct {
    UINT32              u32FrameBufAddr;
    INT32               i32XOffset;
    INT32               i32YOffset;
    INT32               i32Stride;     
    INT16               i16Width;
    INT16               i16Height;      
} S_DRVBLT_DEST_FB;    


// APIs declaration

UINT32
DrvBLT_Open(void);
void DrvBLT_Close(void);
void DrvBLT_SetTransformMatrix(
	S_DRVBLT_MATRIX sMatrix		
);

void DrvBLT_GetTransformMatrix(
	S_DRVBLT_MATRIX* psMatrix		
);

ERRCODE 
DrvBLT_SetSrcFormat(
	E_DRVBLT_BMPIXEL_FORMAT eSrcFmt	// [in] Source Image Format 
);

E_DRVBLT_BMPIXEL_FORMAT 
DrvBLT_GetSrcFormat(void);

ERRCODE 
DrvBLT_SetDisplayFormat(
	E_DRVBLT_DISPLAY_FORMAT eDisplayFmt	// [in] Display Format 
);

E_DRVBLT_DISPLAY_FORMAT 
DrvBLT_GetDisplayFormat(void);

void DrvBLT_EnableInt(void);
void DrvBLT_DisableInt(void);

BOOL 
DrvBLT_IsIntEnabled(void);

void DrvBLT_ClearInt(void);

BOOL 
DrvBLT_PollInt(void);

void DrvBLT_SetColorMultiplier(
	S_DRVBLT_ARGB16 sARGB16	// [in] ARGB Multiplier 
);

void DrvBLT_GetColorMultiplier(
	S_DRVBLT_ARGB16* psARGB16	// [out] ARGB Multiplier 
);

void DrvBLT_SetColorOffset(
	S_DRVBLT_ARGB16 sARGB16	// [in] ARGB offset 
);

void DrvBLT_GetColorOffset(
	S_DRVBLT_ARGB16* psARGB16	// [out] ARGB offset 
);

void DrvBLT_SetSrcImage(
	S_DRVBLT_SRC_IMAGE sSrcImage	// [in] Source Image Setting
);

void DrvBLT_SetDestFrameBuf(
	S_DRVBLT_DEST_FB sFrameBuf	// [in] Frame Buffer Setting
);

void DrvBLT_SetARGBFillColor(
	S_DRVBLT_ARGB8 sARGB8	// [in] ARGB value for fill operation
);

void DrvBLT_GetARGBFillColor(
	S_DRVBLT_ARGB8* psARGB8	// [out] ARGB value for fill operation
);

BOOL 
DrvBLT_GetBusyStatus(void);

void DrvBLT_SetFillAlpha(
BOOL bEnable
);

BOOL 
DrvBLT_GetFillAlpha(void);

void DrvBLT_SetTransformFlag(
	UINT32 u32TransFlag			// [in] A combination of the enum E_DRVBLT_TRANSFORM_FLAG
);

UINT32 
DrvBLT_GetTransformFlag(void);

void DrvBLT_SetPaletteEndian(
	E_DRVBLT_PALETTE_ORDER eEndian	// [in] Palette Endian Type
);

E_DRVBLT_PALETTE_ORDER 
DrvBLT_GetPaletteEndian(void);

void DrvBLT_SetColorPalette(
	UINT32 u32PaletteInx, 		// [in] Color Palette Start index
	UINT32 u32Num, 				// [in] Color Palette number to set
	S_DRVBLT_ARGB8* psARGB	// [in] pointer for Color palette from u32PaletteInx
);

void DrvBLT_SetFillOP(
	E_DRVBLT_FILLOP eOP		// [in] Enable/Disable FillOP
);

BOOL 
DrvBLT_GetFillOP(void);

void DrvBLT_SetFillStyle(
	E_DRVBLT_FILL_STYLE eStyle		// [in] Fill Style for Fill Operation
);

E_DRVBLT_FILL_STYLE 
DrvBLT_GetFillStyle(void);

void DrvBLT_SetRevealAlpha(
	E_DRVBLT_REVEAL_ALPHA eAlpha		// [in] need / no need un-multiply alpha on source image
);

BOOL 
DrvBLT_GetRevealAlpha(void);

void DrvBLT_Trigger(void);

ERRCODE
DrvBLT_InstallCallBack(
	PFN_DRVBLT_CALLBACK pfncallback,    
	PFN_DRVBLT_CALLBACK *pfnOldcallback  	
);

void DrvBLT_SetRGB565TransparentColor(
	UINT16 u16RGB565	// [in] RGB565 Transparent Color
);

UINT16 
DrvBLT_GetRGB565TransparentColor(void);

void DrvBLT_SetRGB565TransparentCtl(
BOOL bEnable
);

BOOL 
DrvBLT_GetRGB565TransparentCtl(void);

UINT32 
DrvBLT_GetVersion(void);


#ifdef  __cplusplus
}
#endif

#endif	// __DRVBLT_H__






