/**************************************************************************//**
 * @file     DrvEDMA.h
 * @brief    EDMA Header file to link with ADC IP for audio recording 
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
 
#ifndef __DRVEDMA_H__
#define __DRVEDMA_H__

// #include header file
//#include "Platform.h"
//#include "System/SysInfra.h"

#ifdef  __cplusplus
extern "C"
{
#endif

#define E_SUCCESS	0
#define E_FAIL		1

//#define outp32(addr,value)	outl(value, addr)
//#define inp32(addr)		inl(addr)
#if 0
typedef enum {
#undef FALSE
	FALSE	= 0,
#undef TRUE
	TRUE	= 1
} bool;
#else
#define bool  						INT32
#endif

#define DESTINATION_DIRECTION_BIT   6
#define SOURCE_DIRECTION_BIT        4
#define TRANSFER_WIDTH_BIT          19
#define MODE_SELECT_BIT             2

#define MAX_TRANSFER_BYTE_COUNT     0x00FFFFFF
#define MAX_CHANNEL_NUM   5

// E_DRVEDMA_FALSE_INPUT		Non-support specified parameter
#define E_DRVEDMA_FALSE_INPUT	-1

// For interrupt CallBack Function
typedef void (*PFN_DRVEDMA_CALLBACK)(unsigned int);

/****************************************************************
	Enumerate Type
 ****************************************************************/
typedef enum
{
	eDRVEDMA_DISABLE =0,
	eDRVEDMA_ENABLE
}E_DRVEDMA_OPERATION;

typedef enum
{
	eDRVEDMA_CHANNEL_0 =0,
	eDRVEDMA_CHANNEL_1,
	eDRVEDMA_CHANNEL_2,
	eDRVEDMA_CHANNEL_3,
	eDRVEDMA_CHANNEL_4	
}E_DRVEDMA_CHANNEL_INDEX;

typedef enum
{
	eDRVEDMA_TARGET_SOURCE =0,
	eDRVEDMA_TARGET_DESTINATION
}E_DRVEDMA_TARGET;


typedef enum
{
	eDRVEDMA_WRAPAROUND_NO_INT =0,    
	eDRVEDMA_WRAPAROUND_EMPTY,	            // empty
	eDRVEDMA_WRAPAROUND_THREE_FOURTHS,	    // 3/4 
	eDRVEDMA_WRAPAROUND_HALF=4,             // 1/2 
	eDRVEDMA_WRAPAROUND_QUARTER=8           // 1/4 
}E_DRVEDMA_WRAPAROUND_SELECT;

typedef enum
{
	eDRVEDMA_TABORT_FLAG =1,
	eDRVEDMA_BLKD_FLAG,
	eDRVEDMA_SG_FLAG=8,	
	eDRVEDMA_WRA_EMPTY_FLAG=0x100,	          // empty	    
	eDRVEDMA_WRA_THREE_FOURTHS_FLAG=0x200,	  // 3/4 
	eDRVEDMA_WRA_HALF_FLAG=0x400,             // 1/2 
	eDRVEDMA_WRA_QUARTER_FLAG=0x800           // 1/4 
}E_DRVEDMA_INT_FLAG;

typedef enum
{
	eDRVEDMA_DIRECTION_INCREMENTED =0,
	eDRVEDMA_DIRECTION_DECREMENTED,
	eDRVEDMA_DIRECTION_FIXED,
	eDRVEDMA_DIRECTION_WRAPAROUND
}E_DRVEDMA_DIRECTION_SELECT;

typedef enum
{
	eDRVEDMA_WIDTH_32BITS=0,    
	eDRVEDMA_WIDTH_8BITS,
	eDRVEDMA_WIDTH_16BITS
}E_DRVEDMA_TRANSFER_WIDTH;

typedef enum
{
	eDRVEDMA_TABORT =1,
	eDRVEDMA_BLKD,
	eDRVEDMA_WAR=4,			
	eDRVEDMA_SG=8		
}E_DRVEDMA_INT_ENABLE;

typedef enum
{
	eDRVEDMA_RGB888=1,    
	eDRVEDMA_RGB555=2,  
	eDRVEDMA_RGB565=4,	  
	eDRVEDMA_YCbCr422=8
}E_DRVEDMA_COLOR_FORMAT;

typedef enum
{
	eDRVEDMA_SPIMS0=0,    
	eDRVEDMA_SPIMS1,  
	eDRVEDMA_UART0,	  
	eDRVEDMA_UART1,
	eDRVEDMA_ADC
}E_DRVEDMA_APB_DEVICE;

typedef enum
{
	eDRVEDMA_READ_APB=0,    
	eDRVEDMA_WRITE_APB  
}E_DRVEDMA_APB_RW;

typedef enum
{
        eDRVEDMA_MEMORY_TO_MEMORY =0,
        eDRVEDMA_DEVICE_TO_MEMORY,
        eDRVEDMA_MEMORY_TO_DEVICE
}E_DRVEDMA_MODE_SELECT;

typedef struct {
    unsigned int u32Addr;
    E_DRVEDMA_DIRECTION_SELECT eAddrDirection;
}S_DRVEDMA_CH_ADDR_SETTING;

typedef struct {
    unsigned int u32SourceAddr;
    E_DRVEDMA_DIRECTION_SELECT eSrcDirection;    
    unsigned int u32DestAddr;
    E_DRVEDMA_DIRECTION_SELECT eDestDirection;     
    unsigned int u32TransferByteCount;
    unsigned int u32Stride;
    unsigned int u32SrcOffset;
    unsigned int u32DestOffset;    
}S_DRVEDMA_DESCRIPT_SETTING;

typedef struct {
    unsigned int u32SourceAddr;
    unsigned int u32DestAddr;
    unsigned int u32StrideAndByteCount;
    unsigned int u32Offset;
    unsigned int u32NextSGTblAddr;
}S_DRVEDMA_DESCRIPT_FORMAT;

/****************************************************************
// APIs declaration
 ****************************************************************/
unsigned int  
DrvEDMA_Open(void);

void DrvEDMA_Close(void);

bool  
DrvEDMA_IsCHBusy(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

void DrvEDMA_EnableCH(
	E_DRVEDMA_CHANNEL_INDEX eChannel,
	E_DRVEDMA_OPERATION eOP
);

bool  
DrvEDMA_IsEnabledCH(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

int  
DrvEDMA_SetTransferSetting(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	S_DRVEDMA_CH_ADDR_SETTING* psSrcAddr, 
	S_DRVEDMA_CH_ADDR_SETTING* psDestAddr, 
	unsigned int u32TransferLength
);

int  
DrvEDMA_GetTransferSetting(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	E_DRVEDMA_TARGET eTarget, 
	unsigned int* pu32Addr, 
	E_DRVEDMA_DIRECTION_SELECT* peDirection
);

int  
DrvEDMA_GetTransferLength(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	unsigned int* pu32TransferLength
);

int  
DrvEDMA_SetAPBTransferWidth(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	E_DRVEDMA_TRANSFER_WIDTH eTransferWidth
);

int  
DrvEDMA_GetAPBTransferWidth(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	E_DRVEDMA_TRANSFER_WIDTH* peTransferWidth
);

int  
DrvEDMA_SetCHForAPBDevice(
    E_DRVEDMA_CHANNEL_INDEX eChannel, 
    E_DRVEDMA_APB_DEVICE eDevice,
    E_DRVEDMA_APB_RW eRWAPB    
);

int  
DrvEDMA_GetCHForAPBDevice(
    E_DRVEDMA_APB_DEVICE eDevice,
    E_DRVEDMA_APB_RW eRWAPB    
);

int  
DrvEDMA_SetWrapIntType(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	E_DRVEDMA_WRAPAROUND_SELECT eType
);

int  
DrvEDMA_GetWrapIntType(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

int  
DrvEDMA_CHSoftwareReset(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

int  
DrvEDMA_CHEnablelTransfer(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

unsigned int  
DrvEDMA_GetCurrentSourceAddr(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

unsigned int  
DrvEDMA_GetCurrentDestAddr(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

unsigned int  
DrvEDMA_GetCurrentTransferCount(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

int  
DrvEDMA_EnableInt(
    E_DRVEDMA_CHANNEL_INDEX eChannel, 
    E_DRVEDMA_INT_ENABLE eIntSource
);

void DrvEDMA_DisableInt(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	E_DRVEDMA_INT_ENABLE eIntSource
);

unsigned int  
DrvEDMA_IsIntEnabled(
	E_DRVEDMA_CHANNEL_INDEX eChannel,
	E_DRVEDMA_INT_ENABLE eIntSource	
);

void DrvEDMA_ClearInt(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	E_DRVEDMA_INT_FLAG eIntFlag
);

bool
DrvEDMA_PollInt(
    E_DRVEDMA_CHANNEL_INDEX eChannel, 
    E_DRVEDMA_INT_FLAG eIntFlag
);

int  
DrvEDMA_SetColorTransformFormat(
	E_DRVEDMA_COLOR_FORMAT u32SourceFormat,
	E_DRVEDMA_COLOR_FORMAT u32DestFormat
);

void DrvEDMA_GetColorTransformFormat(
	E_DRVEDMA_COLOR_FORMAT* pu32SourceFormat,
	E_DRVEDMA_COLOR_FORMAT* pu32DestFormat
);


int  
DrvEDMA_SetColorTransformOperation(
	E_DRVEDMA_OPERATION eColorSpaceTran,
	E_DRVEDMA_OPERATION eStrideMode
);

void DrvEDMA_GetColorTransformOperation(
	E_DRVEDMA_OPERATION* peColorSpaceTran,
	E_DRVEDMA_OPERATION* peStrideMode
);

int  
DrvEDMA_SetSourceStride(
	unsigned int u32StrideByteCount,
	unsigned int u32OffsetByteLength
);

void DrvEDMA_GetSourceStride(
	unsigned int* pu32StrideByteCount,
	unsigned int* pu32OffsetByteLength
);

int  
DrvEDMA_SetDestinationStrideOffset(
	unsigned int u32OffsetByteLength
);

void DrvEDMA_GetDestinationStrideOffset(
	unsigned int* pu32OffsetByteLength
);

void DrvEDMA_SetClamping(
	E_DRVEDMA_OPERATION eOP
);

E_DRVEDMA_OPERATION  
DrvEDMA_GetClamping(void);

unsigned int  
DrvEDMA_GetInternalBufPointer(
	E_DRVEDMA_CHANNEL_INDEX eChannel
);

unsigned int  
DrvEDMA_GetSharedBufData(
	E_DRVEDMA_CHANNEL_INDEX eChannel, 
	unsigned int u32BufIndex
);

int  
DrvEDMA_InstallCallBack(
    E_DRVEDMA_CHANNEL_INDEX eChannel, 
    E_DRVEDMA_INT_ENABLE eIntSource,
	PFN_DRVEDMA_CALLBACK pfncallback,    
	PFN_DRVEDMA_CALLBACK *pfnOldcallback  	
);

void DrvEDMA_GetScatterGatherInfo(
	unsigned int *pu32TblSize,
	unsigned int *pu32MaxTransferBytePerTbl
);

  
int 
DrvEDMA_SetScatterGatherSetting(
    E_DRVEDMA_CHANNEL_INDEX eChannel,
	unsigned int u32SGTblStartAddr,
	S_DRVEDMA_DESCRIPT_SETTING* psDescript
);

unsigned int  
DrvEDMA_GetVersion(void);


#ifdef  __cplusplus
}
#endif

#endif	// __DRVEDMA_H__
