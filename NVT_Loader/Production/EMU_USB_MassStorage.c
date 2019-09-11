#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_reg.h"
#include "usbd.h"
#include "mass_storage_class.h"
#include "nvtfat.h"
#include "w55fa93_sic.h"
#include "Font.h"
#include "emuProduct.h" 


static NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};

NDISK_T MassNDisk;
static UINT32 u32Timer0_10ms_Count=0;
static UINT32 u32Xpos;
static S_DEMO_FONT*	ptTmpFont;
BOOL usbdTimOut(VOID)
{//As time out return FALSE to close USBD
	return ((u32Timer0_10ms_Count>1000) ? FALSE:TRUE);
}

static void Timer0_Callback(void)
{
	u32Timer0_10ms_Count++;
	if((u32Timer0_10ms_Count%100)==0)
	{
		DemoFont_PaintA(ptTmpFont,					
						u32Xpos,					
						0,			
						"*");
		u32Xpos = u32Xpos +	_FONT_OFFSET_;
		if(u32Timer0_10ms_Count>1000)
		{//10 sec will auto disconnect
			sysprintf("USBD deattached\n");
			udcDeinit();			
		}				
	}	
}
BOOL test(VOID)
{
	if(u32Timer0_10ms_Count > 1000)
		return 0;
	else
		return udcIsAttached();
}

INT32 USB_MassStotage(void)
{
#if 0	
    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);  /* clock from PLL */
    sicOpen();
    udcOpen();
#endif     

	/* In here for USB VBus stable. Othwise, USB library can not judge VBus correct  */ 
	udcOpen();	
	sysprintf("UDC open\n");
	sicOpen();
	sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);	
	/* Initialize GNAND */
	if(GNAND_InitNAND(&_nandDiskDriver0, &MassNDisk, TRUE) < 0) 
	{
		sysprintf("GNAND_InitNAND error\n");
		GNAND_UnMountNandDisk(&MassNDisk);
		return ERR_USBD_GNAND_FAIL;
	}	
	
	if(GNAND_MountNandDisk(&MassNDisk) < 0) 
	{
		sysprintf("GNAND_MountNandDisk error\n");
		GNAND_UnMountNandDisk(&MassNDisk);
		return ERR_USBD_GNAND_FAIL;		
	}

	mscdInit();		
	mscdFlashInit((NDISK_T *)&MassNDisk, FMI_NO_SD_CARD);	
	udcInit();
	
	mscdMassEvent(test);//udcIsAttached);	

	mscdDeinit();	
	udcDeinit();
	udcClose();
	
	GNAND_UnMountNandDisk(&MassNDisk);
        fmiSMClose(0);			
	sicClose();
	sysprintf("Sample code End\n");	
}


BOOL HostDetection(void)
{  
	UINT32 volatile u32Delay = 0x100000;
	BOOL bIsUsbDet;	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | USBD_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | UDCRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~UDCRST);	
	outp32(PHY_CTL, (0x20 | Phy_suspend));
	
	outp32(OPER, 0x0);		
	while(inp32(OPER) != 0x0);	
	while(u32Delay--);
	if(inp32(PHY_CTL) & Vbus_status) 
		bIsUsbDet = TRUE;
	else
		bIsUsbDet = FALSE;	
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~USBD_CKE);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) | UDCRST);
	outpw(REG_AHBIPRST, inpw(REG_AHBIPRST) & ~UDCRST);		
	return bIsUsbDet;
}

UINT32 u3210msTick;
void USBDetTimeOut(void)
{
	u3210msTick = u3210msTick +1;
}
INT EMU_USBD(
	S_DEMO_FONT*	ptFont,
	UINT32 			u32FrameBufAddr	
)
{
	//UINT32 	u32BlockSize, u32FreeSize, u32DiskSize;
	UINT32 	u32Ypos=0, u32ExtFreq; 
	INT	   	tmp;
	char 	Array1[64];
	S_DEMO_RECT s_sDemo_Rect;
	INT32 ErrCode;
	
	
	u32Timer0_10ms_Count = 0;
	ptTmpFont = ptFont;
	u3210msTick = 0;
	u32ExtFreq = sysGetExternalClock()*1000;	
	sysSetTimerReferenceClock(TIMER0, u32ExtFreq); 		//External Crystal
	sysSetTimerReferenceClock(TIMER1, u32ExtFreq); 		//External Crystal
	sysSetLocalInterrupt(DISABLE_IRQ);
	
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);          /* 100 ticks/per sec ==> 1tick/10ms */
	tmp = sysSetTimerEvent(TIMER0, 1, (PVOID)Timer0_Callback);	/* 1 ticks = 0.01s call back */

	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	Draw_ItemBorder(ptFont);	
	sprintf(Array1, "[USBD]");
	u32Xpos = (strlen("[USBD]")+1)*_FONT_OFFSET_;

	DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);		
						
	u32Ypos = u32Ypos + _FONT_RECT_HEIGHT_;									
	sprintf(Array1, "Plug in USB Cable");						
	DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);	
	while(1)
	{
		if(HostDetection()==TRUE)
			break;
		if(u32Timer0_10ms_Count>=1000)
			break;			
	}
	u3210msTick = 0;			/* Delay 1S */
	while(u3210msTick==100);	
	if(HostDetection()==TRUE)	
	{	
		s_sDemo_Rect.u32StartX = 4;		/* Avoid border */
		s_sDemo_Rect.u32StartY = u32Ypos;
		s_sDemo_Rect.u32EndX = s_sDemo_Rect.u32StartX + 
									25*_FONT_RECT_WIDTH_;	/* Clear 25 character "Plug in USB Cable" */
		s_sDemo_Rect.u32EndY =u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_RectClear(ptFont,
							&s_sDemo_Rect);	
		sprintf(Array1, "USB Plug in");
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
							0,					//UINT32		u32x,
							u32Ypos,			//UINT32		u32y,
							Array1);								
	}	
	else
	{
		s_sDemo_Rect.u32StartX = 4;		/* Avoid border */
		s_sDemo_Rect.u32StartY = u32Ypos;
		s_sDemo_Rect.u32EndX = s_sDemo_Rect.u32StartX + 
									25*_FONT_RECT_WIDTH_;	/* Clear 25 character "Plug in USB Cable" */
		s_sDemo_Rect.u32EndY =u32Ypos + _FONT_RECT_HEIGHT_;
		DemoFont_RectClear(ptFont,
							&s_sDemo_Rect);	
							
		sprintf(Array1, "Time Out");	
		DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);											
		return ERR_USBD_TIME_OUT;							
	}	
	ErrCode = USB_MassStotage();
	if(ErrCode<0)
		return ErrCode;
	sysClearTimerEvent(TIMER0, tmp);
	Font_ClrFrameBuffer(u32FrameBufAddr);
	Draw_InitialBorder(ptFont);	
	sprintf(Array1, "USB Plug out");
	DemoFont_PaintA(ptFont,					//S_DEMO_FONT* 	ptFont,
						0,					//UINT32		u32x,
						u32Ypos,			//UINT32		u32y,
						Array1);
	return Successful;						
}
						