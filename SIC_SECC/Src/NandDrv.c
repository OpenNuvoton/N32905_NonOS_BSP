/*-----------------------------------------------------------------------------------*/
/* Nuvoton Technology Corporation confidential                                       */
/*                                                                                   */
/* Copyright (c) 2008 by Nuvoton Technology Corporation                              */
/* All rights reserved                                                               */
/*                                                                                   */
/*-----------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "wbio.h"

#include "w55fa93_sic.h"
#include "wblib.h"

#include "fmi.h"
#include "gnand_global.h"

#include "w55fa93_gnand.h"
#include "w55fa93_reg.h"

#define OPT_SOFTWARE_WRITE_ECC
#define OPT_SOFTWARE_ECC

#define OPT_SOFTWARE_WRITE_2KNAND_ECC
#define OPT_SOFTWARE_2KNAND_ECC
#define OPT_BCH_PLUS_SECC_FOR_NAND512

#ifdef OPT_SW_WP
#define SW_WP_DELAY_LOOP		5000
#endif 

#define DBG_PRINTF(...)
//#define DBG_PRINTF	sysprintf
#define NAND_RESERVED_BLOCK		10
#define OPT_TWO_RB_PINS		
#define OPT_FIRST_4BLOCKS_ECC4	

BOOL volatile _fmi_bIsNandFirstAccess = TRUE;
extern BOOL volatile _fmi_bIsSMDataReady;
INT fmiSMCheckBootHeader(INT chipSel, FMI_SM_INFO_T *pSM);
static int _nand_init0 = 0, _nand_init1 = 0;
//UCHAR _fmi_ucSMBuffer[4096];
__align(4096) UCHAR _fmi_ucSMBuffer[4096];

UINT8 *_fmi_pSMBuffer;

/* functions */
INT fmiSMCheckRB(FMI_SM_INFO_T *pSM)
{
#if 0	// no timer in it
    UINT32 ii;
    for (ii=0; ii<100; ii++);	

	while(1)
	{
		if(pSM == pSM0)		
		{
			if (inpw(REG_SMISR) & SMISR_RB0_IF)
			{
				while(! (inpw(REG_SMISR) & SMISR_RB0) );
				outpw(REG_SMISR, SMISR_RB0_IF);
				return 1;
			}
		}
		else
		{
			if (inpw(REG_SMISR) & SMISR_RB1_IF)
			{
				while(! (inpw(REG_SMISR) & SMISR_RB1) );
				outpw(REG_SMISR, SMISR_RB1_IF);
				return 1;
			}
		}				
	}
	return 0;

#else
	unsigned int volatile tick;
	tick = sysGetTicks(TIMER0);

	while(1)
	{
		if(pSM == pSM0)
		{
			if (inpw(REG_SMISR) & SMISR_RB0_IF)
			{
				while(! (inpw(REG_SMISR) & SMISR_RB0) );			
				outpw(REG_SMISR, SMISR_RB0_IF);
				return 1;
			}
		}
		else
		{
			if (inpw(REG_SMISR) & SMISR_RB1_IF)
			{
				while(! (inpw(REG_SMISR) & SMISR_RB1) );			
				outpw(REG_SMISR, SMISR_RB1_IF);
				return 1;
			}
		}				

		if ((sysGetTicks(TIMER0) - tick) > 1000)
			break;
	}
	return 0;
#endif		
}


// SM functions
INT fmiSM_Reset(FMI_SM_INFO_T *pSM)
{
#ifdef OPT_TWO_RB_PINS
	UINT32 volatile i;
	
	if(pSM == pSM0)
		outpw(REG_SMISR, SMISR_RB0_IF);
	else		
		outpw(REG_SMISR, SMISR_RB1_IF);		
		
	outpw(REG_SMCMD, 0xff);
	for (i=100; i>0; i--);

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;
	return 0;
	
#else	
	UINT32 volatile i;
	outpw(REG_SMISR, SMISR_RB0_IF);	
	outpw(REG_SMCMD, 0xff);
	for (i=100; i>0; i--);

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;
	return 0;
#endif	// OPT_TWO_RB_PINS
}

VOID fmiSM_Initial(FMI_SM_INFO_T *pSM)
{
	if (pSM->nPageSize == NAND_PAGE_8KB)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_8K);	// psize:2048
#ifdef DEBUG
//		printf("The Test NAND is 8KB page size\n");
#endif
	}
	else if (pSM->nPageSize == NAND_PAGE_4KB)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_4K);	// psize:2048
#ifdef DEBUG
//		printf("The Test NAND is 4KB page size\n");
#endif
	}
	else if (pSM->nPageSize == NAND_PAGE_2KB)
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_2K);	// psize:2048
#ifdef DEBUG
//		printf("The Test NAND is 2KB page size\n");
#endif
	}
	else 	// pSM->nPageSize = NAND_PAGE_512B
	{
		outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_512);	// psize:512
#ifdef DEBUG
//		printf("The Test NAND is 512B page size\n");
#endif
	}

	outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_ECC_EN);	// enable ECC 

	if (pSM->bIsCheckECC)
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_ECC_CHK);	// enable ECC check
	else
		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_ECC_CHK);	// disable ECC check

#ifdef _SIC_USE_INT_
	if ((_nand_init0 == 0) && (_nand_init1 == 0))
		outpw(REG_SMIER, SMIER_DMA_IE);
#endif	//_SIC_USE_INT_

	// set BCH_Tx and redundant area size
    outpw(REG_SMREAREA_CTL, inpw(REG_SMREAREA_CTL) & ~SMRE_MECC);  		// Redundant area size 	
	if (pSM->nPageSize == NAND_PAGE_8KB)
	{
	    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
	    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T12);   			// BCH_12 is selected
	    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 376);  // Redundant area size 
	}
	else if (pSM->nPageSize == NAND_PAGE_4KB)
	{
		if (pSM->bIsNandECC12 == TRUE)
		{
		    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
		    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T12);   			// BCH_12 is selected
		    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 216);;  // Redundant area size 		    
		}
		else if (pSM->bIsNandECC8 == TRUE)
		{
		    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
		    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);   			// BCH_8 is selected
		    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 128);  // Redundant area size 		    
		}
		else 
		{
		    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
		    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
		    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 128);  // Redundant area size 		    
		
		}
	}
	else if (pSM->nPageSize == NAND_PAGE_2KB)
	{
		if (pSM->bIsNandECC8 == TRUE)
		{
		    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
		    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);   			// BCH_8 is selected
		    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
		}
		else 
		{
		    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
		    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
		    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
		}
	}
	else 
	{
	    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
	    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
	    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 16);  // Redundant area size 		    
	}	
}


INT fmiSM_ReadID(FMI_SM_INFO_T *pSM, NDISK_T *NDISK_info)
{
	UINT32 tempID[5];

	fmiSM_Reset(pSM);
	outpw(REG_SMCMD, 0x90);		// read ID command
	outpw(REG_SMADDR, EOA_SM);	// address 0x00

	tempID[0] = inpw(REG_SMDATA);
	tempID[1] = inpw(REG_SMDATA);
	tempID[2] = inpw(REG_SMDATA);
	tempID[3] = inpw(REG_SMDATA);
	tempID[4] = inpw(REG_SMDATA);

	NDISK_info->vendor_ID = tempID[0];
	NDISK_info->device_ID = tempID[1];

	if (tempID[0] == 0xC2)
		pSM->bIsCheckECC = FALSE;
	else
		pSM->bIsCheckECC = TRUE;

	pSM->bIsNandECC4 = FALSE;
	pSM->bIsNandECC8 = FALSE;
	pSM->bIsNandECC12 = FALSE;
	pSM->bIsNandECC15 = FALSE;	

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

			NDISK_info->NAND_type = NAND_TYPE_SLC;
			NDISK_info->nZone = 1;				/* number of zones */
			NDISK_info->nBlockPerZone = 8192;	/* blocks per zone */
			NDISK_info->nPagePerBlock = 32;		/* pages per block */
			NDISK_info->nLBPerZone = 8000;		/* logical blocks per zone */
			NDISK_info->nPageSize = 512;
			break;

		case 0x76:	// 64M
		case 0x5A:	// 64M XtraROM
			pSM->uSectorPerFlash = 127872;
			pSM->uBlockPerFlash = 4095;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;

			NDISK_info->NAND_type = NAND_TYPE_SLC;
			NDISK_info->nZone = 1;				/* number of zones */
			NDISK_info->nBlockPerZone = 4096;	/* blocks per zone */
			NDISK_info->nPagePerBlock = 32;		/* pages per block */
			NDISK_info->nLBPerZone = 4000;		/* logical blocks per zone */
			NDISK_info->nPageSize = 512;
			break;

		case 0x75:	// 32M
			pSM->uSectorPerFlash = 63936;
			pSM->uBlockPerFlash = 2047;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = FALSE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;

			NDISK_info->NAND_type = NAND_TYPE_SLC;
			NDISK_info->nZone = 1;				/* number of zones */
			NDISK_info->nBlockPerZone = 2048;	/* blocks per zone */
			NDISK_info->nPagePerBlock = 32;		/* pages per block */
			NDISK_info->nLBPerZone = 2000;		/* logical blocks per zone */
			NDISK_info->nPageSize = 512;
			break;

		case 0x73:	// 16M
			pSM->uSectorPerFlash = 31968;	// max. sector no. = 999 * 32
			pSM->uBlockPerFlash = 1023;
			pSM->uPagePerBlock = 32;
			pSM->uSectorPerBlock = 32;
			pSM->bIsMulticycle = FALSE;
			pSM->nPageSize = NAND_PAGE_512B;
			pSM->bIsNandECC4 = TRUE;

			NDISK_info->NAND_type = NAND_TYPE_SLC;
			NDISK_info->nZone = 1;				/* number of zones */
			NDISK_info->nBlockPerZone = 1024;	/* blocks per zone */
			NDISK_info->nPagePerBlock = 32;		/* pages per block */
			NDISK_info->nLBPerZone = 1000;		/* logical blocks per zone */
			NDISK_info->nPageSize = 512;
			break;

		/* page size 2KB */
		case 0xf1:	// 128M
		case 0xd1:	// 128M
			pSM->uBlockPerFlash = 1023;
			pSM->uPagePerBlock = 64;
			pSM->uSectorPerBlock = 256;
			pSM->uSectorPerFlash = 255744;
			pSM->bIsMulticycle = FALSE;
			pSM->nPageSize = NAND_PAGE_2KB;
			pSM->bIsNandECC4 = TRUE;

			NDISK_info->NAND_type = NAND_TYPE_SLC;
			NDISK_info->nZone = 1;				/* number of zones */
			NDISK_info->nBlockPerZone = 1024;	/* blocks per zone */
			NDISK_info->nPagePerBlock = 64;		/* pages per block */
			NDISK_info->nLBPerZone = 1000;		/* logical blocks per zone */
			NDISK_info->nPageSize = 2048;
			
			pSM->bIsNandECC4 = FALSE;
			pSM->bIsNandECC8 = TRUE;
	//		pSM->bIsNandECC4 = TRUE;	//(IBR uses ECC4)
	//		pSM->bIsNandECC8 = FALSE;
			
			break;

		case 0xda:	// 256M
			if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 2047;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;

				NDISK_info->NAND_type = NAND_TYPE_SLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 64;		/* pages per block */
				NDISK_info->nBlockPerZone = 2048;	/* blocks per zone */
				NDISK_info->nLBPerZone = 2000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 1023;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->bIsMLCNand = TRUE;

				NDISK_info->NAND_type = NAND_TYPE_MLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 128;	/* pages per block */
				NDISK_info->nBlockPerZone = 1024;	/* blocks per zone */
				NDISK_info->nLBPerZone = 1000;		/* logical blocks per zone */
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 511488;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_2KB;

			NDISK_info->nPageSize = 2048;

			pSM->bIsNandECC4 = FALSE;
			pSM->bIsNandECC8 = TRUE;
			break;

		case 0xdc:	// 512M
			if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;

				NDISK_info->NAND_type = NAND_TYPE_SLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 64;		/* pages per block */
				NDISK_info->nBlockPerZone = 4096;	/* blocks per zone */
				NDISK_info->nLBPerZone = 4000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 2047;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->bIsMLCNand = TRUE;

				NDISK_info->NAND_type = NAND_TYPE_MLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 128;	/* pages per block */
				NDISK_info->nBlockPerZone = 2048;	/* blocks per zone */
				NDISK_info->nLBPerZone = 2000;		/* logical blocks per zone */
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 1022976;
			pSM->bIsMulticycle = TRUE;
			pSM->nPageSize = NAND_PAGE_2KB;

			NDISK_info->nPageSize = 2048;

			pSM->bIsNandECC4 = FALSE;
			pSM->bIsNandECC8 = TRUE;
			break;

		case 0xd3:	// 1024M
		
			if ((tempID[3] & 0x33) == 0x32)
			{
				pSM->uBlockPerFlash = 2047;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 1024;	/* 128x8 */
				pSM->nPageSize = NAND_PAGE_4KB;
				pSM->bIsMLCNand = TRUE;

				NDISK_info->NAND_type = NAND_TYPE_MLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 128;	/* pages per block */
				NDISK_info->nPageSize = 4096;
				NDISK_info->nBlockPerZone = 2048;	/* blocks per zone */
				NDISK_info->nLBPerZone = 2000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 8191;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;
				pSM->nPageSize = NAND_PAGE_2KB;

				NDISK_info->NAND_type = NAND_TYPE_SLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 64;		/* pages per block */
				NDISK_info->nPageSize = 2048;
				NDISK_info->nBlockPerZone = 8192;	/* blocks per zone */
				NDISK_info->nLBPerZone = 8000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->bIsMLCNand = TRUE;
				pSM->nPageSize = NAND_PAGE_2KB;

				NDISK_info->NAND_type = NAND_TYPE_MLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 128;	/* pages per block */
				NDISK_info->nPageSize = 2048;
				NDISK_info->nBlockPerZone = 4096;	/* blocks per zone */
				NDISK_info->nLBPerZone = 4000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x22)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 512;	/* 64x8 */
				pSM->nPageSize = NAND_PAGE_4KB;
				pSM->bIsMLCNand = FALSE;

				NDISK_info->NAND_type = NAND_TYPE_SLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 64;		/* pages per block */
				NDISK_info->nPageSize = 4096;
				NDISK_info->nBlockPerZone = 4096;	/* blocks per zone */
				NDISK_info->nLBPerZone = 4000;		/* logical blocks per zone */
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 2045952;
			pSM->bIsMulticycle = TRUE;
			
			if ((pSM->nPageSize == NAND_PAGE_4KB) ||
				(pSM->bIsNandECC4 == FALSE) )
			{
				pSM->bIsNandECC4 = FALSE;
				pSM->bIsNandECC8 = TRUE;
				pSM->bIsNandECC12 = FALSE;				
				
			//	pSM->bIsNandECC8 = FALSE;
			//	pSM->bIsNandECC12 = TRUE;				
			}				
			else
			{
				pSM->bIsNandECC4 = FALSE;
				pSM->bIsNandECC8 = TRUE;
			}
			break;

		case 0xd5:	// 2048M
			if ((tempID[3] & 0x33) == 0x32)
			{
				pSM->uBlockPerFlash = 4095;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 1024;	/* 128x8 */
				pSM->nPageSize = NAND_PAGE_4KB;
				pSM->bIsMLCNand = TRUE;

				NDISK_info->NAND_type = NAND_TYPE_MLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 128;	/* pages per block */
				NDISK_info->nPageSize = 4096;
				NDISK_info->nBlockPerZone = 4096;	/* blocks per zone */
				NDISK_info->nLBPerZone = 4000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x11)
			{
				pSM->uBlockPerFlash = 16383;
				pSM->uPagePerBlock = 64;
				pSM->uSectorPerBlock = 256;
				pSM->nPageSize = NAND_PAGE_2KB;

				NDISK_info->NAND_type = NAND_TYPE_SLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 64;		/* pages per block */
				NDISK_info->nPageSize = 2048;
				NDISK_info->nBlockPerZone = 16384;	/* blocks per zone */
				NDISK_info->nLBPerZone = 16000;		/* logical blocks per zone */
			}
			else if ((tempID[3] & 0x33) == 0x21)
			{
				pSM->uBlockPerFlash = 8191;
				pSM->uPagePerBlock = 128;
				pSM->uSectorPerBlock = 512;
				pSM->nPageSize = NAND_PAGE_2KB;
				pSM->bIsMLCNand = TRUE;

				NDISK_info->NAND_type = NAND_TYPE_MLC;
				NDISK_info->nZone = 1;				/* number of zones */
				NDISK_info->nPagePerBlock = 128;	/* pages per block */
				NDISK_info->nPageSize = 2048;
				NDISK_info->nBlockPerZone = 8192;	/* blocks per zone */
				NDISK_info->nLBPerZone = 8000;		/* logical blocks per zone */
			}

			if ((tempID[3] & 0x0C) == 0x04)
				pSM->bIsNandECC4 = TRUE;
			else if ((tempID[3] & 0x0C) == 0x08)
				pSM->bIsNandECC4 = FALSE;

			pSM->uSectorPerFlash = 4091904;
			pSM->bIsMulticycle = TRUE;
			
			if ((pSM->nPageSize == NAND_PAGE_4KB) ||
				(pSM->bIsNandECC4 == FALSE) )
			{
				pSM->bIsNandECC4 = FALSE;
				pSM->bIsNandECC8 = TRUE;
				pSM->bIsNandECC12 = FALSE;				
			}				
			else
			{
				pSM->bIsNandECC4 = FALSE;
				pSM->bIsNandECC8 = TRUE;
			}
			break;

		default:
#ifdef DEBUG
			printf("SM ID not support!![%x][%x]\n", tempID[0], tempID[1]);
#endif
			return FMI_SM_ID_ERR;
	}

#ifdef DEBUG
	printf("SM ID [%x][%x][%x][%x]\n", tempID[0], tempID[1], tempID[2], tempID[3]);
#endif
	return 0;
}


INT fmiSM2BufferM(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT8 ucColAddr)
{
	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	return 0;
}


INT fmiSM2BufferM_RA(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT8 ucColAddr)
{
	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		
	
	outpw(REG_SMCMD, 0x50);		// read command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	return 0;
}


INT fmiSMECCErrCount(UINT32 ErrSt, BOOL bIsECC8)
{
	unsigned int volatile errCount;

	if ((ErrSt & 0x02) || (ErrSt & 0x200) || (ErrSt & 0x20000) || (ErrSt & 0x2000000))
	{
#ifdef DEBUG
		printf("uncorrectable!![0x%x]\n", ErrSt);
#endif
		return FMI_SM_ECC_ERROR;
	}
	if (ErrSt & 0x01)
	{
		errCount = (ErrSt >> 2) & 0xf;
#ifdef DEBUG
		if (bIsECC8)
			printf("Field 5 have %d error!!\n", errCount);
		else
			printf("Field 1 have %d error!!\n", errCount);
#endif
	}
	if (ErrSt & 0x100)
	{
		errCount = (ErrSt >> 10) & 0xf;
#ifdef DEBUG
		if (bIsECC8)
			printf("Field 6 have %d error!!\n", errCount);
		else
			printf("Field 2 have %d error!!\n", errCount);
#endif
	}
	if (ErrSt & 0x10000)
	{
		errCount = (ErrSt >> 18) & 0xf;
#ifdef DEBUG
		if (bIsECC8)
			printf("Field 7 have %d error!!\n", errCount);
		else
			printf("Field 3 have %d error!!\n", errCount);
#endif
	}
	if (ErrSt & 0x1000000)
	{
		errCount = (ErrSt >> 26) & 0xf;
#ifdef DEBUG
		if (bIsECC8)
			printf("Field 8 have %d error!!\n", errCount);
		else
			printf("Field 4 have %d error!!\n", errCount);
#endif
	}
	return errCount;
}

static VOID fmiSM_CorrectData_BCH(UINT8 ucFieidIndex, UINT8 ucErrorCnt, UINT8* pDAddr)
{

    UINT32 uaData[16], uaAddr[16];
	UINT32 uaErrorData[4];
	UINT8	ii, jj;
	UINT32 uPageSize;
	
	uPageSize = inpw(REG_SMCSR) & SMCR_PSIZE;
	    
    jj = ucErrorCnt/4;
	jj ++;
	if (jj > 4)	jj = 4;
    
//    for(ii=0; ii<4; ii++)
    for(ii=0; ii<jj; ii++)    
    {
		uaErrorData[ii] = inpw(REG_BCH_ECC_DATA0 + ii*4);
	}		
	
//    for(ii=0; ii<4; ii++)	
    for(ii=0; ii<jj; ii++)	
    {
	    uaData[ii*4+0] = uaErrorData[ii] & 0xff;
	    uaData[ii*4+1] = (uaErrorData[ii]>>8) & 0xff;    
	    uaData[ii*4+2] = (uaErrorData[ii]>>16) & 0xff;        
	    uaData[ii*4+3] = (uaErrorData[ii]>>24) & 0xff;            
	}		    

    jj = ucErrorCnt/2;
    jj ++;
	if (jj > 8)	jj = 8;    

//    for(ii=0; ii<8; ii++)	
    for(ii=0; ii<jj; ii++)	    
    {
	    uaAddr[ii*2+0] = inpw(REG_BCH_ECC_ADDR0 + ii*4) & 0x1fff;
	    uaAddr[ii*2+1] = (inpw(REG_BCH_ECC_ADDR0 + ii*4)>>16) & 0x1fff;            
    }

    pDAddr += (ucFieidIndex-1)*0x200;
  
    for(ii=0; ii<ucErrorCnt; ii++)	
    {
    	if (uPageSize == PSIZE_8K)		
    	{
			switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
			{
				case BCH_T4:	// 8K+256
				default:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								    	
							uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset, only Field-0
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 7 - uaAddr[ii];											
							uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_32+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
					break;
			
				case BCH_T8:	// 8K+256
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								   
							uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset							 	
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 14 - uaAddr[ii];											
							uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_4+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
				
					break;

				case BCH_T12:	// 8K+376
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								   
							uaAddr[ii] += 23*(ucFieidIndex-1);	// field offset							 	
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 22 - uaAddr[ii];											
							uaAddr[ii] += 23*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
					break;

				case BCH_T15:	
					break;
			}
    	}
    	else if (uPageSize == PSIZE_4K)		
    	{
			switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
			{
				case BCH_T4:	// 4K+128
				default:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								    	
							uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset, only Field-0
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 7 - uaAddr[ii];											
							uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_16+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
					break;
			
				case BCH_T8:	// 4K+128
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								   
							uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset							 	
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 14 - uaAddr[ii];											
							uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
				
					break;

				case BCH_T12:	// 4K+216
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								   
							uaAddr[ii] += 23*(ucFieidIndex-1);	// field offset							 	
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 22 - uaAddr[ii];											
							uaAddr[ii] += 23*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
					break;
			}
    	}
    	else if (uPageSize == PSIZE_2K)		
		{
			switch(inpw(REG_SMCSR) & SMCR_BCH_TSEL)
			{
				case BCH_T4:
				default:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								    	
							uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset, only Field-0
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 7 - uaAddr[ii];											
							uaAddr[ii] += 8*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_8+uaAddr[ii]) ^=  uaData[ii];  
						}
					}			  	        
					break;
			
				case BCH_T8:
			    	if (uaAddr[ii] < 512)
			  	        *(pDAddr+uaAddr[ii]) ^=  uaData[ii];  
					else
					{
				    	if (uaAddr[ii] < 515)
				    	{
							uaAddr[ii] -= 512;								   
							uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset							 	
				  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
				    	}
				    	else
				    	{
							uaAddr[ii] = 543 - uaAddr[ii];					
							uaAddr[ii] = 14 - uaAddr[ii];											
							uaAddr[ii] += 15*(ucFieidIndex-1);	// field offset
				  	        *((UINT8*)REG_SMRA_1+uaAddr[ii]) ^=  uaData[ii];  
						}
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
		    	if (uaAddr[ii] < 515)
		    	{
					uaAddr[ii] -= 512;								    	
		  	        *((UINT8*)REG_SMRA_0+uaAddr[ii]) ^=  uaData[ii];  			    	
		    	}
		    	else
		    	{
					uaAddr[ii] = 543 - uaAddr[ii];					
					uaAddr[ii] = 7 - uaAddr[ii];											
		  	        *((UINT8*)REG_SMRA_2+uaAddr[ii]) ^=  uaData[ii];  
				}
			}

		}			
  	}
}

INT fmiSM_Read_512(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 uDAddr)
{
	int volatile ret=0;
	UINT32 uStatus;
	UINT32 uErrorCnt;
	volatile UINT32 uError = 0;	

	outpw(REG_DMACSAR, uDAddr);
	ret = fmiSM2BufferM(pSM, uSector, 0);
	if (ret < 0)
		return ret;

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);
	
#ifdef _SIC_USE_INT_
	while(!_fmi_bIsSMDataReady);
#else
	while(1)
	{
		if (!(inpw(REG_SMCSR) & SMCR_DRD_EN))
			break;
	}
#endif

	if (pSM->bIsCheckECC)
	{
		while(1) 
		{
			if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
			{
		        if (inpw(REG_SMCSR) & BCH_T4)  	// BCH_ECC selected
		        {
		            uStatus = inpw(REG_SM_ECC_ST0);   
		            uStatus &= 0x3f;        
		
		    		if ((uStatus & 0x03)==0x01) 		// correctable error in 1st field 
		            {
		                uErrorCnt = uStatus >> 2;
		                fmiSM_CorrectData_BCH(1, uErrorCnt, (UINT8*)uDAddr);	                
		                
				#ifdef DEBUG
						printf("Field 1 have %d error!!\n", uErrorCnt);
				#endif
		            }    
		    		else if (((uStatus & 0x03)==0x02)  
		    		      ||((uStatus & 0x03)==0x03)) 	// uncorrectable error or ECC error
		            {
		        #ifdef DEBUG			
		        	    printf("SM uncorrectable error is encountered, %4x !!\n", uStatus);
		        #endif    		
		        		uError = 1;
		            }            
		        }
		        else
		        {
				#ifdef DEBUG
					printf("Wrong BCH setting for page-512 NAND !!\n");
				#endif
		        		uError = 2;				
		        }
		       
		   	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	   	// clear ECC_FLD_Error	        
			}		    

			if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.   		    
			{
				if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
					break;
			}	
		}
	}
	else
		outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
	
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    if (uError)
   		return -1;
    
	return 0;
}

VOID fmiBuffer2SMM(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT8 ucColAddr)
{
	// set the spare area configuration
	/* write byte 514, 515 as used page */
#ifdef OPT_SOFTWARE_WRITE_ECC
	outpw(REG_SMRA_0, 0x0000FFFF);
        outpw(REG_SMRA_1, inpw(REG_SMRA_1) | 0x0000FFFF);
#else
        outpw(REG_SMRA_0, 0x0000FFFF);
        outpw(REG_SMRA_1, 0xFFFFFFFF);
#endif
	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}
}

static INT fmiSM_Write_512_BCH(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 uSAddr)
{
	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);
	outpw(REG_DMACSAR, uSAddr);
	fmiBuffer2SMM(pSM, uSector, 0);

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	// _SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_512: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	return 0;
}

//INT fmiSM_Write_512(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 uSAddr)
static INT fmiSM_Write_512_SECC(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 uSAddr)
{
#ifdef  OPT_SOFTWARE_WRITE_ECC
	unsigned int i; 
	unsigned char u8Parity[10];
	unsigned char *ptr = (unsigned char *)REG_SMRA_0;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/
#endif

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

#ifdef  OPT_SOFTWARE_WRITE_ECC
		outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_ECC_EN);			// disable ECC 		
		outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);	// HW bug (if not to disable this bit, DMAC can't be complete)		
#else
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);
#endif

#ifdef  OPT_SOFTWARE_WRITE_ECC
	*ptr++=0xFF;	/* Byte 0 ~5 recommand by HP */
	*ptr++=0xFF;
	*ptr++=0x00;
	*ptr++=0x00;
	*ptr++=0xFF;
	*ptr++=0xFF;
	Ecc4Encoder((char *)uSAddr, u8Parity, FALSE);
	memcpy(ptr, u8Parity, 10); 	/* copy parity to REG_SMRA_0+6 */
	DBG_PRINTF("Software ECC4 Encoder Successful\n");						
#endif

	outpw(REG_DMACSAR, uSAddr);
	fmiBuffer2SMM(pSM, uSector, 0);

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	// _SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	
#ifdef  OPT_SOFTWARE_WRITE_ECC
	DBG_PRINTF("Complete\n");
	
	if (pSM->bIsCheckECC)
	{
		ptr = (unsigned char *)REG_SMRA_0;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/
		for(i=0; i<16; i=i+1)
		{
			outpw(REG_SMDATA, *ptr++);
		}
	}		
	
#endif
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	outpw(REG_SMCMD, 0x10);		// auto program command

#ifdef  OPT_SOFTWARE_WRITE_ECC
	outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_ECC_EN);			// enable ECC again
	outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);	
#endif

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_512: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	return 0;
}

INT fmiSM_Read_2K(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uDAddr)
{
	UINT32 uStatus;
	UINT32 uErrorCnt, ii;
	volatile UINT32 uError = 0;

//	fmiSM_Reset(pSM);
	
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);                                   
	outpw(REG_DMACSAR, uDAddr);	// set DMA transfer starting address

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		
	if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)	/* ECC_FLD_IF */
	{
		//printf("read: ECC error!!\n");
		outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
	}

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMADDR, 0);	// CA0 - CA7
	outpw(REG_SMADDR, 0);	// CA8 - CA11
	outpw(REG_SMADDR, uPage & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x30);		// read command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

//	if ((pSM->bIsCheckECC) || (inpw(REG_SMCSR)&SMCR_ECC_CHK) )
	if (pSM->bIsCheckECC)
	{
		while(1) 
		{
			if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
			{
	            uStatus = inpw(REG_SM_ECC_ST0);   		
	        	for (ii=1; ii<5; ii++)	        
	        	{
	        	#if 0
		            if (!(uStatus & 0x03)) 
		            {
						uStatus >>= 8;		    		            		            	
		            	continue;
					}
				#endif							            	
	        	
		    		if ((uStatus & 0x03)==0x01)  // correctable error in 1st field 
		            {
		                uErrorCnt = uStatus >> 2;
		                fmiSM_CorrectData_BCH(ii, uErrorCnt, (UINT8*)uDAddr);	                
				#ifdef DEBUG
						printf("Field %d have %d error!!\n", ii, uErrorCnt);
				#endif
		                break;
		            }    
		    		else if (((uStatus & 0x03)==0x02)  
		    		      ||((uStatus & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field                      
		            {
		        #ifdef DEBUG			
		        	    printf("SM uncorrectable error is encountered, %4x !!\n", uStatus);
		        #endif    		
		        		uError = 1;
		                break;		        	
		            }            
					uStatus >>= 8;		    		            
				}		            

		   	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	   	// clear ECC_FLD_Error	        
			}		    

	#ifdef _SIC_USE_INT_
			if (_fmi_bIsSMDataReady)
			{
				if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
					break;
			}
	#else
			if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.   		    
			{
				if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
					break;
			}
	#endif	//_SIC_USE_INT_

		}
	}
	else
	{
		while(1)
		{
			outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	
			if (inpw(REG_SMISR) & SMISR_DMA_IF)
			{
			    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
				break;
			}				
		}			
		
	}		

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    if (uError)
   		return -1;

	return 0;
}

static INT fmiSM_Read_2K_SECC_1st512(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uDAddr)
{
	UINT32 uStatus;
	UINT32 uErrorCnt, ii;

	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);                                   
	outpw(REG_DMACSAR, uDAddr);	// set DMA transfer starting address

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		
	if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)	/* ECC_FLD_IF */
	{
		//printf("read: ECC error!!\n");
		outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
	}

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMADDR, 0);	// CA0 - CA7
	outpw(REG_SMADDR, 0);	// CA8 - CA11
	outpw(REG_SMADDR, uPage & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x30);		// read command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

	{
		while(1)
		{
			outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	
			if (inpw(REG_SMISR) & SMISR_DMA_IF)
			{
			    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
				break;
			}				
		}			
		
	}		

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
	return 0;
}

INT fmiSM_Read_RA(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 ucColAddr)
{
	/* clear R/B flag */
	
//	fmiSM_Reset(pSM);
		
		
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
//	outpw(REG_SMADDR, (ucColAddr >> 8) & 0x1f);	// CA8 - CA12
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xFF);	// CA8 - CA12
	outpw(REG_SMADDR, uPage & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x30);		// read command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	return 0;
}

INT fmiSM_Read_RA_512(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uColumm)
{
	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	outpw(REG_SMCMD, 0x50);		// read command
	outpw(REG_SMADDR, uColumm);	
	outpw(REG_SMADDR, uPage & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	return 0;
}

//INT fmiSM_Write_2K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
static INT fmiSM_Write_2K_BCH(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
{

//	fmiSM_Reset(pSM);

	/* enable DMAC */                                   
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);                                   
	outpw(REG_DMACSAR, uSAddr);	// set DMA transfer starting address

	// set the spare area configuration
	/* write byte 2050, 2051 as used page */
	outpw(REG_SMRA_0, 0x0000FFFF);
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		
	if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)	/* ECC_FLD_IF */
	{
		//printf("error sector !!\n");
		outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
	}

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
//	outpw(REG_SMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff);	// CA8 - CA11
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    
	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_2K: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	
#if 0
	if (inpw(REG_SMRA_0) != 0x0000FFFF)
		while(1);
#endif	
	return 0;
}

//INT fmiSM_Write_2K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
static INT fmiSM_Write_2K_SECC(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
{
	volatile char u8EccStatus = 0;
	unsigned int i; 
	unsigned char u8Parity[10];
	unsigned char *ptr = (unsigned char *)REG_SMRA_0;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_ECC_EN);			// disable ECC 		
	outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);	// HW bug (if not to disable this bit, DMAC can't be complete)		

	outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_512);	// psize:512
	
	/* enable DMAC */                                   
	while(inpw(REG_DMACCSR) & FMI_BUSY);	// wait DMAC FMI ready;
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);                                   
	outpw(REG_DMACSAR, uSAddr);	// set DMA transfer starting address

	if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)	/* ECC_FLD_IF */
	{
		//printf("error sector !!\n");
		outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
	}

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
//	outpw(REG_SMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff);	// CA8 - CA11
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

	/* write 1st 512+16 */
#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	// set the spare area configuration
	*ptr++=0xFF;	/* Byte 0 ~5 recommand by HP */
	*ptr++=0xFF;
	*ptr++=0x00;
	*ptr++=0x00;
	*ptr++=0xFF;
	*ptr++=0xFF;
	Ecc4Encoder((char *)uSAddr, u8Parity, FALSE);
	memcpy(ptr, u8Parity, 10); 	/* copy parity to REG_SMRA_0+6 */
	DBG_PRINTF("Software ECC4 Encoder Successful\n");						
   
	ptr = (unsigned char *)REG_SMRA_0;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/
	for(i=0; i<16; i=i+1)
	{
		outpw(REG_SMDATA, *ptr++);
	}

	/* write 2nd 512+16 */    
#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	// set the spare area configuration
	*ptr++=0xFF;	/* Byte 0 ~5 recommand by HP */
	*ptr++=0xFF;
	*ptr++=0x00;
	*ptr++=0x00;
	*ptr++=0xFF;
	*ptr++=0xFF;
	uSAddr += 512;
	Ecc4Encoder((char *)uSAddr, u8Parity, FALSE);
	memcpy(ptr, u8Parity, 10); 	/* copy parity to REG_SMRA_0+6 */
	DBG_PRINTF("Software ECC4 Encoder Successful\n");						
   
	ptr = (unsigned char *)REG_SMRA_4;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/
	for(i=0; i<16; i=i+1)
	{
		outpw(REG_SMDATA, *ptr++);
	}

	/* write 3rd 512+16 */    
#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	// set the spare area configuration
	*ptr++=0xFF;	/* Byte 0 ~5 recommand by HP */
	*ptr++=0xFF;
	*ptr++=0x00;
	*ptr++=0x00;
	*ptr++=0xFF;
	*ptr++=0xFF;
	uSAddr += 512;
	Ecc4Encoder((char *)uSAddr, u8Parity, FALSE);
	memcpy(ptr, u8Parity, 10); 	/* copy parity to REG_SMRA_0+6 */
	DBG_PRINTF("Software ECC4 Encoder Successful\n");						
   
	ptr = (unsigned char *)REG_SMRA_8;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/
	for(i=0; i<16; i=i+1)
	{
		outpw(REG_SMDATA, *ptr++);
	}
	
	/* write 4th 512+16 */    
#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	
    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	// set the spare area configuration
	*ptr++=0xFF;	/* Byte 0 ~5 recommand by HP */
	*ptr++=0xFF;
	*ptr++=0x00;
	*ptr++=0x00;
	*ptr++=0xFF;
	*ptr++=0xFF;
	uSAddr += 512;
	Ecc4Encoder((char *)uSAddr, u8Parity, FALSE);
	memcpy(ptr, u8Parity, 10); 	/* copy parity to REG_SMRA_0+6 */
	DBG_PRINTF("Software ECC4 Encoder Successful\n");						
   
	ptr = (unsigned char *)REG_SMRA_12;	/* ptr:Logical address, ECC4 encoder data to FMI_BA[0x200]	*/
	for(i=0; i<16; i=i+1)
	{
		outpw(REG_SMDATA, *ptr++);
	}
	
	// program it    
	outpw(REG_SMCMD, 0x10);		// auto program command

	outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_ECC_EN);			// enable ECC again
	outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);	

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_2K);	// psize:2048
	
	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_2K: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	return 0;
}

INT fmiSM_Write_2K_ALC(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
{
	outpw(REG_DMACSAR, uSAddr);	// set DMA transfer starting address

	// set the spare area configuration
	/* write byte 2050, 2051 as used page */
	outpw(REG_SMRA_0, 0x0000FFFF);
	outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0x0f);	// CA8 - CA11
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|0x80000000);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|0x80000000);		// PA16 - PA17
	}

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	// _SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	{
		int i;
		UINT8 *u8pData;
		
		u8pData = (UINT8 *)REG_SMRA_0;

		for (i=0; i<64; i++)
			outpw(REG_SMDATA, *u8pData++);
	}

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_2K: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	return 0;
}

INT fmiSM_Read_4K(FMI_SM_INFO_T *pSM, UINT32 uPage, UINT32 uDAddr)
{
	UINT32 uStatus;
	UINT32 uErrorCnt, ii, jj;
	volatile UINT32 uError = 0;	
	

	outpw(REG_DMACSAR, uDAddr);	// set DMA transfer starting address

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	outpw(REG_SMCMD, 0x00);		// read command
	outpw(REG_SMADDR, 0);	// CA0 - CA7
	outpw(REG_SMADDR, 0);	// CA8 - CA11
	outpw(REG_SMADDR, uPage & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uPage >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uPage >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uPage >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}
	outpw(REG_SMCMD, 0x30);		// read command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);

	if ((pSM->bIsCheckECC) || (inpw(REG_SMCSR)&SMCR_ECC_CHK) )
	{
		while(1) 
		{
			if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)
			{
				for (jj=0; jj<2; jj++)
				{
		            uStatus = inpw(REG_SM_ECC_ST0+jj*4);   				            
		            if (!uStatus) 
		            	continue;
		            	
		        	for (ii=1; ii<5; ii++)	        
		        	{
			            if (!(uStatus & 0x03)) 
			            {
							uStatus >>= 8;		    		            		            	
			            	continue;						
						}		            	

			    		if ((uStatus & 0x03)==0x01)  // correctable error in 1st field 
			            {
			                uErrorCnt = uStatus >> 2;
			                fmiSM_CorrectData_BCH(jj*4+ii, uErrorCnt, (UINT8*)uDAddr);	                
					#ifdef DEBUG
							printf("Field %d have %d error!!\n", jj*4+ii, uErrorCnt);
					#endif
			                break;
			            }    
			    		else if (((uStatus & 0x03)==0x02)  
			    		      ||((uStatus & 0x03)==0x03)) // uncorrectable error or ECC error in 1st field                      
			            {
			        #ifdef _DEBUG			
			        	    printf("SM uncorrectable BCH error is encountered !!\n");
			        #endif    	
			        		uError = 1;
			                break;		        	
			            }            
						uStatus >>= 8;		    		            
					}		            
				}
		   	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	   	// clear ECC_FLD_Error	        
			}		    

	#ifdef _SIC_USE_INT_
			if (_fmi_bIsSMDataReady)
			{
				if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
					break;
			}
	#else
			if (inpw(REG_SMISR) & SMISR_DMA_IF)      // wait to finish DMAC transfer.   		    
			{
				if ( !(inpw(REG_SMISR) & SMISR_ECC_FIELD_IF) )
					break;
			}
	#endif	//_SIC_USE_INT_

		}
	}
	else
	{
		while(1)
		{
			outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	
			if (inpw(REG_SMISR) & SMISR_DMA_IF)
			{
			    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
				break;
			}				
		}			
	}		

    if (uError)
   		return -1;
	
	return 0;
}


INT fmiSM_Write_4K(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
{
	outpw(REG_DMACSAR, uSAddr);	// set DMA transfer starting address

	// set the spare area configuration
	/* write byte 2050, 2051 as used page */
	outpw(REG_SMRA_0, 0x0000FFFF);
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
//	outpw(REG_SMADDR, (ucColAddr >> 8) & 0x1f);	// CA8 - CA12
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff);	// CA8 - CA12
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	//_SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_4K: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	return 0;
}


INT fmiSM_Write_4K_ALC(FMI_SM_INFO_T *pSM, UINT32 uSector, UINT32 ucColAddr, UINT32 uSAddr)
{
	outpw(REG_DMACSAR, uSAddr);	// set DMA transfer starting address

	// set the spare area configuration
	/* write byte 2050, 2051 as used page */
	outpw(REG_SMRA_0, 0x0000FFFF);
	outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);

	/* clear R/B flag */
	if (pSM == pSM0)
	{		
		while(!(inpw(REG_SMISR) & SMISR_RB0));
		outpw(REG_SMISR, SMISR_RB0_IF);
	}
	else
	{
		while(!(inpw(REG_SMISR) & SMISR_RB1));
		outpw(REG_SMISR, SMISR_RB1_IF);
	}		

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
	outpw(REG_SMADDR, (ucColAddr >> 8) & 0x1f);	// CA8 - CA12
	outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

#ifdef _SIC_USE_INT_
	_fmi_bIsSMDataReady = FALSE;
#endif	// _SIC_USE_INT_

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
	outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DWR_EN);

	while(1)
	{
#ifdef _SIC_USE_INT_
		if (_fmi_bIsSMDataReady)
#else
		if (inpw(REG_SMISR) & SMISR_DMA_IF)         	// wait to finish DMAC transfer.
#endif	//_SIC_USE_INT_					
			break;
	}	

    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	

	{
		int i;
		UINT8 *u8pData;
		
		u8pData = (UINT8 *)REG_SMRA_0;

		for (i=0; i<218; i++)
			outpw(REG_SMDATA, *u8pData++);
	}

	outpw(REG_SMCMD, 0x10);		// auto program command

	if (!fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;

	outpw(REG_SMCMD, 0x70);		// status read command
	if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
	{
#ifdef DEBUG
		printf("fmiSM_Write_4K: data error!!\n");
#endif
		return FMI_SM_STATE_ERROR;
	}
	return 0;
}

// mhkuo
INT fmiCheckInvalidBlock(FMI_SM_INFO_T *pSM, UINT32 BlockNo)
{
	int volatile status=0;
	unsigned int volatile sector;
	unsigned char volatile data512=0xff, data517=0xff, blockStatus=0xff;

	if (BlockNo == 0)
		return 0;

//	sector = BlockNo * pSM->uPagePerBlock;
	
#ifdef OPT_SOFTWARE_2KNAND_ECC	
	if (pSM == pSM0)
	{
		if (pSM->bIsMLCNand == TRUE)
			sector = (BlockNo+1) * pSM->uPagePerBlock - 1;
		else
			sector = BlockNo * pSM->uPagePerBlock;	
	
		if (pSM->nPageSize == NAND_PAGE_512B)
			status = fmiSM2BufferM_RA(pSM, sector, 0);
		else
			status = fmiSM_Read_RA(pSM, sector, pSM->nPageSize);		
	}
	else
	{
		sector = BlockNo * pSM->uPagePerBlock;	
		if (pSM->nPageSize == NAND_PAGE_512B)
			status = fmiSM2BufferM_RA(pSM, sector, 0);
		else if (pSM->nPageSize == NAND_PAGE_2KB)
			status = fmiSM_Read_RA(pSM, sector, 512);
		else
		{
			if (pSM->bIsMLCNand == TRUE)
				sector = (BlockNo+1) * pSM->uPagePerBlock - 1;

			status = fmiSM_Read_RA(pSM, sector, pSM->nPageSize);		
		}			
	}
#else
	if (pSM->bIsMLCNand == TRUE)
		sector = (BlockNo+1) * pSM->uPagePerBlock - 1;
	else
		sector = BlockNo * pSM->uPagePerBlock;	

	if (pSM->nPageSize == NAND_PAGE_512B)
		status = fmiSM2BufferM_RA(pSM, sector, 0);
	else 
		status = fmiSM_Read_RA(pSM, sector, pSM->nPageSize);		
#endif		
	
	if (status < 0)
	{
#ifdef DEBUG
		printf("fmiCheckInvalidBlock 0x%x\n", status);
#endif
		return 1;
	}
	
#ifdef OPT_SOFTWARE_2KNAND_ECC		
	if (  (pSM->nPageSize == NAND_PAGE_512B) || 
		 ((pSM->nPageSize == NAND_PAGE_2KB) && (pSM == pSM1) ))
	{
#else	
	if (pSM->nPageSize == NAND_PAGE_512B)	
	{
#endif		
		data512 = inpw(REG_SMDATA) & 0xff;
		data517 = inpw(REG_SMDATA);
		data517 = inpw(REG_SMDATA);
		data517 = inpw(REG_SMDATA);
		data517 = inpw(REG_SMDATA);
		data517 = inpw(REG_SMDATA) & 0xff;
		
//		if ((data512 != 0xFF) || (data517 != 0xFF))
		if ((data512 == 0xFF) && (data517 == 0xFF))
		{
			fmiSM_Reset(pSM);
			
	#ifdef OPT_SOFTWARE_2KNAND_ECC					
			if (pSM->nPageSize == NAND_PAGE_512B)
				status = fmiSM2BufferM_RA(pSM, sector+1, 0);
			else
				status = fmiSM_Read_RA(pSM, sector+1, 512);
	#else		
			status = fmiSM2BufferM_RA(pSM, sector+1, 0);
	#endif			

			if (status < 0)
			{
	#ifdef DEBUG
					printf("fmiCheckInvalidBlock 0x%x\n", status);
	#endif
					return 1;
			}
			data512 = inpw(REG_SMDATA) & 0xff;
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA) & 0xff;
			if ((data512 != 0xFF) || (data517 != 0xFF))
			{
				fmiSM_Reset(pSM);
				return 1;	// invalid block
			}
		}
		else
		{
			fmiSM_Reset(pSM);
			return 1;	// invalid block
		}			
	}
	else
	{
		blockStatus = inpw(REG_SMDATA) & 0xff;	
//		if (blockStatus != 0xFF)
		if (blockStatus == 0xFF)
		{
			fmiSM_Reset(pSM);
		#if 0
			return 1;	// mhkuo@20100614
		#else			
			status = fmiSM_Read_RA(pSM, sector+1, pSM->nPageSize);		
			if (status < 0)
			{
	#ifdef DEBUG
					printf("fmiCheckInvalidBlock 0x%x\n", status);
	#endif
					return 1;
			}
			blockStatus = inpw(REG_SMDATA) & 0xff;	
			if (blockStatus != 0xFF) 
			{
				fmiSM_Reset(pSM);
				return 1;	// invalid block
			}
		#endif			
		}
		else
		{
			fmiSM_Reset(pSM);
			return 1;	// invalid block
		}			
	}

	fmiSM_Reset(pSM);
	return 0;
}

static void sicSMselect(INT chipSel)
{
	if (chipSel == 0)
	{
		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003CC00);		// enable NAND NWR/NRD/RB0 pins 
		outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00F30000);		// enable NAND ALE/CLE/CS0 pins 	
		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_CS0); 
		outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_CS1); 		
	}
	else
	{
		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003F000);		// enable NAND NWR/NRD/RB1 pins 
		outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00FC0000);		// enable NAND ALE/CLE/CS1 pins 	
		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_CS1); 
		outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_CS0); 		
	}
	
}

static INT fmiNormalCheckBlock(FMI_SM_INFO_T *pSM, UINT32 BlockNo)
{
	int volatile status=0;
	unsigned int volatile sector;
	unsigned char data, data517;

	_fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x80000000);

	/* MLC check the 2048 byte of last page per block */
	if (pSM->bIsMLCNand == TRUE)
	{
		if (pSM->nPageSize == NAND_PAGE_2KB)
		{
			sector = (BlockNo+1) * pSM->uPagePerBlock - 1;
			/* Read 2048 byte */
			
			status = fmiSM_Read_RA(pSM, sector, 2048);
			if (status < 0)
			{
#ifdef DEBUG
				printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
				return 1;
			}
			data = inpw(REG_SMDATA) & 0xff;
			if (data != 0xFF)
				return 1;	// invalid block
		}
		else if (pSM->nPageSize == NAND_PAGE_4KB)
		{
			sector = (BlockNo+1) * pSM->uPagePerBlock - 1;

			/* Read 4096 byte */
			status = fmiSM_Read_RA(pSM, sector, 4096);
			if (status < 0)
			{
#ifdef DEBUG
				printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
				return 1;
			}
			data = inpw(REG_SMDATA) & 0xff;
			if (data != 0xFF)
				return 1;	// invalid block
		}
	}
	/* SLC check the 2048 byte of 1st or 2nd page per block */
	else	// SLC
	{
		sector = BlockNo * pSM->uPagePerBlock;

		if (pSM->nPageSize == NAND_PAGE_4KB)
		{
			status = fmiSM_Read_RA(pSM, sector, 4096);	
			if (status < 0)
			{
#ifdef DEBUG
				printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
				return 1;
			}
			data = inpw(REG_SMDATA) & 0xff;			
			if (data == 0xFF)
			{
#ifdef DEBUG
//				printf("find bad block, check next page to confirm it.\n");
#endif
				status = fmiSM_Read_RA(pSM, sector+1, 4096);	
				if (status < 0)
				{
#ifdef DEBUG
					printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
					return 1;
				}
				data = inpw(REG_SMDATA) & 0xff;			
				if (data != 0xFF)
				{
#ifdef DEBUG
					printf("find bad block is conformed.\n");
#endif
				
					return 1;	// invalid block
				}
			}
			else
			{
#ifdef DEBUG
				printf("find bad block is conformed.\n");
#endif
				return 1;	// invalid block
			}
		}
		else if (pSM->nPageSize == NAND_PAGE_2KB)
		{
			status = fmiSM_Read_RA(pSM, sector, 2048);	
			if (status < 0)
			{
#ifdef DEBUG
				printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
				return 1;
			}
			data = inpw(REG_SMDATA) & 0xff;			
			if (data == 0xFF)
			{
#ifdef DEBUG
//				printf("find bad block, check next page to confirm it.\n");
#endif
				status = fmiSM_Read_RA(pSM, sector+1, 2048);	
				if (status < 0)
				{
#ifdef DEBUG
					printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
					return 1;
				}
				data = inpw(REG_SMDATA) & 0xff;			
				if (data != 0xFF)
				{
#ifdef DEBUG
					printf("find bad block is conformed.\n");
#endif
				
					return 1;	// invalid block
				}
			}
			else
			{
#ifdef DEBUG
				printf("find bad block is conformed.\n");
#endif
				return 1;	// invalid block
			}
		}
		else	/* page size 512B */
		{
			status = fmiSM2BufferM_RA(pSM, sector, 0);
			if (status < 0)
			{
#ifdef DEBUG
				printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
				return 1;
			}
			data = inpw(REG_SMDATA) & 0xff;
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA);
			data517 = inpw(REG_SMDATA) & 0xff;
	//		if ((data != 0xFF) || (data517 != 0xFF))
			if ((data == 0xFF) && (data517 == 0xFF))	
			{
				fmiSM_Reset(pSM);
				status = fmiSM2BufferM_RA(pSM, sector+1, 0);
				if (status < 0)
				{
#ifdef DEBUG
					printf("fmiNormalCheckBlock 0x%x\n", status);
#endif
					return 1;
				}
				data = inpw(REG_SMDATA) & 0xff;
				data517 = inpw(REG_SMDATA);
				data517 = inpw(REG_SMDATA);
				data517 = inpw(REG_SMDATA);
				data517 = inpw(REG_SMDATA);
				data517 = inpw(REG_SMDATA) & 0xff;
				if ((data != 0xFF) || (data517 != 0xFF))
				{
					fmiSM_Reset(pSM);
					return 1;	// invalid block
				}
			}
			else
			{
#ifdef DEBUG
				printf("find bad block is conformed.\n");
#endif
				fmiSM_Reset(pSM);
				return 1;	// invalid block
			}
			fmiSM_Reset(pSM);
		}
	}

	return 0;
}

/* function pointer */
FMI_SM_INFO_T *pSM0=0, *pSM1=0;
static INT sicSMInit(INT chipSel, NDISK_T *NDISK_info)
{
	int status=0, count;

    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_EN);                                   
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) | DMAC_SWRST);                                   
    outpw(REG_DMACCSR, inpw(REG_DMACCSR) & ~DMAC_SWRST);                                       
    outpw(REG_FMICR, FMI_SM_EN);    
    
	if ((_nand_init0 == 0) && (_nand_init1 == 0))
	{
		// enable SM
		/* select NAND control pin used */
//		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003FC00);		// enable NAND NWR/NRD/RB0/RB1 pins 
//		outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00FF0000);		// enable NAND ALE/CLE/CS0/CS1 pins 	
//		outpw(REG_GPDFUN, inpw(REG_GPDFUN) | 0x0003C000);		// enable NAND NWR/NRD pins 
//		outpw(REG_GPEFUN, inpw(REG_GPEFUN) | 0x00F00000);		// enable NAND ALE/CLE pins 	

//		outpw(REG_SMTCR, 0x20304);
//		outpw(REG_SMTCR, 0x10204);
		outpw(REG_SMTCR, 0x20305);		
		outpw(REG_SMCSR, (inpw(REG_SMCSR) & ~SMCR_PSIZE) | PSIZE_512);
		outpw(REG_SMCSR, inpw(REG_SMCSR) |  SMCR_ECC_3B_PROTECT);		
//		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_ECC_3B_PROTECT);		
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_ECC_CHK);				

		/* init SM interface */
#ifdef _NAND_PAR_ALC_
		outpw(REG_SMCSR, inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);						
#ifdef DEBUG
		printf("Parity only written to SM registers but not NAND !!\n");
#endif
#else
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);						
#ifdef DEBUG
		printf("Parity written to SM registers and NAND !!\n");
#endif
#endif	// _NAND_PAR_ALC_
	}

	sicSMselect(chipSel);
	
	if (chipSel == 0)
	{
		if (_nand_init0)
			return 0;

		pSM0 = malloc(sizeof(FMI_SM_INFO_T));
		if (pSM0 == NULL)
			return FMI_NO_MEMORY;		
		memset((char *)pSM0, 0, sizeof(FMI_SM_INFO_T));

		if ((status = fmiSM_ReadID(pSM0, NDISK_info)) < 0)
		{
			if (pSM0 != NULL)
			{
				free(pSM0);
				pSM0 = 0;
			}
			return status;
		}
		fmiSM_Initial(pSM0);

#ifdef OPT_SW_WP
        outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA7); 		// port A7 low (WP)
        outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~0x0080); 	// port A7 low (WP)
        outpw(REG_GPIOA_OMD, inpw(REG_GPIOA_OMD) | 0x0080); 	// port A7 output
#endif

		// check NAND boot header
		fmiSMCheckBootHeader(0, pSM0);
		while(1)
		{
//#ifdef OPT_SOFTWARE_2KNAND_ECC
//			if (fmiCheckInvalidBlock(pSM0, pSM0->uLibStartBlock) != 1)	// valid block
//				break;
//#else			
			if (!fmiNormalCheckBlock(pSM0, pSM0->uLibStartBlock))	
				break;
//#endif				
			else
			{
#ifdef DEBUG
				printf("invalid start block %d\n", pSM0->uLibStartBlock);
#endif
				pSM0->uLibStartBlock++;
			}
		}
		if (pSM0->bIsCheckECC)
			if (pSM0->uLibStartBlock == 0)
				pSM0->uLibStartBlock++;
		NDISK_info->nStartBlock = pSM0->uLibStartBlock;		/* available start block */
		pSM0->uBlockPerFlash -= pSM0->uLibStartBlock;
		count = NDISK_info->nBlockPerZone * 2 / 100 + NAND_RESERVED_BLOCK;

		NDISK_info->nBlockPerZone = (NDISK_info->nBlockPerZone * NDISK_info->nZone - NDISK_info->nStartBlock) / NDISK_info->nZone;
		NDISK_info->nLBPerZone = NDISK_info->nBlockPerZone - count;
		NDISK_info->nNandNo = chipSel;
		_nand_init0 = 1;
	}
	else if (chipSel == 1)
	{
		if (_nand_init1)
			return 0;

		pSM1 = malloc(sizeof(FMI_SM_INFO_T));
		if (pSM1 == NULL)
			return FMI_NO_MEMORY;		
		memset((char *)pSM1, 0, sizeof(FMI_SM_INFO_T));

		if ((status = fmiSM_ReadID(pSM1, NDISK_info)) < 0)
		{
			if (pSM1 != NULL)
			{
				free(pSM1);
				pSM1 = 0;
			}
			return status;
		}
		fmiSM_Initial(pSM1);
#ifdef OPT_SW_WP
        outpw(REG_GPAFUN, inpw(REG_GPAFUN) & ~MF_GPA7); 		// port A7 low (WP)
        outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~0x0080); 	// port A7 low (WP)
        outpw(REG_GPIOA_OMD, inpw(REG_GPIOA_OMD) | 0x0080); 	// port A7 output
#endif

		// check NAND boot header
		fmiSMCheckBootHeader(1, pSM1);
		while(1)
		{
#ifdef OPT_SOFTWARE_2KNAND_ECC
			if (fmiCheckInvalidBlock(pSM1, pSM1->uLibStartBlock) != 1)	// valid block
				break;
#else			
			if (!fmiNormalCheckBlock(pSM1, pSM1->uLibStartBlock))	
				break;
#endif				
			else
			{
#ifdef DEBUG
				printf("invalid start block %d\n", pSM0->uLibStartBlock);
#endif
				pSM1->uLibStartBlock++;
			}
		}
		if (pSM1->bIsCheckECC)
			if (pSM1->uLibStartBlock == 0)
				pSM1->uLibStartBlock++;
		NDISK_info->nStartBlock = pSM1->uLibStartBlock;		/* available start block */
		pSM1->uBlockPerFlash -= pSM1->uLibStartBlock;
		count = NDISK_info->nBlockPerZone * 2 / 100 + NAND_RESERVED_BLOCK;

		NDISK_info->nBlockPerZone = (NDISK_info->nBlockPerZone * NDISK_info->nZone - NDISK_info->nStartBlock) / NDISK_info->nZone;
		NDISK_info->nLBPerZone = NDISK_info->nBlockPerZone - count;
		NDISK_info->nNandNo = chipSel;
		_nand_init1 = 1;
	}
	else
		return FMI_SM_INIT_ERROR;
		
	return 0;
}
static INT sicSMpread_BCH(INT chipSel, INT PBA, INT page, UINT8 *buff)
{
	FMI_SM_INFO_T *pSM;
	int pageNo;
	int status=0;
	int i;
	char *ptr;
	
	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);		//removed by mhuko

	PBA += pSM->uLibStartBlock;
	pageNo = PBA * pSM->uPagePerBlock + page;

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// set to ECC4 for Block 0-3 	
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	
	
	if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
	{
		ptr = (char *)REG_SMRA_0;
		fmiSM_Read_RA(pSM, pageNo, 2048);
		for (i=0; i<64; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;

		status = fmiSM_Read_2K(pSM, pageNo, (UINT32)buff);
	}
	else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 4KB */
	{
		ptr = (char *)REG_SMRA_0;
		fmiSM_Read_RA(pSM, pageNo, 4096);
//		for (i=0; i<216; i++)
		for (i=0; i<128; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
		
		status = fmiSM_Read_4K(pSM, pageNo, (UINT32)buff);
	}
	else	/* 512B */
	{
		ptr = (char *)REG_SMRA_0;
		fmiSM_Read_RA_512(pSM, pageNo, 0);
		
	#ifdef OPT_RW_RUNDANT_BY_DMA
		outpw(REG_SMREAREA_CTL,  (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 0x10);
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_REN);
	#else		
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
	#endif			
	
		status = fmiSM_Read_512(pSM, pageNo, (UINT32)buff);	
	}	
		
#ifdef DEBUG
	if (status)
		printf("read NAND page fail !!!\n");																			
#endif

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// restore to ECC8 
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);   			// BCH_8 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	

//	sysFlushCache(I_D_CACHE);
//	sysInvalidCache();

	return status;
}

//static INT sicSMpread(INT chipSel, INT PBA, INT page, UINT8 *buff)
static INT sicSMpread(INT chipSel, INT PBA, INT page, UINT8 *buff)
//INT sicSMpread(INT chipSel, INT PBA, INT page, UINT8 *buff)
{
	FMI_SM_INFO_T *pSM;
	int pageNo;
	int status=0;
	int i;
	char *ptr;
	
#ifdef OPT_SOFTWARE_ECC
	volatile char u8EccStatus = 0;
	unsigned char u8Ecc4Parity[10];	
	unsigned int u32ErrCount;
#endif      
	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);		//removed by mhuko

	PBA += pSM->uLibStartBlock;
	pageNo = PBA * pSM->uPagePerBlock + page;

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// set to ECC4 for Block 0-3 	
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	
	
	if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
	{
#ifdef OPT_SOFTWARE_2KNAND_ECC	
		if(pSM->bIsCheckECC)
			u8EccStatus = 1;
		else
			u8EccStatus = 0;					
			
		pSM->bIsCheckECC = 0;
		outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_ECC_EN);				// disable ECC 		
		outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);		// HW bug (if not to disable this bit, DMAC can't be complete)
		outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_512);	// psize:512

		// read 1st 512+16			
		status = fmiSM_Read_2K_SECC_1st512(pSM, pageNo, (UINT32)buff);
		ptr = (char *)REG_SMRA_0;
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;

		// read 2nd 512+16			
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);
		while(inpw(REG_SMCSR) & SMCR_DRD_EN);
	    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	

		ptr = (char *)REG_SMRA_4;
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
			
		// read 3rd 512+16
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);
		while(inpw(REG_SMCSR) & SMCR_DRD_EN);		
	    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	
		
		ptr = (char *)REG_SMRA_8;
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;

		// read 4th 512+16
		outpw(REG_SMCSR, inpw(REG_SMCSR) | SMCR_DRD_EN);
		while(inpw(REG_SMCSR) & SMCR_DRD_EN);		
	    outpw(REG_SMISR, SMISR_DMA_IF);	                // clear DMA flag	
	    outpw(REG_SMISR, SMISR_ECC_FIELD_IF);	        // clear ECC_FIELD flag	

		ptr = (char *)REG_SMRA_12;
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
				
		if (u8EccStatus)
		{
			// decode 1st 512+16
			ptr = (char *)REG_SMRA_0+6;		
			u32ErrCount = Ecc4Decoder(buff, ptr);			
			if(u32ErrCount)									
	//		if( Ecc4Decoder(buff, ptr) != 0 )		
			{//Ecc4Decoder will do Ecc4Encoder first in the internal. If the content of ptr != the parity that was generated by Ecc4Encoder
				sysprintf("Software ECC4 decoder not match.. Do encode \n");				
				sysprintf("Error bit number %d \n", u32ErrCount);				
				Ecc4Encoder(buff, u8Ecc4Parity, FALSE); 				
				if( Ecc4Decoder(buff, u8Ecc4Parity) != 0 )
				{
					sysprintf("Software ECC4 decoder error\n");
				}
			}			
			DBG_PRINTF("Software ECC4 decoder 1st 512+16 Successful\n");			
			
			// decode 2nd 512+16
			ptr = (char *)REG_SMRA_4+6;		
			u32ErrCount = Ecc4Decoder((UINT8*)(UINT32)buff+512, ptr);			
			if(u32ErrCount)									
	//		if( Ecc4Decoder(buff, ptr) != 0 )		
			{//Ecc4Decoder will do Ecc4Encoder first in the internal. If the content of ptr != the parity that was generated by Ecc4Encoder
				sysprintf("Software ECC4 decoder not match.. Do encode \n");				
				sysprintf("Error bit number %d \n", u32ErrCount);				
				Ecc4Encoder(buff, u8Ecc4Parity, FALSE); 				
				if( Ecc4Decoder(buff, u8Ecc4Parity) != 0 )
				{
					sysprintf("Software ECC4 decoder error\n");
				}
			}			
			DBG_PRINTF("Software ECC4 decoder 2nd 512+16 Successful\n");			

			// decode 3rd 512+16
			ptr = (char *)REG_SMRA_8+6;		
			u32ErrCount = Ecc4Decoder((UINT8*)(UINT32)buff+1024, ptr);			
			if(u32ErrCount)									
	//		if( Ecc4Decoder(buff, ptr) != 0 )		
			{//Ecc4Decoder will do Ecc4Encoder first in the internal. If the content of ptr != the parity that was generated by Ecc4Encoder
				sysprintf("Software ECC4 decoder not match.. Do encode \n");				
				sysprintf("Error bit number %d \n", u32ErrCount);				
				Ecc4Encoder(buff, u8Ecc4Parity, FALSE); 				
				if( Ecc4Decoder(buff, u8Ecc4Parity) != 0 )
				{
					sysprintf("Software ECC4 decoder error\n");
				}
			}			
			DBG_PRINTF("Software ECC4 decoder 3rd 512+16 Successful\n");			

			// decode 3rd 512+16
			ptr = (char *)REG_SMRA_12+6;		
			u32ErrCount = Ecc4Decoder((UINT8*)(UINT32)buff+1536, ptr);			
			if(u32ErrCount)									
	//		if( Ecc4Decoder(buff, ptr) != 0 )		
			{//Ecc4Decoder will do Ecc4Encoder first in the internal. If the content of ptr != the parity that was generated by Ecc4Encoder
				sysprintf("Software ECC4 decoder not match.. Do encode \n");				
				sysprintf("Error bit number %d \n", u32ErrCount);				
				Ecc4Encoder(buff, u8Ecc4Parity, FALSE); 				
				if( Ecc4Decoder(buff, u8Ecc4Parity) != 0 )
				{
					sysprintf("Software ECC4 decoder error\n");
				}
			}			
			DBG_PRINTF("Software ECC4 decoder 1st 512+16 Successful\n");			
		}			

		if(u8EccStatus)
			pSM->bIsCheckECC = 1;
		else
			pSM->bIsCheckECC = 0;
			
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_ECC_EN);	// enable ECC 	/* Why enable ECC if Software ECC ??*/ 	
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);	
		outpw(REG_SMCSR, (inpw(REG_SMCSR)&(~SMCR_PSIZE))|PSIZE_2K);	// psize:2048		
		
#else
		ptr = (char *)REG_SMRA_0;
		fmiSM_Read_RA(pSM, pageNo, 2048);
		for (i=0; i<64; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;

		status = fmiSM_Read_2K(pSM, pageNo, (UINT32)buff);
#endif		
	}
	else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 4KB */
	{
		ptr = (char *)REG_SMRA_0;
		fmiSM_Read_RA(pSM, pageNo, 4096);
//		for (i=0; i<216; i++)
		for (i=0; i<128; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
		
		status = fmiSM_Read_4K(pSM, pageNo, (UINT32)buff);
	}
	else	/* 512B */
	{
#ifdef OPT_SOFTWARE_ECC
		if(pSM->bIsCheckECC)
			u8EccStatus = 1;
		else
			u8EccStatus = 0;					
			
		pSM->bIsCheckECC = 0;
		outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_ECC_EN);			// disable ECC 		
		outpw(REG_SMCSR,  inpw(REG_SMCSR) & ~SMCR_REDUN_AUTO_WEN);	// HW bug (if not to disable this bit, DMAC can't be complete)
		status = fmiSM_Read_512(pSM, pageNo, (UINT32)buff);

	#ifdef OPT_RW_RUNDANT_BY_DMA			
		outpw(REG_SMREAREA_CTL,  (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 0x10);
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_REN);
	#else
		ptr = (char *)REG_SMRA_0;
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
	#endif		
	
#else
		ptr = (char *)REG_SMRA_0;
		fmiSM_Read_RA_512(pSM, pageNo, 0);
		
	#ifdef OPT_RW_RUNDANT_BY_DMA
		outpw(REG_SMREAREA_CTL,  (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 0x10);
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_REN);
	#else		
		for (i=0; i<16; i++)
			*ptr++ = inpw(REG_SMDATA) & 0xff;
	#endif			
	
		status = fmiSM_Read_512(pSM, pageNo, (UINT32)buff);	
#endif			
#ifdef OPT_SOFTWARE_ECC
		if (u8EccStatus)
		{
			ptr = (char *)REG_SMRA_0+6;		//ptr is Logical address
			u32ErrCount = Ecc4Decoder(buff, ptr);			
			if(u32ErrCount)									
	//		if( Ecc4Decoder(buff, ptr) != 0 )		
			{//Ecc4Decoder will do Ecc4Encoder first in the internal. If the content of ptr != the parity that was generated by Ecc4Encoder
				sysprintf("Software ECC4 decoder not match.. Do encode \n");				
				sysprintf("Error bit number %d \n", u32ErrCount);				
				Ecc4Encoder(buff, u8Ecc4Parity, FALSE); 				
				if( Ecc4Decoder(buff, u8Ecc4Parity) != 0 )
				{
					sysprintf("Software ECC4 decoder error\n");
				//	DBG_PRINTF("Software ECC4 decoder error\n");				
				//	while(1);
				}
			}			
			DBG_PRINTF("Software ECC4 decoder Successful\n");			
		}			
#endif	
#ifdef OPT_SOFTWARE_ECC
		if(u8EccStatus)
			pSM->bIsCheckECC = 1;
		else
			pSM->bIsCheckECC = 0;
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_ECC_EN);	// enable ECC 	/* Why enable ECC if Software ECC ??*/ 	
		outpw(REG_SMCSR,  inpw(REG_SMCSR) | SMCR_REDUN_AUTO_WEN);	
#endif	
	}	
		
#ifdef DEBUG
	if (status)
		printf("read NAND page fail !!!\n");																			
#endif

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// restore to ECC8 
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);   			// BCH_8 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	

//	sysFlushCache(I_D_CACHE);
//	sysInvalidCache();

	return status;
}
static INT sicSMpwrite_BCH(INT chipSel, INT PBA, INT page, UINT8 *buff)
{
	FMI_SM_INFO_T *pSM;
	int pageNo;
	int status=0;

//	sysFlushCache(D_CACHE);
	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);			//removed by mhuko

	PBA += pSM->uLibStartBlock;
	pageNo = PBA * pSM->uPagePerBlock + page;

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// set to ECC4 for Block 0-3 	
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	
	
	if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
#ifdef _NAND_PAR_ALC_
		status = fmiSM_Write_2K_ALC(pSM, pageNo, 0, (UINT32)buff);
#else
		status = fmiSM_Write_2K_BCH(pSM, pageNo, 0, (UINT32)buff);
#endif
	else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 2KB */
	{
#ifdef _NAND_PAR_ALC_
		status = fmiSM_Write_4K_ALC(pSM, pageNo, 0, (UINT32)buff);
#else
		status = fmiSM_Write_4K(pSM, pageNo, 0, (UINT32)buff);	
#endif
	}
	else	/* 512B */
		status = fmiSM_Write_512_BCH(pSM, pageNo, (UINT32)buff);

#ifdef DEBUG
	if (status)
		printf("write NAND page fail !!!\n");																			
#endif		

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// restore to ECC8 
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);   			// BCH_8 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	

	return status;
}


static INT sicSMpwrite(INT chipSel, INT PBA, INT page, UINT8 *buff)
//INT sicSMpwrite(INT chipSel, INT PBA, INT page, UINT8 *buff)
{
	FMI_SM_INFO_T *pSM;
	int pageNo;
	int status=0;

//	sysFlushCache(D_CACHE);
	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);			//removed by mhuko

	PBA += pSM->uLibStartBlock;
	pageNo = PBA * pSM->uPagePerBlock + page;

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// set to ECC4 for Block 0-3 	
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T4);   			// BCH_4 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	
	
	if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
#ifdef _NAND_PAR_ALC_
		status = fmiSM_Write_2K_ALC(pSM, pageNo, 0, (UINT32)buff);
#else
	#ifdef OPT_SOFTWARE_WRITE_2KNAND_ECC
		status = fmiSM_Write_2K_SECC(pSM, pageNo, 0, (UINT32)buff);
	#else
		status = fmiSM_Write_2K_BCH(pSM, pageNo, 0, (UINT32)buff);	
	#endif		
#endif
	else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 2KB */
	{
#ifdef _NAND_PAR_ALC_
		status = fmiSM_Write_4K_ALC(pSM, pageNo, 0, (UINT32)buff);
#else
		status = fmiSM_Write_4K(pSM, pageNo, 0, (UINT32)buff);	
#endif
	}
	else	/* 512B */
		status = fmiSM_Write_512_SECC(pSM, pageNo, (UINT32)buff);

#ifdef DEBUG
	if (status)
		printf("write NAND page fail !!!\n");																			
#endif		

#ifdef OPT_FIRST_4BLOCKS_ECC4	
	if (PBA <= 3)
	{
		// restore to ECC8 
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		{
			if (pSM->bIsNandECC8 == TRUE)
			{
			    outpw(REG_SMCSR, inpw(REG_SMCSR) &  ~SMCR_BCH_TSEL);   
			    outpw(REG_SMCSR, inpw(REG_SMCSR) | BCH_T8);   			// BCH_8 is selected
			    outpw(REG_SMREAREA_CTL, (inpw(REG_SMREAREA_CTL) & ~SMRE_REA128_EXT) | 64);  // Redundant area size 		    
			}
		}		
	}		
#endif 	

	return status;
}

static INT sicSM_is_page_dirty(INT chipSel, INT PBA, INT page)
{
	FMI_SM_INFO_T *pSM;
	int pageNo;
	UINT8 data0, data1;

	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;
	
	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);		//removed by mhuko

	PBA += pSM->uLibStartBlock;
	pageNo = PBA * pSM->uPagePerBlock + page;
	
#ifdef OPT_SOFTWARE_WRITE_2KNAND_ECC
	if (pSM == pSM0)
	{
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
			fmiSM_Read_RA(pSM, pageNo, 2050);
		else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 4KB */
			fmiSM_Read_RA(pSM, pageNo, 4098);
		else if (pSM->nPageSize == NAND_PAGE_8KB)	/* 8KB */
			fmiSM_Read_RA(pSM, pageNo, 8194);
		else	/* 512B */
			fmiSM_Read_RA_512(pSM, pageNo, 2);
	}
	else
	{
		if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
			fmiSM_Read_RA(pSM, pageNo, 514);
		else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 4KB */
			fmiSM_Read_RA(pSM, pageNo, 4098);
		else if (pSM->nPageSize == NAND_PAGE_8KB)	/* 8KB */
			fmiSM_Read_RA(pSM, pageNo, 8194);
		else	/* 512B */
			fmiSM_Read_RA_512(pSM, pageNo, 2);
	}
#else	
	if (pSM->nPageSize == NAND_PAGE_2KB)	/* 2KB */
		fmiSM_Read_RA(pSM, pageNo, 2050);
	else if (pSM->nPageSize == NAND_PAGE_4KB)	/* 4KB */
		fmiSM_Read_RA(pSM, pageNo, 4098);
	else if (pSM->nPageSize == NAND_PAGE_8KB)	/* 8KB */
		fmiSM_Read_RA(pSM, pageNo, 8194);
	else	/* 512B */
		fmiSM_Read_RA_512(pSM, pageNo, 2);
#endif		

	data0 = inpw(REG_SMDATA);
	data1 = inpw(REG_SMDATA);

	if (pSM->nPageSize == NAND_PAGE_512B)	/* 512B */
		fmiSM_Reset(pSM);

	//mhkuo
//	if ((data0 == 0) && (data1 == 0x00))
//	if ((data0 == 0) || (data1 == 0x00)) 
	if (data0 == 0x00) 
		return 1;	// used page
	else if (data0 != 0xff) 
		return 1;	// used page

	return 0;	// un-used page
}


static INT sicSM_is_valid_block(INT chipSel, INT PBA)
{
	FMI_SM_INFO_T *pSM;

	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	PBA += pSM->uLibStartBlock;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);

	if (fmiNormalCheckBlock(pSM, PBA))	
	{
#ifdef DEBUG
		printf("invalid block %d\n", PBA);
#endif
		return 0;
	}
	else
		return 1;	// valid block
}

#ifdef OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL

static INT sicSMMarkBadBlock_WhileEraseFail(FMI_SM_INFO_T *pSM, UINT32 BlockNo)
{
    UINT32 uSector, ucColAddr;

	/* check if MLC NAND */
	if (pSM->bIsMLCNand == TRUE)
	{
		uSector = (BlockNo+1) * pSM->uPagePerBlock - 1;	// write last page	
		if (pSM->nPageSize == NAND_PAGE_2KB)
		{
			ucColAddr = 2048;		// write 2048th byte
		}
		else if (pSM->nPageSize == NAND_PAGE_4KB)
		{
			ucColAddr = 4096;		// write 4096th byte
		}
		
		// send command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff);	// CA8 - CA11
		outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
		if (!pSM->bIsMulticycle)
			outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
		}
	    outpw(REG_SMDATA, 0xf0);	// mark bad block (use 0xf0 instead of 0x00 to differ from Old (Factory) Bad Blcok Mark)
	    outpw(REG_SMCMD, 0x10);

	    if (! fmiSMCheckRB(pSM))
			return FMI_SM_RB_ERR;

		fmiSM_Reset(pSM);			
	    return 0;			
	}
	/* SLC check the 2048 byte of 1st or 2nd page per block */
	else	// SLC
	{
		uSector = BlockNo * pSM->uPagePerBlock;		// write lst page		
		if (pSM->nPageSize == NAND_PAGE_2KB)
		{
			ucColAddr = 2048;		// write 2048th byte
		}
		else if (pSM->nPageSize == NAND_PAGE_4KB)
		{
			ucColAddr = 4096;		// write 4096th byte
		}
		else if (pSM->nPageSize == NAND_PAGE_512B)
		{
			ucColAddr = 0;			// write 4096th byte		
			goto _mark_512;
		}			
		
		// send command
		outpw(REG_SMCMD, 0x80);		// serial data input command
		outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMADDR, (ucColAddr >> 8) & 0xff);	// CA8 - CA11
		outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
		if (!pSM->bIsMulticycle)
			outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
		}
	    outpw(REG_SMDATA, 0xf0);	// mark bad block (use 0xf0 instead of 0x00 to differ from Old (Factory) Bad Blcok Mark)
	    outpw(REG_SMCMD, 0x10);

	    if (! fmiSMCheckRB(pSM))
			return FMI_SM_RB_ERR;

		fmiSM_Reset(pSM);
    	return 0;
	    	   
_mark_512:
		
		outpw(REG_SMCMD, 0x50);		// point to redundant area
		outpw(REG_SMCMD, 0x80);		// serial data input command		
		outpw(REG_SMADDR, ucColAddr);	// CA0 - CA7
		outpw(REG_SMADDR, uSector & 0xff);	// PA0 - PA7
		if (!pSM->bIsMulticycle)
			outpw(REG_SMADDR, ((uSector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, (uSector >> 8) & 0xff);		// PA8 - PA15
			outpw(REG_SMADDR, ((uSector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
		}

	    outpw(REG_SMDATA, 0xf0);	// 512
	    outpw(REG_SMDATA, 0xff);
	    outpw(REG_SMDATA, 0xff);
	    outpw(REG_SMDATA, 0xff);
	    outpw(REG_SMDATA, 0xf0);	// 516
	    outpw(REG_SMDATA, 0xf0);	// 517
	    outpw(REG_SMCMD, 0x10);
	    if (! fmiSMCheckRB(pSM))
			return FMI_SM_RB_ERR;

		fmiSM_Reset(pSM);
    	return 0;
	}		
}
#endif		// OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL

INT sicSMMarkBadBlock(FMI_SM_INFO_T *pSM, UINT32 BlockNo)
{
    UINT32 sector, column;

	/* page 0 */
    sector = BlockNo * pSM->uPagePerBlock;
    column = 512;

	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, column);	// CA0 - CA7
	outpw(REG_SMADDR, (column >> 8) & 0x1f);	// CA8 - CA12
	outpw(REG_SMADDR, sector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((sector >> 8) & 0xff)|0x80000000);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (sector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((sector >> 16) & 0xff)|0x80000000);		// PA16 - PA17
	}

    outpw(REG_SMDATA, 0xf0);	// 512
    outpw(REG_SMDATA, 0xff);
    outpw(REG_SMDATA, 0xff);
    outpw(REG_SMDATA, 0xff);
    outpw(REG_SMDATA, 0xf0);	// 516
    outpw(REG_SMDATA, 0xf0);	// 517
    outpw(REG_SMCMD, 0x10);
    if (! fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;
   fmiSM_Reset(pSM);

    /* page 1 */
    sector++;
	// send command
	outpw(REG_SMCMD, 0x80);		// serial data input command
	outpw(REG_SMADDR, column);	// CA0 - CA7
	outpw(REG_SMADDR, (column >> 8) & 0x1f);	// CA8 - CA12
	outpw(REG_SMADDR, sector & 0xff);	// PA0 - PA7
	if (!pSM->bIsMulticycle)
		outpw(REG_SMADDR, ((sector >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
	else
	{
		outpw(REG_SMADDR, (sector >> 8) & 0xff);		// PA8 - PA15
		outpw(REG_SMADDR, ((sector >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
	}

    outpw(REG_SMDATA, 0xf0);	// 512
    outpw(REG_SMDATA, 0xff);
    outpw(REG_SMDATA, 0xff);
    outpw(REG_SMDATA, 0xff);
    outpw(REG_SMDATA, 0xf0);	// 516
    outpw(REG_SMDATA, 0xf0);	// 517
    outpw(REG_SMCMD, 0x10);
    if (! fmiSMCheckRB(pSM))
		return FMI_SM_RB_ERR;
   fmiSM_Reset(pSM);

    return 0;
}

static INT sicSMChangeBadBlockMark(INT chipSel)
{
	int i, status=0;
	FMI_SM_INFO_T *pSM;

	_fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x80000000);

	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	// enable SM
	outpw(REG_FMICR, 0x08);
	fmiSM_Initial(pSM);

//#ifndef OPT_FA93	
#ifdef OPT_SOFTWARE_WRITE_2KNAND_ECC
	// scan all nand chip
	if (pSM == pSM1)
	{
		for (i=1; i<=pSM->uBlockPerFlash; i++)
		{
			if (fmiNormalCheckBlock(pSM, i))	// bad block
			{
				if (pSM->nPageSize == NAND_PAGE_2KB)		
				{
					if (sicSMMarkBadBlock(pSM, i) < 0)
						return FMI_SM_MARK_BAD_BLOCK_ERR;
				}
			}			
		}
	}		
#endif	

	/* read physical block 0 - image information */
#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
	if (!chipSel)
		status = sicSMpread_BCH(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);		
	else
		status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);	
#else	
//	status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-1, _fmi_ucSMBuffer);
	status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
#endif	
	if (status < 0)
		return status;

	/* write specific mark */
	_fmi_ucSMBuffer[pSM->nPageSize-6] = '5';
	_fmi_ucSMBuffer[pSM->nPageSize-5] = '5';
	_fmi_ucSMBuffer[pSM->nPageSize-4] = '0';
	_fmi_ucSMBuffer[pSM->nPageSize-3] = '0';
	_fmi_ucSMBuffer[pSM->nPageSize-2] = '9';
	_fmi_ucSMBuffer[pSM->nPageSize-1] = '1';

	// remove write "550091" in FA93
//	sicSMpwrite(chipSel, 0, pSM->uPagePerBlock-1, _fmi_ucSMBuffer);
//	status = sicSMpwrite(chipSel, 0, pSM->uPagePerBlock-1, _fmi_ucSMBuffer);	//mhkuo@20100730

#ifdef OPT_SOFTWARE_WRITE_2KNAND_ECC
	if (pSM == pSM1)
	{
		if (pSM->nPageSize == NAND_PAGE_2KB)
			status = sicSMpwrite(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);	//mhkuo@20100730
	}			
#endif

	return status;
}

//static INT sicSMblock_erase(INT chipSel, INT PBA)
INT sicSMblock_erase(INT chipSel, INT PBA)
{
	FMI_SM_INFO_T *pSM;
	UINT32 page_no;

	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;
	
	PBA += pSM->uLibStartBlock;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);

	if (fmiCheckInvalidBlock(pSM, PBA) != 1)
//	if (1)
	{
		page_no = PBA * pSM->uPagePerBlock;		// get page address

		/* clear R/B flag */
		if (pSM == pSM0)
		{		
			while(!(inpw(REG_SMISR) & SMISR_RB0));
			outpw(REG_SMISR, SMISR_RB0_IF);
		}
		else
		{
			while(!(inpw(REG_SMISR) & SMISR_RB1));
			outpw(REG_SMISR, SMISR_RB1_IF);
		}		

		if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)	/* ECC_FLD_IF */
		{
#ifdef DEBUG
			printf("erase: error sector !!\n");
#endif
			outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
		}

		outpw(REG_SMCMD, 0x60);		// erase setup command

		outpw(REG_SMADDR, (page_no & 0xff));		// PA0 - PA7
		if (!pSM->bIsMulticycle)
			outpw(REG_SMADDR, ((page_no >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, ((page_no >> 8) & 0xff));		// PA8 - PA15
			outpw(REG_SMADDR, ((page_no >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
		}
	
		outpw(REG_SMCMD, 0xd0);		// erase command

		if (!fmiSMCheckRB(pSM))
			return FMI_SM_RB_ERR;

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("sicSMblock_erase error!!\n");
#endif

#ifdef OPT_MARK_BAD_BLOCK_WHILE_ERASE_FAIL
	#ifdef OPT_SOFTWARE_WRITE_2KNAND_ECC
			if ( (pSM == pSM1)&&(pSM->nPageSize == NAND_PAGE_2KB) )
			{
				if (sicSMMarkBadBlock(pSM, PBA) < 0)
					return FMI_SM_MARK_BAD_BLOCK_ERR;
			}
			else
				sicSMMarkBadBlock_WhileEraseFail(pSM,PBA); 										
	#else
			sicSMMarkBadBlock_WhileEraseFail(pSM,PBA); 							
	#endif	
			
#endif			
			return FMI_SM_STATUS_ERR;
		}
	}
	else
		return FMI_SM_INVALID_BLOCK;

	return 0;
}


//static INT sicSMblock_erase(INT chipSel, INT PBA)
INT sicSMblock_erase_test(INT chipSel, INT PBA)
{
	FMI_SM_INFO_T *pSM;
	UINT32 page_no;

	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;
	

	// enable SM
	outpw(REG_FMICR, 0x08);
	fmiSM_Initial(pSM);

		page_no = PBA * pSM->uPagePerBlock;		// get page address

		/* clear R/B flag */
		if (pSM == pSM0)
		{		
			while(!(inpw(REG_SMISR) & SMISR_RB0));
			outpw(REG_SMISR, SMISR_RB0_IF);
		}
		else
		{
			while(!(inpw(REG_SMISR) & SMISR_RB1));
			outpw(REG_SMISR, SMISR_RB1_IF);
		}		

		if (inpw(REG_SMISR) & SMISR_ECC_FIELD_IF)	/* ECC_FLD_IF */
		{
#ifdef DEBUG
			printf("erase: error sector !!\n");
#endif
			outpw(REG_SMISR, SMISR_ECC_FIELD_IF);
		}

		outpw(REG_SMCMD, 0x60);		// erase setup command

		outpw(REG_SMADDR, (page_no & 0xff));		// PA0 - PA7
		if (!pSM->bIsMulticycle)
			outpw(REG_SMADDR, ((page_no >> 8) & 0xff)|EOA_SM);		// PA8 - PA15
		else
		{
			outpw(REG_SMADDR, ((page_no >> 8) & 0xff));		// PA8 - PA15
			outpw(REG_SMADDR, ((page_no >> 16) & 0xff)|EOA_SM);		// PA16 - PA17
		}
	
		outpw(REG_SMCMD, 0xd0);		// erase command

		if (!fmiSMCheckRB(pSM))
			return FMI_SM_RB_ERR;

		outpw(REG_SMCMD, 0x70);		// status read command
		if (inpw(REG_SMDATA) & 0x01)	// 1:fail; 0:pass
		{
#ifdef DEBUG
			printf("sicSMblock_erase error!!\n");
#endif
			return FMI_SM_STATUS_ERR;
		}

	return 0;
}

static INT sicSMchip_erase(INT chipSel)
{
	int i, status=0;
	FMI_SM_INFO_T *pSM;

	sicSMselect(chipSel);
	if (chipSel == 0)
		pSM = pSM0;
	else
		pSM = pSM1;

	// enable SM
	outpw(REG_FMICR, FMI_SM_EN);
	fmiSM_Initial(pSM);

	// erase all chip
	for (i=0; i<=pSM->uBlockPerFlash; i++)
	{
		status = sicSMblock_erase(chipSel, i);
#ifdef DEBUG
		if (status < 0)
			printf("SM block erase fail <%d>!!\n", i);
#endif
	}

//	return status;
	return 0;
}

/* driver function */
INT nandInit0(NDISK_T *NDISK_info)
{
	return (sicSMInit(0, NDISK_info));
}

INT nandpread0(INT PBA, INT page, UINT8 *buff)
{
#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
	return (sicSMpread_BCH(0, PBA, page, buff));
#else
	return (sicSMpread(0, PBA, page, buff));
#endif	
}

INT nandpwrite0(INT PBA, INT page, UINT8 *buff)
{
#ifdef OPT_SW_WP

	int status;
    UINT32 ii;
	
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x0080); 	// port A7 high (WP)
    for (ii=0; ii<SW_WP_DELAY_LOOP; ii++);	    

	#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
		status = sicSMpwrite_BCH(0, PBA, page, buff);	
	#else    
		status = sicSMpwrite(0, PBA, page, buff);
	#endif	

    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~ 0x0080); 	// port A7 low (WP)	
	return status;
#else
		
	#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
		return (sicSMpwrite_BCH(0, PBA, page, buff));
	#else
		return (sicSMpwrite(0, PBA, page, buff));
	#endif	
#endif	
}

INT nand_is_page_dirty0(INT PBA, INT page)
{
	return (sicSM_is_page_dirty(0, PBA, page));
}

INT nand_is_valid_block0(INT PBA)
{
	return (sicSM_is_valid_block(0, PBA));
}

INT nand_block_erase0(INT PBA)
{
#ifdef OPT_SW_WP
	int status;
    UINT32 ii;
	
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x0080); 	// port A7 high (WP)
    for (ii=0; ii<SW_WP_DELAY_LOOP; ii++);	    
	status = sicSMblock_erase(0, PBA);
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~ 0x0080); 	// port A7 low (WP)	
	return status;
#else
	return (sicSMblock_erase(0, PBA));
#endif	
}

INT nand_chip_erase0(VOID)
{
#ifdef OPT_SW_WP
	int status;
    UINT32 ii;
	
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x0080); 	// port A7 high (WP)
    for (ii=0; ii<SW_WP_DELAY_LOOP; ii++);	    
	status = sicSMchip_erase(0);
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~ 0x0080); 	// port A7 low (WP)	
	return status;
#else
	return (sicSMchip_erase(0));
#endif	
}

INT nand_ioctl(INT param1, INT param2, INT param3, INT param4)
{
	return 0;
}

INT fmiSMCheckBootHeader(INT chipSel, FMI_SM_INFO_T *pSM)
{
	int fmiNandSysArea=0;
	int volatile status, imageCount, i, block;
	unsigned int *pImageList;
	volatile int ii;
	
#define OPT_FOUR_BOOT_IMAGE	

	_fmi_pSMBuffer = (UINT8 *)((UINT32)_fmi_ucSMBuffer | 0x80000000);
	pImageList = (UINT32 *)((UINT32)_fmi_ucSMBuffer | 0x80000000);

	/* read physical block 0 - image information */
	
#ifdef OPT_FOUR_BOOT_IMAGE
	for (ii=0; ii<4; ii++)
	{
	#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
		if (!chipSel)
			status = sicSMpread_BCH(chipSel, ii, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
		else
			status = sicSMpread(chipSel, ii, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
	#else	
		status = sicSMpread(chipSel, ii, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
	#endif	
		
		if (!status)
		{
			if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))		
				break;
		}
	}		
#else	
	#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
		if (!chipSel)
			status = sicSMpread_BCH(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
		else
			status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
	#else	
	//	status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-1, _fmi_ucSMBuffer);
		status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-1, _fmi_pSMBuffer);
	#endif	
#endif	
	
	if (status < 0)
		return status;

	/* check specific mark */
	if (pSM->nPageSize != NAND_PAGE_512B)
	{
		if (((_fmi_ucSMBuffer[pSM->nPageSize-6]) == '5') && ((_fmi_ucSMBuffer[pSM->nPageSize-5]) == '5') &&
			((_fmi_ucSMBuffer[pSM->nPageSize-4]) == '0') && ((_fmi_ucSMBuffer[pSM->nPageSize-3]) == '0') &&
			((_fmi_ucSMBuffer[pSM->nPageSize-2]) == '9') && ((_fmi_ucSMBuffer[pSM->nPageSize-1]) == '1'))
		{
			_fmi_bIsNandFirstAccess = FALSE;
		}
		else
		{
			sicSMChangeBadBlockMark(chipSel);
		}
	}

	if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))
	{
		fmiNandSysArea = *(pImageList+1);
	}

	if ((fmiNandSysArea != 0xFFFFFFFF) && (fmiNandSysArea != 0))
	{
		pSM->uLibStartBlock = (fmiNandSysArea / pSM->uSectorPerBlock) + 1;
	}
	else
	{
		/* read physical block 0 - image information */

#ifdef OPT_FOUR_BOOT_IMAGE
	#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
		for (ii=0; ii<4; ii++)
		{
			if (!chipSel)
				status = sicSMpread_BCH(chipSel, ii, pSM->uPagePerBlock-2, _fmi_pSMBuffer);
			else
				status = sicSMpread(chipSel, ii, pSM->uPagePerBlock-2, _fmi_pSMBuffer);
			if (!status)
			{
				if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))		
					break;
			}
		}		
	#else	
		for (ii=0; ii<4; ii++)
		{
			status = sicSMpread(chipSel, ii, pSM->uPagePerBlock-2, _fmi_pSMBuffer);
			if (!status)
			{
				if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))		
					break;
			}
		}			
	#endif
#else
	#ifdef OPT_BCH_PLUS_SECC_FOR_NAND512
			if (!chipSel)
				status = sicSMpread_BCH(chipSel, 0, pSM->uPagePerBlock-2, _fmi_pSMBuffer);
			else
				status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-2, _fmi_pSMBuffer);
	#else	
	//		status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-2, _fmi_ucSMBuffer);
			status = sicSMpread(chipSel, 0, pSM->uPagePerBlock-2, _fmi_pSMBuffer);
	#endif
#endif		

		if (status < 0)
			return status;

		if (((*(pImageList+0)) == 0x574255aa) && ((*(pImageList+3)) == 0x57425963))
		{
			imageCount = *(pImageList+1);

			/* pointer to image information */
			pImageList = pImageList+4;
			for (i=0; i<imageCount; i++)
			{
				block = (*(pImageList + 1) & 0xFFFF0000) >> 16;
				if (block > pSM->uLibStartBlock)
					pSM->uLibStartBlock = block;

				/* pointer to next image */
				// pImageList = pImageList+12;
				if (chipSel ==0)
					pImageList = pImageList+12;
				else					
					pImageList = pImageList+8;
			}
			pSM->uLibStartBlock++;
		}
	}

	return 0;
}

VOID fmiSMClose(INT chipSel)
{
	if (chipSel == 0)
	{
		_nand_init0 = 0;
		if (pSM0 != 0)
		{
			free(pSM0);
			pSM0 = 0;
		}
	}
	else
	{
		_nand_init1 = 0;
		if (pSM1 != 0)
		{
			free(pSM1);
			pSM1 = 0;
		}
	}
	if ((_nand_init0 == 0) && (_nand_init1 == 0))
	{
		outpw(REG_SMCSR, inpw(REG_SMCSR)|0x06000000);		// CS-0/CS-1 -> HIGH
		outpw(REG_SMISR, 0xfff);
		outpw(REG_FMICR, 0x00);

		outpw(REG_GPDFUN, inpw(REG_GPDFUN) & ~0x0003FC00);		// enable NAND NWR/NRD/RB0/RB1 pins 
		outpw(REG_GPEFUN, inpw(REG_GPEFUN) & ~0x00FF0000);		// enable NAND ALE/CLE/CS0/CS1 pins 	
	}
}


INT nandInit1(NDISK_T *NDISK_info)
{
	return (sicSMInit(1, NDISK_info));
}

INT nandpread1(INT PBA, INT page, UINT8 *buff)
{
	return (sicSMpread(1, PBA, page, buff));
}

INT nandpwrite1(INT PBA, INT page, UINT8 *buff)
{
#ifdef OPT_SW_WP

	int status;
    UINT32 ii;	
			
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x0080); 	// port A7 high (WP)
    for (ii=0; ii<SW_WP_DELAY_LOOP; ii++);    

	status = sicSMpwrite(1, PBA, page, buff);

    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~ 0x0080); 	// port A7 low (WP)	
	return status;
#else
	return (sicSMpwrite(1, PBA, page, buff));
#endif	
}

INT nand_is_page_dirty1(INT PBA, INT page)
{
	return (sicSM_is_page_dirty(1, PBA, page));
}

INT nand_is_valid_block1(INT PBA)
{
	return (sicSM_is_valid_block(1, PBA));
}

INT nand_block_erase1(INT PBA)
{
#ifdef OPT_SW_WP
	int status;
    UINT32 ii;	
	
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x0080); 	// port A7 high (WP)
    for (ii=0; ii<SW_WP_DELAY_LOOP; ii++);    
	status = sicSMblock_erase(1, PBA);
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~ 0x0080); 	// port A7 low (WP)	
	return status;
#else
	return (sicSMblock_erase(1, PBA));
#endif	
}

INT nand_chip_erase1(VOID)
{
#ifdef OPT_SW_WP
	int status;
    UINT32 ii;	
	
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) | 0x0080); 	// port A7 high (WP)
    for (ii=0; ii<SW_WP_DELAY_LOOP; ii++);    
	status = sicSMchip_erase(1);
    outpw(REG_GPIOA_DOUT, inpw(REG_GPIOA_DOUT) & ~ 0x0080); 	// port A7 low (WP)	
	return status;
#else
	return (sicSMchip_erase(1));
#endif	
}


