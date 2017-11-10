#include "wblib.h"
#include "W55FA93_VideoIn.h"
#include "W55FA93_GPIO.h"
#include "sensor_gm7150.h"
#include "DrvI2C.h"
#include "demo.h"

typedef struct
{
	UINT32 u32Width;
	UINT32 u32Height;
	char* pszFileName;
}S_VIDEOIN_REAL;
typedef struct
{
	UINT32 u32Width;
	UINT32 u32Height;
	E_VIDEOIN_OUT_FORMAT eFormat;
	char* pszFileName;
}S_VIDEOIN_PACKET_FMT;

struct OV_RegValue
{
	UINT8	u8RegAddr;		//Register Address
	UINT8	u8Value;			//Register Data
};

struct OV_RegTable{
	struct OV_RegValue *sRegTable;
	UINT32 u32TableSize;
};


extern UINT8 u8PlanarFrameBuffer[];


#define _REG_TABLE_SIZE(nTableName)	(sizeof(nTableName)/sizeof(struct OV_RegValue))


#define __NTSC__
//#define __PAL__			

static struct OV_RegValue g_sGM7150_Init[]=
{
	#include "GM7150\GM7150.dat"
};
static struct OV_RegTable g_OV_InitTable[] =
{//8 bit slave address, 8 bit data. 
	{g_sGM7150_Init,_REG_TABLE_SIZE(g_sGM7150_Init)},
	{0, 0}
};

static UINT8 g_uOvDeviceID[]= 
{
	0xBA,			/* If pin SIAD high, 8 bits slave address = 0xBA */				
					/* If pin SIAD low, 8 bits slave address = 0xB8 */	
};


/*
	Sensor power down and reset may default control on sensor daughter board.
	Reset by RC.
	Sensor alway power on (Keep low)

*/
static void SnrReset(void)
{
#ifdef __DEMO_BOARD__
/* GPB02 reset:	H->L->H 	*/				
	//gpio_open(GPIO_PORTB);					//GPIOB2 as GPIO		
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB2));
	
	gpio_setportval(GPIO_PORTB, 1<<2, 1<<2);	//GPIOB 2 set high default
	gpio_setportpull(GPIO_PORTB, 1<<2, 1<<2);	//GPIOB 2 pull-up 
	gpio_setportdir(GPIO_PORTB, 1<<2, 1<<2);	//GPIOB 2 output mode 
	Delay(1000);			
	gpio_setportval(GPIO_PORTB, 1<<2, 0<<2);	//GPIOB 2 set low
	Delay(1000);				
	gpio_setportval(GPIO_PORTB, 1<<2, 1<<2);	//GPIOb 2 set high
#endif
#ifdef __NUWICAM__
/* GPA7 reset:	H->L->H 	*/						
	outp32(REG_GPAFUN, inp32(REG_GPAFUN) & (~MF_GPA7));
	
	gpio_setportval(GPIO_PORTA, 1<<7, 1<<7);	//GPIOA 7 set high default
	gpio_setportpull(GPIO_PORTA, 1<<7, 1<<7);	//GPIOA 7 pull-up 
	gpio_setportdir(GPIO_PORTA, 1<<7, 1<<7);	//GPIOA 7 output mode 
	Delay(1000);			
	gpio_setportval(GPIO_PORTA, 1<<7, 0<<7);	//GPIOA 7 set low
	Delay(1000);				
	gpio_setportval(GPIO_PORTA, 1<<7, 1<<7);	//GPIOA 7 set high
#endif
}
#ifdef __DEMO_BOARD__
static void SnrPowerDown(BOOL bIsEnable)
{/* GPB3 power down, HIGH for power down */

	//gpio_open(GPIO_PORTB);						//GPIOB as GPIO
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB3));
	
	gpio_setportval(GPIO_PORTB, 1<<3, 1<<3);		//GPIOB 3 set high default
	gpio_setportpull(GPIO_PORTB, 1<<3, 1<<3);		//GPIOB 3 pull-up 
	gpio_setportdir(GPIO_PORTB, 1<<3, 1<<3);		//GPIOB 3 output mode 				
	if(bIsEnable)
		gpio_setportval(GPIO_PORTB, 1<<3, 1<<3);	//GPIOB 3 set high
	else				
		gpio_setportval(GPIO_PORTB, 1<<3, 0);		//GPIOB 3 set low
}
#endif
void GM7150SetVideoformat(int std)
{
	UINT8  uDeviceID;
	UINT8  fmt = 0;
	BOOL  ret;	
	
	uDeviceID  = g_uOvDeviceID[0];
	
	/* First tests should be against specific std */
	if (std == TVP_STD_ALL) {
		fmt=0;	/* Autodetect mode */
	} else if (std & TVP_STD_NTSC_443) {
		fmt=0xa;
	} else if (std & TVP_STD_PAL_M) {
		fmt=0x6;
	} else if (std & (TVP_STD_PAL_N| TVP_STD_PAL_Nc)) {
		fmt=0x8;
	} else {
		/* Then, test against generic ones */
		if (std & TVP_STD_NTSC) {
			fmt=0x2;
		} else if (std & TVP_STD_PAL) {
			fmt=0x4;
		}
	}
	sysprintf("GM7150 Video format : %02x. \r\n",fmt);
	ret = I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, GM7150_VIDEO_STD, fmt);
	if(ret == FALSE)
		sysprintf("Programminmg GM7150 REG 0x%x fail\n", GM7150_VIDEO_STD);
	else
		sysprintf("Programminmg GM7150 REG 0x%x successful\n", GM7150_VIDEO_STD);
}

void GM7150SetInputSource(int src)
{
	UINT8  uDeviceID;
	UINT8  inputsrc = 0;
	
	uDeviceID  = g_uOvDeviceID[0];
	
	/* First tests should be against specific std */
	if (src == GM7150_A1P1A) {
		inputsrc=0x00;	/* A1P1A mode */
		sysprintf("GM7150 Video Input Source : %02x. GM7150 Channel = A1P1A mode. \r\n",inputsrc);
	} else if (src == GM7150_A1P1B) {
		inputsrc=0x02;	/* A1P1B mode */
		sysprintf("GM7150 Video Input Source : %02x. GM7150 Channel = A1P1B mode. \r\n",inputsrc);
	} else {
		inputsrc=0x01;	/* S-Video mode */
		sysprintf("GM7150 Video Input Source : %02x. GM7150 Channel = SVIDEO mode. \r\n",inputsrc);
	}

	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, GM7150_VD_IN_SRC_SEL_1, inputsrc);
}

#if 0
static void TVP_I2C_Delay(UINT32 nCount)
{
	volatile UINT32 i;
	for(;nCount!=0;nCount--)
		for(i=0;i<50;i++);
}
#endif

VOID GM7150_Init(UINT32 nIndex)
{
	UINT32 u32Idx;
	UINT32 u32TableSize;
	UINT8  u8DeviceID;
	UINT8 u8ID;
	
	struct OV_RegValue *psRegValue;
	DBG_PRINTF("Sensor ID = %d\n", nIndex);
	if ( nIndex >= (sizeof(g_uOvDeviceID)/sizeof(UINT8)) )
		return;
	videoIn_Open(96000, 48000);								/* For sensor clock output */	

#ifdef __DEMO_BOARD__
	SnrPowerDown(FALSE);
#endif	
	SnrReset();											
		
	u32TableSize = g_OV_InitTable[nIndex].u32TableSize;
	psRegValue = g_OV_InitTable[nIndex].sRegTable;
	u8DeviceID = g_uOvDeviceID[nIndex];
	DBG_PRINTF("Device Slave Addr = 0x%x\n", u8DeviceID);
	if ( psRegValue == 0 )
		return;	
		
#ifdef __DEMO_BOARD__
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB13));
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB14));
	DrvI2C_Open(eDRVGPIO_GPIOB, 					
				eDRVGPIO_PIN13, 
				eDRVGPIO_GPIOB,
				eDRVGPIO_PIN14, 
				(PFN_DRVI2C_TIMEDELY)Delay);
#endif
#ifdef __NUWICAM__
	outp32(REG_GPAFUN, inp32(REG_GPAFUN) & (~MF_GPA3));
	outp32(REG_GPAFUN, inp32(REG_GPAFUN) & (~MF_GPA4));
	DrvI2C_Open(eDRVGPIO_GPIOA, 					
				eDRVGPIO_PIN3, 	//SCK
				eDRVGPIO_GPIOA,
				eDRVGPIO_PIN4, 	//SDA
				(PFN_DRVI2C_TIMEDELY)Delay);
#endif
							
	for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++){
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, (psRegValue->u8RegAddr), (psRegValue->u8Value));	
		Delay(1000);					
	}
	
#if defined(__PAL__)
	GM7150SetVideoformat(TVP_STD_PAL);	
#elif defined(__NTSC__)
	GM7150SetVideoformat(TVP_STD_NTSC);
#else		// default PAL
	GM7150SetVideoformat(TVP_STD_PAL);	
#endif
	GM7150SetInputSource(GM7150_A1P1A);

	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x0); //Device ID will read back 0x60
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	
	DrvI2C_Close();	
}


/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/

UINT32 Smpl_GM7150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1)
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;

	UINT32 u32GCD;
	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | HCLK4_CKE);
	
#if 0	
	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 
#endif	
#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	//videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_TVD_CCIR656);	
#endif	
	GM7150_Init(0);			
	videoIn_Open(96000, 48000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,
				FALSE,							//Polarity.	
				FALSE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_VYUY, 			//Input Order 
				eVIDEOIN_IN_YUV422	,	//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
#ifdef __NTSC__ 		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				2,							//Vertical start position (Y)
				2,							//Horizontal start position	(X)
				0);							//Useless
#endif
#ifdef __PAL__ 	
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				2,							//Vertical start position (Y)
				2,							//Horizontal start position	(X)
				0);							//Useless
#endif

				
	videoinIoctl(VIDEOIN_IOCTL_CROPPING_DIMENSION,				
				OPT_CROP_HEIGHT,							//UINT16 u16Height, 
				OPT_CROP_WIDTH,							//UINT16 u16Width;	
				0);							//Useless
	
	/* Set Packet dimension */
	u32GCD = GCD(OPT_PREVIEW_HEIGHT, OPT_CROP_HEIGHT);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/u32GCD);		
	u32GCD = GCD(OPT_PREVIEW_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_WIDTH/u32GCD,
				OPT_CROP_WIDTH/u32GCD);		
			
	/* Set Pipes stride */					
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,				
				640,
				0);
				
	/* Set Packet Buffer Address */			
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer0 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );

	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				1,	/* 0: Both fields are disabled. 1: Field 1 enable. 2: Field 2 enable. 3: Both fields are enable */
				eVIDEOIN_TYPE_CCIR656,	/* 0: CCIR601.	1: CCIR656 */
				0);		
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
                eVIDEOIN_PACKET,	
				0 );							//Useless		
	
		
	sysSetLocalInterrupt(ENABLE_IRQ);						
															
	return Successful;			
}	


UINT32 Smpl_GM7150_TwoFields(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1)
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;

	UINT32 u32GCD;
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | HCLK4_CKE);

#if 0	
	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 
#endif	
#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	//videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_TVD_CCIR656);	
#endif	
	GM7150_Init(0);			
	videoIn_Open(96000, 48000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,
				FALSE,							//Polarity.	
				FALSE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_VYUY, 			//Input Order 
				eVIDEOIN_IN_YUV422	,	//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
#ifdef __NTSC__ //OK		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				2,							//Vertical start position (Y)
				2,							//Horizontal start position	(X)
				0);							//Useless
#endif
#ifdef __PAL__ //Need Verify		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				2,							//Vertical start position (Y)
				2,							//Horizontal start position	(X)
				0);							//Useless
#endif

				
	videoinIoctl(VIDEOIN_IOCTL_CROPPING_DIMENSION,				
				OPT_CROP_HEIGHT,							//UINT16 u16Height, 
				OPT_CROP_WIDTH,							//UINT16 u16Width;	
				0);							//Useless
	
	/* Set Packet dimension */
	u32GCD = GCD(OPT_PREVIEW_HEIGHT, OPT_CROP_HEIGHT);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/u32GCD);		
	u32GCD = GCD(OPT_PREVIEW_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_WIDTH/u32GCD,
				OPT_CROP_WIDTH/u32GCD);		
				
	/* Set Pipes stride */					
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,				
				640,
				0);
				
	/* Set Packet Buffer Address */			
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer0 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );
			
	
	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				3,	/* 0: Both fields are disabled. 1: Field 1 enable. 2: Field 2 enable. 3: Both fields are enable */
				eVIDEOIN_TYPE_CCIR656,	/* 0: CCIR601.	1: CCIR656 */
				0);		
	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
	            eVIDEOIN_PACKET,	
				0 );							//Useless		
	
		
	sysSetLocalInterrupt(ENABLE_IRQ);						
															
	return Successful;			
}	