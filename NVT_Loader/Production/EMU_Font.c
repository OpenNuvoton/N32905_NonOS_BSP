/****************************************************************************
*                                                                           *
* Copyright (c) 2011 Nuvoton Tech. Corp. All rights reserved.               *
*                                                                           *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_sic.h"
#include "nvtfat.h"
#include "Font.h"
#include "emuProduct.h"

void EMU_InitFont(
	S_DEMO_FONT* 	ptFont,
	UINT32 			u32FrameBufAddr	
)
{	
	InitFont(ptFont,
				u32FrameBufAddr);
}				