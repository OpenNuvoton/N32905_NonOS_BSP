#include "wblib.h"
#include "W55FA93_VideoIn.h"
#include "W55FA93_GPIO.h"
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

static struct OV_RegValue g_sOV9660_RegValue[]=
{//OV9660
		{0x12, 0x80},
	#if 1
		{0xd5, 0xff}, {0xd6, 0x3f}, {0x3d, 0x3c}, {0x11, 0x80},	{0x2a, 0x00}, {0x2b, 0x00},	//PCLK = SCLK
	#else		
		{0xd5, 0xff}, {0xd6, 0x3f}, {0x3d, 0x3c}, {0x11, 0x81},	{0x2a, 0x00}, {0x2b, 0x00},	//PCLK = SCLK/2
	#endif	
		{0x3a, 0xd9}, {0x3b, 0x00},	{0x3c, 0x58}, {0x3e, 0x50},	{0x71, 0x00}, {0x15, 0x00},
		{0xD7, 0x10}, {0x6a, 0x24},	{0x85, 0xe7}, {0x63, 0x00}, {0x12, 0x40}, {0x4d, 0x09},
	#if 0		
		{0x17, 0x0c}, {0x18, 0x5c},	{0x19, 0x02}, {0x1a, 0x3f},	{0x03, 0x03}, {0x32, 0xb4},	//641*48?
	#else
		{0x17, 0x0b}, {0x18, 0x5c},	{0x19, 0x02}, {0x1a, 0x3f},	{0x03, 0x03}, {0x32, 0xb4}, //648x48?
	#endif		
		{0x2b, 0x00}, {0x5c, 0x80},	{0x36, 0xb4}, {0x65, 0x10}, {0x70, 0x02}, {0x71, 0x9f},
		{0x64, 0xa4}, {0x5c, 0x80},	{0x43, 0x00}, {0x5D, 0x55}, {0x5E, 0x57}, {0x5F, 0x21},
		{0x24, 0x3e}, {0x25, 0x38},	{0x26, 0x72}, {0x14, 0x68}, {0x0C, 0x38}, {0x4F, 0x4f},
		{0x50, 0x42}, {0x5A, 0x67}, {0x7d, 0x30}, {0x7e, 0x00}, {0x82, 0x03}, {0x7f, 0x00},
		{0x83, 0x07}, {0x80, 0x03}, {0x81, 0x04}, {0x96, 0xf0}, {0x97, 0x00}, {0x92, 0x33},
		{0x94, 0x5a}, {0x93, 0x3a},	{0x95, 0x48}, {0x91, 0xfc}, {0x90, 0xff}, {0x8e, 0x4e},
		{0x8f, 0x4e}, {0x8d, 0x13},	{0x8c, 0x0c}, {0x8b, 0x0c},	{0x86, 0x9e}, {0x87, 0x11},
		{0x88, 0x22}, {0x89, 0x05},	{0x8a, 0x03}, {0x9b, 0x0e},	{0x9c, 0x1c}, {0x9d, 0x34},
		{0x9e, 0x5a}, {0x9f, 0x68},	{0xa0, 0x76}, {0xa1, 0x82},	{0xa2, 0x8e}, {0xa3, 0x98},
		{0xa4, 0xa0}, {0xa5, 0xb0},	{0xa6, 0xbe}, {0xa7, 0xd2},	{0xa8, 0xe2}, {0xa9, 0xee},
		{0xaa, 0x18}, {0xAB, 0xe7},	{0xb0, 0x43}, {0xac, 0x04},	{0x84, 0x40}, {0xad, 0x82},
   #if 0		
		{0xd9, 0x11}, {0xda, 0x00},	{0xae, 0x10}, {0xab, 0xe7},	{0xb9, 0x50}, {0xba, 0x3c},		//641*48?	
		{0xbb, 0x50}, {0xbc, 0x3c},	{0xbd, 0x8},  {0xbe, 0x19},	{0xbf, 0x2},  {0xc0, 0x8},
   #else
		{0xd9, 0x11}, {0xda, 0x00},	{0xae, 0x10}, {0xab, 0xe7},	{0xb9, 0x51}, {0xba, 0x3c},		//648x48?
		{0xbb, 0x51}, {0xbc, 0x3c},	{0xbd, 0x8},  {0xbe, 0x19},	{0xbf, 0x2},  {0xc0, 0x8},	
   #endif		
		{0xc1, 0x2a}, {0xc2, 0x34},	{0xc3, 0x2d}, {0xc4, 0x2d},	{0xc5, 0x0},  {0xc6, 0x98},
		{0xc7, 0x18}, {0x69, 0x48},	{0x74, 0xc0}, {0x7c, 0x28},	{0x65, 0x11}, {0x66, 0x00},
		{0x41, 0xc0}, {0x5b, 0x24},	{0x60, 0x82}, {0x05, 0x07},	{0x03, 0x03}, {0xd2, 0x94},
		{0xc8, 0x06}, {0xcb, 0x40},	{0xcc, 0x40}, {0xcf, 0x00},	{0xd0, 0x20}, {0xd1, 0x00},
		{0xc7, 0x18}, {0x0d, 0x92},	{0x0d, 0x90}		
};


static struct OV_RegTable g_OV_InitTable[] =
{//8 bit slave address, 8 bit data. 
	{0, 0},
	{0, 0},//{g_sOV6880_RegValue,	_REG_TABLE_SIZE(g_sOV6880_RegValue)},		
	{0, 0},//{g_sOV7648_RegValue,	_REG_TABLE_SIZE(g_sOV7648_RegValue)},
	{0, 0},//{g_sOV7670_RegValue,	_REG_TABLE_SIZE(g_sOV7670_RegValue)},
	{0, 0},//{g_sOV2640_RegValue,	_REG_TABLE_SIZE(g_sOV2640_RegValue)},	
	{g_sOV9660_RegValue,	_REG_TABLE_SIZE(g_sOV9660_RegValue)},
	{0, 0}
};

static UINT8 g_uOvDeviceID[]= 
{
	0x00,		// not a device ID
	0xc0,		// ov6680
	0x42,		// ov7648
	0x42,		// ov7670
	0x60,		// ov2640
	0x60,		// 0v9660
	0x00		// not a device ID
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
#if defined(__DEMO_BOARD__)
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
#elif defined(__HMI_BOARD__)
static void SnrPowerDown(BOOL bIsEnable)
{/* GPB4 power down, HIGH for power down */

	//gpio_open(GPIO_PORTB);						//GPIOB as GPIO
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB4));
	
	gpio_setportval(GPIO_PORTB, 1<<4, 1<<4);		//GPIOB 4 set high default
	gpio_setportpull(GPIO_PORTB, 1<<4, 1<4);		//GPIOB 4 pull-up 
	gpio_setportdir(GPIO_PORTB, 1<<4, 1<<4);		//GPIOB 4 output mode 				
	if(bIsEnable)
		gpio_setportval(GPIO_PORTB, 1<<4, 1<<4);	//GPIOB 4 set high
	else				
		gpio_setportval(GPIO_PORTB, 1<<4, 0);		//GPIOB 4 set low
}
#endif

VOID OV9660_Init(UINT32 nIndex)
{
	UINT32 u32Idx;
	UINT32 u32TableSize;
	UINT8  u8DeviceID;
	UINT8 u8ID;
	struct OV_RegValue *psRegValue;
	DBG_PRINTF("Sensor ID = %d\n", nIndex);
	if ( nIndex >= (sizeof(g_uOvDeviceID)/sizeof(UINT8)) )
		return;
	videoIn_Open(48000, 24000);								/* For sensor clock output */	

#if defined(__DEMO_BOARD__) || defined(__HMI_BOARD__)
	SnrPowerDown(FALSE);
#endif	
#if defined(__DEMO_BOARD__) || defined(__NUWICAM__)
	SnrReset();	
#endif	/* Sensor used System reset if HMI */										
		
	u32TableSize = g_OV_InitTable[nIndex].u32TableSize;
	psRegValue = g_OV_InitTable[nIndex].sRegTable;
	u8DeviceID = g_uOvDeviceID[nIndex];
	DBG_PRINTF("Device Slave Addr = 0x%x\n", u8DeviceID);
	if ( psRegValue == 0 )
		return;	
		
#if defined(__DEMO_BOARD__) || defined(__HMI_BOARD__)
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
									
	for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++)
	{
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, (psRegValue->u8RegAddr), (psRegValue->u8Value));	
		if ((psRegValue->u8RegAddr)==0x12 && (psRegValue->u8Value)==0x80)
		{	
			Delay(1000);		
			DBG_PRINTF("Delay A loop\n");
		}				
	}
	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x0A);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x0b);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x1C);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x1D);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);

	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0xD7);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x6A);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	
	DrvI2C_Close();	
}


/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/




UINT32 Smpl_OV9660(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;
	PUINT8 pu8PacketBuf;
	UINT32 u32GCD;
	
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer0 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer1 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer2 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	
	InitVPOST(pu8FrameBuffer0);	
	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 
	
#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
#endif	
	OV9660_Init(5);			
	videoIn_Open(48000, 24000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				TRUE,
				FALSE,							//Polarity.	
				TRUE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_UYVY, 			//Input Order 
				eVIDEOIN_IN_YUV422	,	//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0,							//Vertical start position
				4,							//Horizontal start position	
				0);							//Useless
	videoinIoctl(VIDEOIN_IOCTL_CROPPING_DIMENSION,				
				OPT_CROP_HEIGHT,							//UINT16 u16Height, 
				OPT_CROP_WIDTH,							//UINT16 u16Width;	
				0);							//Useless
	
	/* Set Packet dimension */
	u32GCD = GCD(OPT_PREVIEW_HEIGHT, OPT_CROP_HEIGHT);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			//272/480
				OPT_PREVIEW_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/u32GCD);		
	u32GCD = GCD(OPT_PREVIEW_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			//364/640
				OPT_PREVIEW_WIDTH/u32GCD,
				OPT_CROP_WIDTH/u32GCD);		
				
				
	/* Set Planar dimension  */			
	u32GCD = GCD(OPT_ENCODE_HEIGHT, OPT_CROP_HEIGHT);							 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PLANAR,			//640/640
				OPT_ENCODE_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/u32GCD);		
	u32GCD = GCD(OPT_ENCODE_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PLANAR,			//640/640
				OPT_ENCODE_WIDTH/u32GCD,
				OPT_CROP_WIDTH/u32GCD);
	/* Set Pipes stride */					
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,				
				OPT_ENCODE_WIDTH,
				0);
				
	/* Set Packet Buffer Address */			
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer0 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );
	
	/* Set Planar Buffer Address */						
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PLANAR,				
				0, 							//0 = Planar Y buffer address
				(UINT32)(&u8PlanarFrameBuffer) );							
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PLANAR,			
				1, 							//1 = Planar U buffer address
				(UINT32)(&u8PlanarFrameBuffer) +  OPT_ENCODE_WIDTH*OPT_ENCODE_HEIGHT);							
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PLANAR,			
				2, 							//2 = Planar V buffer address
				(UINT32)(&u8PlanarFrameBuffer) +  OPT_ENCODE_WIDTH*OPT_ENCODE_HEIGHT+OPT_ENCODE_WIDTH*OPT_ENCODE_HEIGHT/2 );																					
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipes was enable. 											
				0 );							//Useless		
	sysSetLocalInterrupt(ENABLE_IRQ);						
															
	return Successful;			
}	