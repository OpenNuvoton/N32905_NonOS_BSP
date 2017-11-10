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
	UINT16	u16RegAddr;		//Register Address
	UINT8	u8Value;			//Register Data
};

struct OV_RegTable{
	struct OV_RegValue *sRegTable;
	UINT32 u32TableSize;
};

extern UINT8 u8PlanarFrameBuffer[];


#define _REG_TABLE_SIZE(nTableName)	(sizeof(nTableName)/sizeof(struct OV_RegValue))

struct OV_RegValue g_sNT99050_RegValue[] = 
{
 	//[InitialSetting]
    {0x3021, 0x01},
    {0x32F0, 0x01}, 
    {0x3024, 0x00},
    {0x3270, 0x00}, //[Gamma_MDR]
    {0x3271, 0x0D}, 
    {0x3272, 0x19}, 
    {0x3273, 0x2A}, 
    {0x3274, 0x3C}, 
    {0x3275, 0x4D}, 
    {0x3276, 0x67}, 
    {0x3277, 0x81}, 
    {0x3278, 0x98}, 
    {0x3279, 0xAD}, 
    {0x327A, 0xCE}, 
    {0x327B, 0xE0}, 
    {0x327C, 0xED}, 
    {0x327D, 0xFF}, 
    {0x327E, 0xFF}, 
    {0x3060, 0x01},
    {0x3210, 0x04}, //LSC //D
    {0x3211, 0x04}, //F
    {0x3212, 0x04}, //D
    {0x3213, 0x04}, //D
    {0x3214, 0x04},
    {0x3215, 0x05},
    {0x3216, 0x04},
    {0x3217, 0x04},
    {0x321C, 0x04},
    {0x321D, 0x05},
    {0x321E, 0x04},
    {0x321F, 0x03},
    {0x3220, 0x00},
    {0x3221, 0xA0},
    {0x3222, 0x00},
    {0x3223, 0xA0},
    {0x3224, 0x00},
    {0x3225, 0xA0},
    {0x3226, 0x80},
    {0x3227, 0x88},
    {0x3228, 0x88},
    {0x3229, 0x30},
    {0x322A, 0xCF},
    {0x322B, 0x07},
    {0x322C, 0x04},
    {0x322D, 0x02},
    {0x3302, 0x00},//[CC: Saturation:100%]
    {0x3303, 0x1C},
    {0x3304, 0x00},
    {0x3305, 0xC8},
    {0x3306, 0x00},
    {0x3307, 0x1C},
    {0x3308, 0x07},
    {0x3309, 0xE9},
    {0x330A, 0x06},
    {0x330B, 0xDF},
    {0x330C, 0x01},
    {0x330D, 0x38},
    {0x330E, 0x00},
    {0x330F, 0xC6},
    {0x3310, 0x07},
    {0x3311, 0x3F},
    {0x3312, 0x07},
    {0x3313, 0xFC},
    {0x3257, 0x50}, //CA Setting
    {0x3258, 0x10},
    {0x3251, 0x01},  
    {0x3252, 0x50},  
    {0x3253, 0x9A},  
    {0x3254, 0x00}, 
    {0x3255, 0xd8},  
    {0x3256, 0x60}, 
    {0x32C4, 0x38}, 
    {0x32F6, 0xCF},
    {0x3363, 0x37},
    {0x3331, 0x08},
    {0x3332, 0x6C}, // 60
    {0x3360, 0x10},
    {0x3361, 0x30},
    {0x3362, 0x70},
    {0x3367, 0x40},
    {0x3368, 0x32}, //20
    {0x3369, 0x24}, //1D
    {0x336A, 0x1A},
    {0x336B, 0x20},
    {0x336E, 0x1A},
    {0x336F, 0x16},
    {0x3370, 0x0c},
    {0x3371, 0x12},
    {0x3372, 0x1d},
    {0x3373, 0x24},
    {0x3374, 0x30},
    {0x3375, 0x0A},
    {0x3376, 0x18},
    {0x3377, 0x20},
    {0x3378, 0x30},
    {0x3340, 0x1C},
    {0x3326, 0x03}, //Eext_DIV
    {0x3200, 0x3E}, //1E
    {0x3201, 0x3F},
    {0x3109, 0x82}, //LDO Open
    {0x3106, 0x07},
    {0x303F, 0x02},
    {0x3040, 0xFF},
    {0x3041, 0x01},
    {0x3051, 0xE0},
    {0x3060, 0x01},


  //640x480 PCLK 24 M MCLK 24 FPS 15~30  Bypass
    {0x32BF,0x04}, 
    {0x32C0,0x6A}, 
    {0x32C1,0x6A}, 
    {0x32C2,0x6A}, 
    {0x32C3,0x00}, 
    {0x32C4,0x27}, 
    {0x32C5,0x20}, 
    {0x32C6,0x20}, 
    {0x32C7,0x00}, 
    {0x32C8,0x95}, 
    {0x32C9,0x6A}, 
    {0x32CA,0x8A}, 
    {0x32CB,0x8A}, 
    {0x32CC,0x8A}, 
    {0x32CD,0x8A}, 
    {0x32D0,0x01}, 
    {0x3200,0x1E}, 
    {0x3201,0x0F}, 
    {0x302A,0x00}, 
    {0x302B,0x09}, 
    {0x302C,0x00}, 
    {0x302D,0x04}, 
    {0x3022,0x27}, 
    {0x3023,0x24}, 
    {0x3002,0x00}, 
    {0x3003,0x04}, 
    {0x3004,0x00}, 
    {0x3005,0x04}, 
    {0x3006,0x02}, 
    {0x3007,0x83}, 
    {0x3008,0x01}, 
    {0x3009,0xE3}, 
    {0x300A,0x03}, 
    {0x300B,0x24}, 
    {0x300C,0x01}, 
    {0x300D,0xF1}, 
    {0x300E,0x02}, 
    {0x300F,0x80}, 
    {0x3010,0x01}, 
    {0x3011,0xE0}, 
    {0x32B8,0x3B}, 
    {0x32B9,0x2D}, 
    {0x32BB,0x87}, 
    {0x32BC,0x34}, 
    {0x32BD,0x38}, 
    {0x32BE,0x30}, 
    {0x3201,0x3F}, 
    {0x320A,0x01}, 
    {0x3021,0x06}, 
    {0x3060,0x01}, 
    
    
    {0x32FB, 0x5A},

};

static struct OV_RegTable g_NT99050_InitTable[] =
{//8 bit slave address, 8 bit data. 
	{0, 0},
	{0, 0},//{g_sOV6880_RegValue,	_REG_TABLE_SIZE(g_sOV6880_RegValue)},		
	{0, 0},//{g_sOV7648_RegValue,	_REG_TABLE_SIZE(g_sOV7648_RegValue)},
	{0,0},
	{0, 0},//{g_sOV2640_RegValue,	_REG_TABLE_SIZE(g_sOV2640_RegValue)},	
	{0, 0},//{g_sOV9660_RegValue,	_REG_TABLE_SIZE(g_sOV9660_RegValue)},
	{g_sNT99050_RegValue,	_REG_TABLE_SIZE(g_sNT99050_RegValue)},
	{0, 0}
};

extern UINT8 u8DiffBuf[];
extern UINT8 u8OutLumBuf[];

static UINT8 g_uOvDeviceID[]= 
{
	0x00,		// not a device ID
	0xc0,		// ov6680
	0x42,		// ov7648
	0x42,		// ov7670
	0x60,		// ov2640
	0x60,		// 0v9660
	0x42,		// NT99050 = 6
	0x00		// not a device ID
};


#ifdef __3RD_PORT__
/*
	Sensor power down and reset may default control on sensor daughter board.
	Reset by RC.
	Sensor alway power on (Keep low)

*/
static void SnrReset(void)
{
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
#endif 


BOOL I2C_Write_8bitSlaveAddr_16bitReg_8bitData(UINT8 uAddr, UINT16 uRegAddr, UINT8 uData)
{
	// 3-Phase(ID address, regiseter address, data(8bits)) write transmission
	volatile u32Delay = 0x100;
	DrvI2C_SendStart();
	while(u32Delay--);		
	if ( (DrvI2C_WriteByte(uAddr,DrvI2C_Ack_Have,8)==FALSE) ||			// Write ID address to sensor
		 (DrvI2C_WriteByte((UINT8)(uRegAddr>>8),DrvI2C_Ack_Have,8)==FALSE) ||	// Write register address to sensor
		 (DrvI2C_WriteByte((UINT8)(uRegAddr&0xff),DrvI2C_Ack_Have,8)==FALSE) ||	// Write register address to sensor
		 (DrvI2C_WriteByte(uData,DrvI2C_Ack_Have,8)==FALSE) )		// Write data to sensor
	{
		sysprintf("wnoack Addr = 0x%x \n", uRegAddr);
		DrvI2C_SendStop();
		return FALSE;
	}
	DrvI2C_SendStop();

	if (uRegAddr==0x12 && (uData&0x80)!=0)
	{
		sysDelay(2);			
	}
	return TRUE;
}

UINT8 I2C_Read_8bitSlaveAddr_16bitReg_8bitData(UINT8 uAddr, UINT16 uRegAddr)
{
	UINT8 u8Data;
	
	// 2-Phase(ID address, register address) write transmission
	DrvI2C_SendStart();
	DrvI2C_WriteByte(uAddr,DrvI2C_Ack_Have,8);		// Write ID address to sensor
	DrvI2C_WriteByte((UINT8)(uRegAddr>>8),DrvI2C_Ack_Have,8);	// Write register address to sensor
	DrvI2C_WriteByte((UINT8)(uRegAddr&0xff),DrvI2C_Ack_Have,8);	// Write register address to sensor	
	DrvI2C_SendStop();

	// 2-Phase(ID-address, data(8bits)) read transmission
	DrvI2C_SendStart();
	DrvI2C_WriteByte(uAddr|0x01,DrvI2C_Ack_Have,8);		// Write ID address to sensor
	u8Data = DrvI2C_ReadByte(DrvI2C_Ack_Have,8);		// Read data from sensor
	DrvI2C_SendStop();
	
	return u8Data;

}





VOID NT99050_Init(UINT32 nIndex)
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
#ifdef __3RD_PORT__
	SnrReset();	
	SnrPowerDown(FALSE); 	 										
#endif		
	u32TableSize = g_NT99050_InitTable[nIndex].u32TableSize;
	psRegValue = g_NT99050_InitTable[nIndex].sRegTable;
	u8DeviceID = g_uOvDeviceID[nIndex];
	DBG_PRINTF("Device Slave Addr = 0x%x\n", u8DeviceID);
	if ( psRegValue == 0 )
		return;	

#if 1
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB13));
	outp32(REG_GPBFUN, inp32(REG_GPBFUN) & (~MF_GPB14));
	DrvI2C_Open(eDRVGPIO_GPIOB, 					
				eDRVGPIO_PIN13, 
				eDRVGPIO_GPIOB,
				eDRVGPIO_PIN14, 
				(PFN_DRVI2C_TIMEDELY)Delay);
#else
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & (~MF_GPD9));
	outp32(REG_GPDFUN, inp32(REG_GPDFUN) & (~MF_GPD10));
	DrvI2C_Open(eDRVGPIO_GPIOD, 					
				eDRVGPIO_PIN9, 
				eDRVGPIO_GPIOD,
				eDRVGPIO_PIN10, 
				(PFN_DRVI2C_TIMEDELY)Delay);
#endif				
									
	for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++)
	{
		//printf("Addr = 0x%x\n",  (psRegValue->u16RegAddr));
		I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, (psRegValue->u16RegAddr), (psRegValue->u8Value));
			
		#if 0 
		if ((psRegValue->u8RegAddr)==0x12 && (psRegValue->u8Value)==0x80)
		{	
			Delay(1000);		
			DBG_PRINTF("Delay A loop\n");
		}
		#else
		if ((psRegValue->u16RegAddr)==0x3021 && (psRegValue->u8Value)==0x01)
		{	
			//Delay(1000);
			sysDelay(2);		
			DBG_PRINTF("Delay A loop\n");
		}	
		#endif				
	}
#if 1	
	u8ID = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x0A);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x0b);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x1C);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x1D);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);

	u8ID = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0xD7);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
	u8ID = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x6A);
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);
#endif 	
	DrvI2C_Close();	
}


/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/
UINT32 Smpl_NT99050(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1)
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;
	UINT32 u32GCD;
	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 

#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 12000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	//videoIn_Init(TRUE, 0, 12000, eVIDEOIN_3RD_SNR_CCIR601);
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
#endif	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | HCLK4_CKE);
	NT99050_Init(6);			
	videoIn_Open(48000, 24000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,							//NT99050	
				FALSE,							//Polarity.	
				TRUE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_UYVY, 			//NT99050
				eVIDEOIN_IN_YUV422	,	//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
				//eVIDEOIN_OUT_RGB565);
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0,							//NT99050
				0,							//Horizontal start position	
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
					
	

	/* Set Pipes stride */				
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,				
				640,   //OPT_ENCODE_WIDTH,
				0);
	/* Set Packet Buffer Address */					
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer0 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_PACKET,			// which packet was enable. 											
				0 );							//Useless		
							
	sysSetLocalInterrupt(ENABLE_IRQ);						
																		
														
	return Successful;			
}	

static INT32 IQ_GetBrightness(VOID)
{
	UINT8 u8RegData, u8DeviceID;
	u8DeviceID = g_uOvDeviceID[6];	
	u8RegData = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x32F2); 
	return (UINT32)u8RegData;		
} 
static INT32 IQ_SetBrightness(INT16 i16Brightness)
{
	UINT8 u8DeviceID;
	u8DeviceID = g_uOvDeviceID[6];	
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x32F2, i16Brightness); //Y component Brightnes
	return Successful;
}
/*
Addr-0x3363[4] = 0 to enable sharpness
Addr-0x3300[7:6] = sharpness[9:8]
Addr-0x3301[7:0] = sharpness[7:0]
*/
static INT32 IQ_GetSharpness(VOID)
{
	UINT8 u8DeviceID;
	UINT8 u8SharpH, u8SharpL;
	u8DeviceID = g_uOvDeviceID[6];
	
	u8SharpH = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3300);		/* Write sharp[9:8] */
	u8SharpH = u8SharpH&0xC0;
	u8SharpL = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3301);		/* Write sharp[9:8] */
	u8SharpL = u8SharpL>>2;				
	return (u8SharpH|u8SharpL);
} 
static INT32 IQ_SetSharpness(INT16 i16Sharpness)
{
	UINT8 u8RegData, u8DeviceID;
	UINT8 u8SharpH, u8SharpL;
	
	i16Sharpness = i16Sharpness&0xFF;
	u8SharpH = ((i16Sharpness>>6)<<6);
	
	u8DeviceID = g_uOvDeviceID[6];	
	u8RegData = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3363);		/* Clear bit4 to enable sharpness */
	u8RegData = u8RegData & ~0x8;
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3363, u8RegData);		
	
	u8RegData = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3300);		/* Write sharp[9:8] */
	u8RegData = u8RegData & ~0xC0;
	u8RegData = u8RegData |u8SharpH;
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3300, u8RegData);		
	
	u8SharpL = i16Sharpness<<2;
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3301, u8SharpL);		
		
	return Successful;	
} 

						
static INT8 i8Contrast = 128;						
static INT32 IQ_GetContrast(VOID)
{
	return i8Contrast;
} 
static INT32 IQ_SetContrast(INT16 ucContrastValue)
{
	
	UINT8 i;//,gamma_tmp;
	UINT8 Contrast_Gamma[15] = {0};
	UINT8 u8DeviceID = g_uOvDeviceID[6];
	
	UINT8 GAMMA[3][15] = {{60, 64, 69, 80, 90, 100, 120, 139, 156, 173, 200, 220, 235, 245, 255,},
						{0, 13,  25,  42,  60,  77, 103, 129, 152, 173, 206, 224, 237, 255, 255,},
						{0,  0,   0,   0,   0,  19,  68, 109, 141, 166, 203, 231, 247, 254, 255,},
					};
    
	i8Contrast = ucContrastValue;
	for(i=0; i<15; i++){
	
		if(ucContrastValue == 128){
		    	Contrast_Gamma[i] = GAMMA[1][i];
		}else{
		    	if(ucContrastValue>=128){
		        	Contrast_Gamma[i] = GAMMA[1][i] - ((ucContrastValue-128)*(GAMMA[1][i]-GAMMA[2][i]))/127;
		    	}else{
		        	Contrast_Gamma[i] = GAMMA[0][i] - ((ucContrastValue)*(GAMMA[0][i]-GAMMA[1][i]))/128;
		    	}       
		}
		I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3270+i, Contrast_Gamma[i]);
	}
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x3060, 0x01);
	return Successful;
} 
static INT32 IQ_GetHue(void)
{
	UINT8 u8RegData; 
	UINT8 u8DeviceID = g_uOvDeviceID[6];
	
	u8RegData = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x32FB); 	
	return u8RegData;
}
static INT32 IQ_SetHue(INT16 i16Hue)
{
	//UINT8 u8RegData; 
	UINT8 u8DeviceID = g_uOvDeviceID[6];
	
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x32F1, 0x05); 
	I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, 0x32FB, i16Hue); 	
	return Successful;
} 


IQ_S ImageQualityTbl =
{	
	IQ_GetBrightness, 		
	IQ_SetBrightness,
	IQ_GetSharpness,		
	IQ_SetSharpness,
	IQ_GetContrast,		
	IQ_SetContrast,
	IQ_GetHue,			
	IQ_SetHue,
};
INT32 register_sensor(IQ_S* ps_sensor)
{
	*ps_sensor = ImageQualityTbl;
	return Successful;	
}