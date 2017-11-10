/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                       */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                              */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "w55fa93_reg.h"
#include "wblib.h"
#include "w55fa93_sic.h"
#include "nvtfat.h"
#include "Font.h"
#include "writer.h"


extern UINT32 NAND_BACKUP_BASE;
extern INT fmiSM_Reset(FMI_SM_INFO_T *pSM);
extern INT fmiSMCheckRB(FMI_SM_INFO_T *pSM);

INT8 nIsSysImage;
int volatile gMarkCount=0;
UINT8 *pInfo;
UINT32 gNandBackupAddress=0;
UINT32 gNandBackupAddressTmp=0;
int volatile g_u32ExtraDataSize;
BOOL volatile gbSystemImage;
extern INT32 gNandLoaderSize;

#define NAND_EXTRA_512		16
#define NAND_EXTRA_2K		64
#define NAND_EXTRA_4K		128
#define NAND_EXTRA_8K		376

#define	PRINTF	sysprintf


/* Set BCH function & Page Size */
VOID nvtSM_Initial(FMI_SM_INFO_T *pSM)	/* OK */
{
	gNandBackupAddress = NAND_BACKUP_BASE ;
	gNandBackupAddressTmp = 0;
	outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));

	if (pSM->nPageSize == NAND_PAGE_8KB)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T12 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT|PSIZE_8K));
		g_u32ExtraDataSize = NAND_EXTRA_8K;
		outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
		PRINTF("The Test NAND is 8KB page size\n");
	}
	else if (pSM->nPageSize == NAND_PAGE_4KB)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_4K));
		g_u32ExtraDataSize = NAND_EXTRA_4K;
		outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
		PRINTF("The Test NAND is 4KB page size\n");
	}
	else if (pSM->nPageSize == NAND_PAGE_2KB)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));
		g_u32ExtraDataSize = NAND_EXTRA_2K;
		outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
		PRINTF("The Test NAND is 2KB page size\n");
	}
	else if (pSM->nPageSize == NAND_PAGE_512B)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_512));
		g_u32ExtraDataSize = NAND_EXTRA_512;
		outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);
		PRINTF("The Test NAND is 512B page size\n");
	}
}

// SM functions
INT nvtSM_ReadID(FMI_SM_INFO_T *pSM)	//Moidified
{
	UINT32 tempID[5];

	fmiSM_Reset(pSM);
	outpw(REG_SMCMD, 0x90);		// read ID command
	outpw(REG_SMADDR, 0x80000000);	// address 0x00

	tempID[0] = inpw(REG_SMDATA);
	tempID[1] = inpw(REG_SMDATA);
	tempID[2] = inpw(REG_SMDATA);
	tempID[3] = inpw(REG_SMDATA);
	tempID[4] = inpw(REG_SMDATA);

	switch (tempID[1])
	{
		/* page size 512B */
		case 0x79:	// 128M
			pSM->uSectorPerFlash = 255744;
			pSM->uBlockPerFlash = 8191;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;
			break;

		case 0x76:	// 64M
			pSM->uSectorPerFlash = 127872;
			pSM->uBlockPerFlash = 4095;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;
			break;

		case 0x75:	// 32M
			pSM->uSectorPerFlash = 63936;
			pSM->uBlockPerFlash = 2047;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = FALSE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;
			break;

		case 0x73:	// 16M
			pSM->uSectorPerFlash = 31968;	// max. sector no. = 999 * 32
			pSM->uBlockPerFlash = 1023;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = FALSE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;
			break;

		/* page size 2KB */
		case 0xf1:	// 128M
		case 0xd1:
			pSM->uBlockPerFlash = 1023;
			pSM->uPagePerBlock = 64;
			pSM->uSectorPerBlock = 256;
			pSM->uSectorPerFlash = 255744;
			pSM->bIsMulticycle = FALSE;
			pSM->nPageSize = NAND_PAGE_2KB;
			pSM->bIsNandECC4 = TRUE;
			break;

		case 0xda:	// 256M
			if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 2047;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 1023;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->bIsMLCNand = TRUE;
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 511488;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_2KB;
			break;

		case 0xdc:	// 512M
			if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 2047;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->bIsMLCNand = TRUE;
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 1022976;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_2KB;
			break;

		case 0xd3:	// 1024M
			if ((tempID[3] & 0x33) == 0x32)
			{
				pSM->uBlockPerFlash = 2047;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 1024;	/* 128x8 */
				pSM->nPageSize = NAND_PAGE_4KB;
				pSM->bIsMLCNand = TRUE;
			}
			else if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 8191;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;
				pSM->nPageSize = NAND_PAGE_2KB;
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->bIsMLCNand = TRUE;
				pSM->nPageSize = NAND_PAGE_2KB;
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 2045952;
			pSM->bIsMulticycle = TRUE;
			break;

		case 0xd5:	// 2048M
			if ((tempID[3] & 0x33) == 0x32)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 1024;	/* 128x8 */
				pSM->nPageSize = NAND_PAGE_4KB;
				pSM->bIsMLCNand = TRUE;
			}
			else if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 16383;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;
				pSM->nPageSize = NAND_PAGE_2KB;
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 8191;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->nPageSize = NAND_PAGE_2KB;
				pSM->bIsMLCNand = TRUE;
			}

			if(tempID[0] == 0x98)
			{
					if ((tempID[3]&0x03) == 0x02)	// 8K page size
					{

						pSM->uBlockPerFlash = 2000;
						pSM->uPagePerBlock = 128;
						pSM->uSectorPerBlock = 2048;	/* 128x8 */
						pSM->nPageSize = NAND_PAGE_8KB;
						pSM->bIsMLCNand = TRUE;
					}
			}


			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 4091904;
			pSM->bIsMulticycle = TRUE;
			break;

		default:
#ifdef DEBUG
			PRINTF("SM ID not support!![%x][%x]\n", tempID[0], tempID[1]);
#endif
			return -1;
	}
	PRINTF("SM ID [%x][%x][%x][%x]\n", tempID[0], tempID[1], tempID[2], tempID[3]);

	return 0;

}


INT nvtSM_Write_512(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)	//Modified
{
	int temp;

	// set the spare area configuration
	if (nIsSysImage != 0xFF)
	{
		if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
		{
			outpw(REG_SMRA_0, 0x00A55AFF);		// Stop Marker
			gMarkCount += 0x100;
		}
		else
		{
			temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
			outpw(REG_SMRA_0, temp);
			gMarkCount += 0x100;
			if(gMarkCount == 0x1D)	/* Next page is 0x1D (page 29) */
				gMarkCount = 0xA500;
		}
	}
	else
	{
		outpw(REG_SMRA_0, 0x0000FFFF);
		gMarkCount = 0;
	}
//	outpw(REG_SM_RA1, 0xFFFFFFFF);

	outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

	fmiSM_Reset(pSM);

	/* set Page Size = 512 bytes */
	outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_512));

	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);			// enable Auto Write
	outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_REN);				// enable Read
	/* enable DMAC */
	while(inpw(REG_DMACCSR) & FMI_BUSY);				// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* set DMA Transfer Starting Address */
	outpw(REG_DMACSAR, uSAddr);

    /* set SM columm & page address */
	outpw(REG_SMCMD, 0x80);								// Program setup command
	outpw(REG_SMADDR, ucColAddr & 0xFF);				// CA0 - CA7
	outpw(REG_SMADDR, uSector & 0xFF);				// PA0 - PA7

	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);	// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xFF);	// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);	// PA16 - PA17
	}

	/* begin DMA write transfer */
	outpw(REG_SMISR, SMISR_DMA_IF);	        // clear DMA flag
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(!(inpw(REG_SMISR) & SMISR_DMA_IF));         	// wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);	               		// clear DMA flag

	outpw(REG_SMCMD, 0x10);								// Progeam command

	if (!fmiSMCheckRB(pSM))
		return -1;

	outpw(REG_SMCMD, 0x70);				// read STATUS command

	if (inpw(REG_SMDATA) & 0x01)
	{
	    PRINTF("DrvNAND_WriteOnePage_512: data error [%d]!!\n", uSector);

		return -1;	// Program Fail
	}
	return 0;
}


INT nvtSM_Write_2K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)	//Modified
{
	int temp;

	if (nIsSysImage != 0xFF)
	{
		if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
		{
			outpw(REG_SMRA_0, 0x00A55AFF);		// Stop Marker
			gMarkCount += 0x100;
		}
		else
		{
			temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
			outpw(REG_SMRA_0, temp);
			gMarkCount += 0x100;
		}
	}
	else
	{
		outpw(REG_SMRA_0, 0x0000FFFF);
		gMarkCount = 0;
	}
	/*
	outpw(REG_SM_RA1, 0xFFFFFFFF);
	outpw(REG_SM_RA4, 0x0000FFFF);
	outpw(REG_SM_RA5, 0xFFFFFFFF);
	outpw(REG_SM_RA8, 0x0000FFFF);
	outpw(REG_SM_RA9, 0xFFFFFFFF);
	outpw(REG_SM_RA12, 0x0000FFFF);
	outpw(REG_SM_RA13, 0xFFFFFFFF);*/

	outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

	fmiSM_Reset(pSM);

	/* set Page Size = 2048 bytes */
	if(gbSystemImage)
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));
	else
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));

	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);			// enable Auto Write

	/* enable DMAC */
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* Set DMA Transfer Starting Address */
	outpw(REG_DMACSAR, uSAddr);

	outpw(REG_SMCMD, 0x80);					// Program setup command

	outpw(REG_SMADDR, ucColAddr & 0xFF);
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);
	outpw(REG_SMADDR, uSector & 0xFF);

	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);	// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xFF);	                // PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);	// PA16 - PA17
	}

	/* begin DMA write transfer */
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(!(inpw(REG_SMISR) & SMISR_DMA_IF));       // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag

	outpw(REG_SMCMD, 0x10);							// Progeam command

	if (!fmiSMCheckRB(pSM))
		return -1;

	outpw(REG_SMCMD, 0x70);						// read STATUS command

	if (inpw(REG_SMDATA) & 0x01)
	{
	    PRINTF("DrvNAND_WriteOnePage 2K: data error [%d]!!\n", uSector);

		return -1;	// Program Fail
	}
	return 0;				// Program Success
}


INT nvtSM_Write_4K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)	//Modified
{
	int temp;

	if (nIsSysImage != 0xFF)
	{
		if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
		{
			outpw(REG_SMRA_0, 0x00A55AFF);		// Stop Marker
			gMarkCount += 0x100;
		}
		else
		{
			temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
			outpw(REG_SMRA_0, temp);
			gMarkCount += 0x100;
		}
	}
	else
	{
		outpw(REG_SMRA_0, 0x0000FFFF);
		gMarkCount = 0;
	}


	outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

	fmiSM_Reset(pSM);

	outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_4K));

	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);			// enable Auto Write


	/* enable DMAC */
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* Set DMA Transfer Starting Address */
	outpw(REG_DMACSAR, uSAddr);

	outpw(REG_SMCMD, 0x80);					// Program setup command

	outpw(REG_SMADDR, ucColAddr & 0xFF);
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);
	outpw(REG_SMADDR, uSector & 0xFF);

	if (!pSM->bIsMulticycle)
	{
		outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);	// PA8 - PA15
	}
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xFF);	                // PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);	// PA16 - PA17
	}

	/* begin DMA write transfer */
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(!(inpw(REG_SMISR) & SMISR_DMA_IF));         // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return -1;

	outpw(REG_SMCMD, 0x70);		// status read command

	if (inpw(REG_SMDATA) & 0x1)
	{
	    PRINTF("DrvNAND_WriteOnePage_4K: data error [%d]!!\n", uSector);
		return -1;	// Program Fail
	}

	return 0;
}

INT nvtSM_Write_8K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)	//Added
{
	int temp;

	if (nIsSysImage != 0xFF)
	{
		if (((gMarkCount >> 8) +1) * pSM->nPageSize >= gNandLoaderSize)
		{
			outpw(REG_SMRA_0, 0x00A55AFF);		// Stop Marker
			gMarkCount += 0x100;
		}
		else
		{
			temp = (((nIsSysImage + gMarkCount) & 0xFFFF) << 8) + 0x000000FF;
			outpw(REG_SMRA_0, temp);
			gMarkCount += 0x100;
		}
	}
	else
	{
		outpw(REG_SMRA_0, 0x0000FFFF);
		gMarkCount = 0;
	}

	outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

	fmiSM_Reset(pSM);

	outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T12 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT|PSIZE_8K));

	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);			// enable Auto Write


	/* enable DMAC */
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);

    /* Set DMA Transfer Starting Address */
	outpw(REG_DMACSAR, uSAddr);

	outpw(REG_SMCMD, 0x80);					// Program setup command

	outpw(REG_SMADDR, ucColAddr & 0xFF);
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);
	outpw(REG_SMADDR, uSector & 0xFF);

	if (!pSM->bIsMulticycle)
	{
		outpw(REG_SMADDR, ((uSector >> 8) & 0xFF) | EOA_SM);	// PA8 - PA15
	}
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xFF);	            // PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xFF) | EOA_SM);	// PA16 - PA17
	}

	/* begin DMA write transfer */
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(!(inpw(REG_SMISR) & SMISR_DMA_IF));         // wait to finish DMAC transfer.
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return -1;

	outpw(REG_SMCMD, 0x70);		// status read command

	if (inpw(REG_SMDATA) & 0x1)
	{
	    PRINTF("DrvNAND_WriteOnePage_8K: data error [%d]!!\n", uSector);
		return -1;	// Program Fail
	}
	return 0;
}

/* BCH Correction */
static void nvtSM_CorrectData_BCH(UINT32 u32PageSize, UINT8 ucFieidIndex, UINT8 ucErrorCnt, UINT8* pDAddr)	//Added
{
    UINT32 uaData[16], uaAddr[16];
	UINT32 uaErrorData[4];
	UINT8	ii;


    for(ii=0; ii<4; ii++)
    {
		uaErrorData[ii] = inpw(REG_BCH_ECC_DATA0 + ii*4);
	}

    for(ii=0; ii<4; ii++)
    {
	    uaData[ii*4+0] = uaErrorData[ii] & 0xff;
	    uaData[ii*4+1] = (uaErrorData[ii]>>8) & 0xff;
	    uaData[ii*4+2] = (uaErrorData[ii]>>16) & 0xff;
	    uaData[ii*4+3] = (uaErrorData[ii]>>24) & 0xff;
	}

    for(ii=0; ii<8; ii++)
    {
	    uaAddr[ii*2+0] = inpw(REG_BCH_ECC_ADDR0 + ii*4) & 0x1fff;
	    uaAddr[ii*2+1] = (inpw(REG_BCH_ECC_ADDR0 + ii*4)>>16) & 0x1fff;
    }

    pDAddr += (ucFieidIndex-1)*0x200;

    for(ii=0; ii<ucErrorCnt; ii++)
    {
    	if (u32PageSize == 8192)
    	{
			switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
			{
				case BCH_T4:	// 4K+128
				default:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 7 - uaAddr[ii];
						uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_16+uaAddr[ii]) ^=  uaData[ii];
					}
					break;

				case BCH_T8:	// 4K+128
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 14 - uaAddr[ii];
						uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
					}

					break;

				case BCH_T12:	// 4K+216
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 22 - uaAddr[ii];
						uaAddr[ii] += 23*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
					}

					break;
			}
    	}
    	else if (u32PageSize == 4096)
    	{
			switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
			{
				case BCH_T4:	// 4K+128
				default:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 7 - uaAddr[ii];
						uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_16+uaAddr[ii]) ^=  uaData[ii];
					}
					break;

				case BCH_T8:	// 4K+128
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 14 - uaAddr[ii];
						uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
					}

					break;

				case BCH_T12:	// 4K+216
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 22 - uaAddr[ii];
						uaAddr[ii] += 23*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];
					}

					break;
			}
    	}
		else if (u32PageSize == 2048)
		{
			switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
			{
				case BCH_T4:
				default:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 7 - uaAddr[ii];
						uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];
					}
					break;

				case BCH_T8:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
					else
					{
						uaAddr[ii] = 543 - uaAddr[ii];
						uaAddr[ii] = 14 - uaAddr[ii];
						uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset
			  	        *((UINT8*)REG_SMRA_1+uaAddr[ii]) ^=  uaData[ii];
					}

					break;
			}
		}
		else
		{
	    	if (uaAddr[ii] < 512)
	  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];
			else
			{
				uaAddr[ii] = 543 - uaAddr[ii];
				uaAddr[ii] = 7 - uaAddr[ii];
	  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];
			}
		}
  	}
}

/* Read Extra Data */
int nvtSM_ReadOnePage_ExtraData(FMI_SM_INFO_T *pSM, UINT32 uPageIndex)	//Added
{
	UINT32 ii;
	UINT8 *uDAddr;

	outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

	fmiSM_Reset(pSM);

	/* set Page Size = 8192 bytes */
	if(pSM->nPageSize == 512)
	{
		outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));		//uExtraNo->16
		/* set READ command */
		outpw(REG_SMCMD, 0x50);				// READ command 1 (0x50), read from extra area address (area C)
		outpw(REG_SMADDR, 0x00);
	}
	else
	{
		switch(pSM->nPageSize)
		{
			case 2048:
				outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_2K);	//uExtraNo->64
				/* set READ command */
				outpw(REG_SMCMD, 0x00);		// READ 1st cycle command
				outpw(REG_SMADDR, 0x00);
				outpw(REG_SMADDR, 0x08);
				break;
			case 4096:
				outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_4K);
				/* set READ command */
				outpw(REG_SMCMD, 0x00);		// READ 1st cycle command
				outpw(REG_SMADDR, 0x00);
				outpw(REG_SMADDR, 0x10);

				break;
			case 8192:
				outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE)|PSIZE_8K);
				/* set READ command */
				outpw(REG_SMCMD, 0x00);		// READ 1st cycle command
				outpw(REG_SMADDR, 0x00);
				outpw(REG_SMADDR, 0x20);
				break;
		}

	}


	outpw(REG_SMADDR, uPageIndex & 0xFF);

	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uPageIndex >> 8) & 0xFF) | EOA_SM);	// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uPageIndex >> 8) & 0xFF);	                // PA8 - PA15
		outpw(REG_SMADDR, ((uPageIndex >> 16) & 0xFF) | EOA_SM);	// PA16 - PA17
	}

	if(pSM->nPageSize != 512)
		outpw(REG_SMCMD, 0x30);			// READ 2nd cycle command

	if (!fmiSMCheckRB(pSM))
		return -1;

    uDAddr = (UINT8*) REG_SMRA_0;
	for (ii=0; ii<g_u32ExtraDataSize; ii++)
	{
		*(UINT8*) uDAddr = inpb(REG_SMDATA);

		uDAddr++;
	}

	return 0;
}


/* Read One Page */
int fmiSM_Read(FMI_SM_INFO_T *pSM, UINT32 uPageIndex,UINT32 uDAddr)	//Added
{
	volatile UINT32 uStatus, uErrorCnt;
	volatile UINT32 uF1_status, uF2_status;
	volatile UINT8 ii, jj,EccLoop;

	outpw(REG_FMICR, inpw(REG_FMICR)|FMI_SM_EN);

	fmiSM_Reset(pSM);

	outpw(REG_SMCSR, inpw(REG_SMCSR) & (~SMCR_PSIZE));

	switch(pSM->nPageSize)
	{
		case 512:
			outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_512));
			break;
		case 2048:
			if(gbSystemImage)
				outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T4 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));
			else
				outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_2K));

			break;
		case 4096:
			outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T8 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT | PSIZE_4K));
			break;

		case 8192:
			outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~(SMCR_BCH_TSEL|SMCR_ECC_EN)) | (BCH_T12 | SMCR_ECC_EN | SMCR_ECC_3B_PROTECT|PSIZE_8K));
			break;
	}

	outpw(REG_SMREAREA_CTL, g_u32ExtraDataSize);

	outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_REN);

	/* enable DMAC */
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);


	/* Set DMA Transfer Starting Address */
	outpw(REG_DMACSAR, uDAddr);


	outpw(REG_SMCMD, 0x00);		// READ 1st cycle command

	outpw(REG_SMADDR, 0);

	if(pSM->nPageSize != 512)
		outpw(REG_SMADDR, 0);

	outpw(REG_SMADDR, uPageIndex & 0xFF);

	if (!pSM->bIsMulticycle)
	{
		outpw(REG_SMADDR, ((uPageIndex >> 8) & 0xFF) | EOA_SM);	// PA8 - PA15
	}
	else
	{
		outpw(REG_SMADDR, (uPageIndex >> 8) & 0xFF);	                // PA8 - PA15
		outpw(REG_SMADDR, ((uPageIndex >> 16) & 0xFF) | EOA_SM);	// PA16 - PA17
	}

	if(pSM->nPageSize != 512)
		outpw(REG_SMCMD, 0x30);		// READ 2nd cycle command

	if (!fmiSMCheckRB(pSM))
		return -1;

	uF1_status = 0;
	uF2_status = 0;
	uStatus = 0;
	/* begin DMA read transfer */
	outpw(REG_SMISR, SMISR_DMA_IF);	// clear DMA flag
	outpw(REG_SMISR, inpw(REG_SMISR)| SMISR_DMA_IF| SMISR_ECC_FIELD_IF);	// clear DMA flag
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

	while(1)
	{
		if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
		{
			switch(pSM->nPageSize)
			{
				case 512:
				{
		            uF1_status = inpw(REG_SM_ECC_ST0);

		            uF1_status &= 0x3f;

		    		if ((uF1_status & 0x03)==0x01)  // correctable error in 1st field
		            {
		                uErrorCnt = uF1_status >> 2;
		                nvtSM_CorrectData_BCH(pSM->nPageSize, 1, uErrorCnt, (UINT8*)uDAddr);
		            }
		    		else if (((uF1_status & 0x03)==0x02)
		    		      ||((uF1_status & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
		            {
		              uStatus = 1;
		              PRINTF("Page %d - Uncorrectable Error 0x%X\n",uPageIndex, inpw(REG_SM_ECC_ST0));
		            }
		            break;
				}
				case 2048:
				{

		            uF1_status = inpw(REG_SM_ECC_ST0);
		        	for (ii=1; ii<5; ii++)
		        	{
			    		if ((uF1_status & 0x03)==0x01)  // correctable error in 1st field
			            {
			                uErrorCnt = uF1_status >> 2;
			                nvtSM_CorrectData_BCH(pSM->nPageSize, ii, uErrorCnt, (UINT8*)uDAddr);
			                break;
			            }
			    		else if (((uF1_status & 0x03)==0x02)
			    		      ||((uF1_status & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
			            {
			            	uStatus = 1;
			            	PRINTF("Page %d - Uncorrectable Error 0x%X\n",uPageIndex,inpw(REG_SM_ECC_ST0));
			                break;
			            }
						uF1_status >>= 8;
					}

			        break;
				}
				case 4096:
				case 8192:
				{
					if(pSM->nPageSize == 4096)
						EccLoop = 2;
					else
						EccLoop = 4;

					for (jj=0; jj<EccLoop; jj++)
					{
			            uF1_status = inpw(REG_SM_ECC_ST0+jj*4);
			        	for (ii=1; ii<5; ii++)
			        	{
				    		if ((uF1_status & 0x03)==0x01)  // correctable error in 1st field
				            {
				                uErrorCnt = uF1_status >> 2;
				                nvtSM_CorrectData_BCH(pSM->nPageSize, ii+jj*4, uErrorCnt, (UINT8*)uDAddr);
				                break;
				            }
				    		else if (((uF1_status & 0x03)==0x02)
				    		      ||((uF1_status & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field
				            {
				            	uStatus = 1;
				            	PRINTF("Page %d - Uncorrectable Error 0x%X\n",uPageIndex, inpw(REG_SM_ECC_ST0));
				                break;
				            }
							uF1_status >>= 8;
						}
					}
				}
				break;
			}

	   	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	   	// clear ECC_FLD_Error
		}

		if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.
		{
			if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
				break;
		}
	}

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag

    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	    // clear ECC flag

  	if (uStatus)
		return -1;

    return 0;
}

/* function pointer */
BOOL volatile bIsNandInit = FALSE;
FMI_SM_INFO_T nvtSMInfo, *pNvtSM0, *pNvtSMInfo;


INT nvtSMInit(void)		// Modified
{
	if (!bIsNandInit)
	{

    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);
    // Reset DMAC
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_SWRST);
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) & ~DMAC_SWRST);
    // SM Enable
    outpw(REG_FMICR, FMI_SM_EN);

	//if ((_nand_init0 == 0) && (_nand_init1 == 0))
	{
		/* set CLE/ALE setup and hold time width */
		outpw(REG_SMTCR, 0x10204);	 // CLE/ALE=0x01, R/W_h=0x02, R/W_l=0x04
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_512);
		outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_ECC_3B_PROTECT);
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_ECC_CHK);

		/* init SM interface */
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);
	}

#if (TARGET_Chip_Select == 0)   // target CS0
		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003CC00);		// enable NAND NWR/NRD/RB0 pins
		outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00F30000);		// enable NAND ALE/CLE/CS0 pins
		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~ SMCR_CS0);			// CS0 pin low, enable CS0
		outpw(REG_SMCSR, inpw(REG_SMCSR) |   SMCR_CS1);			// CS1 pin high, disable CS1
#else                           // target CS1
		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003F000);		// enable NAND NWR/NRD/RB1 pins
		outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00FC0000);		// enable NAND ALE/CLE/CS1 pins
		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~ SMCR_CS1);			// CS1 pin low, enable CS1
		outpw(REG_SMCSR, inpw(REG_SMCSR) |   SMCR_CS0);			// CS0 pin high, disable CS0
#endif

		pNvtSMInfo = (FMI_SM_INFO_T *)malloc(sizeof(NVT_SM_INFO_T));
		//memset((char *)&nvtSMInfo, 0, sizeof(NVT_SM_INFO_T));
		memset((char *)pNvtSMInfo, 0, sizeof(NVT_SM_INFO_T));
#if (TARGET_Chip_Select == 0)   // target CS0
		pSM0= pNvtSMInfo;
		pNvtSM0 = pSM0;
#else
		pSM1= pNvtSMInfo;
		pNvtSM0 = pSM1;
#endif

		if (nvtSM_ReadID(pNvtSM0) < 0)
			return -1;
		nvtSM_Initial(pNvtSM0);
		bIsNandInit = TRUE;
	}

	return 0;
}


INT nvtSMpread(INT PBA, INT page, UINT8 *buff)	//Modified
{
	FMI_SM_INFO_T *pSM;
	int pageNo;

	pSM = pNvtSM0;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	if(PBA < 4)
		gbSystemImage = TRUE;
	else
		gbSystemImage = FALSE;

	pageNo = PBA * pSM->uPagePerBlock + page;

	nvtSM_ReadOnePage_ExtraData(pSM, pageNo);

	return (fmiSM_Read(pSM, pageNo, (UINT32)buff));
}


INT nvtSMpwrite(INT PBA, INT page, UINT8 *buff)	// Modified
{
	FMI_SM_INFO_T *pSM;
	int pageNo;

	pSM = pNvtSM0;

	// enable SM
	outpw(REG_FMICR, 0x08);

//	if (PBA != 0)
	if (PBA > 3)
		nIsSysImage = 0xFF;
	else
	{
		if (page == 0)
			gMarkCount = 0;
		else if (page > 32/* 10 */)
		{
			gMarkCount = 0;
			nIsSysImage = 0xFF;
		}
	}

	if(PBA < 4)
		gbSystemImage = TRUE;
	else
		gbSystemImage = FALSE;

	pageNo = PBA * pSM->uPagePerBlock + page;

	if (pSM->nPageSize == NAND_PAGE_2KB)		/* 2KB */
		return (nvtSM_Write_2K(pSM, pageNo, 0, (UINT32)buff));
	else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 4KB */
		return (nvtSM_Write_4K(pSM, pageNo, 0, (UINT32)buff));
	else if (pSM->nPageSize == NAND_PAGE_8KB)	/* 8KB */
		return (nvtSM_Write_8K(pSM, pageNo, 0, (UINT32)buff));
	else	/* 512B */
		return (nvtSM_Write_512(pSM, pageNo, 0, (UINT32)buff));
}


INT nvtSMchip_erase(UINT32 startBlcok, UINT32 endBlock)	//Modified
{
	int i, status=0;
	FMI_SM_INFO_T *pSM;
	pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

	pSM = pNvtSM0;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);

	// erase all chip
	for (i=startBlcok; i<=endBlock; i++)
	{
		status = nand_block_erase0(i);
		if (status < 0)
			PRINTF("SM block erase fail <%d>!!\n", i);
	}
	return 0;
}


INT fmiMarkBadBlock(UINT32 block)
{
	UINT32 ucColAddr, uSector;

	uSector = block * pNvtSM0->uPagePerBlock;
	if (pNvtSM0->nPageSize != NAND_PAGE_512B)
	{
		// send command
		if (pNvtSM0->nPageSize == NAND_PAGE_2KB)
			ucColAddr = 2048;
		else if (pNvtSM0->nPageSize == NAND_PAGE_4KB)
			ucColAddr = 4096;
		else if (pNvtSM0->nPageSize == NAND_PAGE_8KB)
			ucColAddr = 8192;
		else
			return -1;

		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMADDR, (ucColAddr >> 8) & 0x1f);
		outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
		if (!pNvtSM0->bIsMulticycle)
			outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000);		// PA16 - PA17
		}
	}
	else
	{
		ucColAddr = 0;
		outpw(REG_SMCMD, 0x50);		// read RA command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
		if (!pNvtSM0->bIsMulticycle)
			outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000);		// PA16 - PA17
		}
	}

	/* mark data */
	outpw(REG_SMDATA, 0x00);
	outpw(REG_SMDATA, 0x00);
	outpw(REG_SMDATA, 0x00);
	outpw(REG_SMDATA, 0x00);

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pNvtSM0))
		return Fail;

	if (fmiSM_Reset(pNvtSM0) < 0)
		return -1;

	return Successful;
}


/* Check the 1th byte of spare */
INT CheckBadBlockMark(UINT32 block)
{
	pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

	if (nvtSMpread(block, 0, pInfo) < 0)
		return -1;

	if (inpw(REG_SMRA_0) == 0x0)
		return Fail;
	else
		return Successful;
}
