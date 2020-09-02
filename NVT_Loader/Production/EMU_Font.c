/**************************************************************************//**
 * @file     EMU_Font.c
 * @brief    Draw Font and border on panel 
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
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