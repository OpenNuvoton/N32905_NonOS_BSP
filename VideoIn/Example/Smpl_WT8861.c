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

//#define _WT8861

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
#ifdef __GPIO_PIN__	
	gpio_open(GPIO_PORTB, 13);				//GPIOB 13 as GPIO
	gpio_open(GPIO_PORTB, 14);				//GPIOB 14 as GPIO
#else	
	gpio_open(GPIO_PORTB);				//GPIOB as GPIO
#endif	
	DrvI2C_Open(eDRVGPIO_GPIOB, 					
				eDRVGPIO_PIN13, 
				eDRVGPIO_GPIOB,
				eDRVGPIO_PIN14, 
				(PFN_DRVI2C_TIMEDELY)Delay);	
	
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
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_TVD_CCIR601);	
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

