#include "wblib.h"
#include "W55FA93_VideoIn.h"
#include "W55FA93_GPIO.h"
#include "TVP5150\sensor_tvp5150_reg.h"
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
	UINT8	u8RegAddr;		//Register Address
	UINT8	u8Value;			//Register Data
};

struct OV_RegTable{
	struct OV_RegValue *sRegTable;
	UINT32 u32TableSize;
};


extern UINT8 u8PlanarFrameBuffer[];


#define _REG_TABLE_SIZE(nTableName)	(sizeof(nTableName)/sizeof(struct OV_RegValue))


#define __NTSC__
//#define __PAL__			

static struct OV_RegValue g_sTVP5150_RegValue[]=
{//TVP5150
	{ /* 0x00 */	TVP5150_VD_IN_SRC_SEL_1,	0x00	},
	{ /* 0x01 */	TVP5150_ANAL_CHL_CTL,		0x15	},
	{ /* 0x02 */	TVP5150_OP_MODE_CTL,		0x00	},
	{ /* 0x03 */	TVP5150_MISC_CTL,			0x09	},
	{ /* 0x06 */	TVP5150_COLOR_KIL_THSH_CTL,	0x10	},
	{ /* 0x07 */	TVP5150_LUMA_PROC_CTL_1,	0x20	},	// Luma signal delay ::  0 pixel clocks delay 0x20, 0x23, 0x2B
	{ /* 0x08 */	TVP5150_LUMA_PROC_CTL_2,	0x4C	},	//Luminance filter select: 0x48 comb filter disabled, chroma trap filter enable !!!!! 
	{ /* 0x09 */	TVP5150_BRIGHT_CTL,			0x80	},
	{ /* 0x0a */	TVP5150_SATURATION_CTL,		0x80 	},	//TVP5150_SATURATION_CTL,0xA0
	{ /* 0x0b */	TVP5150_HUE_CTL,			0x00	},	
	{ /* 0x0c */	TVP5150_CONTRAST_CTL,		0x80 	},	//TVP5150_CONTRAST_CTL,0x84
	{ /* 0x0d */	TVP5150_DATA_RATE_SEL,		0x47	},
	{ /* 0x0e */	TVP5150_LUMA_PROC_CTL_3,	0x00	},	//Luminance filter stop band bandwidth:: No notch
	{ /* 0x0f */	TVP5150_CONF_SHARED_PIN,	0x08	},
	{ /* 0x11 */	TVP5150_ACT_VD_CROP_ST_MSB,	0x00	},
	{ /* 0x12 */	TVP5150_ACT_VD_CROP_ST_LSB,	0x00	},
	{ /* 0x13 */	TVP5150_ACT_VD_CROP_STP_MSB,0x00	},
	{ /* 0x14 */	TVP5150_ACT_VD_CROP_STP_LSB,0x00	},
	{ /* 0x15 */	TVP5150_GENLOCK,			0x01	},
	{ /* 0x16 */	TVP5150_HORIZ_SYNC_START,	0x80	},
	{ /* 0x18 */	TVP5150_VERT_BLANKING_START,0x00	},
	{ /* 0x19 */	TVP5150_VERT_BLANKING_STOP,	0x00	},
	{ /* 0x1a */	TVP5150_CHROMA_PROC_CTL_1,	0x0c	},
	{ /* 0x1b */	TVP5150_CHROMA_PROC_CTL_2,	0x15	}, //Chrominance filter select: No notch = 0x10
	{ /* 0x1c */	TVP5150_INT_RESET_REG_B,	0x00	},
	{ /* 0x1d */	TVP5150_INT_ENABLE_REG_B,	0x00	},
	{ /* 0x1e */	TVP5150_INTT_CONFIG_REG_B,	0x00	},
	{ /* 0x2e */	TVP5150_MACROVISION_ON_CTR,	0x0f	},
	{ /* 0x2f */	TVP5150_MACROVISION_OFF_CTR,0x01	},
	{ /* 0xbb */	TVP5150_TELETEXT_FIL_ENA,	0x00	},
	{ /* 0xc0 */	TVP5150_INT_STATUS_REG_A,	0x00	},
	{ /* 0xc1 */	TVP5150_INT_ENABLE_REG_A,	0x00	},
	{ /* 0xc2 */	TVP5150_INT_CONF,			0x04	},
	{ /* 0xc8 */	TVP5150_FIFO_INT_THRESHOLD,	0x80	},
	{ /* 0xc9 */	TVP5150_FIFO_RESET,			0x00	},
	{ /* 0xca */	TVP5150_LINE_NUMBER_INT,	0x00	},
	{ /* 0xcb */	TVP5150_PIX_ALIGN_REG_LOW,	0x4e	},
	{ /* 0xcc */	TVP5150_PIX_ALIGN_REG_HIGH,	0x00	},
	{ /* 0xcd */	TVP5150_FIFO_OUT_CTRL,		0x01	},
	{ /* 0xcf */	TVP5150_FULL_FIELD_ENA,		0x00	},
	{ /* 0xd0 */	TVP5150_LINE_MODE_INI,		0x00	},
	{ /* 0xfc */	TVP5150_FULL_FIELD_MODE_REG,0x7f	},
};


static struct OV_RegTable g_OV_InitTable[] =
{//8 bit slave address, 8 bit data. 
	{g_sTVP5150_RegValue,	_REG_TABLE_SIZE(g_sTVP5150_RegValue)},
};

static UINT8 g_uOvDeviceID[]= 
{
	0xBA,		// TVP5150	
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


void TVP5150SetVideoformat(int std)
{
	UINT8  uDeviceID;
	UINT8  fmt = 0;
	
	uDeviceID  = g_uOvDeviceID[0];
	
	/* First tests should be against specific std */
	if (std == TVP_STD_ALL) {
		fmt=0;	/* Autodetect mode */
	} else if (std & TVP_STD_NTSC_443) {
		fmt=0xa;
	} else if (std & TVP_STD_PAL_M) {
		fmt=0x6;
	} else if (std & (TVP_STD_PAL_N| TVP_STD_PAL_Nc)) {
		fmt=0x8;
	} else {
		/* Then, test against generic ones */
		if (std & TVP_STD_NTSC) {
			fmt=0x2;
		} else if (std & TVP_STD_PAL) {
#if 0		
			I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, 0x7F, 0x00);			
#endif			
			fmt=0x4;
		}
	}
	sysprintf("TVP5150 Video format : %02x. \r\n",fmt);
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, TVP5150_VIDEO_STD, fmt);
#if 0
	sysprintf("Dannel Suggestion\n");
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, 0x01, 0x14);				
	do{
		sysprintf("TVP5150 V/H unlock\n");
		fmt = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, 0x88);
	}while( (fmt & 0x6) != 0x06 );
	sysprintf("TVP5150 V/H lock\n");
	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, 0x03, 0x69);		
#endif		

}

void TVP5150SetInputSource(int src)
{
	UINT8  uDeviceID;
	UINT8  inputsrc = 0;
	
	uDeviceID  = g_uOvDeviceID[0];
	
	/* First tests should be against specific std */
	if (src == TVP5150_A1P1A) {
		inputsrc=0x00;	/* A1P1A mode */
		sysprintf("TVP5150 Video Input Source : %02x. TVP5150 Channel = A1P1A mode. \r\n",inputsrc);
	} else if (src == TVP5150_A1P1B) {
		inputsrc=0x02;	/* A1P1B mode */
		sysprintf("TVP5150 Video Input Source : %02x. TVP5150 Channel = A1P1B mode. \r\n",inputsrc);
	} else {
		inputsrc=0x01;	/* S-Video mode */
		sysprintf("TVP5150 Video Input Source : %02x. TVP5150 Channel = SVIDEO mode. \r\n",inputsrc);
	}

	I2C_Write_8bitSlaveAddr_8bitReg_8bitData(uDeviceID, TVP5150_VD_IN_SRC_SEL_1, inputsrc);
}

static void TVP_I2C_Delay(UINT32 nCount)
{
	volatile UINT32 i;
	for(;nCount!=0;nCount--)
		for(i=0;i<50;i++);
}


static void dump_reg(UINT8 c)
{
	sysprintf("tvp5150: Video input source selection -REG:0x%x = 0x%02x\n",
					TVP5150_VD_IN_SRC_SEL_1, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VD_IN_SRC_SEL_1));
	sysprintf("tvp5150: Analog channel controls -REG:0x%x  = 0x%02x\n",
					TVP5150_ANAL_CHL_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ANAL_CHL_CTL));
	sysprintf("tvp5150: Operation mode controls -REG:0x%x = 0x%02x\n",
					TVP5150_OP_MODE_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_OP_MODE_CTL));
	sysprintf("tvp5150: Miscellaneous controls -REG:0x%x = 0x%02x\n",
					TVP5150_MISC_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_MISC_CTL));
	sysprintf("tvp5150: Autoswitch mask -REG:0x%x = 0x%02x\n",
					TVP5150_AUTOSW_MSK, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_AUTOSW_MSK));
	sysprintf("tvp5150: Color killer threshold control -REG:0x%x  = 0x%02x\n",
					TVP5150_COLOR_KIL_THSH_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_COLOR_KIL_THSH_CTL));
	sysprintf("tvp5150: Luminance processing controls REG:0x%x  = 0x%02x\n",
					TVP5150_LUMA_PROC_CTL_1, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_LUMA_PROC_CTL_1));
					
	sysprintf("tvp5150: Luminance processing controls -REG:0x%x  = 0x%02x\n",
					TVP5150_LUMA_PROC_CTL_2, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_LUMA_PROC_CTL_2));
					
	sysprintf("tvp5150: Luminance processing controls -REG:0x%x  = 0x%02x\n",
					TVP5150_LUMA_PROC_CTL_3, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_LUMA_PROC_CTL_3));								

	sysprintf("tvp5150: Brightness control -REG:0x%x  = 0x%02x\n",
					TVP5150_BRIGHT_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_BRIGHT_CTL));
	sysprintf("tvp5150: Color saturation control -REG:0x%x  = 0x%02x\n",
					TVP5150_SATURATION_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_SATURATION_CTL));
	sysprintf("tvp5150: Hue control -REG:0x%x  = 0x%02x\n",
					TVP5150_HUE_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_HUE_CTL));
	sysprintf("tvp5150: Contrast control -REG:0x%x  = 0x%02x\n",
					TVP5150_CONTRAST_CTL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_CONTRAST_CTL));
	sysprintf("tvp5150: Outputs and data rates select -REG:0x%x  = 0x%02x\n",
					TVP5150_DATA_RATE_SEL, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_DATA_RATE_SEL));
	sysprintf("tvp5150: Configuration shared pins -REG:0x%x  = 0x%02x\n",
					TVP5150_CONF_SHARED_PIN, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_CONF_SHARED_PIN));
					
	sysprintf("tvp5150: Active video cropping start -REG:0x%x  = 0x%02x\n",
					TVP5150_ACT_VD_CROP_ST_MSB, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ACT_VD_CROP_ST_MSB));
					
	sysprintf("tvp5150: Active video cropping start -REG:0x%x  = 0x%02x\n",
					TVP5150_ACT_VD_CROP_ST_LSB, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ACT_VD_CROP_ST_LSB));
			
	sysprintf("tvp5150: Active video cropping stop -REG:0x%x  = 0x%02x\n",
					TVP5150_ACT_VD_CROP_STP_MSB, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ACT_VD_CROP_STP_MSB));
					
	sysprintf("tvp5150: Active video cropping stop -REG:0x%x  = 0x%02x\n",
					TVP5150_ACT_VD_CROP_STP_LSB, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ACT_VD_CROP_STP_LSB));						
	
	sysprintf("tvp5150: Genlock/RTC -REG:0x%x  = 0x%02x\n",
					TVP5150_GENLOCK, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_GENLOCK));
					
	sysprintf("tvp5150: Horizontal sync start -REG:0x%x  = 0x%02x\n",
					TVP5150_HORIZ_SYNC_START, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_HORIZ_SYNC_START));
	sysprintf("tvp5150: Vertical blanking start  -REG:0x%x = 0x%02x\n",
					TVP5150_VERT_BLANKING_START, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VERT_BLANKING_START));
	sysprintf("tvp5150: Vertical blanking stop -REG:0x%x  = 0x%02x\n",
					TVP5150_VERT_BLANKING_STOP, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VERT_BLANKING_STOP));
					
					
	sysprintf("tvp5150: Chrominance processing control  -REG:0x%x = %02x\n",
					TVP5150_CHROMA_PROC_CTL_1, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_CHROMA_PROC_CTL_1));
	sysprintf("tvp5150: Chrominance processing control  -REG:0x%x = %02x\n",				
					TVP5150_CHROMA_PROC_CTL_2, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_CHROMA_PROC_CTL_2));
					
	sysprintf("tvp5150: Interrupt reset register B -REG:0x%x  = 0x%02x\n",
					TVP5150_INT_RESET_REG_B, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_RESET_REG_B));
					
	sysprintf("tvp5150: Interrupt enable register B -REG:0x%x  = 0x%02x\n",
					TVP5150_INT_ENABLE_REG_B, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_ENABLE_REG_B));
					
	sysprintf("tvp5150: Interrupt configuration register B -REG:0x%x  = 0x%02x\n",
					TVP5150_INTT_CONFIG_REG_B, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INTT_CONFIG_REG_B));
					
	sysprintf("tvp5150: Video standard -REG:0x%x  = 0x%02x\n",
					TVP5150_VIDEO_STD, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VIDEO_STD));
					
	sysprintf("tvp5150: Chroma gain factor: Cb -REG:0x%x  =0x%02x\n",
					TVP5150_CB_GAIN_FACT, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_CB_GAIN_FACT));
	sysprintf("tvp5150: Chroma gain factor: Cr -REG:0x%x  =0x%02x\n",				
					TVP5150_CR_GAIN_FACTOR, I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_CR_GAIN_FACTOR));
					
	sysprintf("tvp5150: Macrovision on counter -REG:0x%x  = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_MACROVISION_ON_CTR));
	sysprintf("tvp5150: Macrovision off counter -REG:0x%x  = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_MACROVISION_OFF_CTR));
	sysprintf("tvp5150: ITU-R BT.656.%d timing(TVP5150AM1 only)\n",
					(I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_REV_SELECT)&1)?3:4);
					
	sysprintf("tvp5150: Device ID = %02 - %02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_MSB_DEV_ID),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_LSB_DEV_ID));		
					
	sysprintf("tvp5150: ROM version = (hex) %02x - %02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ROM_MAJOR_VER),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_ROM_MINOR_VER));
	sysprintf("tvp5150: Vertical line count = 0x%02x - %02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VERT_LN_COUNT_MSB),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VERT_LN_COUNT_LSB));
	sysprintf("tvp5150: Interrupt status register B = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_STATUS_REG_B));
	sysprintf("tvp5150: Interrupt active register B = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_ACTIVE_REG_B));
	sysprintf("tvp5150: Status regs #1 to #5 = 0x%02x - 0x%02x - 0x%02x - 0x%02x - 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_STATUS_REG_1),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_STATUS_REG_2),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_STATUS_REG_3),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_STATUS_REG_4),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_STATUS_REG_5));

	sysprintf("tvp5150: Teletext filter enable = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_TELETEXT_FIL_ENA));
	sysprintf("tvp5150: Interrupt status register A = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_STATUS_REG_A));
	sysprintf("tvp5150: Interrupt enable register A = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_ENABLE_REG_A));
	sysprintf("tvp5150: Interrupt configuration = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_INT_CONF));
	sysprintf("tvp5150: VDP status register = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_VDP_STATUS_REG));
	sysprintf("tvp5150: FIFO word count = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_FIFO_WORD_COUNT));
	sysprintf("tvp5150: FIFO interrupt threshold = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_FIFO_INT_THRESHOLD));
	sysprintf("tvp5150: FIFO reset = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_FIFO_RESET));
	sysprintf("tvp5150: Line number interrupt = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_LINE_NUMBER_INT));
	sysprintf("tvp5150: Pixel alignment register = 0x%02x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_PIX_ALIGN_REG_HIGH),
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_PIX_ALIGN_REG_LOW));
	sysprintf("tvp5150: FIFO output control = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_FIFO_OUT_CTRL));
	sysprintf("tvp5150: Full field enable = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_FULL_FIELD_ENA));
	sysprintf("tvp5150: Full field mode register = 0x%02x\n",
					I2C_Read_8bitSlaveAddr_8bitReg_8bitData(c, TVP5150_FULL_FIELD_MODE_REG));
}

VOID TVP5150_Init(UINT32 nIndex)
{
	UINT32 u32Idx;
	UINT32 u32TableSize;
	UINT8  u8DeviceID;
	UINT8 u8ID;
	UINT8 u8Retry=0;
	
	struct OV_RegValue *psRegValue;
	DBG_PRINTF("Sensor ID = %d\n", nIndex);
	if ( nIndex >= (sizeof(g_uOvDeviceID)/sizeof(UINT8)) )
		return;
	videoIn_Open(96000, 48000);								/* For sensor clock output */	

#ifdef __DEMO_BOARD__
	SnrPowerDown(FALSE);
#endif	
	SnrReset();											
	sysDelay(1);
		
	u32TableSize = g_OV_InitTable[nIndex].u32TableSize;
	psRegValue = g_OV_InitTable[nIndex].sRegTable;
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
							
	for(u32Idx=0;u32Idx<u32TableSize; u32Idx=u32Idx+1, psRegValue = psRegValue+1)
	{
		UINT8 u8Data;
		
		I2C_Write_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, (psRegValue->u8RegAddr), (psRegValue->u8Value));	
		u8Data = I2C_Read_8bitSlaveAddr_8bitReg_8bitData(u8DeviceID, psRegValue->u8RegAddr);
		if(u8Data != (psRegValue->u8Value)){	
			//sysprintf("Wrong I2C initial address 0x=%x\n", psRegValue->u8RegAddr);
			u8Retry = u8Retry+1;
			if((psRegValue->u8RegAddr == 0xC0))
				continue;
			else{			
				psRegValue = psRegValue-1;	
				u32Idx=u32Idx-1;		
			}
		}
		else
			sysprintf("I2C initial address 0x%x , data = %d, successful\n", psRegValue->u8RegAddr, psRegValue->u8Value);			
	}

#if 0	
#if defined(__PAL__)
	TVP5150SetVideoformat(TVP_STD_PAL);	
#elif defined(__NTSC__)
	TVP5150SetVideoformat(TVP_STD_NTSC);	
#else		// default PAL
	TVP5150SetVideoformat(TVP_STD_PAL);	
#endif	
#else
	TVP5150SetVideoformat(TVP_STD_ALL);
#endif		
	TVP5150SetInputSource(TVP5150_A1P1A);
	
	u8DeviceID = g_uOvDeviceID[nIndex];
	dump_reg(u8DeviceID);


	
	DrvI2C_Close();	
}


/*===================================================================
	LCD dimension = (OPT_LCD_WIDTH, OPT_LCD_HEIGHT)	
	Packet dimension = (OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT)	
	Stride should be LCM resolution  OPT_LCD_WIDTH.
	Packet frame start address = VPOST frame start address + (OPT_LCD_WIDTH-OPT_PREVIEW_WIDTH)/2*2 	
=====================================================================*/



#ifndef UDC
UINT32 Smpl_TVP5150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
#else
UINT32 Smpl_TVP5150_OneField(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1)
#endif
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;
#ifndef UDC
	PUINT8 pu8PacketBuf;
#endif
	UINT32 u32GCD;
#ifndef UDC
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer0 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer1 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer2 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);

	InitVPOST(pu8FrameBuffer0);	
#endif
#if 0
	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 
#endif	
#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	//videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_TVD_CCIR656);	
#endif	
	TVP5150_Init(0);			
	videoIn_Open(96000, 48000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,
				FALSE,							//Polarity.	
				TRUE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_VYUY, 			//Input Order 
				eVIDEOIN_IN_YUV422	,	//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
#ifdef __NTSC__ 		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				2,							//Vertical start position (Y)
				4,							//Horizontal start position	(X)
				0);							//Useless
#endif
#ifdef __PAL__ 	
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				2,							//Vertical start position (Y)
				4,							//Horizontal start position	(X)
				0);							//Useless
#endif

				
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
				
#ifndef UDC		
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
#endif				
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
#ifndef UDC	
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
#endif	
	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				1,	/* 0: Both fields are disabled. 1: Field 1 enable. 2: Field 2 enable. 3: Both fields are enable */
				eVIDEOIN_TYPE_CCIR656,	/* 0: CCIR601.	1: CCIR656 */
				0);		
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
#ifndef UDC
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipes was enable. 											
#else
                                eVIDEOIN_PACKET,	
#endif
				0 );							//Useless		
	
		
	sysSetLocalInterrupt(ENABLE_IRQ);						
															
	return Successful;			
}	


UINT32 Smpl_TVP5150_TwoFields(UINT8* pu8FrameBuffer0, UINT8* pu8FrameBuffer1, UINT8* pu8FrameBuffer2)
{
	PFN_VIDEOIN_CALLBACK pfnOldCallback;
#ifndef UDC	
	PUINT8 pu8PacketBuf;
#endif	
	UINT32 u32GCD;
	
#ifndef UDC	
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer0 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer1 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	pu8PacketBuf = (PUINT8)((UINT32)pu8FrameBuffer2 | 0x80000000);
	memset(pu8PacketBuf, 0x0, OPT_PREVIEW_WIDTH*OPT_PREVIEW_HEIGHT*2);
	
	InitVPOST(pu8FrameBuffer0);	
#endif	
#if 0	
	#ifdef __3RD_PORT__
		// GPIOD2 pull high
		gpio_setportval(GPIO_PORTD, 0x04, 0x04);    //GPIOD2 high to enable Amplifier 
		gpio_setportpull(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 pull high
		gpio_setportdir(GPIO_PORTD, 0x04, 0x04);	//GPIOD2 output mode
	#endif 
#endif	
#ifdef __1ST_PORT__	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_SNR_CCIR601);	
#endif
#ifdef __2ND_PORT__
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_2ND_SNR_CCIR601);	
#endif	
#ifdef __3RD_PORT__
	//videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_SNR_CCIR601);	
	videoIn_Init(TRUE, 0, 24000, eVIDEOIN_3RD_TVD_CCIR656);	
#endif	
	TVP5150_Init(0);			
	videoIn_Open(96000, 48000);		
	videoIn_EnableInt(eVIDEOIN_VINT);
	
	videoIn_InstallCallback(eVIDEOIN_VINT, 
						(PFN_VIDEOIN_CALLBACK)VideoIn_InterruptHandler,
						&pfnOldCallback	);	//Frame End interrupt
						
	videoIn_SetPacketFrameBufferControl(FALSE, FALSE);	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_POLARITY,
				FALSE,
				FALSE,							//Polarity.	
				TRUE);							
												
	videoinIoctl(VIDEOIN_IOCTL_ORDER_INFMT_OUTFMT,								
				eVIDEOIN_IN_VYUY, 			//Input Order 
				eVIDEOIN_IN_YUV422	,	//Intput format
				eVIDEOIN_OUT_YUV422);		//Output format for packet 														
#ifdef __NTSC__ //OK		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				4,							//Vertical start position (Y)
				2,							//Horizontal start position	(X)
				0);							//Useless
#endif
#ifdef __PAL__ //Need Verify		
	videoinIoctl(VIDEOIN_IOCTL_SET_CROPPING_START_POSITION,				
				4,							//Vertical start position (Y)
				2,							//Horizontal start position	(X)
				0);							//Useless
#endif

				
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
				
#ifndef UDC				
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
#endif				
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
#ifndef UDC	
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
#endif				
	
	videoinIoctl(VIDEOIN_IOCTL_SET_INPUT_TYPE,
				3,	/* 0: Both fields are disabled. 1: Field 1 enable. 2: Field 2 enable. 3: Both fields are enable */
				eVIDEOIN_TYPE_CCIR656,	/* 0: CCIR601.	1: CCIR656 */
				0);		
	
	
	videoinIoctl(VIDEOIN_IOCTL_SET_PIPE_ENABLE,
				TRUE, 						// Engine enable ?
#ifndef UDC
				eVIDEOIN_BOTH_PIPE_ENABLE,		// which pipes was enable. 											
#else
                                eVIDEOIN_PACKET,	
#endif
				0 );							//Useless		
	
		
	sysSetLocalInterrupt(ENABLE_IRQ);						
															
	return Successful;			
}	