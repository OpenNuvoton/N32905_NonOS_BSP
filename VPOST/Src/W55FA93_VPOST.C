/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
 * 
 * FILENAME
 *     NUC930_VPOST.c
 *
 * VERSION
 *     0.1 
 *
 * DESCRIPTION
 *
 *
 *
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *
 *
 *     
 * HISTORY
 *     2009.03.16		Created by Shu-Ming Fan
 *
 *
 * REMARK
 *     None
 *
 *
 **************************************************************************/
#include "wblib.h"
#include "w55fa93_vpost.h"
#include "w55fa93_reg.h"


#include <stdio.h>

VOID * g_VAFrameBuf = NULL;
VOID * g_VAOrigFrameBuf = NULL;

INT32 vpostLCMInit(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{

#ifdef HAVE_SHARP_LQ035Q1DH02
	return vpostLCMInit_SHARP_LQ035Q1DH02(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_WINTEK_WMF3324
	return vpostLCMInit_WINTEK_WMF3324(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_GIANTPLUS_GPM1006D0
	return vpostLCMInit_GIANTPLUS_GPM1006D0(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_HANNSTAR_HSD043I9W1
	return vpostLCMInit_HANNSTAR_HSD043I9W1(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_GOWORLD_GWMTF9406A
	return vpostLCMInit_GOWORLD_GWMTF9406A(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_GOWORLD_GWMTF9360A
	return vpostLCMInit_GOWORLD_GWMTF9360A(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_GOWORLD_GWMTF9615A
	return vpostLCMInit_GOWORLD_GWMTF9615A(plcdformatex, pFramebuf);
#endif
#ifdef HAVE_GOWORLD_GWMTF9360A_MODIFY
	return vpostLCMInit_GOWORLD_GWMTF9360A_modify(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_GOWORLD_GW8973
	return vpostLCMInit_GOWORLD_GW8973(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_VG680
	return vpostLCMInit_VG680(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_TVOUT_720x480
	return vpostLCMInit_TVOUT_720x480(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_TVOUT_640x480
	return vpostLCMInit_TVOUT_640x480(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_TVOUT_320x240
	return vpostLCMInit_TVOUT_320x240(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_AMPIRE_800x600
	return vpostLCMInit_AMPIRE_800x600(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_HANNSTAR_HSD070IDW1
	return vpostLCMInit_HANNSTAR_HSD070IDW1(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_AMPIRE_800x480
	return vpostLCMInit_AMPIRE_800x480(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_GOWORLD_GW07000GNWV
	return vpostLCMInit_GOWORLD_GW07000GNWV(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_KD070D10_800x480
	return vpostLCMInit_KD070D10_800x480(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_HIMAX_HX8346
	return vpostLCMInit_HIMAX_HX8346(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_HIMAX_HX8357
	return vpostLCMInit_HIMAX_HX8357(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_ILITEK_ILI9481DS
	return vpostLCMInit_ILITEK_ILI9481DS(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_TECHTRON_R61505V
	return vpostLCMInit_TECHTRON_R61505V(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_RISETECH_OTA5182A
	return vpostLCMInit_RISETECH_OTA5182A(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_RISETECH_OTA5182A_CCIR656
	return vpostLCMInit_RISETECH_OTA5182A_CCIR656(plcdformatex, pFramebuf);
#endif

#ifdef HAVE_TOPPOLY_TD028THEA1
	return vpostLCMInit_TOPPOLY_TD028THEA1(plcdformatex, pFramebuf);
#endif	

#ifdef HAVE_SITRONIX_ST7567
	return vpostLCMInit_SITRONIX_ST7567(plcdformatex, pFramebuf);
#endif	

#ifdef HAVE_TIANMA_TM022HDH31
	return vpostLCMInit_TIANMA_TM022HDH31(plcdformatex, pFramebuf);
#endif
}

INT32 vpostLCMDeinit(void)
{
#ifdef HAVE_SHARP_LQ035Q1DH02
	return vpostLCMDeinit_SHARP_LQ035Q1DH02();
#endif

#ifdef HAVE_WINTEK_WMF3324
	return vpostLCMDeinit_WINTEK_WMF3324();
#endif

#ifdef HAVE_GIANTPLUS_GPM1006D0
	return vpostLCMDeinit_GIANTPLUS_GPM1006D0();
#endif

#ifdef HAVE_HANNSTAR_HSD043I9W1
	return vpostLCMDeinit_HANNSTAR_HSD043I9W1();
#endif

#ifdef HAVE_GOWORLD_GW8973
	return vpostLCMDeinit_GOWORLD_GW8973();
#endif

#ifdef HAVE_GOWORLD_GWMTF9406A
	return vpostLCMDeinit_GOWORLD_GWMTF9406A();
#endif

#ifdef HAVE_GOWORLD_GWMTF9360A
	return vpostLCMDeinit_GOWORLD_GWMTF9360A();
#endif

#ifdef HAVE_GOWORLD_GWMTF9615A
	return vpostLCMDeinit_GOWORLD_GWMTF9615A();
#endif
#ifdef HAVE_GOWORLD_GWMTF9360A_MODIFY
	return vpostLCMDeinit_GOWORLD_GWMTF9360A_modify();
#endif

#ifdef HAVE_VG680
	return vpostLCMDeinit_VG680();
#endif

#ifdef HAVE_TVOUT_720x480
	return vpostLCMDeinit_TVOUT_720x480();
#endif

#ifdef HAVE_TVOUT_640x480
	return vpostLCMDeinit_TVOUT_640x480();
#endif

#ifdef HAVE_TVOUT_320x240
	return vpostLCMDeinit_TVOUT_320x240();
#endif

#ifdef HAVE_AMPIRE_800x600
	return vpostLCMDeinit_AMPIRE_800x600();
#endif

#ifdef HAVE_AMPIRE_800x480
	return vpostLCMDeinit_AMPIRE_800x480();
#endif

#ifdef HAVE_GOWORLD_GW07000GNWV
	return vpostLCMDeinit_GOWORLD_GW07000GNWV();
#endif

#ifdef HAVE_KD070D10_800x480
	return vpostLCMDeinit_KD070D10_800x480();
#endif

#ifdef HAVE_HANNSTAR_HSD070IDW1
	return vpostLCMDeinit_HANNSTAR_HSD070IDW1();
#endif

#ifdef HAVE_HIMAX_HX8346
	return vpostLCMDeinit_HIMAX_HX8346();
#endif

#ifdef HAVE_HIMAX_HX8357
	return vpostLCMDeinit_HIMAX_HX8357();
#endif

#ifdef HAVE_ILITEK_ILI9481DS
	return vpostLCMDeinit_ILITEK_ILI9481DS();
#endif

#ifdef HAVE_TECHTRON_R61505V
	return vpostLCMDeinit_TECHTRON_R61505V();
#endif

#ifdef HAVE_RISETECH_OTA5182A
	return vpostLCMDeinit_RISETECH_OTA5182A();
#endif

#ifdef HAVE_RISETECH_OTA5182A_CCIR656
	return vpostLCMDeinit_RISETECH_OTA5182A_CCIR656();
#endif

#ifdef HAVE_TOPPOLY_TD028THEA1
	return vpostLCMDeinit_TOPPOLY_TD028THEA1();
#endif	

#ifdef HAVE_SITRONIX_ST7567
	return vpostLCMDeinit_SITRONIX_ST7567();
#endif	

#ifdef HAVE_TIANMA_TM022HDH31
	return vpostLCMDeinit_TIANMA_TM022HDH31();
#endif
}

VOID* vpostGetFrameBuffer(void)
{
    return g_VAFrameBuf;
}

VOID vpostSetFrameBuffer(UINT32 pFramebuf)
{ 
	g_VAFrameBuf = (VOID *)pFramebuf;
	g_VAFrameBuf = (VOID*)((UINT32)g_VAFrameBuf | 0x80000000);
    outpw(REG_LCM_FSADDR, (UINT32)pFramebuf);
}


void LCDDelay(unsigned int nCount)
{
	unsigned volatile int i;
		
	for(;nCount!=0;nCount--)
//		for(i=0;i<100;i++);
		for(i=0;i<10;i++);
}
