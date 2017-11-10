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

struct OV_RegValue g_sOV7670_RegValue[] = 
{
#if 0 //Old table 
	{0x12,0x80},{0x11,0x83},{0x1e,0x11},{0x3a,0x04},{0x12,0x00},{0x17,0x13},{0x18,0x02},{0x32,0x80},//{0x17,0x13},{0x18,0x01},{0x32,0xb6},
	{0x19,0x03},{0x1a,0x7b},{0x03,0x0a},{0x0c,0x00},{0x3e,0x00},{0x70,0x3a},{0x71,0x35},{0x72,0x11},
	{0x73,0xf0},{0xa2,0x02},{0x7a,0x20},{0x7b,0x1c},{0x7c,0x28},{0x7d,0x3c},{0x7e,0x5a},{0x7f,0x68},
	{0x80,0x76},{0x81,0x80},{0x82,0x88},{0x83,0x8f},{0x84,0x96},{0x85,0xa3},{0x86,0xaf},{0x87,0xc4},
	{0x88,0xd7},{0x89,0xe8},{0x13,0xe0},{0x00,0x00},{0x10,0x00},{0x0d,0x40},{0x14,0x38},{0xa5,0x05},
	{0xab,0x07},{0x24,0x95},{0x25,0x33},{0x26,0xe3},{0x9f,0x78},{0xa0,0x68},{0xa1,0x0b},{0xa6,0xd8},
	{0xa7,0xd8},{0xa8,0xf0},{0xa9,0x90},{0xaa,0x94},{0x13,0xe5},{0x0e,0x61},{0x0f,0x4b},{0x16,0x02},
	{0x21,0x02},{0x22,0x91},{0x29,0x07},{0x33,0x03},{0x35,0x0b},{0x37,0x1c},{0x38,0x71},{0x3c,0x78},
	{0x4d,0x40},{0x4e,0x20},{0x69,0x55},//{0x6b,0x4a},
	{0x74,0x19},{0x8d,0x4f},{0x8e,0x00},{0x8f,0x00},
	{0x90,0x00},{0x91,0x00},{0x96,0x00},{0x9a,0x80},{0xb0,0x8c},{0xb1,0x0c},{0xb2,0x0e},{0xb3,0x82},
	{0xb8,0x0a},{0x43,0x14},{0x44,0xf0},{0x45,0x34},{0x46,0x58},{0x47,0x28},{0x48,0x3a},{0x59,0x88},
	{0x5a,0x88},{0x5b,0x44},{0x5c,0x67},{0x5d,0x49},{0x5e,0x0e},{0x6c,0x0a},{0x6d,0x55},{0x6e,0x11},
	{0x6f,0x9f},{0x6a,0x40},{0x01,0x40},{0x02,0x40},{0x13,0xe7},{0x4f,0x80},{0x50,0x80},{0x51,0x00},
	{0x52,0x22},{0x53,0x5e},{0x54,0x80},{0x58,0x9e},{0x41,0x08},{0x3f,0x00},{0x75,0x05},{0x76,0x61},
	{0x4c,0x00},{0x77,0x01},{0x3d,0xc2},{0x4b,0x09},{0xc9,0x60},{0x41,0x38},{0x56,0x40},{0x34,0x11},
	{0x3b,0x02},{0xa4,0x88},{0x96,0x00},{0x97,0x30},{0x98,0x20},{0x99,0x20},{0x9a,0x84},{0x9b,0x29},
	{0x9c,0x03},{0x9d,0x4c},{0x9e,0x3f},{0x78,0x04},{0x79,0x01},{0xc8,0xf0},{0x79,0x0f},{0xc8,0x20},
	{0x79,0x10},{0xc8,0x7e},{0x79,0x0b},{0xc8,0x01},{0x79,0x0c},{0xc8,0x07},{0x79,0x0d},{0xc8,0x20},
	{0x79,0x09},{0xc8,0x80},{0x79,0x02},{0xc8,0xc0},{0x79,0x03},{0xc8,0x40},{0x79,0x05},{0xc8,0x30},
	{0x79,0x26}
#else
	{0x12, 0x80},{0x11, 0x80},{0x3A, 0x04},{0x12, 0x00},{0x17, 0x13},{0x18, 0x01},{0x32, 0xB6}, 
	{0x2B, 0x10},{0x19, 0x02},{0x1A, 0x7A},{0x03, 0x0F},{0x0C, 0x00},{0x3E, 0x00},{0x70, 0x3A},
	{0x71, 0x35},{0x72, 0x11},{0x73, 0xF0},{0xA2, 0x3B},{0x1E, 0x07},{0x7a, 0x1e},{0x7b, 0x09},
	{0x7c, 0x14},{0x7d, 0x29},{0x7e, 0x50},{0x7f, 0x5F},{0x80, 0x6C},{0x81, 0x79},{0x82, 0x84},
	{0x83, 0x8D},{0x84, 0x96},{0x85, 0xA5},{0x86, 0xB0},{0x87, 0xC6},{0x88, 0xD8},{0x89, 0xE9},
	{0x55, 0x00},{0x13, 0xE0},{0x00, 0x00},{0x10, 0x00},{0x0D, 0x40},{0x42, 0x00},{0x14, 0x18},
	{0xA5, 0x02},{0xAB, 0x03},{0x24, 0x48},{0x25, 0x40},{0x26, 0x82},{0x9F, 0x78},{0xA0, 0x68},
	{0xA1, 0x03},{0xA6, 0xd2},{0xA7, 0xd2},{0xA8, 0xF0},{0xA9, 0x80},{0xAA, 0x14},{0x13, 0xE5},
	{0x0E, 0x61},{0x0F, 0x4B},{0x16, 0x02},{0x21, 0x02},{0x22, 0x91},{0x29, 0x07},{0x33, 0x0B},
	{0x35, 0x0B},{0x37, 0x1D},{0x38, 0x71},{0x39, 0x2A},{0x3C, 0x78},{0x4D, 0x40},{0x4E, 0x20},
	{0x69, 0x00},{0x6B, 0x0A},{0x74, 0x10},{0x8D, 0x4F},{0x8E, 0x00},{0x8F, 0x00},{0x90, 0x00},
	{0x91, 0x00},{0x96, 0x00},{0x9A, 0x80},{0xB0, 0x84},{0xB1, 0x0C},{0xB2, 0x0E},{0xB3, 0x7e},
	{0xB1, 0x00},{0xB1, 0x0c},{0xB8, 0x0A},{0x44, 0xfF},{0x43, 0x00},{0x45, 0x4a},{0x46, 0x6c},
	{0x47, 0x26},{0x48, 0x3a},{0x59, 0xd6},{0x5a, 0xff},{0x5c, 0x7c},{0x5d, 0x44},{0x5b, 0xb4},
	{0x5e, 0x10},{0x6c, 0x0a},{0x6d, 0x55},{0x6e, 0x11},{0x6f, 0x9e},{0x6A, 0x40},{0x01, 0x40},
	{0x02, 0x40},{0x13, 0xf7},{0x4f, 0x78},{0x50, 0x72},{0x51, 0x06},{0x52, 0x24},{0x53, 0x6c},
	{0x54, 0x90},{0x58, 0x1e},{0x62, 0x08},{0x63, 0x10},{0x64, 0x08},{0x65, 0x00},{0x66, 0x05},
	{0x41, 0x08},{0x3F, 0x00},{0x75, 0x44},{0x76, 0xe1},{0x4C, 0x00},{0x77, 0x01},{0x3D, 0xC2},
	{0x4B, 0x09},{0xC9, 0x60},{0x41, 0x18},{0x56, 0x40},{0x34, 0x11},{0x3b, 0x02},{0xa4, 0x89},
	{0x92, 0x00},{0x96, 0x00},{0x97, 0x30},{0x98, 0x20},{0x99, 0x20},{0x9A, 0x84},{0x9B, 0x29},
	{0x9C, 0x03},{0x9D, 0x99},{0x9E, 0x7F},{0x78, 0x00},{0x94, 0x08},{0x95, 0x0D},{0x79, 0x01},
	{0xc8, 0xf0},{0x79, 0x0f},{0xc8, 0x00},{0x79, 0x10},{0xc8, 0x7e},{0x79, 0x0a},{0xc8, 0x80},
	{0x79, 0x0b},{0xc8, 0x01},{0x79, 0x0c},{0xc8, 0x0f},{0x79, 0x0d},{0xc8, 0x20},{0x79, 0x09},
	{0xc8, 0x80},{0x79, 0x02},{0xc8, 0xc0},{0x79, 0x03},{0xc8, 0x40},{0x79, 0x05},{0xc8, 0x30},
	{0x79, 0x26},{0x3b, 0x82},{0x43, 0x02},{0x44, 0xf2}
#endif	
};

static struct OV_RegTable g_OV_InitTable[] =
{//8 bit slave address, 8 bit data. 
	{0, 0},
	{0, 0},//{g_sOV6880_RegValue,	_REG_TABLE_SIZE(g_sOV6880_RegValue)},		
	{0, 0},//{g_sOV7648_RegValue,	_REG_TABLE_SIZE(g_sOV7648_RegValue)},
	{g_sOV7670_RegValue,	_REG_TABLE_SIZE(g_sOV7670_RegValue)},
	{0, 0},//{g_sOV2640_RegValue,	_REG_TABLE_SIZE(g_sOV2640_RegValue)},	
	{0, 0},//{g_sOV9660_RegValue,	_REG_TABLE_SIZE(g_sOV9660_RegValue)},
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



VOID OV7670_Init(UINT32 nIndex)
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
UINT32 Smpl_OV7670(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
#if 0		
#ifdef __3RD_PORT__	
		outp32(REG_GPDFUN, inp32(REG_GPDFUN) & ~MF_GPD2);
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
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
#endif	

	OV7670_Init(3);			
	videoIn_Open(64000, 24000);		
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
				eVIDEOIN_IN_YUV422	,		//Intput format
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
				eVIDEOIN_BOTH_PIPE_ENABLE,			// which packet was enable. 											
				0 );							//Useless		
							
	sysSetLocalInterrupt(ENABLE_IRQ);						
														
	return Successful;			
}	