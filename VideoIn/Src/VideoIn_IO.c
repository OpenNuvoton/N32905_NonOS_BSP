/**************************************************************************//**
 * @file     VideoIn_IO.c
 * @version  V3.00
 * @brief    N3290x series VideoIn driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "W55FA93_reg.h"
#include "wblib.h"
#include "VideoIn.h"
#include "W55FA93_VideoIn.h"

VOID videoInIoctl(UINT32 u32Cmd, UINT32 u32Element, UINT32 u32Arg0, UINT32 u32Arg1) 
{
	switch(u32Cmd)
	{
		case VIDEOIN_IOCTL_SET_BUF_ADDR:
			DrvVideoIn_SetBaseStartAddress((E_VIDEOIN_PIPE)u32Element,	
										(E_VIDEOIN_BUFFER)u32Arg0,
										(UINT32)u32Arg1);
			break;							
		case VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT:
			DrvVideoIn_SetDataFormatAndOrder((E_VIDEOIN_ORDER)u32Element,	
										(E_VIDEOIN_IN_FORMAT)u32Arg0,
										(E_VIDEOIN_OUT_FORMAT)u32Arg1);	
			break;				
		case VIDEOIN_IOCTL_SET_POLARITY:
			DrvVideoIn_SetSensorPolarity((BOOL)u32Element, 
										(BOOL)u32Arg0, 
										(BOOL)u32Arg1); 	//Polarity	
								
			break;					
		case VIDEOIN_IOCTL_SET_CROPPING_START_POSITION:
			DrvVideoIn_SetCropWinStartAddr(u32Element, 	
										u32Arg0);	//
			break;
		case VIDEOIN_IOCTL_CROPPING_DIMENSION:
			DrvVideoIn_SetCropWinSize(u32Element,					//UINT16 u16Height, 
							 		u32Arg0);				//UINT16 u16Width;		
			break;
		case VIDEOIN_IOCTL_VSCALE_FACTOR:
			DrvVideoIn_SetVerticalScaleFactor((E_VIDEOIN_PIPE)u32Element,
									u32Arg0,					//UINT16 u16Height, 
							 		u32Arg1);				//UINT16 u16Width;								 			
			break;	
		case VIDEOIN_IOCTL_HSCALE_FACTOR:
			DrvVideoIn_SetHorizontalScaleFactor((E_VIDEOIN_PIPE)u32Element,
									u32Arg0,					//UINT16 u16Height, 
							 		u32Arg1);				//UINT16 u16Width;								 			
			break;		
		case VIDEOIN_IOCTL_SET_STRIDE:
			DrvVideoIn_SetStride(u32Element, 							//UINT32 u32PacketStride,
								u32Arg0);						//UINT32 u32PlanarStride,									
			break;					
		case	VIDEOIN_IOCTL_SET_PIPE_ENABLE: 
			DrvVideoIn_SetPipeEnable(u32Element,
									(E_VIDEOIN_PIPE)u32Arg0);
			break;						
		case VIDEOIN_IOCTL_SET_INPUT_TYPE: 
			DrvVideoIn_SetInputType(u32Element,			//3: Both field enable
						(E_VIDEOIN_TYPE)u32Arg0,						//VIDEOIN_TYPE eDRVVIDEOIN_TYPE_CCIR656 or eDRVVIDEOIN_TYPE_CCIR601, 
						u32Arg1);						//BOOL bFieldSwap		
			break;							
		case VIDEOIN_IOCTL_SET_FIELD_DET:
			DrvVideoIn_SetFieldDetection(u32Element, 		//Detection position, v-start=0 or v-end=1 if Detection method = 0		
										u32Arg0);		//0: Detect field by Vsync & Hsync(SA711113). 1: Detect field by field pin(WT8861)	
			break;							
	}
}

