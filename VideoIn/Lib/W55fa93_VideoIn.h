/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved. *
 *                                                              *
 ****************************************************************/
 
#ifndef __W55FA93_VIDEOIN_H__
#define __W55FA93_VIDEOIN_H__

// #include header file
#ifdef  __cplusplus
extern "C"
{
#endif

/* Define data type (struct, union¡K) */
#define ERRCODE		UINT32
// #define Constant


//Error message
// E_VIDEOIN_INVALID_INT					Invalid interrupt
// E_VIDEOIN_INVALID_BUF					Invalid buffer
// E_VIDEOIN_INVALID_PIPE				Invalid pipe

#define E_VIDEOIN_INVALID_INT   			0xFFFF3001
#define E_VIDEOIN_INVALID_BUF   			0xFFFF3002
#define E_VIDEOIN_INVALID_PIPE  			0xFFFF3003


typedef void (*PFN_VIDEOIN_CALLBACK)(
				UINT8 u8PacketBufID, 
				UINT8 u8PlanarBufID, 
				UINT8 u8FrameRate, 
				UINT8 u8Filed);

/* Input device  */
typedef enum
{
	eVIDEOIN_SNR_CCIR601 =0,
	eVIDEOIN_SNR_CCIR656,
	eVIDEOIN_TVD_CCIR601,
	eVIDEOIN_TVD_CCIR656,
	eVIDEOIN_2ND_SNR_CCIR601,
	eVIDEOIN_3RD_SNR_CCIR601,
	eVIDEOIN_2ND_TVD_CCIR656,
	eVIDEOIN_3RD_TVD_CCIR656
}E_VIDEOIN_DEV_TYPE;

/* Interrupt type */
typedef enum
{
	eVIDEOIN_MDINT = 0x100000,
	eVIDEOIN_ADDRMINT = 0x80000,
	eVIDEOIN_MEINT = 0x20000,
	eVIDEOIN_VINT = 0x10000	
}E_VIDEOIN_INT_TYPE;


/* Pipe enable */
typedef enum
{
	eVIDEOIN_BOTH_PIPE_DISABLE = 0,
	eVIDEOIN_PLANAR = 1,
	eVIDEOIN_PACKET = 2,
	eVIDEOIN_BOTH_PIPE_ENABLE = 3	
}E_VIDEOIN_PIPE;

/* Base address */
typedef enum
{
	eVIDEOIN_BUF0 =0,
	eVIDEOIN_BUF1,	
	eVIDEOIN_BUF2
}E_VIDEOIN_BUFFER;

/* For DrvVideoIn_SetOperationMode */
#define VIDEOIN_CONTINUE   1   

/* Input Data Order For YCbCr */
typedef enum
{
	eVIDEOIN_IN_UYVY =0,
	eVIDEOIN_IN_YUYV,
	eVIDEOIN_IN_VYUY,		
	eVIDEOIN_IN_YVYU
}E_VIDEOIN_ORDER;


typedef enum
{
	eVIDEOIN_IN_YUV422 = 0,
	eVIDEOIN_IN_RGB565
}E_VIDEOIN_IN_FORMAT;                                  
                                                                
typedef enum
{
	eVIDEOIN_OUT_YUV422 = 0,
	eVIDEOIN_OUT_ONLY_Y,
	eVIDEOIN_OUT_RGB555,		
	eVIDEOIN_OUT_RGB565
}E_VIDEOIN_OUT_FORMAT;	

typedef enum
{
	eVIDEOIN_TYPE_CCIR601 = 0,
	eVIDEOIN_TYPE_CCIR656
}E_VIDEOIN_TYPE;     

typedef enum
{
	eVIDEOIN_SNR_APLL = 2,
	eVIDEOIN_SNR_UPLL = 3
}E_VIDEOIN_SNR_SRC;  


#define VIDEOIN_IOCTL_SET_BUF_ADDR						0
/* Packet buffer 0 and 1. Planar Y, U and V 0 and 1 */
#define VIDEOIN_IOCTL_SET_STRIDE						(VIDEOIN_IOCTL_SET_BUF_ADDR+2)
/* Packet stride or planar stride */ 	
#define VIDEOIN_IOCTL_SET_PIPE_ENABLE					(VIDEOIN_IOCTL_SET_STRIDE+2)
/* Planar pipe or packet pipe enable */
#define VIDEOIN_IOCTL_SET_CROPPING_START_POSITION		(VIDEOIN_IOCTL_SET_PIPE_ENABLE+2)
/* Cropping start position */
#define VIDEOIN_IOCTL_SET_FRAME_RATE					(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION+2)
/* Set the divider */
#define VIDEOIN_IOCTL_UPDATE_REGISTER					(VIDEOIN_IOCTL_SET_FRAME_RATE+2)
/* Update the register to registers */
#define VIDEOIN_IOCTL_CROPPING_DIMENSION				(VIDEOIN_IOCTL_UPDATE_REGISTER+2)
/* VIDEOIN_IOCTL_SET_CROPPING_DIMENSION */
#define VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT				(VIDEOIN_IOCTL_CROPPING_DIMENSION+2)
/* Sensor data in order, in format and out format */

#define VIDEOIN_IOCTL_SET_POLARITY						(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT+2)
/* Sensor interface */

#define VIDEOIN_IOCTL_VSCALE_FACTOR					(VIDEOIN_IOCTL_SET_POLARITY+2)

#define VIDEOIN_IOCTL_HSCALE_FACTOR					(VIDEOIN_IOCTL_VSCALE_FACTOR+2)

#define VIDEOIN_IOCTL_SET_INPUT_TYPE					(VIDEOIN_IOCTL_HSCALE_FACTOR+2)

#define VIDEOIN_IOCTL_SET_FIELD_DET						(VIDEOIN_IOCTL_SET_INPUT_TYPE+2)
            
/* Motion detection block size */
#define VIDEOIN_IOCTL_SET_MD_BLOCK_SIZE			0x30
/* Motion detection save mode 1bit or 1bit+7diff */
#define VIDEOIN_IOCTL_SET_MD_SAVE_MODE			0x32
/* Motion detection ebable or disable */
#define VIDEOIN_IOCTL_SET_MD_ENABLE				0x34
/* Motion detection frequency and threshold */
#define VIDEOIN_IOCTL_SET_MD_FREQ_THRESHOLD		0x36
/* Motion detection out buffer and Y output buffer */
#define VIDEOIN_IOCTL_SET_MD_ADDR					0x38


#define videoInIoctl		videoinIoctl


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

VOID videoInIoctl(UINT32 u32Cmd, 
			UINT32 u32Element, 
			UINT32 u32Arg0, 
			UINT32 u32Arg1);

ERRCODE 
videoIn_EnableInt(
	E_VIDEOIN_INT_TYPE eIntType
);

ERRCODE
videoIn_DisableInt(
	E_VIDEOIN_INT_TYPE eIntType
);

void videoIn_SetPacketFrameBufferControl(
	BOOL bFrameSwitch,
	BOOL bFrameBufferSel
);

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
void 
DrvVideoIn_SetOperationMode(
	BOOL bIsOneSutterMode
);


BOOL 
DrvVideoIn_GetOperationMode(
void
	);

#ifdef __cplusplus
}
#endif

#endif

















