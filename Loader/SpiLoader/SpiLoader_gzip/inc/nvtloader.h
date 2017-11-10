/****************************************************************************
*                                                                           *
* Copyright (c) 2009 Nuvoton Tech. Corp. All rights reserved.               *
*                                                                           *
*****************************************************************************/

#define KERNEL_PATH_SD 	"x:\\conprog.bin"
#define MOVIE_PATH_SD 	"x:\\movie.avi"
#define LOGO_PATH_SD	"x:\\logo.bin"

#define KERNEL_PATH 	"c:\\conprog.bin"
#define LOGO_PATH 		"c:\\logo.bin"
#define MOVIE_PATH 		"c:\\movie.avi"
#define VOLUME_PATH		"c:\\volume.cfg"

#define CP_SIZE 16 * 1024


/* PLL clock option */
//#deinfe __CLK_CTL__			/* Clock change in NVT-Loader */ 
#ifdef __CLK_CTL__
	/* Clock Skew */ 
	#define E_CLKSKEW	0x00888800
		
	/* DDR timing option */
	#define __DDR_6__
	//#define __DDR_75__
	
	/* PLL setting */
	#define __UPLL_240__
	//#define __UPLL_192__
	//#define __UPLL_288__
#endif

/* Start for option for VPOST frame buffer */
#if defined(__TV__)
	#ifdef __TV_QVGA__ 
	#define PANEL_WIDTH		320
	#define PANEL_HEIGHT	240
	#else
	#define PANEL_WIDTH		640
	#define PANEL_HEIGHT	480
	#endif
#elif defined( __LCM_800x600__) 
	#define PANEL_WIDTH		800
	#define PANEL_HEIGHT	600
#elif defined( __LCM_480x272__)
	#define PANEL_WIDTH		480
	#define PANEL_HEIGHT	272
#elif defined( __LCM_800x480__)
	#define PANEL_WIDTH		800
	#define PANEL_HEIGHT	480
#elif defined( __LCM_QVGA__)
	#define PANEL_WIDTH		320
	#define PANEL_HEIGHT	240
#else 
	#define PANEL_WIDTH	480
	#define PANEL_HEIGHT	272
#endif
/* End for option for VPOST frame buffer */

#define PANEL_BPP		2
#define FB_ADDR		0x500000



#ifdef __DEBUG__
#define DBG_PRINTF		sysprintf
#else
#define DBG_PRINTF(...)		
#endif
 
 
 /* Turn on the optional. Back light enable */
 /* Turn off the optional, ICE can connect to */
 /* Default Demo Board  GPD1 keep pull high */
 /* 								*/ 
//#define __BACKLIGHT__
 

/* NAND1-1 Size */
#define NAND1_1_SIZE	 32	/* MB unit */
#define SD1_1_SIZE	 	128 /* MB unit */ 