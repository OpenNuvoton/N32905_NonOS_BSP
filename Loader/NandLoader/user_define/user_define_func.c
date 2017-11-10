/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                       */
/*                                                                                   */
/* Copyright (c) 2013 by Nuvoton Technology Corporation                              */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
#ifdef __USER_DEFINE_FUNC

#include "user_define_func.h"

void initVPostShowLogo(void);
INT32 vpostLCMInit(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf);
LCDFORMATEX lcdInfo;

/*-----------------------------------------------------------------------------------*/
/* The entry point of User Define Function that called by NandLoader.                */
/*-----------------------------------------------------------------------------------*/
void user_define_func()
{
    //--- This is a sample code for user define function.
    //--- This sample code will show Logo to panel on Nuvoton FA93 Demo Board.
    initVPostShowLogo();
}


static UINT32 bIsInitVpost=FALSE;
void initVPostShowLogo(void)
{
    if(bIsInitVpost==FALSE)
    {
        bIsInitVpost = TRUE;
        lcdInfo.ucVASrcFormat = DRVVPOST_FRAME_RGB565;
        lcdInfo.nScreenWidth = PANEL_WIDTH;
        lcdInfo.nScreenHeight = PANEL_HEIGHT;
        vpostLCMInit(&lcdInfo, (UINT32*)FB_ADDR);
    }
}

#else

/*-----------------------------------------------------------------------------------*/
/* The entry point of User Define Function that called by NandLoader.                */
/*-----------------------------------------------------------------------------------*/
void user_define_func()
{
    //--- Keep empty if user define nothing for User Define Function.
}

#endif  // end of #ifdef __USER_DEFINE_FUNC
