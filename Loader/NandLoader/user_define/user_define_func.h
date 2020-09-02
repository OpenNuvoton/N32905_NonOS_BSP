/**************************************************************************//**
 * @file     user_define_func.h
 * @version  V3.00
 * @brief    NandLoader header file for user define function.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#ifndef _W55FA93_NANDLOADER_USER_DEFINE_FUNC_H
#define _W55FA93_NANDLOADER_USER_DEFINE_FUNC_H

/*-----------------------------------------------------------------------------------*/
/* This include file should keep empty if user define nothing for user_define_func() */
/*-----------------------------------------------------------------------------------*/

#ifdef __USER_DEFINE_FUNC

#include <string.h>
#include "wblib.h"
#include "turbowriter.h"
#include "W55FA93_VPOST.h"

// for Nuvoton FA93 Demo Board panel 320x240
#define PANEL_WIDTH     320
#define PANEL_HEIGHT    240

// got Logo image from memory address FB_ADDR
#define FB_ADDR         0x00500000

#endif  // end of #ifdef __USER_DEFINE_FUNC

#endif  // end of _W55FA93_NANDLOADER_USER_DEFINE_FUNC_H
