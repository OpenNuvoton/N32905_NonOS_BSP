#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_vpost.h"
#include "AviLib.h"
#include "nvtfat.h"
#include "nvtloader.h"
#include "w55fa93_kpi.h"
#include "w55fa93_gpio.h"
#include "InitFont.h"

__align(32) static S_DEMO_FONT s_sDemo_Font;
__align(32) UINT16 FrameBuffer[480*272];


void NVT_MassProduction(void)
{
	initVPost(FrameBuffer);
	
	int result;
	while(1)
	{
		result = kpi_read(KPI_NONBLOCK);
		sysprintf("0x%x\n", result);
	}
}	