/****************************************************************************
*                                                                           *
* Copyright (c) 2009 Nuvoton Tech. Corp. All rights reserved.               *
*                                                                           *
*****************************************************************************/

/****************************************************************************
* FILENAME
*   libkpi.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   KPI library source file
*
* DATA STRUCTURES
*   None
*
* FUNCTIONS
*
* HISTORY
*
* REMARK
*   None
****************************************************************************/
#include "w55fa93_gpio.h"
#include "w55fa93_kpi.h"

// latest key pressed recorded by ISR, might _not_ be the most updated status
static unsigned int _key;
// interrupt number for kpi
static unsigned char _int;
static unsigned char _opened = 0;

//#define CONFIG_FA93_1X4_KPI
//#define CONFIG_FA93_2X2_KPI
//#define CONFIG_FA93_2X3_KPI

static unsigned int readkey(void)
{
#ifdef CONFIG_FA93_1X4_KPI
	unsigned int read0;
	gpio_readport(GPIO_PORTA, (unsigned short *)&read0);
	read0 &= 0x3C;
	if(read0 == 0x3C)
		return(0);

	return ((~read0 & 0x3C) >> 2);
#elif defined CONFIG_FA93_2X2_KPI
	unsigned int read0, read1, read2;
	gpio_readport(GPIO_PORTA, (unsigned short *)&read0);
	read0 = ((read0>>2) & 0x01) | ((read0>>6) & 0x02);
	if(read0 == 0x03)
		return(0);
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6), (1 << 5));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read1);
	read1 = ((read1>>2) & 0x01) | ((read1>>6) & 0x02);
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6), (1 << 6));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read2);
	read2 = ((read2>>2) & 0x01) | ((read2>>6) & 0x02);
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6), 0);
	
	return((read0 ^ read1) | ((read0 ^ read2)<<2));
#elif defined CONFIG_FA93_2X3_KPI
	unsigned int read0, read1, read2;
	gpio_readport(GPIO_PORTA, (unsigned short *)&read0);
	read0 &= 0xE0;
	if(read0 == 0xE0)
		return(0);
	gpio_setportval(GPIO_PORTA, (1 << 2), (1 << 2));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read1);	
	read1 &= 0xE0;
	if(read1 == 0xE0)
		read2 = (read0 ^ read1)>>5;
	else
		read2 = ((~read0 & 0xE0)>>2);
	
	gpio_setportval(GPIO_PORTA, (1 << 2), 0);
	
	return read2;
#else
	unsigned int read0, read1, read2, read3;
	gpio_readport(GPIO_PORTA, (unsigned short *)&read0);
	read0 &= 0x1C;
	if(read0 == 0x1C)
		return(0);
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), (1 << 5));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read1);	
	read1 &= 0x1C;
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), (1 << 6));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read2);
	read2 &= 0x1C;
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), (1 << 7));
	gpio_readport(GPIO_PORTA, (unsigned short *)&read3);
	read3 &= 0x1C;
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), 0);
	
	return(((read0 ^ read1) >> 2) | (((read0 ^ read2) >> 2)<<3) | (((read0 ^ read3) >> 2)<<6));
#endif	
}

static void kpi_isr(void)
{	
	_key = readkey();
	//(*(unsigned int volatile *)REG_AIC_SCCR) = (1 << (_int + 2));
	gpio_cleartriggersrc(GPIO_PORTA);
	
	return;

}

void kpi_init(void)
{

	_opened = 1;
	
#ifdef CONFIG_FA93_1X4_KPI 
	// PORTA[2-5]
	gpio_setportdir(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)), 0);
	gpio_setportpull(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)), 0);	
	gpio_setintmode(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)), 
					((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)), ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)));
#elif defined CONFIG_FA93_2X2_KPI
	// PORTA[2,7]
	gpio_setportdir(GPIO_PORTA, ((1 << 2) | (1 << 7)), 0);
	gpio_setportpull(GPIO_PORTA, ((1 << 2) | (1 << 7)), 0);	
	gpio_setintmode(GPIO_PORTA, ((1 << 2) | (1 << 7)), 
					((1 << 2) | (1 << 7)), ((1 << 2) | (1 << 7)));
        
	// PORTA[5-6]	
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6), 0);
	gpio_setportpull(GPIO_PORTA, (1 << 5) | (1 << 6), 0);
	gpio_setportdir(GPIO_PORTA, (1 << 5) | (1 << 6), (1 << 5) | (1 << 6));

#elif defined CONFIG_FA93_2X3_KPI
	// PORTA[5-7]
	gpio_setportdir(GPIO_PORTA, ((1 << 5) | (1 << 6) | (1 << 7)), 0);
	gpio_setportpull(GPIO_PORTA, ((1 << 5) | (1 << 6) | (1 << 7)), 0);	
	gpio_setintmode(GPIO_PORTA, ((1 << 5) | (1 << 6) | (1 << 7)), 
					((1 << 5) | (1 << 6) | (1 << 7)), ((1 << 5) | (1 << 6) | (1 << 7)));
        
	// PORTA[2]	
	gpio_setportval(GPIO_PORTA, (1 << 2), 0);
	gpio_setportpull(GPIO_PORTA, (1 << 2), 0);
	gpio_setportdir(GPIO_PORTA, (1 << 2), (1 << 2));

#else
	// PORTA[2-4]
	gpio_setportdir(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), 0);
	gpio_setportpull(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), 0);	
	gpio_setintmode(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), 
					((1 << 2) | (1 << 3) | (1 << 4)), ((1 << 2) | (1 << 3) | (1 << 4)));
        
	// PORTA[5-7]	
	gpio_setportval(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), 0);
	gpio_setportpull(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), 0);
	gpio_setportdir(GPIO_PORTA, (1 << 5) | (1 << 6) | (1 << 7), (1 << 5) | (1 << 6) | (1 << 7));

#endif

    return;    
}

int kpi_open(unsigned int src)
{
	if(_opened == 0)
		kpi_init();
	if(_opened != 1)
		return(-1);
	
	_opened = 2;
	if(src > 3)
		return(-1);
		
	_int = src;
	
#ifdef CONFIG_FA93_1X4_KPI
	gpio_setsrcgrp(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5)), src);
#elif defined CONFIG_FA93_2X2_KPI
	gpio_setsrcgrp(GPIO_PORTA, ((1 << 2) | (1 << 7)), src);
#elif defined CONFIG_FA93_2X3_KPI
	gpio_setsrcgrp(GPIO_PORTA, ((1 << 5) | (1 << 6) | (1 << 7)), src);
#else
	gpio_setsrcgrp(GPIO_PORTA, ((1 << 2) | (1 << 3) | (1 << 4)), src);
#endif
	gpio_setdebounce(128, 1 << src);
	gpio_setlatchtrigger(1 << src);

	sysInstallISR(IRQ_LEVEL_7, src + 2, (PVOID)kpi_isr);	
		
	sysSetInterruptType(src + 2, HIGH_LEVEL_SENSITIVE);
	sysSetLocalInterrupt(ENABLE_IRQ);	

    return(0);    
}

void kpi_close(void)
{
	if(_opened != 2)
		return;
	_opened = 1;
	sysDisableInterrupt(_int + 2);  
	return;
}


int kpi_read(unsigned char mode)
{
	// add this var in case key released right before return.
	int volatile k = 0;
	
	if(_opened != 2)
		return(-1);
	
	if(mode != KPI_NONBLOCK && mode != KPI_BLOCK) {
		//sysDisableInterrupt(_int + 2);  
		return(-1);
	}
	sysEnableInterrupt(_int + 2);
	
	if(_key == 0) {
		// not pressed, non blocking, return immediately
		if(mode == KPI_NONBLOCK) {
			sysDisableInterrupt(_int + 2);  
			return(0);
		}
		// not pressed, non blocking, wait for key pressed
#pragma O0
// ARMCC is tooooo smart to compile this line correctly, so ether set O0 or use pulling....
		while((k = _key) == 0);
	} else {
		// read latest key(s) and return.
		sysDisableInterrupt(_int + 2);
		do {		
			k = readkey();		
		} while(k == 0 && mode != KPI_NONBLOCK);
	}
	return(k);
}


