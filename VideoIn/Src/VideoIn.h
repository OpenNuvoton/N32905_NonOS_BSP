/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved. *
 *                                                              *
 ****************************************************************/
 
#ifndef __DRVVIDEOIN_H__
#define __DRVVIDEOIN_H__

#include "W55FA93_VideoIn.h"
// #include header file
#ifdef  __cplusplus
extern "C"
{
#endif

#define ERRCODE		UINT32

/* Define data type (struct, union¡K) */

/* Define function */
void videoIn_Init(
	BOOL bIsEnableSnrClock,
	E_VIDEOIN_SNR_SRC eSnrSrc,	
	UINT32 u32SensorFreq,						//KHz unit
	E_VIDEOIN_DEV_TYPE eDevType
);

ERRCODE 
videoIn_Open(
	UINT32 u32EngFreqKHz, 
	UINT32 u32SensorFreq
);			
	
void videoIn_Close(void);
	
void videoIn_Reset(void);

ERRCODE 
videoIn_InstallCallback(
	E_VIDEOIN_INT_TYPE eIntType, 
	PFN_VIDEOIN_CALLBACK pfnCallback,
	PFN_VIDEOIN_CALLBACK *pfnOldCallback
);	

ERRCODE 
DrvVideoIn_EnableInt(
	E_VIDEOIN_INT_TYPE eIntType
);

ERRCODE
DrvVideoIn_DisableInt(
	E_VIDEOIN_INT_TYPE eIntType
);

BOOL
DrvVideoIn_IsIntEnabled(
	E_VIDEOIN_INT_TYPE eIntType
);

BOOL
DrvVideoIn_PollInt(
	E_VIDEOIN_INT_TYPE eIntType
);
	
void DrvVideoIn_SetPipeEnable(
	BOOL bEngEnable, 
	E_VIDEOIN_PIPE ePipeEnable
);    
	
void DrvVideoIn_GetPipeEnable(
	PBOOL pbEngEnable, 
	E_VIDEOIN_PIPE* pePipeEnable
);
	
void DrvVideoIn_SetShadowRegister(void);
	
void DrvVideoIn_SetSensorPolarity(
	BOOL bVsync, 
	BOOL bHsync, 
	BOOL bPixelClk
);
	
void DrvVideoIn_GetSensorPolarity(
	PBOOL pbVsync, 
	PBOOL pbHsync, 
	PBOOL pbPixelClk
);


void DrvVideoIn_SetDataFormatAndOrder(
	E_VIDEOIN_ORDER eInputOrder, 
	E_VIDEOIN_IN_FORMAT eInputFormat, 
	E_VIDEOIN_OUT_FORMAT eOutputFormat
);
	
void DrvVideoIn_GetDataFormatAndOrder(
	E_VIDEOIN_ORDER* pe8InputOrder, 
	E_VIDEOIN_IN_FORMAT* peInputFormat, 
	E_VIDEOIN_OUT_FORMAT* peOutputFormat
	);
	
void DrvVideoIn_SetPlanarFormat(
	BOOL bIsYUV420
);	
BOOL DrvVideoIn_GetPlanarFormat(void);	
	
void DrvVideoIn_SetMotionDet(
	BOOL bEnable,
	BOOL bBlockSize,	
	BOOL bSaveMode
);
	
void DrvVideoIn_GetMotionDet(
	PBOOL pbEnable,
	PBOOL pbBlockSize,	
	PBOOL pbSaveMode
);
		
void DrvVideoIn_SetMotionDetEx(
	UINT32 u32DetFreq,
	UINT32 u32Threshold,
	UINT32 u32OutBuffer,
	UINT32 u32LumBuffer
);	

void DrvVideoIn_GetMotionDetEx(
	PUINT32 pu32DetFreq,
	PUINT32 pu32Threshold,
	PUINT32 pu32OutBuffer,
	PUINT32 pu32LumBuffer
);	
	
void DrvVideoIn_SetOperationMode(
	BOOL bIsOneSutterMode
);



	
BOOL 
DrvVideoIn_GetOperationMode(void);
	
UINT32 
DrvVideoIn_GetProcessedDataCount(
	E_VIDEOIN_PIPE ePipe
);
	
void DrvVideoIn_SetCropWinStartAddr(
	UINT32 u32VerticalStart, 
	UINT32 u32HorizontalStart
);
	
void DrvVideoIn_GetCropWinStartAddr(
	PUINT32 pu32VerticalStart, 
	PUINT32 pu32HorizontalStart
);

void DrvVideoIn_SetCropWinSize(
	UINT32 u32Height, 
	UINT32 u32Width
);

void DrvVideoIn_GetCropWinSize(
	PUINT32 pu32Height, 
	PUINT32 pu32Width
);

ERRCODE 
DrvVideoIn_SetVerticalScaleFactor(
	E_VIDEOIN_PIPE ePipe,
	UINT8 u8Numerator, 
	UINT8 u8Denominator
);
	
ERRCODE 
DrvVideoIn_GetVerticalScaleFactor(
	E_VIDEOIN_PIPE ePipe,
	PUINT8 pu8Numerator, 
	PUINT8 pu8Denominator
);
	
ERRCODE 
DrvVideoIn_SetHorizontalScaleFactor(
	E_VIDEOIN_PIPE ePipe,
	UINT8 u8Numerator, 
	UINT8 u8Denominator
);
	
ERRCODE 
DrvVideoIn_GetHorizontalScaleFactor(
	E_VIDEOIN_PIPE ePipe,
	PUINT8 pu8Numerator, 
	PUINT8 pu8Denominator
);
	
void DrvVideoIn_SetFrameRateScaleFactor(
	UINT8 u8Numerator, 
	UINT8 u8Denominator
);

void DrvVideoIn_GetFrameRateScaleFactor(
	PUINT8 pu8Numerator, 
	PUINT8 pu8Denominator
);
	
void DrvVideoIn_SetAddressMatch(
	UINT32 u32AddressMatch
);
	
void DrvVideoIn_GetAddressMatch(
	PUINT32 pu32AddressMatch
);
	
void DrvVideoIn_SetStride(
	UINT32 u32PacketStride, 
	UINT32 u32PlanarStride
);
	
void DrvVideoIn_GetStride(
	PUINT32 pu32PacketStride, 
	PUINT32 pu32PlanarStride
);
	
ERRCODE 
DrvVideoIn_SetBaseStartAddress(
	E_VIDEOIN_PIPE ePipe,
	E_VIDEOIN_BUFFER eBuf,
	UINT32 u32BaseStartAddr
);	
	

ERRCODE 
DrvVideoIn_GetBaseStartAddress(
	E_VIDEOIN_PIPE ePipe,
	E_VIDEOIN_BUFFER eBuf,
	PUINT32 pu32BaseStartAddr
);
	
void DrvVideoIn_SetInputType(
	UINT32 u32FieldEnable,	
	E_VIDEOIN_TYPE bInputType, 
	BOOL bFieldSwap
);	
	
void DrvVdeoIn_GetInputType(
	PUINT32 pu32FieldEnable, 
	E_VIDEOIN_TYPE* pbInputType, 
	PBOOL pbFieldSwap
);	
			
void DrvVideoIn_SetFieldDetection(
	BOOL bDetPosition,
	BOOL bFieldDetMethod
);	
	
void DrvVideoIn_GetFieldDetection(
	PBOOL pbDetPosition, 
	PBOOL pbFieldDetMethod
);
	
void DrvVideoIn_SetPacketFrameBufferControl(
	BOOL bFrameSwitch,
	BOOL bFrameBufferSel
);
	
void DrvVideoIn_GetPacketFrameBufferControl(
	PBOOL pbFrameSwitch,
	PBOOL pbFrameBufferSel
);
	
void DrvVideoIn_SetInitFrame(void);
	
UINT32 
DrvVideoIn_GetSkipFrame(void);
	
UINT32 
DrvVideoIn_GetVersion(void);


#ifdef __cplusplus
}
#endif

#endif

















