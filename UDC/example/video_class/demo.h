#ifndef _VIDEOIN_DEMO
#define _VIDEOIN_DEMO


#define OPT_UART
#ifdef OPT_UART
#define DBG_PRINTF		sysprintf
#else
#define DBG_PRINTF		printf
#endif

#define OPT_STRIDE		640    //320   //640, 480
#define OPT_CROP_WIDTH		640
#define OPT_CROP_HEIGHT		480
#define OPT_PREVIEW_WIDTH		640
#define OPT_PREVIEW_HEIGHT		480

// current sensor definition
#define OV_7670			3
#define OV_7725			4
void VideoIn_InterruptHandler(void);

#define BITSTREAM_OFFSET 64*1024

//GCD.c
UINT8 GCD(UINT16 m1, UINT16 m2);

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


//Smpl_OV9660.c
UINT32 Smpl_OV9660(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_OV7670(UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_OV7725(UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);
UINT32 Smpl_WT8861(UINT8* pu8FrameBufferid);
UINT32 Smpl_NT99050(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_OV7725(UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2);

UINT32 Smpl_GC0308(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_GM7150(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_TVP5150(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
INT32 register_sensor(IQ_S* ps_sensor);
UINT32 Smpl_TVP5150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_TVP5150_TwoFields(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_GM7150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_GM7150_TwoFields(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);
UINT32 Smpl_NT99141_VGA(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1);


/* Buffer for Packet & Planar format */
extern UINT32 u32PacketFrameBuffer0, u32PacketFrameBuffer1,u32BitstreamBuffer0,u32BitstreamBuffer1;
extern UINT8  u8FrameBuffer0[];	
extern UINT8  u8FrameBuffer1[];	


/* Current Width & Height */
extern UINT16 u16CurWidth, u16CurHeight;

void Delay(UINT32 nCount);

/* UVC Main */
VOID uvc_main(VOID);
/* UVC event */
VOID uvcdEvent(VOID);
/* Change VideoIN Setting for Frame size or Buffer Address */
VOID ChangeFrame(BOOL bChangeSize, UINT32 u32Address, UINT16 u16Width,UINT16 u16Height);
/* VideoIN Buffer Address Control when Frame End */
VOID VideoInFrameEnd_InterruptHandler(VOID);
/* Get Image Buffer for USB transfer */
INT GetImageBuffer(VOID);
/* JPEG Encode function */
UINT32 jpegEncode(UINT32 u32YAddress,UINT32 u32BitstreamAddress, UINT16 u16Width,UINT16 u16Height);
/* Process Unit Control */
UINT32 ProcessUnitControl(UINT32 u32ItemSelect,UINT32 u32Value);
/* Get Image Size and Address (Image data control for Foramt and Frame)*/
INT GetImage(PUINT32 pu32Addr, PUINT32 pu32transferSize);

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

#endif /* !_VIDEOIN_DEMO */