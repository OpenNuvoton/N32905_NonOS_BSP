/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                       */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                              */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
/****************************************************************************
 * 
 * FILENAME
 *     spi.c
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     This file contains SPI library APIs.
 *
 * DATA STRUCTURES
 *     None
 *
 * FUNCTIONS
 *     None
 *
 * HISTORY
  *     10/12/07      Create Ver 1.0
 *
 * REMARK
 *     None
 **************************************************************************/
/* Header files */
#ifdef ECOS
#include "stdlib.h"
#include "string.h"
#include "drv_api.h"
#include "diag.h"
#include "wbtypes.h"
#include "wbio.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#endif

#include "w55fa93_reg.h"
#include "w55fa93_spi.h"


/*-----------------------------------------------------------------------------------*/
int spiActive(int port)
{
	if (port == 0)
	{
		outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL)|0x01);
		while(inpw(REG_SPI0_CNTRL) & 0x01);
	}
	else
	{
		outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL)|0x01);
		while(inpw(REG_SPI1_CNTRL) & 0x01);
	}
	return 0;
}

/*-----------------------------------------------------------------------------------*/
int spiTxLen(int port, int count, int bitLen)
{
	unsigned int reg;

	if (port == 0)
		reg = inpw(REG_SPI0_CNTRL);
	else
		reg = inpw(REG_SPI1_CNTRL);

	if ((count < 0) || (count > 3))
		return -1;

	if ((bitLen <= 0) || (bitLen > 32))
		return -1;

	if (bitLen == 32)
		reg = reg & 0xffffff07;
	else
		reg = (reg & 0xffffff07) | (bitLen << 3);
	reg = (reg & 0xfffffcff) | (count << 8);

	if (port == 0)
		outpw(REG_SPI0_CNTRL, reg);
	else
		outpw(REG_SPI1_CNTRL, reg);

	return 0;
}

/*-----------------------------------------------------------------------------------*/
void spiSetClock(int port, int clock_by_MHz, int output_by_kHz)
{
	int volatile divider;

	divider = (clock_by_MHz * 1000) / (2 * output_by_kHz) - 1;
	if (port == 0)
		outpw(REG_SPI0_DIVIDER, divider);
	else
		outpw(REG_SPI1_DIVIDER, divider);
}


/*-----------------------------------------------------------------------------------*/
static _spi_init_flag0 = FALSE, _spi_init_flag1 = FALSE;
int spiOpen(SPI_INFO_T *pInfo)
{
	int volatile i;

	if (pInfo->nPort == 0)
	{
		if(_spi_init_flag0)
		{
			sysprintf("SPI0 pin is unavailable!!\n");
			return -1;
		}

		if (!_spi_init_flag0)
		{
			outpw(REG_APBCLK, inpw(REG_APBCLK) | SPIMS0_CKE);   // enable the clocks of SPI
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI0RST);
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI0RST);
			for (i=0; i<200; i++);

			outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0xFF000000);	// enable spi0 pin function

			if (pInfo->bIsSlaveMode)
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | 0x40000);
			else	// master mode
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~0x40000);

			if (pInfo->bIsClockIdleHigh)
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | 0x800);
			else	// clock idle low
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~0x800);

			if (pInfo->bIsLSBFirst)
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) | 0x400);
			else	// MSB first
				outpw(REG_SPI0_CNTRL, inpw(REG_SPI0_CNTRL) & ~0x400);

			if (pInfo->bIsAutoSelect)
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x8);
			else
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~0x8);

			if (pInfo->bIsActiveLow)
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~0x4);
			else	// active high
				outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x4);

			if (pInfo->bIsTxNegative)
				outpw(REG_SPI0_CNTRL, (inpw(REG_SPI0_CNTRL) & ~0x6)| 0x4); // Tx falling; Rx rising
			else
				outpw(REG_SPI0_CNTRL, (inpw(REG_SPI0_CNTRL) & ~0x6)| 0x2); // Tx rising; Rx falling

			_spi_init_flag0 = TRUE;
		}
	}
	else
	{
		if(_spi_init_flag1)
		{
			sysprintf("SPI1 pin is unavailable!!\n");
			return -1;
		}

		if (!_spi_init_flag1)
		{
			outpw(REG_APBCLK, inpw(REG_APBCLK) | SPIMS1_CKE);   // enable the clocks of SPI
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) | SPI1RST);
			outpw(REG_APBIPRST, inpw(REG_APBIPRST) & ~SPI1RST);
			for (i=0; i<200; i++);

			outpw(REG_GPBFUN, inpw(REG_GPBFUN) | 0x03FC0000);	// enable spi1 pin function

			if (pInfo->bIsSlaveMode)
			{
				sysprintf("SPI1 only support Master mode!!\n");
				return -1;
			}
			else	// master mode
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~0x40000);

			if (pInfo->bIsClockIdleHigh)
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) | 0x800);
			else	// clock idle low
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~0x800);

			if (pInfo->bIsLSBFirst)
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) | 0x400);
			else	// MSB first
				outpw(REG_SPI1_CNTRL, inpw(REG_SPI1_CNTRL) & ~0x400);

			if (pInfo->bIsAutoSelect)
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) | 0x8);
			else
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~0x8);

			if (pInfo->bIsActiveLow)
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~0x4);
			else	// active high
				outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) | 0x4);

			if (pInfo->bIsTxNegative)
				outpw(REG_SPI1_CNTRL, (inpw(REG_SPI1_CNTRL) & ~0x6)| 0x4); // Tx falling; Rx rising
			else
				outpw(REG_SPI1_CNTRL, (inpw(REG_SPI1_CNTRL) & ~0x6)| 0x2); // Tx rising; Rx falling

			_spi_init_flag1 = TRUE;
		}
	}
	return 0;
}


/*-----------------------------------------------------------------------------------*/

VOID spiIoctl(INT32 spiPort, INT32 spiFeature, INT32 spiArg0, INT32 spiArg1)
{
	switch(spiFeature)
	{
		case SPI_SET_CLOCK:
				spiSetClock(spiPort, spiArg0, spiArg1);
			break;
	}
}

/*-----------------------------------------------------------------------------------*/

INT spiEnable(INT32 spiPort)
{
	if (spiPort == 0)
		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) | 0x01);
	else
		outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) | 0x01);
	return 0;
}

/*-----------------------------------------------------------------------------------*/

INT spiDisable(INT32 spiPort)
{
	if (spiPort == 0)
		outpw(REG_SPI0_SSR, inpw(REG_SPI0_SSR) & ~0x01);
	else
		outpw(REG_SPI1_SSR, inpw(REG_SPI1_SSR) & ~0x01);
	return 0;
}

INT spiRead(INT port, INT RxBitLen, INT len, CHAR *pDst)
{
	switch (RxBitLen)
	{
		case SPI_32BIT:
			if (port == 0)
				outpw(REG_SPI0_TX0, 0xffffffff);
			else
				outpw(REG_SPI1_TX0, 0xffffffff);

			while (len-- > 0)
			{
				spiTxLen(port, 0, SPI_32BIT);
				spiActive(port);
				if (port == 0)
					*((INT32 *)pDst) = inpw(REG_SPI0_RX0) & 0xffffffff;
				else
					*((INT32 *)pDst) = inpw(REG_SPI1_RX0) & 0xffffffff;
				pDst += 4;
			}

			break;

		case SPI_16BIT:
			if (port == 0)
				outpw(REG_SPI0_TX0, 0xffff);
			else
				outpw(REG_SPI1_TX0, 0xffff);

			while (len-- > 0)
			{
				spiTxLen(port, 0, SPI_16BIT);
				spiActive(port);
				if (port == 0)
					*((INT16 *)pDst) = inpw(REG_SPI0_RX0) & 0xffff;
				else
					*((INT16 *)pDst) = inpw(REG_SPI1_RX0) & 0xffff;
				pDst += 2;
			}

			break;

		case SPI_8BIT:
			if (port == 0)
				outpw(REG_SPI0_TX0, 0xff);
			else
				outpw(REG_SPI1_TX0, 0xff);

			while (len-- > 0)
			{
				spiTxLen(port, 0, SPI_8BIT);
				spiActive(port);
				if (port == 0)
					*((INT8 *)pDst) = inpw(REG_SPI0_RX0) & 0xff;
				else
					*((INT8 *)pDst) = inpw(REG_SPI1_RX0) & 0xff;
				pDst ++;
			}

			break;
	}
	return 0;
}

INT spiWrite(INT port, INT TxBitLen, INT len, CHAR *pSrc)
{
	switch (TxBitLen)
	{
		case SPI_32BIT:
			while (len-- > 0)
			{
				if (port == 0)
					outpw(REG_SPI0_TX0, *((INT32 *)pSrc));
				else
					outpw(REG_SPI1_TX0, *((INT32 *)pSrc));
				spiTxLen(port, 0, SPI_32BIT);
				spiActive(port);
				pSrc += 4;
			}

			break;

		case SPI_16BIT:
			while (len-- > 0)
			{
				if (port == 0)
					outpw(REG_SPI0_TX0, *((INT16 *)pSrc));
				else
					outpw(REG_SPI1_TX0, *((INT16 *)pSrc));
				spiTxLen(port, 0, SPI_16BIT);
				spiActive(port);
				pSrc += 2;
			}

			break;

		case SPI_8BIT:
			while (len-- > 0)
			{
				if (port == 0)
					outpw(REG_SPI0_TX0, *((INT8 *)pSrc));
				else
					outpw(REG_SPI1_TX0, *((INT8 *)pSrc));
				spiTxLen(port, 0, SPI_8BIT);
				spiActive(port);
				pSrc ++;
			}

			break;
	}
	return 0;
}


