/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     FA93_VPOST_HIMAX_HX8346.c
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

#if defined(HAVE_HIMAX_HX8346)

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

static delay_loop(UINT32 u32delay)
{
	volatile UINT32 ii, jj;
	for (jj=0; jj<u32delay; jj++)
		for (ii=0; ii<200; ii++);
}

#define SpiEnable()		outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~0x0800)
#define SpiDisable()	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) |  0x0800)	
#define SpiHighSCK()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x2000)	
#define SpiLowSCK()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x2000)
#define SpiHighSDA()	outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) |  0x4000)	
#define SpiLowSDA()		outp32(REG_GPIOB_DOUT, inp32(REG_GPIOB_DOUT) & ~0x4000)



static INT Clock_Control(void)
{
}

static VOID DrvVPOST_MPULCM_RegIndexWrite_16Bits
(
	UINT8  u16AddrIndex		// LCD register address
)
{
	volatile UINT32 u32Flag, u32ModeBackup;

	u32ModeBackup = inp32(REG_LCM_MPUCMD);
	u32ModeBackup &= MPUCMD_MPU_SI_SEL;
	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~(MPUCMD_MPU_ON|MPUCMD_MPU_CS|MPUCMD_MPU_RWn) );	// CS=0 	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_CMD_DISn );					// turn on Command Mode			

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_WR_RS );					// RS=0 (write address)	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_RWn );					// Write Command/Data Selection			
	
	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD)&(~MPUCMD_MPU_SI_SEL)) | (0x05<<16));	// MPU 16-bit mode select
	
	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_CMD) | u16AddrIndex);	// WRITE register data
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_ON);						// trigger command output
	
	while(inp32(REG_LCM_MPUCMD) & MPUCMD_BUSY);								// wait command to be sent
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_ON );					// reset command ON flag
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_CS |MPUCMD_WR_RS);				// CS=1, RS=1	

	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD)&(~MPUCMD_MPU_SI_SEL)) | u32ModeBackup);// restore MPU mode 
	
	u32Flag = 1000;
	while( u32Flag--);	//delay for a while on purpose.
	
} // DrvVPOST_LCMRegIndexWrite_16Bits

static VOID DrvVPOST_MPULCM_RegWrite_16Bits
(
	UINT8  u16WriteData		// LCD register data
)
{
	volatile UINT32 u32ModeBackup;

	u32ModeBackup = inp32(REG_LCM_MPUCMD);
	u32ModeBackup &= MPUCMD_MPU_SI_SEL;

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~(MPUCMD_MPU_ON|MPUCMD_MPU_CS|MPUCMD_MPU_RWn) );	// CS=0 	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_CMD_DISn );					// turn on Command Mode			

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_WR_RS );						// RS=1	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_RWn );					// Write Command/Data Selection			

	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD)&(~MPUCMD_MPU_SI_SEL)) | (0x05<<16));	// MPU 16-bit mode select	
	
	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_CMD) | u16WriteData);	// WRITE register data
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_ON);						// trigger command output
	
	while(inp32(REG_LCM_MPUCMD) & MPUCMD_BUSY);								// wait command to be sent
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_ON );					// reset command ON flag
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_CS);						// CS=1
	
	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD)&(~MPUCMD_MPU_SI_SEL)) | u32ModeBackup);// restore MPU mode 	
   	
} // DrvVPOST_LCMRegWrite_16Bits


static void HIMAX_HX8346_Init(void)  
{  
/*       Gamma setting			*/
  	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x46);                                                                                                                                                                                                                  
	DrvVPOST_MPULCM_RegWrite_16Bits(0x45);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x47);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x54);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x48);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x49);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x67);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x4A);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x07);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x4B);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x07);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x4C);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x01);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x4D);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x77);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x4E);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00); 
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x4F);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x29);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x50);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x04); 
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x51);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x40);    

 /*   240x320 window setting    */	
 	// changed to 320x240 window
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x02);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x03);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
	
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x04);
   	DrvVPOST_MPULCM_RegWrite_16Bits(0x01);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x05);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x3F);

    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x06);
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x07);
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00); 

    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x08);
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x09);
    DrvVPOST_MPULCM_RegWrite_16Bits(0xEF);

 /*   	 Display setting        */    
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x01);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x06);
    
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x16);                                                                                                                                                                                                                 
//		DrvVPOST_MPULCM_RegWrite_16Bits(0x28);	// X-Y changed, RGB-BGR
	DrvVPOST_MPULCM_RegWrite_16Bits(0xa8);	// X-Y changed,X-mirror, RGB-BGR
//		DrvVPOST_MPULCM_RegWrite_16Bits(0xe8);	// X-Y changed,X-mirror, Y-mirror, RGB-BGR


    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x21);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x23);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x95);
    //DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x24);                                                                                                                                                                                                               
    // DrvVPOST_MPULCM_RegWrite_16Bits(0x95);
    // DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x25);                                                                                                                                                                                                              
	//DrvVPOST_MPULCM_RegWrite_16Bits(0xFF);  
	
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x28);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x01);
   // DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x29);                                                                                                                                                                                                               
	//DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
   //DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x2A);                                                                                                                                                                                                                
	//DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x2B);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
   // DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x2C);                                                                                                                                                                                                               
	//DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
   // DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x2D);                                                                                                                                                                                                               
	//DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
	
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x3A);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x01);
   //DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x3B);                                                                                                                                                                                                                
	//DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
   // DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x3C);                                                                                                                                                                                                               
	//DrvVPOST_MPULCM_RegWrite_16Bits(0xF0);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x3D);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
	

    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x36);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
  	
    delay_loop(200);

 /*  Power supply setting    */ 
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x19);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x43);
    delay_loop(100);

    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x20);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x50);
    
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x1D);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x25);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x1E);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x1F);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x06);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x44);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x40);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x45);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x0F);
    delay_loop(100);
    
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x1C);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x04);
    delay_loop(200);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x1B);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x18);
    
    delay_loop(300);
    
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x1B);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x10);
    
    delay_loop(300);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x43);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x80);
    delay_loop(600);

 /*   Display ON setting   */    
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x30);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x08);
    delay_loop(300);
    
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x26);                                                                                                                                                                                                                    
	DrvVPOST_MPULCM_RegWrite_16Bits(0x04);
	delay_loop(300);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x26);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x24);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x26);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x2C);
    delay_loop(300);
	
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x26);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x3C);
    
 /*Fixed the abnormal pixel data issue*/
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x57);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x02);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x56);                                                                                                                                                                                                                 
	DrvVPOST_MPULCM_RegWrite_16Bits(0x05);
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x57);
	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);
	
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x22);
    
    delay_loop(500);    

}


INT vpostLCMInit_HIMAX_HX8346(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_MPULCM_CTRL sMPU;
	volatile S_DRVVPOST_MPULCM_TIMING sTiming = {5,5,10,6};
	volatile S_DRVVPOST_MPULCM_WINDOW sWindow = {240,320};

	UINT32 nBytesPixel, u32PLLclk, u32ClockDivider;

	// VPOST clock control
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	delay_loop(2);			
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	delay_loop(2);				
	
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 27000);	// CLK_IN = 27 MHz
//	u32PLLclk = sysGetPLLOutputKhz(eSYS_UPLL, 12000);	// CLK_IN = 12 MHz
	u32PLLclk = GetPLLOutputKhz(eUPLL, 12000);			// CLK_IN = 12 MHz
	u32ClockDivider = u32PLLclk / 20000;
	
	u32ClockDivider /= 2;
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N0) | 1);						
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((u32ClockDivider & 0xFF) << 8));						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) | (3<<3));		// VPOST clock from UPLL		

	// reset LCM by GPIOD_12
	outp32(REG_GPDFUN, inp32(REG_GPDFUN)&~MF_GPD12);		// GPD_12 pin enabled

	outp32(REG_GPIOD_OMD, inp32(REG_GPIOD_OMD) | BIT12);	// GPD_12 is set to output mode
	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~BIT12);	// GPD_12 = LOW
    delay_loop(60);
	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) | BIT12);	// GPD_12 = HIGH
    delay_loop(100);

	vpostVAStopTriggerMPU();	

	// Enable VPOST function pins
	vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);	
//	vpostSetDataBusPin(eDRVVPOST_DATA_8BITS);	
//	outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~0x00C00000);		// disable VDEN pin
		  
	// LCD image source select
	vpostSetLCM_ImageSource(eDRVVPOST_FRAME_BUFFER);
	
	// configure LCD interface
	vpostSetLCM_TypeSelect(eDRVVPOST_MPU);	

	// configure LCD timing sync or async with TV timing	
	vpostsetLCM_TimingType(eDRVVPOST_ASYNC_TV);

	// MPU LCM initial 	
	HIMAX_HX8346_Init();
	
	// set MPU bus mode
	vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_16);
	
    // set MPU timing 
    vpostSetMPULCM_TimingConfig(&sTiming);

    // set MPU LCM window 
	vpostSetMPULCM_ImageWindow(&sWindow);
    
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
	
	// enable MPU LCD controller
	vpostVAStartTrigger_MPUContinue();
	
//	BacklightControl(TRUE);			
	return 0;
}

INT32 vpostLCMDeinit_HIMAX_HX8346(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}
#endif    //HAVE_HIMAX_HX8346