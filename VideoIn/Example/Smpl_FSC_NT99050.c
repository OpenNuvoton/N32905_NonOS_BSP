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
	#include "NT99050\NT99050_30F.dat"
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

#ifdef __DEMO_BOARD__
	SnrPowerDown(FALSE);
#endif	
	SnrReset();	
		
	u32TableSize = g_NT99050_InitTable[nIndex].u32TableSize;
	psRegValue = g_NT99050_InitTable[nIndex].sRegTable;
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
UINT32 Smpl_NT99050(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
				eVIDEOIN_PACKET,			
				OPT_PREVIEW_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/u32GCD);		
	u32GCD = GCD(OPT_PREVIEW_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			
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
				
	
	DrvVideoIn_SetMotionDet(TRUE,		 //BOOL bEnable,
						TRUE, 		//TRUE=8x8. FALSE = 16x16 BOOL bBlockSize,	
						FALSE);		//FALSE for 1 bit Threshold+7 bits diff// BOOL bSaveMode
								
	DrvVideoIn_SetMotionDetEx(0,						//UINT32 u32DetFreq,
							20,					//UINT32 u32Threshold,
							(UINT32)u8DiffBuf,		//UINT32 u32OutBuffer,	
							(UINT32)u8OutLumBuf);	//UINT32 u32YBuffer			
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which Pipes was enable. 											
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
	
	UINT8 i,gamma_tmp;
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
	UINT8 u8RegData; 
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