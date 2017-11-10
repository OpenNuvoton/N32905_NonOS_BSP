#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_reg.h"
#include "usbd.h"

void vcomInit(void);
void VCOM_MainProcess(void);
	
int main(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;
			
	u32ExtFreq = sysGetExternalClock();    	/* Hz unit */		
	uart.uiFreq = u32ExtFreq * 1000;
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);	
	
    sysprintf("\nVirtual COM Demo\n");	
	/* Enable USB */
	udcOpen(); 
	vcomInit();
	udcInit();
	VCOM_MainProcess();
}

