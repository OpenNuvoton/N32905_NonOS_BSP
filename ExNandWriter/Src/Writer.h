
#define	MAJOR_VERSION_NUM	1
#define	MINOR_VERSION_NUM	1

// 2011/10/27 by CJChen1@nuvoton.com, 0 to set NAND target to CS0, 1 to set NAND target to CS1
#define TARGET_Chip_Select  1

extern UINT	g_Font_Height, g_Font_Width, g_Font_Step;

#define	COLOR_RGB16_RED		0xF800
#define	COLOR_RGB16_WHITE	0xFFFF
#define	COLOR_RGB16_GREEN	0x07E0
#define	COLOR_RGB16_BLACK	0x0000

#define NVT_SM_INFO_T FMI_SM_INFO_T

/* F/W update information */
typedef struct fw_update_info_t
{
	UINT16	imageNo;
	UINT16	imageFlag;
	UINT16	startBlock;
	UINT16	endBlock;
	UINT32	executeAddr;
	UINT32	fileLen;
	CHAR	imageName[32];
} FW_UPDATE_INFO_T;

typedef struct IBR_boot_struct_t
{
	UINT32	BootCodeMarker;
	UINT32	ExeAddr;
	UINT32	ImageSize;
	UINT32  SkewMarker;
	UINT32	DQSODS;
	UINT32	CLKQSDS;
	UINT32	DRAMMarker;
	UINT32	DRAMSize;
} IBR_BOOT_STRUCT_T;

typedef struct INI_Info {
	char NandLoader[32];
	char Logo[32];
	char NVTLoader[32];
	int	 SystemReservedMegaByte;
	int	 NAND1_1_SIZE;
	int  NAND1_1_FAT;
	int  NAND1_2_FAT;
} INI_INFO_T;

/* extern parameters */
extern UINT32 infoBuf;
extern FMI_SM_INFO_T *pNvtSM0;
extern UINT8 *pInfo;
extern INT8 nIsSysImage;

INT nvtSMInit(void);
INT nvtSMchip_erase(UINT32 startBlcok, UINT32 endBlock);
INT nvtSMpread(INT PBA, INT page, UINT8 *buff);
INT nvtSMpwrite(INT PBA, INT page, UINT8 *buff);
INT nvtSMblock_erase(INT PBA);
INT CheckBadBlockMark(UINT32 block);

void Draw_Font(UINT16 u16RGB,S_DEMO_FONT* ptFont,UINT32	u32x,UINT32 u32y,PCSTR	pszString);
void Draw_Status(UINT32	u32x,UINT32	u32y,int Status);
void Draw_Clear(int xStart, int yStart, int xEnd, int yEnd, UINT16 color);
void Draw_CurrentOperation(PCSTR pszString, int Retcode);
void Draw_FinalStatus(int bIsAbort);
void Draw_Init(void);
void Draw_Wait_Status(UINT32 u32x, UINT32 u32y);
void Draw_Clear_Wait_Status(UINT32 u32x, UINT32 u32y);

int ProcessINI(char *fileName);
