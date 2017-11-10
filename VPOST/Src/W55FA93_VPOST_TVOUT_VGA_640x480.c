/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2010 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     W55FA93_VPOST_TVOUT_640x480.c
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
#include "stdio.h"
#include "stdlib.h"
////#include "NUC930_VPOST_Regs.h"
//#include "w55fa93_vpost.h"
#include "w55fa93_vpost.h"

extern void LCDDelay(unsigned int nCount);

#ifdef	HAVE_TVOUT_640x480

typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

static UINT32 GetPLLOutputKhz(
	E_CLK eSysPll,
	UINT32 u32FinKHz
	)
{
	UINT32 u32Freq, u32PllCntlReg;
	UINT32 NF, NR, NO;
	
	UINT8 au8Map[4] = {1, 2, 2, 4};
	if(eSysPll==eSYS_APLL)
		u32PllCntlReg = inp32(REG_APLLCON);
	else if(eSysPll==eSYS_UPLL)	
		u32PllCntlReg = inp32(REG_UPLLCON);		
	
	NF = (u32PllCntlReg&FB_DV)+2;
	NR = ((u32PllCntlReg & IN_DV)>>9)+2;
	NO = au8Map[((u32PllCntlReg&OUT_DV)>>14)];
//	sysprintf("PLL regster = 0x%x\n", u32PllCntlReg);	
//	sysprintf("NF = %d\n", NF);
//	sysprintf("NR = %d\n", NR);
//	sysprintf("NOr = %d\n", NO);
		
	u32Freq = u32FinKHz*NF/NR/NO;
//	sysprintf("PLL Freq = %d\n", u32Freq);
	return u32Freq;
}

static void BacklightControl(int OnOff)
{

}

static INT Clock_Control(void)
{
}

INT vpostLCMInit_TVOUT_640x480(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;	
	UINT32 u32Clkin;	
	
#if 0	
	g_nScreenWidth = plcdformatex->nScreenWidth = 640;
	g_nScreenHeight = plcdformatex->nScreenHeight = 480;
	
	if ( (plcdformatex->ucVASrcFormat == DRVVPOST_FRAME_RGB555) || (plcdformatex->ucVASrcFormat == DRVVPOST_FRAME_RGB565) ||
		(plcdformatex->ucVASrcFormat == DRVVPOST_FRAME_CBYCRY) || (plcdformatex->ucVASrcFormat == DRVVPOST_FRAME_YCBYCR) ||
		(plcdformatex->ucVASrcFormat == DRVVPOST_FRAME_CRYCBY) || (plcdformatex->ucVASrcFormat == DRVVPOST_FRAME_YCRYCB))//2 BytesPixel
	{
	    nBytesPixel = 2;
	}
	else
	{
	    return ERR_BAD_PARAMETER;
	}
	plcdformatex->nFrameBufferSize = g_nScreenWidth*g_nScreenHeight*nBytesPixel;
#endif	

	
//	if(Clock_Control() == FALSE)
//		return ERR_POWER_STATE;

	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	

//	u32Clkin = sysGetExternalClock();
	
	if((inp32(REG_CHIPCFG) & 0xC) == 0x8)
		u32Clkin = 12000;
	else 
		u32Clkin = 27000;	
				
	if(u32Clkin == 27000)
	{
		// VPOST clock from 27MHz_clkin
		outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_N0);
		outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_N1);
		outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);	
	}		
	else
	{
//		u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
		u32PLLclk = GetPLLOutputKhz(eUPLL, u32Clkin);		// CLK_IN = 12 MHz
		u32ClockDivider = u32PLLclk / 27000;
		
		u32ClockDivider /= 2;
		u32ClockDivider--;
		outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0) | 1);						
		outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
		outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
		outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		
	}		

 	vpostVAStopTrigger();	
		
	// configure LCD interface  // enable sync with TV, data interface select : YUV422

    // TV control register
    vpostSetTVEnableConfig(	DRVVPOST_FRAME_BUFFER_VGA_SIZE, 	/* Frame Buffer Size in TV */
    						DRVVPOST_COLOR_FRAMEBUFFER, 		/* LCD Color Source */
    						DRVVPOST_COLOR_FRAMEBUFFER, 		/* TV Color Source */
//    						1, 									/* Notch Filter 1:enable 0:disable */
    						0, 									/* Notch Filter 1:enable 0:disable */    						
    						0,									/* TV DAC 1:Disable 0:Enable */
    						1, 									/* 1:Interlance 0:Non-Interlance */
//    						0, 									/* 1:Interlance 0:Non-Interlance */    						
    						0, 									/* TV System Select 1:PAL 0:NTSC */
    						1									/* TV Encoder 1:enable 0:disable */
    						);

#if 1
	outpw(REG_LCM_TVCtl, inpw(REG_LCM_TVCtl) | BIT26);		// enable polyphase fileterl

#endif
  
  
  	// set TV timing
//  	outpw(REG_LCM_TVDisCtl, 0x10f0129d);

    /*set frambuffer start and end phy addr*/
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable LCD controller
	vpostVAStartTrigger();

	return 0;
}

INT32 vpostLCMDeinit_TVOUT_640x480(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	return 0;
}

#endif    //HAVE_TVOUT_640x480