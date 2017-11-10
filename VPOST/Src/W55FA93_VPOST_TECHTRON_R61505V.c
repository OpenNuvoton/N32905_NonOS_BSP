/***************************************************************************
 *                                                                         *
 * Copyright (c) 2009 - 2010 Iflytek Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *     W55FA93_VPOST_R61505V.c
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
 *     2010.07.07		Created by yongwu@iflytek.com
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

#ifdef HAVE_TECHTRON_R61505V

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

#define NUC930_VA_MPU2EBI_BASE 		0x90000000
#define NUC930_PA_MPU2EBI_BASE		0x90000000

//#define DRVMPU_FEQ81MHZ 0x6223A

unsigned short *g_pu16Data = (unsigned short *)NUC930_VA_MPU2EBI_BASE;

static delay_loop(UINT32 u32delay)
{
	volatile UINT32 ii, jj;
	for (jj=0; jj<u32delay; jj++)
		for (ii=0; ii<200; ii++);
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

static void BacklightControl(int OnOff)
{
	outp32(REG_GPDFUN, inp32(REG_GPDFUN)&~MF_GPD12);		// GPD_12 pin enabled

	outp32(REG_GPIOD_OMD, inp32(REG_GPIOD_OMD) | BIT12);	// GPD_12 is set to output mode
	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) & ~BIT12);	// GPD_12 = LOW
    delay_loop(2);
	outp32(REG_GPIOD_DOUT, inp32(REG_GPIOD_DOUT) | BIT12);	// GPD_12 = HIGH
    delay_loop(10);
}

static VOID DrvVPOST_MPULCM_RegIndexWrite_16Bits
(
	UINT16  u16AddrIndex		// LCD register address
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
	UINT16  u16WriteData		// LCD register data
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



VOID WriteReg(UINT16 u16RegIndex, UINT16 u16RegContents )
{
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(u16RegIndex);
	DrvVPOST_MPULCM_RegWrite_16Bits(u16RegContents);
	LCDDelay(1);
}

void R61505V_Init(void)
{	
	UINT16 u16Code;
	
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x00);
	u16Code = vpostMPULCDReadData16Bit();
	
	// DISPLAY OFF
	//WriteReg(0x0007,0x0000);
	//POWER OFF SEQUENCE
	//WriteReg(0x0012,0x0000);
	//LCDDelay(200);
	
	//INIT STARTUP
	WriteReg(0x00A4,0x0001);
	LCDDelay(50);
	
	WriteReg(0x0001,0x0100);
	WriteReg(0x0002,0x0700);
	WriteReg(0x0003,0x1028);
	WriteReg(0x0008,0x0303);	
	WriteReg(0x000A,0x0000);
	WriteReg(0x000C,0x0000);
	WriteReg(0x000D,0x0000);
	WriteReg(0x000e,0x0000);
	
	//POWER ON SEQUENCE
	WriteReg(0x0010,0x0710);
	WriteReg(0x0011,0x0137);
	WriteReg(0x0012,0x01b8);
	LCDDelay(200);
	WriteReg(0x0013,0x0f00);     
	WriteReg(0x002a,0x0080);
	WriteReg(0x0029,0x003F);    //0x003A
	LCDDelay(200);
	
	// SET RAM ADDRESS
	WriteReg(0x0020,0x00EF);
	WriteReg(0x0021,0x0000);
	
	//ADJUST THE GAMMA CURVE
	WriteReg(0x0030,0x0214);
	WriteReg(0x0031,0x3715);
	WriteReg(0x0032,0x0604);
	WriteReg(0x0033,0x0e16);
	WriteReg(0x0034,0x2211);
	/*
	WriteReg(0x0035,0x1500);
	WriteReg(0x0036,0x8507);
	WriteReg(0x0037,0x1407);
	WriteReg(0x0038,0x1403);
	WriteReg(0x0039,0x0020);*/
	
	WriteReg(0x0035,0x0214);
	WriteReg(0x0036,0x3715);
	WriteReg(0x0037,0x0604);
	WriteReg(0x0038,0x0e16);
	WriteReg(0x0039,0x2211);
	
	//  SET GRAM AREA
	WriteReg(0x0050,0x0000);
	WriteReg(0x0051,0x00EF);
	WriteReg(0x0052,0x0000);
	WriteReg(0x0053,0x013F);
	WriteReg(0x0060,0x2700);
	WriteReg(0x0061,0x0001);
	WriteReg(0x006A,0x0000);
	
	//Partial Display Control
	WriteReg(0x0080, 0x0000);
	WriteReg(0x0081, 0x0000);
	WriteReg(0x0082, 0x0000);
	
	//Panel Control
	WriteReg(0x0090,0x001a);
	WriteReg(0x0092,0x0000);
	WriteReg(0x0093,0x0000);
	
	// DISPLAY ON
	WriteReg(0x0007,0x0100);
	LCDDelay(100);

	DrvVPOST_MPULCM_RegIndexWrite_16Bits(0x22 );
}

void LCD_EnterSleep_R61505V(void)
{
	WriteReg(0x0007, 0x0131); // Set D1=0, D0=1
	LCDDelay(10);
	WriteReg(0x0007, 0x0130); // Set D1=0, D0=0
	LCDDelay(10);
	WriteReg(0x0007, 0x0000); // display OFF
	//************* Power OFF sequence **************//
	WriteReg(0x0010, 0x0000); // SAP, BT[3:0], APE, AP, DSTB, SLP
	WriteReg(0x0011, 0x0000); // DC1[2:0], DC0[2:0], VC[2:0]
	WriteReg(0x0012, 0x0000); // VREG1OUT voltage
	WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	LCDDelay(200); // Dis-charge capacitor power voltage
	WriteReg(0x0010, 0x0002); // SAP, BT[3:0], APE, AP, DSTB, SLP
}

void LCD_ExitSleep_R61505V(void)
{
	//*************Power On sequence ******************//
	WriteReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP
	WriteReg(0x0011, 0x0000); // DC1[2:0], DC0[2:0], VC[2:0]
	WriteReg(0x0012, 0x0000); // VREG1OUT voltage
	WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
	LCDDelay(200); // Dis-charge capacitor power voltage
	WriteReg(0x0010, 0x1490); // SAP, BT[3:0], AP, DSTB, SLP, STB
	WriteReg(0x0011, 0x0227); // R11h=0x0221 at VCI=3.3V DC1[2:0], DC0[2:0], VC[2:0]
	LCDDelay(50); // Delay 50ms
	WriteReg(0x0012, 0x001A); // External reference voltage =Vci;
	LCDDelay(50); // Delay 50ms
	WriteReg(0x0013, 0x0F00); // Set VDV[4:0] for VCOM amplitude
	WriteReg(0x0029, 0x0019); // Set VCM[5:0] for VCOMH
	LCDDelay(50); // Delay 50ms
	WriteReg(0x0007, 0x0133); // 262K color and display ON
}

static INT Clock_Control(void)
{
}

int vpostLCMInit_TECHTRON_R61505V(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
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

	// set MPU bus mode
	vpostSetMPULCM_BusModeSelect(eDRVVPOST_MPU_16);
	
    // set MPU timing 
    vpostSetMPULCM_TimingConfig(&sTiming);

	// panel init
	R61505V_Init();
	
    // set MPU LCM window 
	vpostSetMPULCM_ImageWindow(&sWindow);
    
    // set frambuffer base address
    if(pFramebuf != NULL) {
		vpostAllocVABufferFromAP(pFramebuf);
	} else {
    	if( vpostAllocVABuffer(plcdformatex, nBytesPixel)==FALSE)
    		return ERR_NULL_BUF;
    }
	
	// set frame buffer data format
	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);
	
	// enable MPU LCD controller
	vpostVAStartTrigger_MPUContinue();
	
	BacklightControl(TRUE);
	return 0;
}
INT vpostLCMDeinit_TECHTRON_R61505V()
{
	BacklightControl(FALSE);
	LCD_EnterSleep_R61505V();
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
	
}

#endif   //HAVE_TECHTRON_R61505V
