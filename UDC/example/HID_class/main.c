#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_reg.h"
#include "usbd.h"
#include "HID.h"
#ifdef HID_KEYPAD  

#include "w55fa93_kpi.h"
#include "w55fa93_gpio.h"
#endif
	
int main(void)
{
	WB_UART_T uart;
	UINT32 u32ExtFreq;
	
	u32ExtFreq = sysGetExternalClock();
	uart.uiFreq = u32ExtFreq*1000;	
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
    uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);

    sysprintf("Sample code Start\n");	

#ifdef HID_KEYPAD  
	kpi_init();
	kpi_open(3); // use nIRQ0 as external interrupt source
	
#endif
	/* Enable USB */
	udcOpen();  
	
	hidInit();
	udcInit();
	
	while(1)
	{
#ifdef HID_KEYBOARD
		HID_SetInReport();
#endif
#ifdef HID_MOUSE
		HID_UpdateMouseData();
#endif	
	};
}

