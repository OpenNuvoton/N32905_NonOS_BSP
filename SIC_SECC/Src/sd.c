/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                       */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                              */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
#ifdef ECOS
#include "drv_api.h"
#include "diag.h"
#include "wbtypes.h"
#include "wbio.h"
#else
#include "wblib.h"
#endif

//#include "nuc930_reg.h"
//#include "nuc930_sic.h"
#include "w55fa93_reg.h"
#include "w55fa93_sic.h"

#include "fmi.h"
#include "nvtfat.h"

#define SD_BLOCK_SIZE	512

#define FMI_SD_INITCOUNT 2000
#define FMI_TICKCOUNT	1000

#define FMI_TYPE_UNKNOWN	0
#define FMI_TYPE_SD_HIGH	1
#define FMI_TYPE_SD_LOW		2
#define FMI_TYPE_MMC		3

// global variables
UINT32 _fmi_uR3_CMD=0;
UINT32 _fmi_uR7_CMD=0;

__align(4096) UCHAR _fmi_ucSDHCBuffer[64];
UINT8 *_fmi_pSDHCBuffer;


void fmiCheckRB()
{
	while(1)
	{
		outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);
		while(inpw(REG_SDCR) & SDCR_8CLK_OE);
		if (inpw(REG_SDISR) & SDISR_SD_DATA0)
			break;
	}
}

INT fmiSDCommand(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg)
{
	outpw(REG_SDARG, uArg);
	
	outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN));
	
	while(inpw(REG_SDCR) & SDCR_CO_EN)
	{

		if (pSD->bIsCardInsert == FALSE)
			return FMI_NO_SD_CARD;
	}

	return Successful;
}


INT fmiSDCmdAndRsp(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg, INT ntickCount)
{
//	unsigned int reg;

	outpw(REG_SDARG, uArg);

	outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN | SDCR_RI_EN));

	if (ntickCount > 0)
	{

		while(inpw(REG_SDCR) & SDCR_RI_EN)
		{
            if(ntickCount-- == 0) {
             	outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_SWRST);	// reset SD engine
				return 2;   
            }
			if (pSD->bIsCardInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}
	else
	{

		while(inpw(REG_SDCR) & SDCR_RI_EN)
		{

			if (pSD->bIsCardInsert == FALSE)
				return FMI_NO_SD_CARD;
		}
	}

	if (_fmi_uR7_CMD)
	{
		if (((inpw(REG_SDRSP1) & 0xff) != 0x55) && ((inpw(REG_SDRSP0) & 0xf) != 0x01))
		{
			_fmi_uR7_CMD = 0;
			return FMI_SD_CMD8_ERROR;
		}
	}

	if (!_fmi_uR3_CMD)
	{
		if (inpw(REG_SDISR) & SDISR_CRC_7)		// check CRC7
			return Successful;
		else
		{
#ifdef DEBUG
			sysprintf("response error [%d]!\n", ucCmd);
#endif
			return FMI_SD_CRC7_ERROR;
		}
	}
	else
	{
		_fmi_uR3_CMD = 0;
		outpw(REG_SDISR, SDISR_CRC_IF);
		return Successful;
	}
}


// Get 16 bytes CID or CSD
INT fmiSDCmdAndRsp2(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg, UINT *puR2ptr)
{
	unsigned int i;
	unsigned int tmpBuf[5];
//	unsigned int reg;

	outpw(REG_SDARG, uArg);

	outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN | SDCR_R2_EN));

	while(inpw(REG_SDCR) & SDCR_R2_EN)
	{

		if (pSD->bIsCardInsert == FALSE)
			return FMI_NO_SD_CARD;
	}

	if (inpw(REG_SDISR) & SDISR_CRC_7)
	{
		for (i=0; i<5; i++)
			tmpBuf[i] = Swap32(inpw(REG_FB_0+i*4));

		for (i=0; i<4; i++)
			*puR2ptr++ = ((tmpBuf[i] & 0x00ffffff)<<8) | ((tmpBuf[i+1] & 0xff000000)>>24);
		return Successful;
	}
	else
		return FMI_SD_CRC7_ERROR;
}


INT fmiSDCmdAndRspDataIn(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg)
{
//	unsigned int reg;

	outpw(REG_SDARG, uArg);

	outpw(REG_SDCR, (inpw(REG_SDCR)&(~SDCR_CMD_CODE))|(ucCmd << 8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DI_EN));

	while (inpw(REG_SDCR) & SDCR_RI_EN)
	{

		if (pSD->bIsCardInsert == FALSE)
			return FMI_NO_SD_CARD;
	}

	while (inpw(REG_SDCR) & SDCR_DI_EN)
	{

		if (pSD->bIsCardInsert == FALSE)
			return FMI_NO_SD_CARD;
	}

	if (!(inpw(REG_SDISR) & SDISR_CRC_7))		// check CRC7
	{
#ifdef DEBUG
		sysprintf("fmiSDCmdAndRspDataIn: response error [%d]!\n", ucCmd);
#endif
		return FMI_SD_CRC7_ERROR;
	}

	if (!(inpw(REG_SDISR) & SDISR_CRC_16))		// check CRC16
	{
#ifdef DEBUG
		sysprintf("fmiSDCmdAndRspDataIn: read data error!\n");
#endif
		return FMI_SD_CRC16_ERROR;
	}
	return Successful;
}

// Initial
INT fmiSD_Init(FMI_SD_INFO_T *pSD)
{
	int volatile i, status, rate;
	unsigned int resp;
	unsigned int CIDBuffer[4];
	unsigned int volatile u32CmdTimeOut;

#if 1
	// set the clock to 200KHz
	/* divider */
	rate = _fmi_uFMIReferenceClock / 200;
	if ((_fmi_uFMIReferenceClock % 200) == 0)
		rate = rate - 1;
#else
	// set the clock to 400KHz
	/* divider */
	rate = _fmi_uFMIReferenceClock / 400;
	if ((_fmi_uFMIReferenceClock % 400) == 0)
		rate = rate - 1;
#endif		

	for(i=0; i<100; i++);
//	outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x02 << 19)); 	// SD clock from APLL
	outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x03 << 19)); 	// SD clock from UPLL
	
	outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N0) | (0x07 << 16)); 	// SD clock divided by 8
	rate /= 8;
	rate &= 0xFF;
	if (rate) rate--;
	outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N1) | (rate << 24)); 	// SD clock divider		

	for(i=0; i<1000; i++);

	// power ON 74 clock
	outpw(REG_SDCR, inpw(REG_SDCR) | SDCR_74CLK_OE);	

	while(inpw(REG_SDCR) & SDCR_74CLK_OE)
	{
		if (pSD->bIsCardInsert == FALSE)
			return FMI_NO_SD_CARD;
	}

	fmiSDCommand(pSD, 0, 0);		// reset all cards
	for (i=0x100; i>0; i--);

	// initial SDHC
	_fmi_uR7_CMD = 1;
	u32CmdTimeOut = 5000;
	
	i = fmiSDCmdAndRsp(pSD, 8, 0x00000155, u32CmdTimeOut);
	if (i == Successful)
	{
		// SD 2.0
		fmiSDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
		_fmi_uR3_CMD = 1;
		fmiSDCmdAndRsp(pSD, 41, 0x40ff8000, u32CmdTimeOut);	// 2.7v-3.6v
		resp = inpw(REG_SDRSP0);

		while (!(resp & 0x00800000))		// check if card is ready
		{
			fmiSDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
			_fmi_uR3_CMD = 1;
			fmiSDCmdAndRsp(pSD, 41, 0x40ff8000, u32CmdTimeOut);	// 3.0v-3.4v
			resp = inpw(REG_SDRSP0);

		}
		if (resp & 0x00400000)
			pSD->uCardType = FMI_TYPE_SD_HIGH;
		else
			pSD->uCardType = FMI_TYPE_SD_LOW;
	}
	else
	{
		// SD 1.1
		
		fmiSDCommand(pSD, 0, 0);		// reset all cards		
		for (i=0x100; i>0; i--);		
		
		i = fmiSDCmdAndRsp(pSD, 55, 0x00, u32CmdTimeOut);
		if (i == 2)		// MMC memory
		{
			fmiSDCommand(pSD, 0, 0);		// reset
			for (i=0x100; i>0; i--);

			_fmi_uR3_CMD = 1;
			if (fmiSDCmdAndRsp(pSD, 1, 0x80ff8000, u32CmdTimeOut) != 2)	// MMC memory
			{
				resp = inpw(REG_SDRSP0);

				while (!(resp & 0x00800000))		// check if card is ready
				{
					_fmi_uR3_CMD = 1;
					fmiSDCmdAndRsp(pSD, 1, 0x80ff8000, u32CmdTimeOut);		// high voltage
					resp = inpw(REG_SDRSP0);

				}
				pSD->uCardType = FMI_TYPE_MMC;
			}
			else
			{
				pSD->uCardType = FMI_TYPE_UNKNOWN;
				return FMI_ERR_DEVICE;
			}
		}
		else if (i == 0)	// SD Memory
		{
			_fmi_uR3_CMD = 1;
			fmiSDCmdAndRsp(pSD, 41, 0x00ff8000, u32CmdTimeOut);	// 3.0v-3.4v
			resp = inpw(REG_SDRSP0);
			while (!(resp & 0x00800000))		// check if card is ready
			{
				fmiSDCmdAndRsp(pSD, 55, 0x00,u32CmdTimeOut);
				_fmi_uR3_CMD = 1;
				fmiSDCmdAndRsp(pSD, 41, 0x00ff8000, u32CmdTimeOut);	// 3.0v-3.4v
				resp = inpw(REG_SDRSP0);

			}
			pSD->uCardType = FMI_TYPE_SD_LOW;
		}
		else
		{
			pSD->uCardType = FMI_TYPE_UNKNOWN;
#ifdef DEBUG
			sysprintf("CMD55 CRC error !!\n");
#endif
			return FMI_SD_INIT_ERROR;
		}
	}

	// CMD2, CMD3
	if (pSD->uCardType != FMI_TYPE_UNKNOWN)
	{
		fmiSDCmdAndRsp2(pSD, 2, 0x00, CIDBuffer);
		if (pSD->uCardType == FMI_TYPE_MMC)
		{
			if ((status = fmiSDCmdAndRsp(pSD, 3, 0x10000, 0)) != Successful)		// set RCA
				return status;
			pSD->uRCA = 0x10000;
		}
		else
		{
			if ((status = fmiSDCmdAndRsp(pSD, 3, 0x00, 0)) != Successful)		// get RCA
				return status;
			else
				pSD->uRCA = (inpw(REG_SDRSP0) << 8) & 0xffff0000;
		}
	}

#ifdef DEBUG
	if (pSD->uCardType == FMI_TYPE_SD_HIGH)
		sysprintf("This is high capacity SD memory card\n");
	if (pSD->uCardType == FMI_TYPE_SD_LOW)
		sysprintf("This is standard capacity SD memory card\n");
	if (pSD->uCardType == FMI_TYPE_MMC)
		sysprintf("This is MMC memory card\n");
#endif

	// set data transfer clock

	return Successful;
}

INT fmiSwitchToHighSpeed(FMI_SD_INFO_T *pSD)
{
	int volatile status=0;
	UINT16 current_comsumption, fun1_info, switch_status, busy_status0, busy_status1;

	outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);	// set DMA transfer starting address
	outpw(REG_SDBLEN, 63);	// 512 bit

	if ((status = fmiSDCmdAndRspDataIn(pSD, 6, 0x00ffff01)) != Successful)
		return Fail;

	current_comsumption = _fmi_pSDHCBuffer[0]<<8 | _fmi_pSDHCBuffer[1];
	if (!current_comsumption)
		return Fail;

	fun1_info =  _fmi_pSDHCBuffer[12]<<8 | _fmi_pSDHCBuffer[13];

	switch_status =  _fmi_pSDHCBuffer[16] & 0xf;

	busy_status0 = _fmi_pSDHCBuffer[28]<<8 | _fmi_pSDHCBuffer[29];

	if (!busy_status0)	// function ready
	{
		outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);	// set DMA transfer starting address
		outpw(REG_SDBLEN, 63);	// 512 bit

		if ((status = fmiSDCmdAndRspDataIn(pSD, 6, 0x80ffff01)) != Successful)
			return Fail;

		// function change timing: 8 clocks
		outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_8CLK_OE);
		while(inpw(REG_SDCR) & SDCR_8CLK_OE);

		current_comsumption = _fmi_pSDHCBuffer[0]<<8 | _fmi_pSDHCBuffer[1];
		if (!current_comsumption)
			return Fail;

		busy_status1 = _fmi_pSDHCBuffer[28]<<8 | _fmi_pSDHCBuffer[29];
#ifdef DEBUG
		if (!busy_status1)
			sysprintf("switch into high speed mode !!!\n");
#endif
		return Successful;
	}
	else
		return Fail;
}


INT fmiSelectCard(FMI_SD_INFO_T *pSD)
{
	int volatile status=0, i;

	if ((status = fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0)) != Successful)
		return status;
		
	fmiCheckRB();

	// if SD card set 4bit
	if (pSD->uCardType == FMI_TYPE_SD_HIGH)
	{
		_fmi_pSDHCBuffer = (UINT8 *)((UINT32)_fmi_ucSDHCBuffer | 0x80000000);
		outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);	// set DMA transfer starting address
		outpw(REG_SDBLEN, 7);	// 64 bit

		if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
			return status;
		if ((status = fmiSDCmdAndRspDataIn(pSD, 51, 0x00)) != Successful)
			return status;

// #define SDHC_FREQ	50000
//#define SDHC_FREQ	48000

	{
		int rate;
		if ((_fmi_ucSDHCBuffer[0] & 0xf) == 0x2)
		{
			status = fmiSwitchToHighSpeed(pSD);
			if (status == Successful)
			{
				/* divider */
				rate = _fmi_uFMIReferenceClock / SDHC_FREQ;
		//		if ((_fmi_uFMIReferenceClock % SDHC_FREQ) == 0)
		//			rate = rate - 1;
				if ((_fmi_uFMIReferenceClock % SDHC_FREQ) > 0)
					rate ++;
		

				for(i=0; i<100; i++);
				
		//		outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x02 << 19)); 	// SD clock from APLL
				outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_S) | (0x03 << 19)); 	// SD clock from UPLL
				
				outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N0) | (0x01 << 16)); 	// SD clock divided by 8

				if (rate % 2)
				{
					rate /= 2;								
					rate &= 0xFF;
				}					
				else
				{
					rate /= 2;				
					rate &= 0xFF;					
					rate--;					
				}
				
				outpw(REG_CLKDIV2, (inpw(REG_CLKDIV2) & ~SD_N1) | (rate << 24)); 	// SD clock divider		
										
				for(i=0; i<1000; i++);
			}
		}
	}

		if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
			return status;
		if ((status = fmiSDCmdAndRsp(pSD, 6, 0x02, 0)) != Successful)	// set bus width
			return status;

		outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_DBW);
	}
	else if (pSD->uCardType == FMI_TYPE_SD_LOW)
	{
#if 0		
		_fmi_pSDHCBuffer = (UINT8 *)((UINT32)_fmi_ucSDHCBuffer | 0x80000000);
		outpw(REG_DMACSAR, (UINT32)_fmi_pSDHCBuffer);	// set DMA transfer starting address
		outpw(REG_SDBLEN, 7);	// 64 bit

		if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
			return status;
		if ((status = fmiSDCmdAndRspDataIn(pSD, 51, 0x00)) != Successful)
			return status;
#endif
		if ((status = fmiSDCmdAndRsp(pSD, 55, pSD->uRCA, 0)) != Successful)
			return status;
		if ((status = fmiSDCmdAndRsp(pSD, 6, 0x02, 0)) != Successful)	// set bus width
			return status;

		outpw(REG_SDCR, inpw(REG_SDCR)|SDCR_DBW);
	}
	else if (pSD->uCardType == FMI_TYPE_MMC)
	{
		outpw(REG_SDCR, inpw(REG_SDCR) & ~SDCR_DBW);
	}

	if ((status = fmiSDCmdAndRsp(pSD, 16, SD_BLOCK_SIZE, 0)) != Successful)	// set block length
		return status;

	fmiSDCommand(pSD, 7, 0);

#ifdef _SIC_USE_INT_
	outpw(REG_SDIER, inpw(REG_SDIER)|SDIER_BLKD_IEN);
#endif	//_SIC_USE_INT_

	return Successful;
}


INT fmiSD_Read_in(FMI_SD_INFO_T *pSD, UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr)
{
	BOOL volatile bIsSendCmd=FALSE;
	unsigned int volatile reg;
	int volatile i, loop, status;

	if ((status = fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0)) != Successful)
		return status;
	fmiCheckRB();

//	outpw(REG_SDBLEN, 0x1ff);	// 512 bytes
	outpw(REG_SDBLEN, SD_BLOCK_SIZE - 1);

	if (pSD->uCardType == FMI_TYPE_SD_HIGH)
		outpw(REG_SDARG, uSector);
	else
		outpw(REG_SDARG, uSector * SD_BLOCK_SIZE);

	outpw(REG_DMACSAR, uDAddr);

	loop = uBufcnt / 255;
	for (i=0; i<loop; i++)
	{
#ifdef _SIC_USE_INT_
		_fmi_bIsSDDataReady = FALSE;
#endif	//_SIC_USE_INT_

		reg = inpw(REG_SDCR) & ~SDCR_CMD_CODE;
		reg = reg | 0xff0000;
		if (bIsSendCmd == FALSE)
		{
			outpw(REG_SDCR, reg|(18<<8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DI_EN));
			bIsSendCmd = TRUE;
		}
		else
			outpw(REG_SDCR, reg | SDCR_DI_EN);

#ifdef _SIC_USE_INT_
		while(!_fmi_bIsSDDataReady)
#else
		while(1)
#endif	//_SIC_USE_INT_
		{
#ifndef _SIC_USE_INT_
			if ((inpw(REG_SDISR) & SDISR_BLKD_IF) && (!(inpw(REG_SDCR) & SDCR_DI_EN)))
			{
				outpw(REG_SDISR, SDISR_BLKD_IF);
				break;
			}
#endif


			if (pSD->bIsCardInsert == FALSE)
				return FMI_NO_SD_CARD;
		}

		if (!(inpw(REG_SDISR) & SDISR_CRC_7))		// check CRC7
		{
#ifdef DEBUG
			sysprintf("fmiSD_Read: response error!\n");
#endif
			return FMI_SD_CRC7_ERROR;
		}

		if (!(inpw(REG_SDISR) & SDISR_CRC_16))		// check CRC16
		{
#ifdef DEBUG
			sysprintf("fmiSD_Read_in:read data error!\n");
#endif
			return FMI_SD_CRC16_ERROR;
		}
	}

	loop = uBufcnt % 255;
	if (loop != 0)
	{
#ifdef _SIC_USE_INT_
		_fmi_bIsSDDataReady = FALSE;
#endif	//_SIC_USE_INT_

		reg = inpw(REG_SDCR) & (~SDCR_CMD_CODE);
		reg = reg & (~SDCR_BLKCNT);
		reg |= (loop << 16);
		
		if (bIsSendCmd == FALSE)
		{
			outpw(REG_SDCR, reg|(18<<8)|(SDCR_CO_EN | SDCR_RI_EN | SDCR_DI_EN));
			bIsSendCmd = TRUE;
		}
		else
			outpw(REG_SDCR, reg | SDCR_DI_EN);

#ifdef _SIC_USE_INT_
		while(!_fmi_bIsSDDataReady)
#else
		while(1)
#endif	//_SIC_USE_INT_
		{
#ifndef _SIC_USE_INT_
			if ((inpw(REG_SDISR) & SDISR_BLKD_IF) && (!(inpw(REG_SDCR) & SDCR_DI_EN)))
			{
				outpw(REG_SDISR, SDISR_BLKD_IF);
				break;
			}
#endif


			if (pSD->bIsCardInsert == FALSE)
				return FMI_NO_SD_CARD;
		}

		if (!(inpw(REG_SDISR) & SDISR_CRC_7))		// check CRC7
		{
#ifdef DEBUG
			sysprintf("fmiSD_Read: response error!\n");
#endif
			return FMI_SD_CRC7_ERROR;
		}

		if (!(inpw(REG_SDISR) & SDISR_CRC_16))		// check CRC16
		{
#ifdef DEBUG
			sysprintf("fmiSD_Read_in:read data error!\n");
#endif
			return FMI_SD_CRC16_ERROR;
		}
	}

	if (fmiSDCmdAndRsp(pSD, 12, 0, 0))		// stop command
	{
#ifdef DEBUG
		sysprintf("stop command fail !!\n");
#endif
		return FMI_SD_CRC7_ERROR;
	}
	fmiCheckRB();

	fmiSDCommand(pSD, 7, 0);

	return Successful;
}




INT fmiSD_Write_in(FMI_SD_INFO_T *pSD, UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr)
{
	BOOL volatile bIsSendCmd=FALSE;
	unsigned int volatile reg;
	int volatile i, loop, status;

	if ((status = fmiSDCmdAndRsp(pSD, 7, pSD->uRCA, 0)) != Successful)
		return status;
	fmiCheckRB();

//	outpw(REG_SDBLEN, 0x1ff);	// 512 bytes
	outpw(REG_SDBLEN, SD_BLOCK_SIZE - 1);

	if (pSD->uCardType == FMI_TYPE_SD_HIGH)
		outpw(REG_SDARG, uSector);
	else
		outpw(REG_SDARG, uSector * SD_BLOCK_SIZE);

	outpw(REG_DMACSAR, uSAddr);
	loop = uBufcnt / 255;
	for (i=0; i<loop; i++)
	{
#ifdef _SIC_USE_INT_
		_fmi_bIsSDDataReady = FALSE;
#endif	//_SIC_USE_INT_

		reg = inpw(REG_SDCR) & 0xff00c080;
		reg = reg | 0xff0000;
		if (!bIsSendCmd)
		{
			outpw(REG_SDCR, reg|(25<<8)|0x0b);
			bIsSendCmd = TRUE;
		}
		else
			outpw(REG_SDCR, reg | 0x08);

#ifdef _SIC_USE_INT_
		while(!_fmi_bIsSDDataReady)
#else
		while(1)
#endif	//_SIC_USE_INT_
		{
#ifndef _SIC_USE_INT_
			if ((inpw(REG_SDISR) & 0x01) && (!(inpw(REG_SDCR) & 0x08)))
			{
				outpw(REG_SDISR, 0x01);
				break;
			}
#endif
			if (pSD->bIsCardInsert == FALSE)
				return FMI_NO_SD_CARD;
		}

		if ((inpw(REG_SDISR) & 0x02) != 0)		// check CRC
		{
#ifdef DEBUG
			sysprintf("1. fmiSD_Write:write data error [0x%x]\n", inpw(REG_SDISR));
#endif
			outpw(REG_SDISR, 0x02);
			return FMI_SD_CRC_ERROR;
		}
	}

	loop = uBufcnt % 255;
	if (loop != 0)
	{
#ifdef _SIC_USE_INT_
		_fmi_bIsSDDataReady = FALSE;
#endif	//_SIC_USE_INT_
		reg = (inpw(REG_SDCR) & 0xff00c080) | (loop << 16);
		if (!bIsSendCmd)
		{
			outpw(REG_SDCR, reg|(25<<8)|0x0b);
			bIsSendCmd = TRUE;
		}
		else
			outpw(REG_SDCR, reg | 0x08);

#ifdef _SIC_USE_INT_
		while(!_fmi_bIsSDDataReady)
#else
		while(1)
#endif	//_SIC_USE_INT_
		{
#ifndef _SIC_USE_INT_
			if ((inpw(REG_SDISR) & 0x01) && (!(inpw(REG_SDCR) & 0x08)))
			{
				outpw(REG_SDISR, 0x01);
				break;
			}
#endif
			if (pSD->bIsCardInsert == FALSE)
				return FMI_NO_SD_CARD;
		}

		if ((inpw(REG_SDISR) & 0x02) != 0)		// check CRC
		{
#ifdef DEBUG
			sysprintf("2. fmiSD_Write:write data error [0x%x]!\n", inpw(REG_SDISR));
#endif
			outpw(REG_SDISR, 0x02);
			return FMI_SD_CRC_ERROR;
		}
	}
	outpw(REG_SDISR, 0x02);

	if (fmiSDCmdAndRsp(pSD, 12, 0, 0))		// stop command
	{
#ifdef DEBUG
		sysprintf("stop command fail !!\n");
#endif
		return FMI_SD_CRC7_ERROR;
	}
	fmiCheckRB();

	fmiSDCommand(pSD, 7, 0);

	return Successful;
}


VOID fmiGet_SD_info(FMI_SD_INFO_T *pSD, DISK_DATA_T *_info)
{
	unsigned int i;
	unsigned int R_LEN, C_Size, MULT, size;
	unsigned int Buffer[4];
	unsigned char *ptr;

	fmiSDCmdAndRsp2(pSD, 9, pSD->uRCA, Buffer);

#ifdef DEBUG
	sysprintf("max. data transfer rate [%x][%08x]\n", Buffer[0]&0xff, Buffer[0]);
#endif

	if ((Buffer[0] & 0xc0000000) && (pSD->uCardType != FMI_TYPE_MMC))
	{
		C_Size = ((Buffer[1] & 0x0000003f) << 16) | ((Buffer[2] & 0xffff0000) >> 16);
		size = (C_Size+1) * 512;	// Kbytes

		_info->diskSize = size;
		_info->totalSectorN = size << 1;
	}
	else
	{
		R_LEN = (Buffer[1] & 0x000f0000) >> 16;
		C_Size = ((Buffer[1] & 0x000003ff) << 2) | ((Buffer[2] & 0xc0000000) >> 30);
		MULT = (Buffer[2] & 0x00038000) >> 15;
		size = (C_Size+1) * (1<<(MULT+2)) * (1<<R_LEN);

		_info->diskSize = size / 1024;
		_info->totalSectorN = size / 512;
	}
	_info->sectorSize = 512;

	fmiSDCmdAndRsp2(pSD, 10, pSD->uRCA, Buffer);

	_info->vendor[0] = (Buffer[0] & 0xff000000) >> 24;
	ptr = (unsigned char *)Buffer;
	ptr = ptr + 4;
	for (i=0; i<5; i++)
		_info->product[i] = *ptr++;
	ptr = ptr + 10;
	for (i=0; i<4; i++)
		_info->serial[i] = *ptr++;
}

#if 0
int SdChipErase(FMI_SD_INFO_T *pSD, DISK_DATA_T *_info)
{
	int status=0;
//sysprintf("doing erase ...\n");
	status = fmiSDCmdAndRsp(pSD, 32, 512, 6000);
	if (status < 0)
	{
//sysprintf("1.. ");
		return status;
	}
	status = fmiSDCmdAndRsp(pSD, 33, _info->totalSectorN*512, 6000);
	if (status < 0)
	{
//sysprintf("2.. ");
		return status;
	}
	status = fmiSDCmdAndRsp(pSD, 38, 0, 6000);
	if (status < 0)
	{
//sysprintf("3.. ");
		return status;
	}
	fmiCheckRB();

//sysprintf("finish erase !!!\n");
	return 0;
}
#endif
