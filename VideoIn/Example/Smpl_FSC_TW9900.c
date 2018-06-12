#include "wblib.h"
#include "W55FA93_VideoIn.h"
#include "W55FA93_GPIO.h"
#include "DrvI2C.h"
#include "demo.h"
//#include "w55fa92_i2c.h"

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
#define OV7725_CCIR656
struct OV_RegValue
{
	UINT8	u8RegAddr;		//Register Address
	UINT8	u8Value;			//Register Data
};

struct OV_RegTable{
    struct OV_RegValue *sRegTable;
	UINT32 u32TableSize;
};

#define __NTSC__
//#define __PAL__

#define REG_VALUE_INIT	0
#define REG_VALUE_NTSC	1
#define REG_VALUE_PAL		2

extern UINT8 u8PlanarFrameBuffer[];

#define _REG_TABLE_SIZE(nTableName)	(sizeof(nTableName)/sizeof(struct OV_RegValue))

static struct OV_RegValue g_sTW9900_Init[]=
{
	#include "TW9900\TW9900_Init.dat"
};
static struct OV_RegValue g_sTW9900_NTSC[]=
{
	#include "TW9900\TW9900_NTSC.dat"
};
static struct OV_RegValue g_sTW9900_PAL[]=
{
	#include "TW9900\TW9900_PAL.dat"
};
static struct OV_RegTable g_OV_InitTable[] =
{//8 bit slave address, 8 bit data. 
	{g_sTW9900_Init,_REG_TABLE_SIZE(g_sTW9900_Init)},
	{g_sTW9900_NTSC, _REG_TABLE_SIZE(g_sTW9900_NTSC)},	
	{g_sTW9900_PAL,_REG_TABLE_SIZE(g_sTW9900_PAL)},

	{0, 0}
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


static UINT8 g_uOvDeviceID[]= 
{
	0x8A,		// TW9900 = 14
};


VOID TW9900_Init(UINT32 nIndex)
{
	UINT32 u32Idx;
	UINT32 u32TableSize;
	UINT8  u8DeviceID;
	UINT8 u8ID;
	UINT8 i=0;
	
	struct OV_RegValue *psRegValue;
	DBG_PRINTF("Sensor ID = %d\n", nIndex);
	if ( nIndex > (sizeof(g_uOvDeviceID)/sizeof(UINT8)) )
		return;					

#if defined(__DEMO_BOARD__) || defined(__HMI_BOARD__)
	SnrPowerDown(FALSE);
#endif	
#if defined(__DEMO_BOARD__) || defined(__NUWICAM__)
	SnrReset();	
#endif	/* Sensor used System reset if HMI */						
		
	u32TableSize = g_OV_InitTable[REG_VALUE_INIT].u32TableSize;
	psRegValue = g_OV_InitTable[REG_VALUE_INIT].sRegTable;
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
				
	for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++){
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, (psRegValue->u8RegAddr), (psRegValue->u8Value));					
		#if 0
		Delay(1000);					
		u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x0);
		if(u8ID!=(psRegValue->u8Value))
			sysprintf("Init Sesnor Fail = (psRegValue->u8RegAddr)= %x\n", psRegValue->u8RegAddr);
		else
			sysprintf("Init Sesnor Successful = (psRegValue->u8RegAddr)= %x\n", psRegValue->u8RegAddr);	
		#endif	
	}
#ifdef __NTSC__		
	u32TableSize = g_OV_InitTable[REG_VALUE_NTSC].u32TableSize;
	psRegValue = g_OV_InitTable[REG_VALUE_NTSC].sRegTable;
	u8DeviceID = g_uOvDeviceID[nIndex];
	if ( psRegValue == 0 )
		return;	
#endif 
#ifdef __PAL__		
	u32TableSize = g_OV_InitTable[REG_VALUE_PAL].u32TableSize;
	psRegValue = g_OV_InitTable[REG_VALUE_PAL].sRegTable;
	u8DeviceID = g_uOvDeviceID[nIndex];
	if ( psRegValue == 0 )
		return;	
#endif 
		
	for(u32Idx=0;u32Idx<u32TableSize; u32Idx++, psRegValue++){
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, (psRegValue->u8RegAddr), (psRegValue->u8Value));	
		#if 0
		Delay(1000);		
		u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x0);
		if(u8ID!=(psRegValue->u8Value))
			sysprintf("Init Sesnor Fail = (psRegValue->u8RegAddr)= %x\n", psRegValue->u8RegAddr);
		else
			sysprintf("Init Sesnor Successful = (psRegValue->u8RegAddr)= %x\n", psRegValue->u8RegAddr);					
		#endif	
	}

	u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x0); //Device ID will read back 0x60
	DBG_PRINTF("Sensor ID0 = 0x%x\n", u8ID);

	for(i=0;i<0x6F;i++)
	{
		u8ID = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, i); //Device ID will read back 0x60
		DBG_PRINTF("0x%02x = 0x%02x\n", i,u8ID);
	}
	
	DrvI2C_Close();	
}
/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/
UINT32 Smpl_TW9900_VGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
	videoIn_Init(TRUE, 0, 64000, eVIDEOIN_SNR_CCIR601);
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 64000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	videoIn_Init(TRUE, 0, 64000, eVIDEOIN_3RD_SNR_CCIR601);	
#endif

	TW9900_Init(0);		
			
	videoIn_Open(96000, 64000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,
				TRUE,							//Polarity.	
				TRUE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_VYUY, 			//Input Order 
				eVIDEOIN_IN_YUV422,		//Intput format//eVIDEOIN_IN_YUV422,		//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
#ifdef __PAL__		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0x0,							//Vertical start position
				0x0,							//Horizontal start position	
				0);	
#endif

#ifdef __NTSC__		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0x0,							//Vertical start position
				0x0,							//Horizontal start position	
				0);	
#endif										//Useless
	videoinIoctl(VIDEOIN_IOCTL_CROPPING_DIMENSION,				
				OPT_CROP_HEIGHT/2,							//UINT16 u16Height, 
				OPT_CROP_WIDTH,							//UINT16 u16Width;	
				0);							//Useless
	
	/* Set Packet dimension */
	u32GCD = GCD(OPT_PREVIEW_HEIGHT, OPT_CROP_HEIGHT/2);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			//272/480
				OPT_PREVIEW_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/2/u32GCD);		
	u32GCD = GCD(OPT_PREVIEW_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			//364/640
				OPT_PREVIEW_WIDTH/u32GCD,
				OPT_CROP_WIDTH/u32GCD);		
				
	/* Set Planar dimension  */					
	u32GCD = GCD(OPT_ENCODE_HEIGHT, OPT_CROP_HEIGHT/2);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PLANAR,			//640/640
				OPT_ENCODE_HEIGHT/u32GCD,
				OPT_CROP_HEIGHT/2/u32GCD);		
	u32GCD = GCD(OPT_ENCODE_WIDTH, OPT_CROP_WIDTH);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PLANAR,			//640/640
				OPT_ENCODE_WIDTH/u32GCD,
				OPT_CROP_WIDTH/u32GCD);
				
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
	
	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				1,	/* 0: Both fields are disabled. 1: Field 1 enable. 2: Field 2 enable. 3: Both fields are enable */
				eVIDEOIN_TYPE_CCIR656,	/* 0: CCIR601.	1: CCIR656 */
				0);		
				
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipes was enable. 											
				0 );							//Useless		
	
	DrvVideoIn_SetOperationMode(TRUE);	
	while(DrvVideoIn_GetOperationMode()==TRUE);
	sysSetLocalInterrupt(ENABLE_IRQ);										
	return Successful;			
}	


UINT32 Smpl_TW9900_VGA_TWO_Field(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
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
	videoIn_Init(TRUE, 0, 64000, eVIDEOIN_SNR_CCIR601);
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 64000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	videoIn_Init(TRUE, 0, 64000, eVIDEOIN_3RD_SNR_CCIR601);	
#endif

	TW9900_Init(0);		
			
	videoIn_Open(96000, 64000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,
				TRUE,							//Polarity.	
				TRUE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_VYUY, 			//Input Order 
				eVIDEOIN_IN_YUV422,		//Intput format//eVIDEOIN_IN_YUV422,		//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
#ifdef __PAL__		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0x0,							//Vertical start position
				0x0,							//Horizontal start position	
				0);	
#endif

#ifdef __NTSC__		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0x0,							//Vertical start position
				0x0,							//Horizontal start position	
				0);	
#endif										//Useless
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
	
	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				3,	/* 0: Both fields are disabled. 1: Field 1 enable. 2: Field 2 enable. 3: Both fields are enable */
				eVIDEOIN_TYPE_CCIR656,	/* 0: CCIR601.	1: CCIR656 */
				0);		
				
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipes was enable. 											
				0 );							//Useless		
							
							
	sysSetLocalInterrupt(ENABLE_IRQ);										
	return Successful;			
}	
#if 0
static void AdjustConstraint(void)
{
	UINT8 u8Level;
	UINT32 u32Item;
	sysprintf("Please input constraint level\n");	
	do{
		u32Item = sysGetChar();
		u8Level = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x11);
		sysprintf("Current constraint level = %d\n", u8Level);	
		if(u32Item=='+')
			u8Level= u8Level+1;
		else if(u32Item=='-')
			u8Level = u8Level-1;		
		sysprintf("Write Levell = %d\n", u8Level);		
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x11, u8Level);
		u8Level = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x11);
		sysprintf("Readback Levell = %d\n", u8Level);	
	}while(u32Item!='q');	
}	

static void AdjustBrightness(void)
{
	INT8 i8Level;
	UINT32 u32Item;
	sysprintf("Please input Brightness level\n");	

	do{
		u32Item = sysGetChar();
		i8Level = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x10);
		sysprintf("Current brightness level = %d\n", i8Level);	
		if(u32Item=='+')
			i8Level= i8Level+1;
		else if(u32Item=='-')
			i8Level = i8Level-1;		
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x10, i8Level);
	}while(u32Item!='q');
}	

static void AdjustVdelay(void)
{
	INT8 i8Level;
	UINT32 u32Item;
	sysprintf("Please input Vdelay level\n");	

	do{
		u32Item = sysGetChar();
		i8Level = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x08);
		sysprintf("Current Vdelay level = %d\n", i8Level);	
		if(u32Item=='+')
			i8Level= i8Level+1;
		else if(u32Item=='-')
			i8Level = i8Level-1;
		if((u32Item=='+') || (u32Item=='-'))	
			I2C_Write_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x08, i8Level);
	}while( (u32Item=='-') ||  (u32Item=='+'));
}	

static void AdjustVactive(void)
{
	INT8 i8Level;
	UINT32 u32Item;
	sysprintf("Please input Vdelay level\n");	

	do{
		u32Item = sysGetChar();
		i8Level = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x09);
		sysprintf("Current Vdelay level = %d\n", i8Level);	
		if(u32Item=='+')
			i8Level= i8Level+1;
		else if(u32Item=='-')
			i8Level = i8Level-1;
		if((u32Item=='+') || (u32Item=='-'))	
			I2C_Write_8bitSlaveAddr_8bitReg_8bitData(0x88, 0x09, i8Level);
	}while( (u32Item=='-') ||  (u32Item=='+'));
}	
#endif


static INT32 IQ_GetBrightness(VOID)
{//OK
	UINT8 u8RegData, u8DeviceID;
	u8DeviceID = g_uOvDeviceID[0];	
	u8RegData = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x10); 
	return (UINT32)u8RegData;		
} 
static INT32 IQ_SetBrightness(INT16 i16Brightness)
{//OK
	UINT8 u8DeviceID;
	u8DeviceID = g_uOvDeviceID[0];	
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x10, i16Brightness); //Y component Brightnes
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
	UINT8 u8Sharp;
	u8DeviceID = g_uOvDeviceID[0];
	
	u8Sharp = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x12);		/* Write sharp[9:8] */
	
	return (u8Sharp);
} 
static INT32 IQ_SetSharpness(INT16 i16Sharpness)
{
	UINT8 u8DeviceID = g_uOvDeviceID[0];
	
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x12, (UINT8)i16Sharpness);		
		
	return Successful;	
} 

										
static INT32 IQ_GetContrast(VOID)
{//OK
	UINT8 u8DeviceID;
	UINT8 u8Contrast;
	u8DeviceID = g_uOvDeviceID[0];
	
	u8Contrast = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x11);		/* Write sharp[9:8] */
	
	return (u8Contrast);
} 
static INT32 IQ_SetContrast(INT16 ucContrastValue)
{//OK
	
	UINT8 u8DeviceID = g_uOvDeviceID[0];
	
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x11, (UINT8)ucContrastValue);		
		
	return Successful;
} 
static INT32 IQ_GetHue(void)
{
	UINT8 u8Hue;
	UINT8 u8DeviceID = g_uOvDeviceID[0];
	
	u8Hue = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x15);		
		
	return Successful;
}
static INT32 IQ_SetHue(INT16 i16Hue)
{
	UINT8 u8DeviceID = g_uOvDeviceID[0];
	
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, 0x15, (UINT8)i16Hue);		
		
	return Successful;
} 


static IQ_S ImageQualityTbl =
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
INT32 register_TW9900_sensor(IQ_S* ps_sensor)
{
	*ps_sensor = ImageQualityTbl;
	return Successful;	
}