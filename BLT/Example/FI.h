typedef struct {
    S_DRVBLT_MATRIX          sMatrix;
    E_DRVBLT_BMPIXEL_FORMAT  eSrcFmt;
    E_DRVBLT_DISPLAY_FORMAT  eDestFmt;
    
    INT32                       i32Flag;
    S_DRVBLT_ARGB16          sColorMultiplier;
    S_DRVBLT_ARGB16          sColorOffset;
    E_DRVBLT_FILL_STYLE      eFilStyle;
} S_FI_BLITTRANSFORMATION;


typedef struct {
    S_DRVBLT_RECT            sRect;
    S_DRVBLT_ARGB8           sARGB8;
    UINT32                      u32FBAddr;
    INT32                       i32Stride;
    E_DRVBLT_DISPLAY_FORMAT  eDisplayFmt;
    INT32                       i32Blend;
} S_FI_FILLOP;

typedef struct {
	S_DRVBLT_ARGB8			*psARGB8;
	S_DRVBLT_SRC_IMAGE		sDrvSrcImage;
} S_FI_SRC_IMAGE;

typedef struct {
    S_FI_SRC_IMAGE       		sFISrcImage;
    S_DRVBLT_DEST_FB         sDestFB;
    S_FI_BLITTRANSFORMATION		*psTransform;
} S_FI_BLITOP;

#define		SD_CARD_SOURCE		1
//#define     NULL            0

#define		STRIDE_FOR_BYTE		1		// 0 : Stride for pixel, 1 : Stride for Byte

#define     VPOST_WIDTH     320
#define     VPOST_HEIGHT    240

#define		CACHE_ADDR		0x80000000

//#define     SRC_IMAGE_ADDR          (0x1800000 | CACHE_ADDR)		// Source Pattern
//#define     SRC2_IMAGE_ADDR         (0x1900000 | CACHE_ADDR)
//#define     SRC_DEST_ADDR           (0x1A00000 | CACHE_ADDR)	// Destination Address for VPOST to display
//#define     SRC_DEST2_ADDR          (0x1B80000 | CACHE_ADDR)	// BackUp Destination data for next alpha blending
//#define     VPOST_DISPLAY_ADDR      SRC_DEST_ADDR

extern UINT32 SRC_IMAGE_ADDR,SRC_DEST_ADDR,SRC_DEST2_ADDR, VPOST_DISPLAY_ADDR;



#define     MIN_RECT_TEST_SIZE   16
#define     MAX_RECT_TEST_SIZE   64


extern S_DRVBLT_ARGB8 default_pallete[];
extern UINT32	gDisplayFormat;


void ClearFrameBuffer(void);
void FillFrameBuffer(UINT32 u32BufAddr, UINT32 Width, UINT32 Height, E_DRVBLT_DISPLAY_FORMAT eDestFmt,UINT8 u8R, UINT8 u8G, UINT8 u8B, UINT8 u8A);

void SI_2DBlit(S_FI_BLITOP sBiltOp);
void SI_Fill(S_FI_FILLOP sFillOp);

void BlitOpTest(void);
void FillOpTest(void);

void BlitCopyImageToFB(UINT32 u32SrcAddr,UINT32 u32Xoffset, UINT32 u32Yoffset, UINT32 Width, UINT32 Height, E_DRVBLT_DISPLAY_FORMAT eDestFmt);
void CopySrcImageToFB(void);
void CopyDestImageToFB(void);
void defaultSetting(void);
void WaitOpCompleted(void);
void BlitAlpahTest(void);

UINT32 GetDestRowByte(E_DRVBLT_DISPLAY_FORMAT eDestFmt, UINT32 u32pixelwidth);
UINT32 GetSrcRowByte(E_DRVBLT_BMPIXEL_FORMAT eSrcFmt, UINT32 u32pixelwidth);





















