#ifndef _VIDEOIN_DEMO
#define _VIDEOIN_DEMO

/* Progressive Sensor */
//#define NT99141_VGA
//#define NT99141_SVGA
//#define NT99141_HD
//#define NT99050_VGA
//#define GC0308_VGA
//#define OV7725_VGA
//#define OV7670_VGA

/* Interlace TV Decoder */
#define TVP5150_HVGA		//One Field
//#define GM7150_HVGA		//One Field
//#define TW9900_HVGA		//One Fields	
//#define TVP5150_VGA		//Two Fields
//#define GM7150_VGA		//Two Fields
//#define TW9900_VGA		//Two Fields




#if defined(NT99141_VGA) || defined(NT99050_VGA) || defined(GC0308_VGA) ||\
	 defined(OV7725_VGA) || defined(OV7670_VGA)  || defined(TVP5150_VGA) || defined(GM7150_VGA) || defined(TW9900_VGA) 
#define OPT_CROP_WIDTH		640
#define OPT_CROP_HEIGHT		480
#define OPT_ENCODE_WIDTH		640
#define OPT_ENCODE_HEIGHT		480
#elif defined(TVP5150_HVGA) || defined(GM7150_HVGA)  || defined(TW9900_HVGA) 
#define OPT_CROP_WIDTH		640
#define OPT_CROP_HEIGHT		240
#define OPT_ENCODE_WIDTH		640
#define OPT_ENCODE_HEIGHT		240
#elif defined(NT99050_SVGA) 
#define OPT_CROP_WIDTH		640
#define OPT_CROP_HEIGHT		240
#define OPT_ENCODE_WIDTH		640
#define OPT_ENCODE_HEIGHT		240
#elif defined(NT99141_HD) 
#define OPT_CROP_WIDTH		1280
#define OPT_CROP_HEIGHT		720
#define OPT_ENCODE_WIDTH		1280
#define OPT_ENCODE_HEIGHT		720
#else
	#error "Please select one sensor"
#endif

typedef struct{
	INT32 (*IQ_GetBrightness)(void);
	INT32 (*IQ_SetBrightness)(INT16);
	INT32 (*IQ_GetSharpness)(void);	
	INT32 (*IQ_SetSharpness)(INT16);	
	INT32 (*IQ_GetContrast)(void);	
	INT32 (*IQ_SetContrast)(INT16);	
	INT32 (*IQ_GetHue)(void);
	INT32 (*IQ_SetHue)(INT16);	
}IQ_S;


#define OPT_UART
#ifdef OPT_UART
#define DBG_PRINTF		sysprintf
#else
#define DBG_PRINTF		printf
#endif



/* You need to specify the preview dimension (packet pipe), the value should equal or less cropping window */
#ifdef __TV__
#define OPT_STRIDE				640
#define OPT_LCM_WIDTH			640
#define OPT_LCM_HEIGHT		480
#define OPT_PREVIEW_WIDTH		640
#define OPT_PREVIEW_HEIGHT		480
#elif defined(__LCM_320x240__)
#define OPT_STRIDE				320
#define OPT_LCM_WIDTH			320
#define OPT_LCM_HEIGHT		240
#define OPT_PREVIEW_WIDTH		320
#define OPT_PREVIEW_HEIGHT		240
#elif defined(__LCM_480x272__)
#define OPT_STRIDE				480
#define OPT_LCM_WIDTH			480
#define OPT_LCM_HEIGHT		272
#define OPT_PREVIEW_WIDTH		364
#define OPT_PREVIEW_HEIGHT		272
#endif

void VideoIn_InterruptHandler(void);

//GCD.c
UINT8 GCD(UINT16 m1, UINT16 m2);

//Smpl_OV9660.c

UINT32 Smpl_OV9660(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_OV7670(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_OV7725(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_NT99050(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_WT8861(UINT8* pu8FrameBufferid);
UINT32 Smpl_NT99140_VGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_NT99140_HD(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_NT99141_VGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_NT99141_HD(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_NT99141_SVGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_GC0308(UINT8* u8PacketFrameBuffer, UINT8* u8PacketFrameBuffer1, UINT8* u8PacketFrameBuffer2);				
UINT32 Smpl_TVP5150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_TVP5150_TwoFields(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_GM7150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_GM7150_TwoFields(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_TW9900_VGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_TW9900_VGA_TWO_Field(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);


void Delay(UINT32 nCount);

//Smpl_VPOST.C
void InitVPOST(UINT8* pu8FrameBuffer);

//demo.C
void VideoIn_InterruptHandler(void);
UINT32 VideoIn_GetCurrFrameCount(void);
void VideoIn_ClearFrameCount(void);

INT32 register_sensor(IQ_S* ps_sensor);
INT32 register_TW9900_sensor(IQ_S* ps_sensor);

//FrameSyn.c
VOID pfnFSC_Ch0_ReadSwitchCallback(VOID);
VOID pfnFSC_Ch0_WriteSwitchCallback(VOID);
VOID pfnFSC_Ch0_ReadErrorCallback(VOID);
VOID pfnFSC_Ch0_WriteErrorCallback(VOID);

//Smpl_I2C.C
BOOL 
I2C_Write_8bitSlaveAddr_8bitReg_8bitData(
	UINT8 uAddr, 
	UINT8 uRegAddr, 
	UINT8 uData	
);
UINT8 
I2C_Read_8bitSlaveAddr_8bitReg_8bitData(
	UINT8 uAddr, 
	UINT8 uRegAddr
);

#define __60HZ__
//#define __50HZ__

#endif /* !_VIDEOIN_DEMO */