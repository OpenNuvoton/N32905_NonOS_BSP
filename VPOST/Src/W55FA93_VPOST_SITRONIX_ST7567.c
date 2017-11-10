/***************************************************************************
 *                                                                         *
 * Copyright (c) 2007 - 2009 Nuvoton Technology Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 *
 * FILENAME
 *     FA93_VPOST_SITRONIX_ST7567.c
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

#if defined(HAVE_SITRONIX_ST7567)

static UINT32 g_nScreenWidth;
static UINT32 g_nScreenHeight;

typedef enum 
{
	eEXT 	= 0,
	eX32K 	= 1,
	eAPLL  	= 2,
	eUPLL  	= 3
}E_CLK;

//LCM Init
//***************************************************************************************************
static UINT16  ST7567_InitCMD[]={
        0xa2, // bias,1/9   
        0xa0, //adc选择 set 0~131seg       
        0xc8,//0xc4, //COM     或者说SHL: set为com1～com64.   //Reverse direction (COM63->COM0)???   //should change to Normal direction (COM0->COM63)?? 0xc0  
        0x27, //a0   //0x27: RR =6.5                     
        0x81, //electronic volume mode set      
        0x2f,//12.3v  // EV=0x2f=47,  V0=RR X [ ( 99 + EV ) / 162 ] X 2.1 = 6.5*[(99+47)/162]*2.1 = 12.3V
        0xf8,//Set Booster
        0x02,// 10b = 6X 
        0xe3,//nop
        0xe3,//nop
        0x2f, //POWER control ：set vc、vr、vf 为on. 
        0xaf,  //display on. 
 };

static UINT16  ST7565_InitCMD[]={
        0xa2, // B0=1:1/7 Bias,B0=0:1/9 Bias 
        0xa0, //B0=1:ADCReverse ,B0=0:ADC Normal
        0xc8,//B3=1:Reverse DIR,B3=0:Normal DIR
        0xA4, //B0=1:Display All dot,B0=0:Normal Display                    
        0x40, //B5~0=000000:Set Display Start Line 0     
        0x2f,//;B2=1:Converter ON,B2=0:Converter OFF; B1=1:Regulator ON,B1=0:Regulator OFF  ;B0=1:Follower ON,B0=0:Follower OFF
        0x24,//B2~0=111:V5 Voltage Ratio 6.48
        0x81,// Set V5 Voltage
        0x20,// B5~0=100000
        0xf8,//Select Booster Ratio
        0x00,//0x02,//;B1~0=00:2x,3x,4x;B1~0=01:5x;B1~0=10:6x
        0xA6, //B0=1:Dot Reverse,B0=0:Dot Normal 
        0xaf,  //B0=1:LCD Display ON,B0=0:LCD Display OFF
 };

//P090410HA  (ST7565R)
static UINT16  ST7565R_InitCMD[]={
	0xae,0xa2,0xa0,0xc0,0x24,0x81,0x25,0x2c,0x2e,
    0xc8,//B3=1:Reverse DIR,B3=0:Normal DIR
	0x2f,0xaf
};

#define ST7565R_INITCMD_NUM     sizeof(ST7565R_InitCMD)/sizeof(UINT16)
#define ST7567_INITCMD_NUM     sizeof(ST7567_InitCMD)/sizeof(UINT16)

//#define DEBUG
#define DISPLAY_WIDTH				128
#define DISPLAY_HEIGHT				64
#define ADC_REVERSE_PLUS			0

/*  MPU Command for ST7565S chip  */
#define CMD_DISPLAY_OFF            	(0xAE)
#define CMD_DISPLAY_ON             	(0xAF)
#define CMD_SET_START_LINE         	(0x40)
#define CMD_SET_PAGE               	(0xB0)
#define CMD_COL_HIGH_ADDR          	(0x10)
#define CMD_COL_LOW_ADDR           	(0x00)
#define CMD_SET_ADC_NORMAL 			(0xA0)
#define CMD_SET_ADC_REVERSE 		(0xA1)
#define CMD_ALL_POINT_ON_NORMAL    	(0xA4)
#define CMD_ALL_POINT_ON           	(0xA5)
#define CMD_NORMAL_DISPLAY			(0xA6)
#define CMD_REVERSE_DISPLAY        	(0xA7)
#define CMD_READ_MODIFY_WRITE		(0xE0)
#define CMD_END						(0xEE)
#define CMD_RESET					(0xE2)
#define CMD_COM_NORMAL_DIRECTION	(0xC0)
#define CMD_COM_REVERSE_DIRECTION	(0xC8)
#define CMD_INDICATOR_MODE_OFF		(0xAC)
#define CMD_INDICATOR_MODE_ON		(0xAD)
#define CMD_SET_INDICATOR_OFF		(0x00)
#define CMD_SET_INDICATOR_1			(0x01)
#define CMD_SET_INDICATOR_1_2		(0x02)
#define CMD_SET_INDICATOR_ON		(0x03)
#define CMD_SET_EV_MODE				(0x81)
#define CMD_LCD_BIAS_1_9			(0xA2)
#define CMD_LCD_BIAS_1_7			(0xA3)
#define CMD_SET_BOOSTER_RATIO_MODE	(0xF8)
#define CMD_NOP						(0xE3)

/*  Return Error Code  */
#define SUCCESSFUL					0
#define FAIL						1
#define ERR_OUT_RANGE				-FAIL

/* Draw */
#define BLACK_MONO					1
#define WHITE_MONO					2



static delay_loop(UINT32 u32delay)
{
	volatile UINT32 ii, jj;
	for (jj=0; jj<u32delay; jj++)
		for (ii=0; ii<200; ii++);
}



static VOID DrvVPOST_MPULCM_RegIndexWrite_16Bits
(
	UINT8  u16AddrIndex		// LCD register address
)
{
	volatile UINT32 u32Flag, u32ModeBackup;
	volatile int ii;

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~(MPUCMD_MPU_ON|MPUCMD_MPU_CS|MPUCMD_MPU_RWn) );	// CS=0 	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_CMD_DISn );					// turn on Command Mode			

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_WR_RS );					// RS=0 (write address)	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_RWn );					// Write Command/Data Selection			
	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_CMD) | u16AddrIndex);	// WRITE register data
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_ON);						// trigger command output
	for(ii=0; ii<20; ii++);
	while(inp32(REG_LCM_MPUCMD) & MPUCMD_BUSY);								// wait command to be sent
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_ON );					// reset command ON flag
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_CS |MPUCMD_WR_RS);				// CS=1, RS=1	
	
	u32Flag = 1000;
	while( u32Flag--);	//delay for a while on purpose.
	
} // DrvVPOST_LCMRegIndexWrite_16Bits

static VOID DrvVPOST_MPULCM_RegWrite_16Bits
(
	UINT8  u16WriteData		// LCD register data
)
{
	volatile UINT32 u32ModeBackup;
	volatile int ii;	

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~(MPUCMD_MPU_ON|MPUCMD_MPU_CS|MPUCMD_MPU_RWn) );	// CS=0 	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_CMD_DISn );					// turn on Command Mode			

	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_WR_RS );						// RS=1	
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_RWn );					// Write Command/Data Selection			
	outp32(REG_LCM_MPUCMD, (inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_CMD) | u16WriteData);	// WRITE register data
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_ON);						// trigger command output
	for(ii=0; ii<20; ii++);	
	while(inp32(REG_LCM_MPUCMD) & MPUCMD_BUSY);								// wait command to be sent
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) & ~MPUCMD_MPU_ON );					// reset command ON flag
	outp32(REG_LCM_MPUCMD, inp32(REG_LCM_MPUCMD) | MPUCMD_MPU_CS);						// CS=1
} // DrvVPOST_LCMRegWrite_16Bits

int GetColumnM(int column)
{
	int ret;
	#if ADC_REVERSE_PLUS
		column+=4;
	#endif
	ret = column>>4;
	return ret;
}
int GetColumnL(int column)
{
	int ret;
	#if ADC_REVERSE_PLUS
		column+=4;
	#endif
	ret = column;
	ret = ret & 0xf;
	return ret;
}

void StandbyModeOff(void)
{
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_ALL_POINT_ON_NORMAL);  								// Cancel standby mode
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_NOP);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_NOP);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_DISPLAY_ON);  										// DISPLAY ON
}
void StandbyModeOn(void)
{
	//outpw(REG_LCM_DEV_CTRL,0xA40000E0);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_INDICATOR_MODE_ON);
	//outpw(REG_LCM_DEV_CTRL,0x800000E0);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_DISPLAY_OFF);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_ALL_POINT_ON);
	
}
void SleepModeOff(void)
{
	
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_ALL_POINT_ON_NORMAL);  // Cancel sleep mode
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_NOP);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_NOP);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_DISPLAY_ON);  // DISPLAY ON
	
}
void SleepModeOn(void)
{
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_INDICATOR_MODE_OFF);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_DISPLAY_OFF);
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_ALL_POINT_ON);
}
void clear_all_screen(void)
{
    int ColCount = 0;
    int Page = 0;
    int column = 0;
	
    DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_SET_START_LINE+0);  //Set Start Line 0
       
    for(Page = 0; Page < 8; Page++)
    {
    	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_SET_PAGE+Page); //Set page
        DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_COL_HIGH_ADDR+ GetColumnM(column)); //Higher column address
        DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_COL_LOW_ADDR + GetColumnL(column)); //Lower column address  //Col 8
        
        for(ColCount =0; ColCount< 132; ColCount++)
        {
        	DrvVPOST_MPULCM_RegWrite_16Bits(0x00);  
        }
    }
}


/* font text draw api is called by library */
void bitmap_draw(int column, int row, int width, int height, unsigned char *bitmap)
{
	unsigned char *buffer;  // temporary buffer
	int page_num=0;         // page number for command
	int shift_bits=0;       // how many bits to shift the bitmap
	int write_page_count=0;  // writing count on page according the height
	int write_page_count_tmp=0;
	int page_time=1;
	int table_index=0;      // the index of shifted buffer 
	
	if(row >= DISPLAY_HEIGHT || column >= DISPLAY_WIDTH) return;
	
	/* calculate the first page number and how many bits to shift */
	page_num = row / 8;  						// start page number
	shift_bits = row % 8; 						// bitmap source shift bits

	/* how many page count on panel for buffer bitmap needed */
	write_page_count = height/8;
	if((height%8) > 0) write_page_count++;
	while(height > (8-shift_bits)+(8*(write_page_count-1))) {
		write_page_count++;
	}
	//printf("row is %i, write_page_count is %i\n",row, write_page_count);
	
	write_page_count_tmp = write_page_count;
	
	/* Create temporary buffer for storing bitmap */
	//buffer = (unsigned char *)malloc(write_page_count*width);
	//memset(buffer, 0, sizeof(buffer));
	
	DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_SET_START_LINE);   	// display start line 0
			
	//shift_bitmap(column, row, width, height, write_page_count, buffer, shift_bits, bitmap);  // shift bitmap source
	buffer = bitmap;
	
	while(write_page_count_tmp>0)
	{	
		DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_SET_PAGE+page_num);   					// page number
		DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_COL_HIGH_ADDR+ GetColumnM(column));   	// colume MSB
		DrvVPOST_MPULCM_RegIndexWrite_16Bits(CMD_COL_LOW_ADDR + GetColumnL(column));   	// colume LSB
		while(table_index < width*page_time)
		{
			DrvVPOST_MPULCM_RegWrite_16Bits(buffer[table_index]);
			table_index++;
		}
		page_time++;
		write_page_count_tmp--;
		page_num++;
	}
		
	/* free the temporary buffer */
	//free(buffer);
}

static void SITRONIX_ST7567_Init(void)  
{  
	UINT32 ii;

//	outpw(REG_LCM_LCDCPrm, inpw(REG_LCM_LCDCPrm) | LCDCPrm_LCDSynTv);	// sync TV	
	
     for(ii = 0; ii < ST7567_INITCMD_NUM; ii++)
//     for(ii = 0; ii < ST7565R_INITCMD_NUM; ii++)     
     {
		DrvVPOST_MPULCM_RegIndexWrite_16Bits(ST7565R_InitCMD[ii]);
	 	delay_loop(50);    
     }
 	delay_loop(200);         
//	outpw(REG_LCM_LCDCPrm, (inpw(REG_LCM_LCDCPrm) & (~LCDCPrm_LCDSynTv)));	// async TV     
}


INT vpostLCMInit_SITRONIX_ST7567(PLCDFORMATEX plcdformatex, UINT32 *pFramebuf)
{
	volatile S_DRVVPOST_MPULCM_CTRL sMPU;
	volatile S_DRVVPOST_MPULCM_TIMING sTiming = {5,5,10,10};
//	volatile S_DRVVPOST_MPULCM_TIMING sTiming = {2,0x7c,0xeb,0x84};	
	volatile S_DRVVPOST_MPULCM_WINDOW sWindow = {240,320};

	UINT32 nBytesPixel, u32PLLclk;

	// VPOST clock control
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) | VPOST_CKE | HCLK4_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | VPOSTRST);
	delay_loop(2);			
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~VPOSTRST);	
	delay_loop(2);				
	
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_N0);						
	outpw(REG_CLKDIV1, (inpw(REG_CLKDIV1) & ~VPOST_N1) | ((2 & 0xFF) << 8));	// clock divider = 3						
	outpw(REG_CLKDIV1, inpw(REG_CLKDIV1) & ~VPOST_S);	// VPOT clock from clock_in

	outpw(REG_LCM_LCDCCtl,inpw(REG_LCM_LCDCCtl) & ~LCDCCtl_LCDRUN); // diable VPOST
	
	vpostVAStopTriggerMPU();	

	// Enable VPOST function pins
	vpostSetDataBusPin(eDRVVPOST_DATA_16BITS);	
		  
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

	// set frame buffer data format
//	vpostSetFrameBuffer_DataType(plcdformatex->ucVASrcFormat);
	vpostSetFrameBuffer_DataType(eDRVVPOST_FRAME_RGB565);	
	
	vpostSetYUVEndianSelect(eDRVVPOST_YUV_LITTLE_ENDIAN);

	// MPU LCM initial 	
	SITRONIX_ST7567_Init();
	
	return 0;
}

INT32 vpostLCMDeinit_SITRONIX_ST7567(VOID)
{
	vpostVAStopTrigger();
	vpostFreeVABuffer();
	outpw(REG_AHBCLK, inpw(REG_AHBCLK) & ~VPOST_CKE);	
	return 0;
}
#endif    //HAVE_SITRONIX_ST7567