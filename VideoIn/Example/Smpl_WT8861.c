#include "wblib.h"
#include "W55FA93_VideoIn.h"
#include "W55FA93_GPIO.h"
#include "DrvI2C.h"
#include "demo.h"

typedef struct tagRef
{
	UINT8	u8RegAddr;
	UINT8	u8RegData; 
}T_WT_I2C;

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

UINT32 I2CWrite(UINT32 u8Addr, UINT32 u8Index, UINT32 u8Data)
{

	DrvI2C_SendStart();
	Delay(1);
	if ( (DrvI2C_WriteByte(u8Addr,DrvI2C_Ack_Have,8)==FALSE) ||			// Write ID address to sensor
		 (DrvI2C_WriteByte(u8Index,DrvI2C_Ack_Have,8)==FALSE) ||	// Write register address to sensor
		 (DrvI2C_WriteByte(u8Data,DrvI2C_Ack_Have,8)==FALSE) )		// Write data to sensor
	{
		DrvI2C_SendStop();
		return FALSE;
	}
	DrvI2C_SendStop();

	if (u8Index==0x12 && (u8Data&0x80)!=0)
		Delay(1000);
   	return TRUE;
}


UINT8 I2CRead(UINT32 u8Addr,	UINT32 u8Index)
{
	
	UINT8 u8Data;
	DrvI2C_SendStart();
	if(DrvI2C_WriteByte(u8Addr,DrvI2C_Ack_Have,8)==FALSE)
	{
		DrvI2C_SendStop();
		return FALSE;
	}
	if(DrvI2C_WriteByte(u8Addr,DrvI2C_Ack_Have,8)==FALSE)
	{
		DrvI2C_SendStop();
		return FALSE;
	}

	// OMNI7620	must issue stop	here, others don't need
	 DrvI2C_SendStop();

	u8Addr |= 0x01;
	DrvI2C_SendStart();
	if(DrvI2C_WriteByte(u8Addr,DrvI2C_Ack_Have,8)==FALSE)
	{
		DrvI2C_SendStop();
		return FALSE;
	}
 	u8Data = DrvI2C_ReadByte(DrvI2C_Ack_Have,8);	
 	DrvI2C_SendStop();
 	return u8Data;
}

/*
static T_WT_I2C g_sWT8861_NTSC_RegValue_CCIR601[]=
{	
	{0xC0, 0x04},		//0X5E,
	{0xC1, 0x09},		//0X05,
	
	{0x01, 0x01},		//0X5E,

	{0x07, 0x20},		//0X05,

	{0x2E, 0x84},             //set NTSC mode
	{0x2F, 0x50},            //V SYNC AGC VCR NOISE ENABLE
	{0x31, 0xC1},            //AGC VALUE
	{0x04, 0x10},       //AGC GATE START			
	{0x12, 0x05},       //AGC GATE START
	{0x13, 0xC8},       //AGC GATE WIDTH
	{0x14, 0x38},       //AGC TIME DELAY
	{0x15, 0x58},       //CDTO
		
	{0x18, 0x26},             //CDTO
	{0x19, 0x2D},             //CDTO
	{0x1A, 0x8B},             //CDTO
	{0x1B, 0xA2},             //CDTO		
	{0xCD, 0x0A},	//0XC7,
	{0x08, 0x81}, //
	{0x09, 0x50}, 	//0X15,
	{0x0A, 0xA0},
	
	{0x04, 0xdd},
	{0x31, 0x61},
	{0xB0, 0xfa},        
	{0xD5, 0xe4},            
	{0xDC, 0x9f},
	{0xDE, 0x81} 

 };
 */
 static T_WT_I2C g_sWT8861_NTSC_RegValue_CCIR656[]=
{	
	{0xC0, 0x04},		//0X5E,
	{0xC1, 0x09},		//0X05,
	
	{0x01, 0x01},		//0X5E,
	{0x07, 0xA0},		//0X05,

	{0x2E, 0x84},             //set NTSC mode
	{0x2F, 0x50},            //V SYNC AGC VCR NOISE ENABLE
	{0x31, 0xC1},            //AGC VALUE
	{0x04, 0x10},       //AGC GATE START			
	{0x12, 0x05},       //AGC GATE START
	{0x13, 0xC8},       //AGC GATE WIDTH
	{0x14, 0x38},       //AGC TIME DELAY
	{0x15, 0x58},       //CDTO
		
	{0x18, 0x26},             //CDTO
	{0x19, 0x2D},             //CDTO
	{0x1A, 0x8B},             //CDTO
	{0x1B, 0xA2},             //CDTO		
	{0xCD, 0x0A},	//0XC7,
	{0x08, 0x81}, //
	{0x09, 0x50}, 	//0X15,
	{0x0A, 0xA0},
//	{0x0A, 0x80},	
	
	
	{0x04, 0xdd},
	{0x31, 0x61},
	{0xB0, 0xfa},        
	{0xD5, 0xe4},            
	{0xDC, 0x9f},
	{0xDE, 0x81} 
 };
 
UINT32 InitWT8861(UINT32 u32Mode)
{

	int i=0;

	videoIn_Open(48000, 24000);	
	
#if defined(__DEMO_BOARD__) || defined(__HMI_BOARD__)
	SnrPowerDown(FALSE);
#endif	
#if defined(__DEMO_BOARD__) || defined(__NUWICAM__)
	SnrReset();	
#endif	/* Sensor used System reset if HMI */
				
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
				
	
	for(i=0;i<(sizeof(g_sWT8861_NTSC_RegValue_CCIR656)/sizeof(g_sWT8861_NTSC_RegValue_CCIR656[0])); i++)
	{
		UINT8 u8Data;
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(0x24, (g_sWT8861_NTSC_RegValue_CCIR656[i].u8RegAddr), (g_sWT8861_NTSC_RegValue_CCIR656[i].u8RegData));
		u8Data = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(0x24, (g_sWT8861_NTSC_RegValue_CCIR656[i].u8RegAddr));
		DBG_PRINTF("Addrr 0x%x = 0x%x\n", (g_sWT8861_NTSC_RegValue_CCIR656[i].u8RegAddr), u8Data);
	}	
	return 0;
}

/*===================================================================
	VPOST's dimension = (480x272)	
	Packet dimension = (364*272)	
	Stride should be LCM resolution  480.
	Packet frame start address = VPOST frame start address + (480-364)/2*2 	
=====================================================================*/
UINT32 Smpl_WT8861(UINT8* pu8FrameBuffer)
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;
	UINT32 u32GCD;
	PUINT8 pu8PacketBuf;
	
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer | 0x80000000);
	memset(pu8PacketBuf, 0x0, 640*480*2);
	InitVPOST(pu8FrameBuffer);

	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 
	
#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 72000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	//videoIn_Init(TRUE, 0, 12000, eVIDEOIN_3RD_SNR_CCIR601);
	videoIn_Init(TRUE, 0, 72000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	videoIn_Init(TRUE, 0, 72000, eVIDEOIN_3RD_SNR_CCIR601);	
#endif	
		
	InitWT8861(0);			//0 for CCIR656	
	videoIn_Open( 48000, 24000);
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	
							);	//Frame End interrupt

	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				3,							//UINT32 u32FieldEnable,	
				eVIDEOIN_TYPE_CCIR656,		//VIDEOIN_TYPE bInputType, 
				FALSE);						//BOOL bFieldSwap		
				
	videoinIoctl(VIDEOIN_IOCTL_SET_FIELD_DET,			
				0,			//Detection position, v-start=0 or v-end=1 if Detection method = 0	
				0,			//0: Detect field by Vsync & Hsync. 1:Detect field by CCIR656. The API is only available as CCIR601
				0);			//Useless
										
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				TRUE,
				FALSE,							//Polarity.	
				FALSE);		
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_UYVY, 			//Input Order 
				eVIDEOIN_IN_YUV422,		//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				0x0,							//Vertical start position
				0x38,						//Horizontal start position	
				0);							//Useless
	videoinIoctl(VIDEOIN_IOCTL_CROPPING_DIMENSION,				
				480,							//UINT16 u16Height, 
				640,							//UINT16 u16Width;	
				0);	
				
	u32GCD = GCD(272, 480);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PACKET,			//272/480
				272/u32GCD,
				480/u32GCD);		
	u32GCD = GCD(364, 640);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PACKET,			//364/640
				364/u32GCD,
				640/u32GCD);		
	u32GCD = GCD(480, 480);						 							 
	videoinIoctl(VIDEOIN_IOCTL_VSCALE_FACTOR,
				eVIDEOIN_PLANAR,			//640/640
				480/u32GCD,
				480/u32GCD);		
	u32GCD = GCD(640, 640);																
	videoinIoctl(VIDEOIN_IOCTL_HSCALE_FACTOR,
				eVIDEOIN_PLANAR,			//640/640
				640/u32GCD,
				640/u32GCD);					
	videoinIoctl(VIDEOIN_IOCTL_SET_STRIDE,										
				480,				
				480,
				0);				
				
	videoinIoctl(VIDEOIN_IOCTL_SET_BUF_ADDR,
				eVIDEOIN_PACKET,			
				0, 							//Packet buffer addrress 0	
				(UINT32)((UINT32)pu8FrameBuffer + (480-364)/2*2) );	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
				eVIDEOIN_PACKET,			// which packet was enable. 											
				0 );										
										
	sysSetLocalInterrupt(ENABLE_IRQ);																																																																											
	return Successful;
}

