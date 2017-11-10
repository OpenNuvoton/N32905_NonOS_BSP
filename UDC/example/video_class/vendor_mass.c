#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_reg.h"
#include "usbd.h"
#include "mass_storage_class.h"

#ifndef __RAM_DISK_ONLY__
	#include "nvtfat.h"
	#include "w55fa93_sic.h"
	#include "w55fa93_gnand.h"
#endif

BOOL USB_Check(VOID)
{
	return TRUE;
}

VOID vendor_mass(VOID)
{
	INT32 status;

	udcOpen(); 	
	mscdInit();		
	mscdFlashInit(NULL,status);	
	udcInit();
	mscdMassEvent(USB_Check);	
	mscdDeinit();	
	udcDeinit();	
	udcClose();
}

