/**************************************************************************//**
 * @file     mass.c
 * @brief    Updating storage's content through USB mass-storage protocol
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "usbd.h"
#include "mass_storage_class.h"
#include "W55FA93_SIC.h"
#include "NVTFAT.h"
#include "nvtloader.h"
extern int g_ibr_boot_sd_port; 

BOOL connected(void)
{
    return(udcIsAttached());
}


void mass(NDISK_T *disk, INT32 i32TotalSector)
{
#if 0
	WB_CLKFREQ_T clock;		
	
	udcFlashInit((char *)disk);  	
    sysGetClockFreq(&clock);
    	udcInit(clock.pll_clk_freq);	
	udcMassBulk((T_CallbackFunc*) connected);	
	udcDeinit();
	return;
#else	

#if 0
	/* initial nuvoton file system */
	fsInitFileSystem();
	fsAssignDriveNumber('C', DISK_TYPE_SMART_MEDIA, 0, 1);     // NAND 0, 2 partitions
	fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 2);     // NAND 0, 2 partitions

	/* initialize FMI (Flash memory interface controller) */
	sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);  /* clock from PLL */
	sicOpen();
#endif
  	
	mscdInit();	
#if defined(__ENABLE_SD_CARD_0__)||defined(__ENABLE_SD_CARD_1__)||defined(__ENABLE_SD_CARD_2__)		
	mscdSdPortSelect(g_ibr_boot_sd_port);
	mscdFlashInit(NULL, i32TotalSector);	
#elif defined(__SPI_ONLY__)
  mscdFlashInit(NULL, i32TotalSector);
#else	
	mscdFlashInit((NDISK_T *)disk, i32TotalSector);
#endif			
	udcInit();
	mscdMassEvent(connected);
	udcDeinit();
	udcClose();	
#endif	
}
