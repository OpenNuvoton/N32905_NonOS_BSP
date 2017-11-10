#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#pragma import(__use_no_semihosting_swi)

#include "wbio.h"
#include "wblib.h"

#include "w55fa93_sic.h"
#include "nvtfat.h"
#include "spu.h"

#include "AviLib.h"

//#pragma import(__use_no_semihosting_swi)

MV_CFG_T	_tMvCfg;

void  mp3_play_callback(MV_CFG_T *ptMvCfg)
{
	MV_INFO_T 	*ptMvInfo;
	static INT	last_time = 0;

	mflGetMovieInfo(ptMvCfg, &ptMvInfo);

	if ((sysGetTicks(TIMER0) - last_time > 100) &&
		ptMvInfo->uAuTotalFrames)
	{
		sysprintf("T=%d, Progress = %02d:%02d / %02d:%02d\n", 
					sysGetTicks(TIMER0) / 100,
					ptMvInfo->uPlayCurTimePos / 6000, (ptMvInfo->uPlayCurTimePos / 100) % 60,
					ptMvInfo->uMovieLength / 6000, (ptMvInfo->uMovieLength / 100) % 60);
		last_time = sysGetTicks(TIMER0);
	}
}

int main()
{
    WB_UART_T 	uart;
	CHAR		suFileName[128];
	INT			nStatus;

	
	/* CACHE_ON	*/
	sysEnableCache(CACHE_WRITE_BACK);

	/*-----------------------------------------------------------------------*/
	/*  CPU/HCLK/APB:  192/96/48                                             */
	/*-----------------------------------------------------------------------*/
	sysSetSystemClock(eSYS_UPLL, 	//E_SYS_SRC_CLK eSrcClk,	
									192000,		//UINT32 u32PllKHz, 	
									192000,		//UINT32 u32SysKHz,
									192000,		//UINT32 u32CpuKHz,
									192000/2,		//UINT32 u32HclkKHz,
									  192000/4);		//UINT32 u32ApbKHz

	/*-----------------------------------------------------------------------*/
	/*  Init UART, N,8,1, 115200                                             */
	/*-----------------------------------------------------------------------*/
	sysUartPort(1);
	uart.uiFreq = 12000000;					//use XIN clock
    uart.uiBaudrate = 115200;
    uart.uiDataBits = WB_DATA_BITS_8;
    uart.uiStopBits = WB_STOP_BITS_1;
    uart.uiParity = WB_PARITY_NONE;
	uart.uiRxTriggerLevel = LEVEL_1_BYTE;
    sysInitializeUART(&uart);
	sysprintf("UART initialized.\n");

	/*-----------------------------------------------------------------------*/
	/*  Init timer                                                           */
	/*-----------------------------------------------------------------------*/
	sysSetTimerReferenceClock (TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);

	/*-----------------------------------------------------------------------*/
	/*  Init FAT file system                                                 */
	/*-----------------------------------------------------------------------*/
	sysprintf("fsInitFileSystem.\n");
	fsInitFileSystem();

	/*-----------------------------------------------------------------------*/
	/*  Init SD card                                                         */
	/*-----------------------------------------------------------------------*/
	sicIoctl(SIC_SET_CLOCK, 200000, 0, 0);
	sicOpen();
	sysprintf("total sectors (%x)\n", sicSdOpen0());
	
	spuOpen(eDRVSPU_FREQ_8000);
	spuDacOn(1);

	/*-----------------------------------------------------------------------*/
	/*                                                                       */
	/*  MP3 File playback          	                                         */
	/*                                                                       */
	/*-----------------------------------------------------------------------*/
	memset((UINT8 *)&_tMvCfg, 0, sizeof(_tMvCfg));

	fsAsciiToUnicode("c:\\1.mp3", suFileName, TRUE);
	
	_tMvCfg.eInMediaType			= MFL_MEDIA_MP3;
	_tMvCfg.eInStrmType				= MFL_STREAM_FILE;
	_tMvCfg.szIMFAscii				= NULL;
	_tMvCfg.suInMetaFile			= NULL;
	_tMvCfg.szITFAscii				= NULL;
	_tMvCfg.nAudioPlayVolume		= 31;
	_tMvCfg.uStartPlaytimePos		= 0;
	_tMvCfg.nAuABRScanFrameCnt		= 50;
	_tMvCfg.ap_time					= mp3_play_callback;
	_tMvCfg.suInMediaFile			= suFileName;
	
	if ( (nStatus = mflMediaPlayer(&_tMvCfg)) < 0 )
		sysprintf("Playback failed, code = %x\n", nStatus);
	else
		sysprintf("Playback done.\n");
	
	while(1);		
}
