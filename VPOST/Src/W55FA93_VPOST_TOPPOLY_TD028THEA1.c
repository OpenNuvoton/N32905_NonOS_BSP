/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     NUC930_VPOST_TOPPOLY_TD028THEA1.c
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
//#include "NUC930_VPOST_Regs.h"
#include "w55fa93_vpost.h"

extern void LCDDelay(unsigned int nCount);

#ifdef	HAVE_TOPPOLY_TD028THEA1

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

static void BacklightControl(int OnOff)
{

}

int DrvGPIO_WriteByte(unsigned char u8Data, unsigned char u8Length)
{
	unsigned char   u8DataCount;
	unsigned int 	i32HoldPinValue;

	// use GPIOA[7,8]
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000100);
	// Write data to device and the most signification bit(MSB) first
	for ( u8DataCount=0; u8DataCount<u8Length; u8DataCount++ )
	{
		if ( u8Data & 0x80 )
			outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000100);
		else
			outpw(REG_GPIOA_DOUT, ((inpw(REG_GPIOA_DOUT) & 0x0000FFFF)&(~0x00000100) )& 0x0000FFFF);

		LCDDelay(3);
		
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000080);
		u8Data<<=1;
		LCDDelay(2);
		
		outpw(REG_GPIOA_DOUT, ((inpw(REG_GPIOA_DOUT) & 0x0000FFFF)&(~0x00000080) )& 0x0000FFFF);
		LCDDelay(2);
	}
		
	return 0;	
}

void TPG_WtReg(unsigned char nRegAddr, unsigned char nRegValue )
{	

	outpw(REG_GPIOB_DOUT, (inpw(REG_GPIOB_DOUT) & 0x00007FFF));

	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000080);
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000100);
	outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000080);
	outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000100);
	outpw(REG_GPIOA_PUEN, (inpw(REG_GPIOA_PUEN) & 0x0000FFFF)| 0x00000100);
		
	DrvGPIO_WriteByte(nRegAddr<<2, 8);
	DrvGPIO_WriteByte(nRegValue, 8);

	outpw(REG_GPIOB_DOUT, (inpw(REG_GPIOB_DOUT) & 0x0000FFFF)| 0x00008000);
}
	
void Topply_LCD_INIT(void)
{
	// use GPIOA[7,8], GPIOB[15]
	unsigned int u32PinFun;

	// store the setting of UART function pin
	u32PinFun = inpw(REG_PINFUN)& 0x80;		
	// disable UART function
	outpw(REG_PINFUN, inpw(REG_PINFUN) & 0xFFFFFF7F);
				
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000080);
	outpw(REG_GPIOB_OMD, (inpw(REG_GPIOB_OMD) & 0x0000FFFF)| 0x00008000);

	outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000080);
	outpw(REG_GPIOB_DOUT, (inpw(REG_GPIOB_DOUT) & 0x0000FFFF)| 0x00008000);

	outpw(REG_GPIOA_PUEN, (inpw(REG_GPIOA_PUEN) & 0x0000FFFF) | 0x00000080);
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000100);
	outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FEFF));
	outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FF7F));

	TPG_WtReg(0x03,0x00); 
	LCDDelay(100);

	TPG_WtReg(0x02,0x1);
	LCDDelay(100);
	TPG_WtReg(0x05,0x13);
	LCDDelay(100);

  	// restore the setting of UART function pin
	outpw(REG_PINFUN, (inpw(REG_PINFUN) & 0xFFFFFF7F)|u32PinFun);
		
}

static INT Clock_Control(void)
{
	// clock control
	if( (inpw(REG_PINFUN) & 0x00000300) ||	
			(inpw(REG_PINFUN) & 0x00000002) ||
			(inpw(REG_PINFUN) & 0x00000001) )
				return FALSE;
  	// 2.Enable IP!|s clock
	outpw(REG_CLKMAN, inpw(REG_CLKMAN) | 0x00000040);
  	// 3.Reset IP
	outpw(REG_RSTCON, inpw(REG_RSTCON) | 0x00000020);
	outpw(REG_CLKSEL, (inpw(REG_CLKSEL) & ~(0x10000000)) | (0x1<< 28));   // 27 MHz source
	outpw(REG_CLKSEL, (inpw(REG_CLKSEL) & ~(BIT(20,16))) | ((0x5 & 0x1F) << 16));  // 27 / 6 = 4.5 Mhz 
}

INT vpostLCMInit_TOPPOLY_TD028THEA1(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	UINT32 nBytesPixel;
	
	g_nScreenWidth = plcdformatex->nScreenWidth = 320;
	g_nScreenHeight = plcdformatex->nScreenHeight = 240;
	
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
	
	if(Clock_Control() == FALSE)
		return ERR_POWER_STATE;
		
	// configure LCD interface
    vpostSetLCDConfig(	1,    							/* Sync with TV 1:enable 0:disable */
    					DRVVPOST_LCDDATA_YUV422, 		/* Data Interface Select */
    					DRVVPOST_LCD_SYNCTYPE_TFT		/* LCD Tyep Select */
    					);

	// TV control register
    vpostSetTVEnableConfig(	DRVVPOST_FRAME_BUFFER_QVGA_SIZE, 	/* Frame Buffer Size in TV */
    						DRVVPOST_COLOR_FRAMEBUFFER, 		/* LCD Color Source */
    						DRVVPOST_COLOR_FRAMEBUFFER, 		/* TV Color Source */
    						1, 									/* Notch Filter 1:enable 0:disable */
    						0,									/* TV DAC 1:Disable 0:Enable */
    						0, 									/* 1:Interlance 0:Non-Interlance */
    						0, 									/* TV System Select 1:PAL 0:NTSC */
    						1									/* TV Encoder 1:enable 0:disable */
    						);
    
	Topply_LCD_INIT();
		
	outpw(REG_PINFUN, inpw(REG_PINFUN) | 0x00000060);
		  
	outpw(REG_LCM_TVCtl, (inpw(REG_LCM_TVCtl)& 0xFE000000)|(0x00F016A5));
	  
    // set Horizontal scanning line timing for Syn type LCD 
	outpw(REG_LCM_TCON1, (inpw(REG_LCM_TCON1)& 0xFF000000)|(0x0002EF26));
	  
	// set Vertical scanning line timing for Syn type LCD   
	outpw(REG_LCM_TCON2, (inpw(REG_LCM_TCON2)& 0xFF000000)|(0x00011400));
	  
	// set both "active pixel per line" and "active lines per screen" for Syn type LCD   
    outpw(REG_LCM_TCON3, 0x4FF00F0);
    /* 
    set Total Active Pixel number perl ine, 
    VSYNC/HSYNC/VDEN signals "Active" state  and clock polarity for Syn type LCD
    */
    outpw(REG_LCM_TCON4, (inpw(REG_LCM_TCON4)& 0xFC00FFF0)|(0x01400010));
    	
    /*set frambuffer start and end phy addr*/
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(plcdformatex, pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
	
	// enable LCD controller
	vpostSetLCDEnable(	1, 								/* YUV 1:little endian 0:big endian */
						plcdformatex->ucVASrcFormat, 			/* Frame Buffer Data Select */
						LCDCCtl_LCDRUN					/* LCD Controller Run */
						);
						
	BacklightControl(TRUE);			
	return 0;					
}

INT vpostLCMDeinit_TOPPOLY_TD028THEA1()
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
}
#endif    //HAVE_TOPPOLY_TD028THEA1