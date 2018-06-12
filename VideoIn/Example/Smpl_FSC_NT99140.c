#include "wblib.h"
#include "W55FA93_VideoIn.h"
#include "W55FA93_GPIO.h"
#include "DrvI2C.h"
#include "demo.h"

struct NT_RegValue
{
	UINT16	u16RegAddr;		//Register Address
	UINT8	u8Value;			//Register Data
};
#define _REG_TABLE_SIZE(nTableName)	sizeof(nTableName)/sizeof(struct NT_RegValue)

#define REG_VALUE_INIT	0
#define REG_VALUE_HD720	1	//1280X720
#define REG_VALUE_D1		2	//720X480
#define REG_VALUE_VGA		3	//640X480



#define DrvVideoIn_nt99140	1
#define CHIP_VERSION_H		0x3000
#define CHIP_VERSION_L		0x3001
#define NT99140_CHIP_ID	0x14

struct NT_RegTable{
	struct NT_RegValue *sRegTable;
	UINT16 uTableSize;
};

extern UINT8 u8PlanarFrameBuffer[];

struct NT_RegValue g_sNT99140_Init[] = 
{
	{0x32F0,0x01},
	{0x3028,0x05}, 
	{0x3029,0x02}, 
	{0x302a,0x00}, 

	//{0x306a,0x02},	
	{0x306a,0x01},
		
	{0x3336,0x14},
	{0x3210,0x04},
	{0x3211,0x04},
	{0x3212,0x04},
	{0x3213,0x04},
	{0x3214,0x04},
	{0x3215,0x04},
	{0x3216,0x04},
	{0x3217,0x04},
	{0x3218,0x04},
	{0x3219,0x04},
	{0x321A,0x04},
	{0x321B,0x04},
	{0x321C,0x04},
	{0x321D,0x04},
	{0x321E,0x04},
	{0x321F,0x04},
	{0x3230,0x1D},
	{0x3231,0x00},
	{0x3232,0x04},
	{0x3233,0x30},	
	{0x3234,0x42},
	{0x3270,0x00}, 
	{0x3271,0x0e},
	{0x3272,0x1a},
	{0x3273,0x31},
	{0x3274,0x4B},
	{0x3275,0x5D},
	{0x3276,0x7E},
	{0x3277,0x94},
	{0x3278,0xA6},
	{0x3279,0xB6},
	{0x327A,0xcc},
	{0x327B,0xde},
	{0x327C,0xeb},
	{0x327D,0xF6},
	{0x327E,0xFF},
	{0x3302,0x00},
	{0x3303,0x1D},
	{0x3304,0x00},
	{0x3305,0x97},
	{0x3306,0x00},
	{0x3307,0x4C},
	{0x3308,0x07},
	{0x3309,0xF2},
	{0x330A,0x06},
	{0x330B,0x8F},
	{0x330C,0x01},
	{0x330D,0x7F},
	{0x330E,0x00},
	{0x330F,0xB9},
	{0x3310,0x07},
	{0x3311,0x96},
	{0x3312,0x07},
	{0x3313,0xB2},
	{0x3300,0x70},
	{0x3301,0x40},
	{0x3024,0x00},
	{0x3040,0x04},
	{0x3041,0x02},
	{0x3042,0xFF},
	{0x3043,0x14},
	{0x3052,0xd0},
	{0x3057,0x80},
	{0x3058,0x00},
	{0x3059,0x2F},
	{0x305f,0x22},
	{0x32b0,0x00},
	{0x32b1,0x90},
	{0x32BB,0x0b},
	{0x32bd,0x10},
	{0x32be,0x05},
	{0x32bf,0x4a},
	{0x32c0,0x40},
	{0x32C3,0x08},
	{0x32c5,0x1f},
	{0x32cd,0x01},
	{0x32d3,0x00},
	{0x32f6,0x0c},
	{0x3324,0x00},
	{0x3118,0xF2},
	{0x3119,0xF2},
	{0x311A,0x13},
	{0x3106,0x03},
	{0x3108,0x55},
	{0x3105,0x41},
	{0x3112,0x21},
	{0x3113,0x55},
	{0x3114,0x05},
	{0x333b,0x20},
	{0x333c,0x28},
	{0x3320,0x20},
	{0x3335,0x01},
	{0x3200,0x3e},     
	{0x3201,0x3f},     
	{0x3344,0x28},
	{0x3345,0x30},
	{0x3346,0x30},
	{0x3348,0x00},
	{0x3349,0x40},
	{0x334a,0x30},
	{0x334b,0x00},
	{0x334d,0x15},
	{0x329b,0x01},
	{0x32a1,0x01},
	{0x32a2,0x40},
	{0x32a3,0x01},
	{0x32a4,0xc0},
	{0x32a5,0x01},
	{0x32a6,0x40},
	{0x32a7,0x02},
	{0x32a8,0x10},
	{0x32a9,0x11},
	{0x3054,0x05},
};

struct NT_RegValue g_sNT99140_HD720[] = 
{
	{0x3028, 0x0D}, 	//56MHz
	{0x3029, 0x02}, 
	{0x302a, 0x00}, 
	{0x3022, 0x24}, 
	{0x3023, 0x24}, 

	{0x3002, 0x00}, 
	{0x3003, 0x04}, // start x 
	{0x3004, 0x00}, 
	{0x3005, 0x04}, // start y
	{0x3006, 0x05}, 
	{0x3007, 0x03}, // end x 
	{0x3008, 0x02}, 
//	{0x3009, 0xd3}, // end y
	{0x3009, 0xd5}, // end y

	{0x300a, 0x07}, 
	{0x300b, 0x12}, 
	{0x300c, 0x02}, 
//	{0x300d, 0xe0}, // 736 
	{0x300d, 0xe2}, // 738
	{0x300e, 0x05}, 
	{0x300f, 0x00}, 
	{0x3010, 0x02}, 
//	{0x3011, 0xd0}, //720 
	{0x3011, 0xd2}, //722 

	{0x32b0, 0x00}, 
	{0x32b1, 0x00}, 
	{0x32b2, 0x01}, 
	{0x32b3, 0x80}, 
	{0x32b4, 0x00}, 
	{0x32b5, 0x68}, 
	{0x32b6, 0x99}, 
	{0x32bb, 0x1b}, 
	{0x32bc, 0x40}, 
	{0x32c1, 0x23}, 
	{0x32c2, 0x05}, 
	{0x32c8, 0x4d}, 
	{0x32c9, 0x40}, 
	{0x32c4, 0x00}, 
	{0x3201, 0x3f}, 
	{0x3021, 0x06}, 
	{0x3060, 0x01}


};


struct NT_RegValue g_sNT99140_D1[] = 
{
	{0x32e0, 0x02}, 
	{0x32e1, 0xd0}, 
	{0x32e2, 0x01}, 
	{0x32e3, 0xe2}, //482
	{0x32e4, 0x00}, 
	{0x32e5, 0x55}, 
	{0x32e6, 0x00}, 
	{0x32e7, 0x56}, 
	{0x3028, 0x0D}, 	
	{0x3029, 0x02}, 
	{0x302a, 0x00}, 
	{0x3022, 0x24}, 
	{0x3023, 0x24}, 
	// 1080x720 ---> 720x480
	{0x3002, 0x00}, 
	{0x3003, 0x68}, //start x 104 
	{0x3004, 0x00}, 
	{0x3005, 0x04}, //start y 4 
	{0x3006, 0x04}, 
	{0x3007, 0x9f}, //end x 1183
	{0x3008, 0x02}, 
	{0x3009, 0xd5}, //end y 725 
	{0x300a, 0x05},
	{0x300b, 0x70}, //line length  1080 + 312
	{0x300c, 0x02}, 
	{0x300d, 0xe2}, //frame length 720 + 16
	{0x300e, 0x04}, 
	{0x300f, 0x38}, //1080
	{0x3010, 0x02}, 
	{0x3011, 0xd2}, //722

	{0x32b0, 0x00}, 
	{0x32b1, 0x00}, 
	{0x32b2, 0x00}, 
	{0x32b3, 0xe0}, 
	{0x32b4, 0x00}, 
	{0x32b5, 0xc0}, 
	{0x32b6, 0x98}, 
	{0x32bb, 0x1b}, 
	{0x32bc, 0x40}, 
	{0x32c1, 0x22}, 
	{0x32c2, 0x94}, 
	{0x32c8, 0x6e}, 
	{0x32c9, 0x5c}, 
	{0x32c4, 0x00}, 
	{0x3201, 0x7f}, 
	{0x3021, 0x06}, 
	{0x3060, 0x01}, 
};

struct NT_RegValue g_sNT99140_VGA[] = 
{
	{0x32e0, 0x02}, 
	{0x32e1, 0x80}, 
	{0x32e2, 0x01}, 
	{0x32e3, 0xe2},  //482 
	{0x32e4, 0x00}, 
	{0x32e5, 0x80}, 
	{0x32e6, 0x00}, 
	{0x32e7, 0x80}, 
	{0x3028, 0x0D}, 	
	{0x3029, 0x02}, 
	{0x302a, 0x00}, 
	{0x3022, 0x24}, 
	{0x3023, 0x24}, 

	// 960x722 ---> 640x482
	{0x3002, 0x00}, 
	{0x3003, 0xa4}, //start x 164 
	{0x3004, 0x00}, 
	{0x3005, 0x04}, //start y 4 
	{0x3006, 0x04}, 
	{0x3007, 0x63}, //end x  1123
	{0x3008, 0x02}, 
	{0x3009, 0xd5}, //end y  725
	{0x300a, 0x04}, 
	{0x300b, 0xf8}, //line length  960 + 312
	{0x300c, 0x02}, 
	{0x300d, 0xe2}, //frame length 720 + 16 
	{0x300e, 0x03}, 
	{0x300f, 0xc0}, // 960 
	{0x3010, 0x02}, 
	{0x3011, 0xd2}, // 722 

	{0x32b0, 0x00}, 
	{0x32b1, 0x00}, 
	{0x32b2, 0x00}, 
	{0x32b3, 0xe0}, 
	{0x32b4, 0x00}, 
	{0x32b5, 0x68}, 
	{0x32b6, 0x99}, 
	{0x32bb, 0x1b}, 
	{0x32bc, 0x40}, 
	{0x32c1, 0x22}, 
	{0x32c2, 0x94}, 
	{0x32c8, 0x6e}, 
	{0x32c9, 0x5c}, 
	{0x32c4, 0x00}, 
	{0x3201, 0x7f}, 
//	{0x3201, 0x3f}, 

	{0x3021, 0x06}, 
	{0x3060, 0x01}, 	



};

struct NT_RegTable g_NT99140_InitTable[] =
{
	{g_sNT99140_Init,_REG_TABLE_SIZE(g_sNT99140_Init)},
	{g_sNT99140_HD720,_REG_TABLE_SIZE(g_sNT99140_HD720)},
	{g_sNT99140_D1,_REG_TABLE_SIZE(g_sNT99140_D1)},
	{g_sNT99140_VGA,_REG_TABLE_SIZE(g_sNT99140_VGA)},		
	{0,0}
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
	0x54,		// NT99140 = 7	
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

static BOOL I2C_Write_8bitSlaveAddr_16bitReg_8bitData(UINT8 uAddr, UINT16 uRegAddr, UINT8 uData)
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

	return TRUE;
}

static UINT8 I2C_Read_8bitSlaveAddr_16bitReg_8bitData(UINT8 uAddr, UINT16 uRegAddr)
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

VOID NT99140_Init(UINT32 nIndex, UINT32 u32Resolution)
{
	UINT32 u32Idx;
	UINT32 u32TableSize;
	UINT8  u8DeviceID;
	UINT8 id0, id1;
	struct NT_RegValue *psRegValue;
	DBG_PRINTF("Sensor ID = %d\n", nIndex);
	if ( nIndex >= (sizeof(g_uOvDeviceID)/sizeof(UINT8)) )
		return;	
	videoIn_Open(48000, 24000);								/* For sensor clock output */	
	sysDelay(2);
#if defined(__DEMO_BOARD__) || defined(__HMI_BOARD__)
	SnrPowerDown(FALSE);
#endif	
#if defined(__DEMO_BOARD__) || defined(__NUWICAM__)
	SnrReset();	
#endif	/* Sensor used System reset if HMI */

	u32TableSize = g_NT99140_InitTable[0].uTableSize;
	psRegValue = g_NT99140_InitTable[0].sRegTable;
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
		I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, (psRegValue->u16RegAddr), (psRegValue->u8Value));						
	}
	id0 = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID,CHIP_VERSION_H);
	id1 = I2C_Read_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID,CHIP_VERSION_L);
	//static UINT8 I2C_Read_8bitSlaveAddr_16bitReg_8bitData(UINT8 uAddr, UINT16 uRegAddr)
	sysprintf("Detectd sensor id0=%0x id1=%02x\n",id0, id1);
	
	
	
	if(u32Resolution == REG_VALUE_HD720)
	{
		u32TableSize = g_NT99140_InitTable[1].uTableSize;
		psRegValue = g_NT99140_InitTable[1].sRegTable;
		u8DeviceID = g_uOvDeviceID[nIndex];
		for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++)
		{		
			I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, (psRegValue->u16RegAddr), (psRegValue->u8Value));						
		}
	}
	else if(u32Resolution == REG_VALUE_D1)
	{
		u32TableSize = g_NT99140_InitTable[2].uTableSize;
		psRegValue = g_NT99140_InitTable[2].sRegTable;
		u8DeviceID = g_uOvDeviceID[nIndex];
		for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++)
		{		
			I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, (psRegValue->u16RegAddr), (psRegValue->u8Value));						
		}
	}	
	else if(u32Resolution == REG_VALUE_VGA)
	{
		u32TableSize = g_NT99140_InitTable[3].uTableSize;
		psRegValue = g_NT99140_InitTable[3].sRegTable;
		u8DeviceID = g_uOvDeviceID[nIndex];
		for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++)
		{		
			I2C_Write_8bitSlaveAddr_16bitReg_8bitData(u8DeviceID, (psRegValue->u16RegAddr), (psRegValue->u8Value));						
		}
	}	
	
	DrvI2C_Close();	
}


/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/
UINT32 Smpl_NT99140_VGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
	outp32(REG_AHBCLK, 0xFFFFFFFF);

	InitVPOST(pu8FrameBuffer0);	
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
	NT99140_Init(7, REG_VALUE_VGA);			
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
				OPT_CROP_HEIGHT,				//UINT16 u16Height, 
				OPT_CROP_WIDTH,				//UINT16 u16Width;	
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
	
	DrvVideoIn_SetMotionDet(TRUE, //BOOL bEnable,
								TRUE, //TRUE=8x8. FALSE = 16x16 BOOL bBlockSize,	
								FALSE);//FALSE for 1 bit Threshold+7 bits diff// BOOL bSaveMode
								
	DrvVideoIn_SetMotionDetEx(0,			//UINT32 u32DetFreq,
								20,			//UINT32 u32Threshold,
								(UINT32)u8DiffBuf,	//UINT32 u32OutBuffer,	
								(UINT32)u8OutLumBuf);	//UINT32 u32YBuffer			
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipe was enable. 											
				0 );							//Useless		
							
					
							
	sysSetLocalInterrupt(ENABLE_IRQ);						
														
	return Successful;			
}	

/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/
UINT32 Smpl_NT99140_D1(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
	outp32(REG_AHBCLK, 0xFFFFFFFF);

	InitVPOST(pu8FrameBuffer0);	
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
	NT99140_Init(7, REG_VALUE_D1);			
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
				480,							//UINT16 u16Height, 
				720,							//UINT16 u16Width;	
				0);							//Useless
	u32GCD = GCD(OPT_PREVIEW_HEIGHT, 480);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			//272/480
				OPT_PREVIEW_HEIGHT/u32GCD,
				480/u32GCD);		
	u32GCD = GCD(OPT_PREVIEW_WIDTH, 720);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			//364/640
				OPT_PREVIEW_WIDTH/u32GCD,
				720/u32GCD);		
				
	u32GCD = GCD(480, 480);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PLANAR,			
				480/u32GCD,
				480/u32GCD);		
	u32GCD = GCD(720, 720);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PLANAR,			
				720/u32GCD,
				720/u32GCD);
	
#ifdef __TV__
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,				
				OPT_STRIDE,
				0);
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer0 ) );		
				//(UINT32)((UINT32)pu8FrameBuffer + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );					
#else				
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,			//Packet 	
				720,					//Planar
				0);
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer0 + (OPT_STRIDE-OPT_PREVIEW_WIDTH)/2*2) );					
#endif			
	
	DrvVideoIn_SetMotionDet(TRUE, //BOOL bEnable,
								TRUE, //TRUE=8x8. FALSE = 16x16 BOOL bBlockSize,	
								FALSE);//FALSE for 1 bit Threshold+7 bits diff// BOOL bSaveMode
								
	DrvVideoIn_SetMotionDetEx(0,			//UINT32 u32DetFreq,
								20,			//UINT32 u32Threshold,
								(UINT32)u8DiffBuf,	//UINT32 u32OutBuffer,	
								(UINT32)u8OutLumBuf);	//UINT32 u32YBuffer			
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_PACKET,			// which packet was enable. 											
				0 );							//Useless		
							
					
							
	sysSetLocalInterrupt(ENABLE_IRQ);						
														
	return Successful;			
}
/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/
UINT32 Smpl_NT99140_HD(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
	outp32(REG_AHBCLK, 0xFFFFFFFF);

	InitVPOST(pu8FrameBuffer0);	
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
	NT99140_Init(7, REG_VALUE_HD720);			
	videoIn_Open(72000, 24000);		
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
	/* Set Cropping Window */
	videoinIoctl(VIDEOIN_IOCTL_CROPPING_DIMENSION,				
				720,							//UINT16 u16Height, 
				1280,						//UINT16 u16Width;	
				0);							//Useless
				
	/* Set Packet dimension */			
	u32GCD = GCD(OPT_PREVIEW_HEIGHT, 720);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_HEIGHT/u32GCD,
				720/u32GCD);		
	
	
	u32GCD = GCD(OPT_PREVIEW_WIDTH, 1280);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_WIDTH/u32GCD,
				1280/u32GCD);		
	/* Set Planar dimension  */				
	u32GCD = GCD(OPT_ENCODE_HEIGHT, 720);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PLANAR,			
				OPT_ENCODE_HEIGHT/u32GCD,
				720/u32GCD);		
							
	u32GCD = GCD(OPT_ENCODE_WIDTH, 1280);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PLANAR,			
				OPT_ENCODE_WIDTH/u32GCD,
				1280/u32GCD);
					
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				OPT_STRIDE,			//Packet preview width
				OPT_ENCODE_WIDTH,	//Planar encode width
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
	
	
	DrvVideoIn_SetMotionDet(TRUE, //BOOL bEnable,
								TRUE, //TRUE=8x8. FALSE = 16x16 BOOL bBlockSize,	
								FALSE);//FALSE for 1 bit Threshold+7 bits diff// BOOL bSaveMode
								
	DrvVideoIn_SetMotionDetEx(0,			//UINT32 u32DetFreq,
								20,			//UINT32 u32Threshold,
								(UINT32)u8DiffBuf,	//UINT32 u32OutBuffer,	
								(UINT32)u8OutLumBuf);	//UINT32 u32YBuffer			
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipe was enable. 											
				0 );							//Useless		
							
					
							
	sysSetLocalInterrupt(ENABLE_IRQ);						
														
	return Successful;			
}	