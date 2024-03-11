/**************************************************************************//**
 * @file     kpi.c
 * @brief    N3290x series keypad demo code
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wblib.h"

#include "W55FA93_KPI.h"

int main(void)
{

	WB_UART_T uart;	
	UINT32 u32ExtFreq;
	unsigned int key;
	
	u32ExtFreq = sysGetExternalClock();
	sysUartPort(1);	
	uart.uiFreq = u32ExtFreq*1000;	
	uart.uiBaudrate = 115200;
	uart.uiDataBits = WB_DATA_BITS_8;
	uart.uiStopBits = WB_STOP_BITS_1;
	uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
	sysInitializeUART(&uart);

	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
						192000,		//UINT32 u32PllKHz, 	
						192000,		//UINT32 u32SysKHz,
						192000,		//UINT32 u32CpuKHz,
						  96000,		//UINT32 u32HclkKHz,
						  48000);		//UINT32 u32ApbKHz		

	sysprintf("start kpi test...\n");

	kpi_init();
	kpi_open(3); // use nIRQ0 as external interrupt source
	
	while(1) {
#ifdef _KPI_NONBLOCK_
		key = kpi_read(KPI_NONBLOCK);
#else
		key = kpi_read(KPI_BLOCK);
#endif		
		sysprintf("key is %d\n", key);
		sysDelay(20);
	
	}
	kpi_close();
	sysprintf("quit kpi test...\n");
	return(0);
	
}
