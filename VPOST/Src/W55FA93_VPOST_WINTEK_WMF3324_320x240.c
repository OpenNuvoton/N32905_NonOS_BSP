/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     W55FA93_VPOST_WINTEK_WMF3324.c
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
 *     2009.09.21		Created by xxx
 *
 *
 * REMARK
 *     None
 *
 *
 **************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "wblib.h"

#include "w55fa93_vpost.h"

extern void LCDDelay(unsigned int nCount);

#ifdef	HAVE_WINTEK_WMF3324

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

static void BacklightControl(int OnOff)
{
#if 0
	unsigned int u32PinFun;

	// store the setting of IIS PIN Enable 1
	u32PinFun = inpw(REG_PINFUN)& 0x40000;		
	// disable IIS PIN Enable 1 function
	outpw(REG_PINFUN, inpw(REG_PINFUN) & 0xFFFBFFFF);
	
	// GPE[7] set OUTPUT mode  => control the backlight
	outpw(REG_GPIOE_OMD, (inpw(REG_GPIOE_OMD) & 0x0000FFFF)| 0x00000080);
	if(OnOff==TRUE) {
		// GPE[7] turn on the backlight
		outpw(REG_GPIOE_DOUT, (inpw(REG_GPIOE_DOUT) & 0x0000FFFF)| 0x00000080);
	} else {
		// GPE[7] diable backlight
		outpw(REG_GPIOE_DOUT, (inpw(REG_GPIOE_DOUT) & 0x0000FFFF) & 0xFFFFFF7F);
	}
	
	// restore the setting of IIS PIN Enable 1
	//outpw(REG_PINFUN, (inpw(REG_PINFUN) & 0xFFFBFFFF)|u32PinFun);
#endif	
}

// GPB[5] = SCL
// GPB[6] = SDA
// GPB[4] = CS
#define CS_HIGH  do { outpw( REG_GPIOB_DOUT, inpw(REG_GPIOB_DOUT) | 1<<4); }while(0);
#define CS_LOW   do { outpw( REG_GPIOB_DOUT, inpw(REG_GPIOB_DOUT) & 0xFFFFFFEF); }while(0);
#define SCL_HIGH do { outpw( REG_GPIOB_DOUT, inpw(REG_GPIOB_DOUT) | 1<<5); }while(0);
#define SCL_LOW  do { outpw( REG_GPIOB_DOUT, inpw(REG_GPIOB_DOUT) & 0xFFFFFFDF); }while(0);
#define SDA_HIGH do { outpw( REG_GPIOB_DOUT, inpw(REG_GPIOB_DOUT) | 1<<6); }while(0);
#define SDA_LOW  do { outpw( REG_GPIOB_DOUT, inpw(REG_GPIOB_DOUT) & 0xFFFFFFBF); }while(0);

void sharp_register_set(unsigned short usRegIndex, unsigned short usRegData)
{
	int i;
	char regcmd = 0x74;
	char datacmd = 0x76;
	
	CS_HIGH
	SCL_HIGH
	SDA_HIGH
	CS_LOW  

	/* register setting */
	for(i=7;i>=0;i--)
 	{
 		SCL_LOW
  		if((regcmd >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW
   
  		LCDDelay(0x50);  
  		SCL_HIGH
  		LCDDelay(0x50);
 	}
	for(i=15;i>=0;i--)
 	{
 		SCL_LOW
  		if((usRegIndex >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW
   
  		LCDDelay(0x50); 
  		SCL_HIGH
  		LCDDelay(0x50);
 	}

	SDA_HIGH
	LCDDelay(0x25);
	CS_HIGH
	LCDDelay(0x50);

	/* data transfor  */
	CS_LOW

	for(i=7;i>=0;i--)
 	{
 		SCL_LOW
  		if((datacmd >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW

  		LCDDelay(0x50); 
  		SCL_HIGH
  		LCDDelay(0x50);
 	}
	for(i=15;i>=0;i--)
 	{
 		SCL_LOW
  		if((usRegData >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW

  		LCDDelay(0x50); 
  		SCL_HIGH
  		LCDDelay(0x50);
 	}

	SDA_HIGH
	LCDDelay(0x25);
	CS_HIGH
	LCDDelay(0x50);
	
} 

void sharp_read_register(unsigned short usRegIndex, unsigned short *usRegData)
{
	int i;
	char regcmd = 0x74;
	char datacmd = 0x77;
	
	CS_HIGH
	SCL_HIGH
	SDA_HIGH
	CS_LOW  

	/* register setting */
	for(i=7;i>=0;i--)
 	{
 		SCL_LOW
  		if((regcmd >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW
   
  		LCDDelay(0x50);  
  		SCL_HIGH
  		LCDDelay(0x50);
 	}
	for(i=15;i>=0;i--)
 	{
 		SCL_LOW
  		if((usRegIndex >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW
   
  		LCDDelay(0x50); 
  		SCL_HIGH
  		LCDDelay(0x50);
 	}

	SDA_HIGH
	LCDDelay(0x25);
	CS_HIGH
	LCDDelay(0x50);

	/* read data */
	CS_LOW

	for(i=7;i>=0;i--)
 	{
 		SCL_LOW
  		if((datacmd >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW

  		LCDDelay(0x50); 
  		SCL_HIGH
  		LCDDelay(0x50);
 	}
	for(i=15;i>=0;i--)
 	{
 		SCL_LOW
 		/*
  		if((usRegData >> i ) & 0x1)
   			SDA_HIGH 
  		else
   			SDA_LOW
		*/			
  		LCDDelay(0x50); 
  		SCL_HIGH
  		LCDDelay(0x50);
  		if( inpw(REG_GPIOA_DOUT) & 0x200)
			*usRegData |= 0x1<<i;
 	}

	SDA_HIGH
	LCDDelay(0x25);
	CS_HIGH
	LCDDelay(0x50);
	
}

void Sharp_LCD_INIT(void)
{
	unsigned int u32PinFun;
	unsigned short readData = 0x00;


	// set GPB[6:4] to GPIO mode
	outpw(REG_GPBFUN, inpw(REG_GPBFUN) & ~MF_GPB4);
	outpw(REG_GPBFUN, inpw(REG_GPBFUN) & ~MF_GPB5);
	outpw(REG_GPBFUN, inpw(REG_GPBFUN) & ~MF_GPB6);		
	
	// GPB[4] set OUTPUT mode
	outpw(REG_GPIOB_OMD, inpw(REG_GPIOB_OMD) | 0x00000010);
	// GPB[5] set OUTPUT mode
	outpw(REG_GPIOB_OMD, inpw(REG_GPIOB_OMD) | 0x00000020);
	// GPB[6] set OUTPUT mode
	outpw(REG_GPIOB_OMD, inpw(REG_GPIOB_OMD) | 0x00000040);

//	GPE[11] set OUTPUT mode as LCD_EN
//	outpw(REG_GPIOE_OMD, (inpw(REG_GPIOE_OMD) & 0x0000FFFF)| 0x00000800);
//	GPE[11] set LOW
//	outpw(REG_GPIOE_DOUT, (inpw(REG_GPIOE_DOUT) & 0xFFFFF7FF));
//	LCDDelay(100000);
	LCDDelay(1000);

	/* Initial sequence */
	sharp_register_set(0x01, 0x2AEF);    // Driver output control
	sharp_register_set(0x02, 0x0300);    // LCD driving AC control
	sharp_register_set(0x03, 0x080E);    // Power control 1
	sharp_register_set(0x0B, 0xD000);    // Frame cycle control
	sharp_register_set(0x0C, 0x0005);    // Power control 2
	sharp_register_set(0x0D, 0x000F);    // Power control 3
	sharp_register_set(0x0E, 0x2C00);    // Power control 4
	sharp_register_set(0x11, 0x0000);    // Shut and 8 color 
//	sharp_register_set(0x12, 0x0064);    // Entry mode  // SYNC mode
	sharp_register_set(0x12, 0x0060);    // Entry mode  // DEN mode
	sharp_register_set(0x16, 0x9F86);    // Pixel per line
	sharp_register_set(0x17, 0x0002);    // Vertical porch
	sharp_register_set(0x1E, 0x0000);    // Power control 5
	sharp_register_set(0x28, 0x0006);    // Extended command 1
	sharp_register_set(0x2A, 0x0187);    // Extended command 2
	sharp_register_set(0x30, 0x0000);    // Gamma Control 1
	sharp_register_set(0x31, 0x0103);    // Gamma Control 2
	sharp_register_set(0x32, 0x0001);    // Gamma Control 3
	sharp_register_set(0x33, 0x0501);    // Gamma Control 4
	sharp_register_set(0x34, 0x0607);    // Gamma Control 5
	sharp_register_set(0x35, 0x0406);    // Gamma Control 6
	sharp_register_set(0x36, 0x0707);    // Gamma Control 7
	sharp_register_set(0x37, 0x0305);    // Gamma Control 8
	sharp_register_set(0x3A, 0x0F0F);    // Gamma Control 9
	sharp_register_set(0x3B, 0x0F02);    // Gamma Control 10

}

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

static INT Clock_Control(void)
{
}

INT vpostLCMInit_WINTEK_WMF3324(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	
	volatile S_DRVVPOST_SYNCLCM_WINDOW sWindow = {320,240,320};	
//	volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming = {1,0x7,(UINT8)0x8};
//	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {1,0x1,0x2};
	volatile S_DRVVPOST_SYNCLCM_HTIMING sHTiming = {3,0x41,(UINT8)0x56};
	volatile S_DRVVPOST_SYNCLCM_VTIMING sVTiming = {2,0xD,0x3};
	volatile S_DRVVPOST_SYNCLCM_POLARITY sPolarity = {TRUE,TRUE,TRUE,FALSE};
	
	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;
	
	
//	WB_CLKFREQ_T pllClock;

//#define OPT_24BIT_MODE

	// VPOST clock control
//	Clock_Control();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	
//#define FA93_FPGA
#ifndef FA93_FPGA

//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 27000);	// CLK_IN = 27 MHz
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 12000);	// CLK_IN = 12 MHz
	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
//	u32ClockDivider = u32PLLclk / 9000;
	u32ClockDivider = u32PLLclk / 6500;
	
	u32ClockDivider /= 2;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0) | 1);						
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		
#endif

	vpostVAStopTrigger();	

	// Enable VPOST function pins
#ifdef	OPT_24BIT_MODE		
	vpostSetDataBusPin(eDRVVPOST_DATA_24BITS);
#else
	vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);
//	vpostSetDataBusPin(eDRVVPOST_DATA_18BITS);	
#endif	
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_HIGH_RESOLUTINO_SYNC);

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);
	
    // Configure Parallel LCD interface (16/18/24-bit data bus)
#ifdef	OPT_24BIT_MODE		
    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_24BITS);
#else    
    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_16BITS);
//    vpostSetParalelSyncLCM_Interface(eDRVVPOST_PRGB_18BITS);    
#endif    
    
//	Sharp_LCD_INIT();
	
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
	
	BacklightControl(TRUE);			
	return 0;

}

INT32 vpostLCMDeinit_WINTEK_WMF3324(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}
#endif    //HAVE_WINTEK_WMF3324