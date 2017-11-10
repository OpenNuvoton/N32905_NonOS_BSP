/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                      */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                             */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
/* File Contents:                                                                    */
/*   i2c_24LC64.c                                                                    */
/*                                                                                   */
/* This file contains:                                                               */
/*                                                                                   */
/* Project:                                                                          */
/*                                                                                   */
/* Description:                                                                      */
/*   This file is a sample program used to access EEPROM (24LC64) on W90P910         */
/*   EV Board.                                                                       */
/*                                                                                   */
/* Remark:                                                                           */
/*   1. Execute this program on 910 EV board that should set slave addr 0x50 for     */
/*      I2C0 and set slave addr 0x51 for I2C1. It's H/W disign issue.                */
/*   2. Microchip 24LC64 is EEPROM, 64K bits size, page-write buffer for up to       */
/*      32bytes.                                                                     */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/ 
 
#ifdef ECOS 
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "kapi.h"
#include "diag.h"
#include "wbio.h"
#else 
#include <stdio.h>
#include <string.h>
#include "wblib.h"
#endif

#include "w55fa93_i2c.h"


/* If test I2C on module board, should configure switch of GPIOH pins. */
//#define MODULE_BOARD

#define RETRY		1000  /* Programming cycle may be in progress. Please refer to 24LC64 datasheet */

#define TXSIZE		512
#define ADDROFFSET  1024

#ifdef MODULE_BOARD
#define REG_GPIOH_DIR		0xB8003054
#define REG_GPIOH_DATAOUT	0xB8003058
#endif

#ifdef ECOS
#define i2cprintf		diag_printf
#define i2cgetchar		diag_getc
#else
#define i2cprintf		sysprintf
#define i2cgetchar		sysGetChar
#endif 

#ifdef ECOS
static cyg_handle_t  	thread_handle;
static cyg_thread 		thread;
#define STACKSIZE		(128*1024)
static UINT8            _Statck[STACKSIZE];
#endif

//------------------------- Program -------------------------//
#ifdef ECOS
void i2cExample (cyg_addrword_t tdata)
#else
void i2cExample (VOID)
#endif
{
	unsigned char data[TXSIZE], value[TXSIZE];
	int i, j, err, cnt;
	INT32 rtval;
	
#ifdef MODULE_BOARD	
	/* control GPIOH[7:4], 1 0 1 0 */
	outpw(REG_MFSEL, inpw(REG_MFSEL) & ~(1 << 25));
	outpw(REG_GPIOH_DIR, inpw(REG_GPIOH_DIR) | 0xF0);
	outpw(REG_GPIOH_DATAOUT, ( inpw(REG_GPIOH_DATAOUT) & (~0xF0) ) | 0xA0 );
#endif	

	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
						192000,		//UINT32 u32PllKHz, 	
						192000,		//UINT32 u32SysKHz,
						192000,		//UINT32 u32CpuKHz,
						  96000,		//UINT32 u32HclkKHz,
						  48000);		//UINT32 u32ApbKHz	
	
	/* initialize test data */
	for(i = 0 ; i < TXSIZE ; i++)
		data[i] = i + 1;
	
	i2cInit();	
	
	/* Byte Write/Random Read */
	i2cprintf("\nI2C Byte Write/Random Read ...\n");
	
	rtval = i2cOpen();
	if(rtval < 0)
	{
		i2cprintf("Open I2C error!\n");
		goto exit_test;
	}	
	i2cIoctl(I2C_IOC_SET_DEV_ADDRESS, 0x50, 0);  
	i2cIoctl(I2C_IOC_SET_SPEED, 100, 0);
	
	/* Tx porcess */
	i2cprintf("Start Tx\n");
	for(i = 0 ; i < TXSIZE ; i++)
	{
		i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, i, 2);	
		j = RETRY;	
		while(j-- > 0) 
		{
			if(i2cWrite(&data[i], 1) == 1)
				break;
		}						
		if(j <= 0)
			i2cprintf("WRITE ERROR [%d]!\n", i);
		sysDelay(1);
	}
	
	/* Rx porcess */	
	i2cprintf("Start Rx\n");
	memset(value, 0 , TXSIZE);
	for(i = 0 ; i < TXSIZE ; i++)
	{
		i2cIoctl(I2C_IOC_SET_SUB_ADDRESS, i, 2);	
		j = RETRY;
		while(j-- > 0) 
		{
			if(i2cRead(&value[i], 1) == 1)
				break;
		}
		if(j <= 0)
			i2cprintf("Read ERROR [%d]!\n", i);
	}
	
	/* Compare process */
	i2cprintf("Compare data\n");
	err = 0;
	cnt = 0;
	for(i = 0 ; i < TXSIZE ; i++)
	{
		if(value[i] != data[i])	
		{
			i2cprintf("[%d] addr = 0x%08x, val = 0x%02x (should be 0x%02x)\n", i, i + ADDROFFSET, value[i], data[i]);
			err = 1;
			cnt++;			
		}
	}					
	if(err)
		i2cprintf("Test failed [%d]!\n\n", cnt);	
	else
		i2cprintf("Test success\n\n");	
	
	i2cClose();		
	/* Test End */
	
exit_test:
	
	i2cprintf("\nTest finish ...\n");	
	i2cExit();
	
	return;
}	

int main (VOID)
{
#ifdef ECOS
	cyg_thread_create(21, i2cExample, 0, "i2cEEPROM", _Statck, STACKSIZE, &thread_handle, &thread);
	cyg_thread_resume(thread_handle);
#else
	i2cExample();
#endif	
	
	return 0;
}
