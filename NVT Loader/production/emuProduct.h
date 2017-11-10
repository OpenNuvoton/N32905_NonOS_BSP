/*************************************************************************
 * Nuvoton Electronics Corporation confidential
 *
 * Copyright (c) 2008 by Nuvoton Electronics Corporation
 * All rights reserved
 * 
 * FILENAME
 *     nvtfat.h
 *
 * REMARK
 *     None
 **************************************************************************/ 
#ifndef _EMU_PRODUCT_
#define _EMU_PRODUCT_
#include "wblib.h"
#include "Font.h"


#define _EMU_ADC_
#define _EMU_DDR_
#define _EMU_USBD_
#define _EMU_KPI_
#define _EMU_SPU_
#define _EMU_VPOST_
#define _EMU_NandFlash_
#define _EMU_SDCard_
//#define _EMU_USBH_

#define LEFT_KEY		0x40
#define UP_KEY			0x01
#define RIGHT_KEY		0x08
#define DOWN_KEY		0x02	
#define HOME_KEY		0x10
#define ENTER_KEY		0x80
#define VOLP_KEY		0x04
#define VOLM_KEY		0x20


INT32 EMU_InitSdCard(void);
void EMU_InitVPost(PUINT16 fb);
void EMU_InitFont(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
void Timer0_DelayTick(UINT32 u32Tick);

INT EMU_SPU(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_NandFlash(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);	
INT EMU_Memory(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);	
INT EMU_AVI(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_VPOST(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_SDCard(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_USBH(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_AIC(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_InitTimer0(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_InitTimer1(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);
INT EMU_USBD(S_DEMO_FONT* ptFont,UINT32 u32FrameBufAddr);
INT EMU_ADC(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);	
INT EMU_KPI(S_DEMO_FONT* ptFont, UINT32 u32FrameBufAddr);	

void Draw_InitialBorder(S_DEMO_FONT* ptFont);
void Draw_ItemBorder(S_DEMO_FONT* ptFont);

void Timer0_DelayTick(UINT32 u32Tick);

#define ERR_OUT_MEMORY			0xFFF00000	/* out of memory */
#define ERR_UNKNOW_DISK			0xFFF00001	/* unknown disk type */
#define ERR_GNAND_INIT_FAIL		0xFFF00002	/* GNAND init fail */
#define ERR_GNAND_MOUNT_FAIL	0xFFF00003	/* GNAND mount fail */
#define ERR_NAND_INIT_FAIL		0xFFF00004	/* NAND init fail */	
#define ERR_NAND_INFO_FAIL		0xFFF00005	/* NAND get information fail */
#define ERR_NAND_ACCESS_FAIL	0xFFF00006	/* NAND access fail */
#define ERR_AVI_NOT_FOUND		0xFFF00007	/* AVI file not found */
#define ERR_SDCARD_ACCESS_FAIL	0xFFF00008	/* SD card access fail */
#define ERR_TIMER0_FAIL			0xFFF00009	/* Timer 0 fail */
#define ERR_TIMER1_FAIL			0xFFF0000A	/* Timer 1 fail */
#define ERR_SDRAM_FAIL			0xFFF0000B	/* SDRAM Access fail */
#define ERR_ADC_TIME_OUT		0xFFF0000C	/* ADC test time out */ // Over 10 sec not reach the escape area */
#define ERR_KPI_TIME_OUT		0xFFF0000D	/* KPI test time out */ // Over 10 sec not any key pressing */
#define ERR_USBD_TIME_OUT		0xFFF0000E	/* USB device test time out.  Over 10 sec not any attached */
#define ERR_USBH_TIME_OUT		0xFFF0000F	/* USB device test time out . Over 10 sec not any attached */
#define ERR_USBD_GNAND_FAIL	0xFFF00010	/* USB device test GNAND Fail.  */
#endif	/* __EMU_PRODUCT__¡@*/