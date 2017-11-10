/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     FA93_VPOST_RISETECH_OTA5182A.c
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
#include "w55fa93_vpost.h"

extern void LCDDelay(unsigned int nCount);

#if defined(HAVE_RISETECH_OTA5182A)

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;


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
	// GPA[11] set OUTPUT mode  => control the backlight
	outpw(REG_GPIOA_OMD, (inpw(REG_GPIOA_OMD) & 0x0000FFFF)| 0x00000800);
	if(OnOff==TRUE) {
		// GPA[11] turn on the backlight
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF)| 0x00000800);
	} else {
		// GPA[11] diable backlight
		outpw(REG_GPIOA_DOUT, (inpw(REG_GPIOA_DOUT) & 0x0000FFFF) & 0xFFFFF7FF);
	}
}

static delay_loop(UINT8 u8delay)
{
	volatile UINT8 ii, jj;
	for (jj=0; jj<u8delay; jj++)
		for (ii=0; ii<200; ii++);
}

#define SpiEnable()		outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) & ~0x0080)
#define SpiDisable()	outp32(REG_GPIOA_DOUT, inp32(REG_GPIOA_DOUT) |  0x0080)	
#define SpiHighSCK()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x2000)	
#define SpiLowSCK()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x2000)
#define SpiHighSDA()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x4000)	
#define SpiLowSDA()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x4000)

void OTA5182A_WriteReg(UINT8 u8Reg, UINT16 u16Data)
{
	UINT16 ii;
	UINT32 u32SentData;
	
	// write register index
	u32SentData = u8Reg;
	u32SentData <<= 12;
	u32SentData |= (0x7FF & u16Data);	

	SpiDisable();
	SpiHighSCK();
	SpiHighSDA();
	delay_loop(2);
	SpiEnable();
	delay_loop(1);	

	for (ii=0; ii<16; ii++)
	{
		SpiLowSCK();
		delay_loop(1);			
		if (u32SentData & BIT15)
			SpiHighSDA();
		else
			SpiLowSDA();
		delay_loop(1);						
		SpiHighSCK();			
		delay_loop(1);								
		u32SentData <<= 1;		
	}
	SpiHighSCK();				
	SpiHighSDA();	
	delay_loop(1);									
	SpiDisable();	
	delay_loop(5);						
}

void RISETECH_OTA5182A_Init(void)
{
	outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA7);				// set GPD_7 to be GPIO pin
	outpw(REG_GPBFUN, inpw(REG_GPBFUN) & ~0x3C000000);			// set GPB_14/GPB_13 to be GPIO pins	
	
	// set output mode
	outpw(REG_GPIOB_OMD, inpw(REG_GPIOB_OMD) | (BIT14+BIT13));	// set GPB_14/GPB_13 to be output mode
//	outpw(REG_GPIOD_OMD, inpw(REG_GPIOB_OMD) | (BIT14+BIT13));	// set GPD_11 to be output mode	
	outpw(REG_GPIOA_OMD, inpw(REG_GPIOA_OMD) | (BIT7));			// set GPA_7 to be output mode	

	// set internal pull-up resistor
	outpw(REG_GPIOB_PUEN, inpw(REG_GPIOB_PUEN) | (BIT14+BIT13));// set GPB_14/GPB_13 to be internal pullup
	outpw(REG_GPIOA_PUEN, inpw(REG_GPIOA_PUEN) | (BIT7));		// set GPA_7 to be internal pullup
	
	// initial LCM register
	delay_loop(10);		

	OTA5182A_WriteReg(0x06, 0x07); 	// CCIR656
}

static INT Clock_Control(void)
{
}

INT vpostLCMInit_RISETECH_OTA5182A(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow = {480,234,160};	
//	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow = {640,320,240};	
	volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming = {1,96,(UINT8)5};
	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {2,16,10};	
	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {TRUE,TRUE,FALSE,TRUE};

	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;

	// VPOST clock control
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	delay_loop(2);			
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	delay_loop(2);				
	
	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 27000);	// CLK_IN = 27 MHz
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 12000);	// CLK_IN = 12 MHz
//	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
	u32ClockDivider = u32PLLclk / 27000;
	
	u32ClockDivider /= 2;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0) | 1);						
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		

	vpostVAStopTrigger();	

	// Enable VPOST function pins
	vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);	
//	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~0x00C00000);		// disable VDEN pin (must be disabled)
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_SYNC);
//	vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);	

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);
	
    // Configure Serial LCD interface (8-bit data bus)
    vpostSetSerialSyncLCM_Interface(eDRVVPOST_SRGB_RGBTHROUGH);    
 

    // Configure Serial LCD interface Even/Odd lin color order
	vpostSetSerialSyncLCM_ColorOrder(eDRVVPOST_SRGB_GBR, eDRVVPOST_SRGB_RGB); 

//	RISETECH_OTA5182A_Init();
	
    // set Horizontal scanning line timing for Syn type LCD 
    vpostSetSyncLCM_HTiming(&sHTiming);

	// set Vertical scanning line timing for Syn type LCD   
    vpostSetSyncLCM_VTiming(&sVTiming);
	
	// set both "active pixel per line" and "active lines per screen" for Syn type LCD   
	vpostSetSyncLCM_ImageWindow(&sWindow);

  	// set Hsync/Vsync/Vden/Pclk poalrity
	vpostSetSyncLCM_SignalPolarity(&sPolarity);  	
    
#if 0
	vpostSetFrameBuffer_BaseAddress(pFramebuf);
#else    
    // set frambuffer base address
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
#endif    
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable LCD controller
	vpostVAStartTrigger();
	
//	BacklightControl(TRUE);			
	return 0;
}

INT32 vpostLCMDeinit_RISETECH_OTA5182A(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}
#endif    //HAVE_RISETECH_OTA5182A