/****************************************************************************
*                                                                           *
* Copyright (c) 2009 Nuvoton Tech. Corp. All rights reserved.               *
*                                                                           *
*****************************************************************************/

/****************************************************************************
* FILENAME
*   adc.c
*
* VERSION
*   1.0
*
* DESCRIPTION
*   ADC sample application using ADC library
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wblib.h"
#include "W55FA93_adc.h"
//#include "nvtfat.h"
#include "DrvEDMA.h"
#include "usbd.h"
#include "videoclass.h"

INT32 InitializeUAC(UINT32 u32SampleRate);
void StartUAC(void);
void StopUAC(void);
void Send_AudioOneMSPacket(PUINT32 pu32Address, PUINT32 pu32Length);


#define UAC_MUTE_ADDR		0

#define E_AUD_BUF  64*1  // 64 bytes
#define TRAN_SAMPLE  16*1   // 64/2 = 32 bytes, for half, 32 bytes = 2 x 16, 16 samples   

#define IN_DATA_BUF_NUM  128*4


#define IGNORE_NO	0x500  // 1 seconds data igore, = 2 x 16 x 1000


//static volatile INT8 g_i8PcmReady = FALSE;
__align(32) INT16 g_pi16SampleBuf[E_AUD_BUF/2];		/* Keep max 16K sample rate */
__align(32) INT16 g_pi16AudioBuf[E_AUD_BUF/2*4*4];

UINT8 bPlaying = FALSE;
UINT8 bOver = FALSE;
UINT32 g_u32IgnoreNo;

volatile INT16  s_i16RecOutPos;
volatile INT16  s_i16RecInPos;   
volatile INT16  s_i16RecInSample;

UINT32 g_u32RecorderByte;
UINT32 g_u32SampleCount;

// recorder
__align(4) INT32 g_i32RecorderAttr[5] = {
0,        // mute off
FU_VOLUME_CUR,    //2560,     // current volume,
FU_VOLUME_MIN,      //-32768,   // min
FU_VOLUME_MAX,     // 32768,    // max
1 	  // res 	
};	
				
static void pfnRecordCallback(void)
{
//	g_i8PcmReady = TRUE;
}

//volatile BOOL bIsBufferDone=0;
void edmaCallback(UINT32 u32WrapStatus)
{
	UINT32 u32Period, u32Attack, u32Recovery, u32Hold;
    INT16 *pi16srcADC;
    UINT32 i;

    g_u32IgnoreNo ++;	
	if(u32WrapStatus==256)
	{
//		bIsBufferDone = 1;
//		sysprintf("I %d\n\n", bIsBufferDone);
		if ( bPlaying == TRUE )	
		{
/*			if ( g_i32RecorderAttr[UAC_MUTE_ADDR] == 1 ) // mute on
			{
				for (i= 0; i< TRAN_SAMPLE; i++)
	            {
					g_pi16AudioBuf[s_i16RecInPos++] = 0 ;
                }

			}
			else
			*/
			{
/*		        if ( g_i32RecorderAttr[1] == 0 ) // volume = 0;
				{
					for (i= 0; i< TRAN_SAMPLE; i++)
		            {
						g_pi16AudioBuf[s_i16RecInPos++]= 0;
					}	
				}
				else
				*/
				{
			//        pi16srcADC = (INT16 *)((UINT32)g_pi16SampleBuf+E_AUD_BUF/2);
                    pi16srcADC = (INT16 *)(((UINT32)g_pi16SampleBuf+E_AUD_BUF/2) | 0x80000000);  			        
					for (i= 0; i< TRAN_SAMPLE; i++)
		            {
						g_pi16AudioBuf[s_i16RecInPos++]= *pi16srcADC++;
					}	
				}
			}			
 			if (s_i16RecInPos ==  IN_DATA_BUF_NUM)
   			{
				s_i16RecInPos = 0;
			}		
			s_i16RecInSample += TRAN_SAMPLE;
        }
	}	
	else if(u32WrapStatus==1024)
	{
		
//		bIsBufferDone = 2;		
//		sysprintf("I %d\n\n", bIsBufferDone);
		if ( bPlaying == TRUE )	
		{
/*			if ( g_i32RecorderAttr[UAC_MUTE_ADDR] == 1 ) // mute on
			{
				for (i= 0; i< TRAN_SAMPLE; i++)
	            {
					g_pi16AudioBuf[s_i16RecInPos++] = 0 ;
                }

			}
			else
			*/
			{
/*		        if ( g_i32RecorderAttr[1] == 0 ) // volume = 0;
				{
					for (i= 0; i< TRAN_SAMPLE; i++)
		            {
						g_pi16AudioBuf[s_i16RecInPos++]= 0;
					}	
				}
				else
				*/
				{
//			        pi16srcADC = (INT16 *)((UINT32)g_pi16SampleBuf);
                    pi16srcADC = (INT16 *)(((UINT32)g_pi16SampleBuf) | 0x80000000) ;
					for (i= 0; i< TRAN_SAMPLE; i++)
		            {
						g_pi16AudioBuf[s_i16RecInPos++]= *pi16srcADC++;
					}	
				}
			}			
 			if (s_i16RecInPos ==  IN_DATA_BUF_NUM)
   			{
				s_i16RecInPos = 0;
			}		
			s_i16RecInSample += TRAN_SAMPLE;
		}
	}
	/* AGC response speed */
	ADC_GetAutoGainTiming(&u32Period, &u32Attack, &u32Recovery, &u32Hold);
	if(u32Period<128)
	{		
		u32Period = u32Period+16;
		ADC_SetAutoGainTiming(u32Period, u32Attack, u32Recovery, u32Hold);		
	}
/*	
	if ( g_u32IgnoreNo >= IGNORE_NO )
	{
       bPlaying = TRUE; 
    }       
    */

}
/*
	The EDMA will move audio data to start address = g_pi16SampleBuf[0] with range u32Length. 
	The EDMA will issue interrupt in data reach to half of u32Length and u32Length. 	
	Then repeat to filled audio data to  g_pi16SampleBuf[0].
	Programmer must write audio data ASAP. 
*/
void InitEDMA(UINT32 u32Length)
{
	S_DRVEDMA_CH_ADDR_SETTING pSrc, pDest;
	PFN_DRVEDMA_CALLBACK *pfnOldcallback;  
	
	pSrc.u32Addr = REG_AUDIO_BUF0;
	pSrc.eAddrDirection = eDRVEDMA_DIRECTION_FIXED;
	pDest.u32Addr = (unsigned int)g_pi16SampleBuf | 0x80000000;
	pDest.eAddrDirection = eDRVEDMA_DIRECTION_WRAPAROUND;
	DrvEDMA_Open();

	DrvEDMA_EnableCH(eDRVEDMA_CHANNEL_1,eDRVEDMA_ENABLE);
	DrvEDMA_SetAPBTransferWidth(eDRVEDMA_CHANNEL_1,eDRVEDMA_WIDTH_16BITS);	// Mode 0 : 16 Bit
//	DrvEDMA_SetAPBTransferWidth(eDRVEDMA_CHANNEL_1,eDRVEDMA_WIDTH_32BITS);	// Mode 1 : 32 bit	
	DrvEDMA_SetCHForAPBDevice(eDRVEDMA_CHANNEL_1, eDRVEDMA_ADC, eDRVEDMA_READ_APB);
	//DrvEDMA_SetTransferSetting(eDRVEDMA_CHANNEL_1, &pSrc, &pDest,16*8000 ); //16K 
	DrvEDMA_SetTransferSetting(eDRVEDMA_CHANNEL_1, &pSrc, &pDest, u32Length); //16K
	DrvEDMA_SetWrapIntType(eDRVEDMA_CHANNEL_1,eDRVEDMA_WRAPAROUND_HALF | eDRVEDMA_WRAPAROUND_EMPTY);
	DrvEDMA_EnableInt(eDRVEDMA_CHANNEL_1,eDRVEDMA_WAR);	
	
	DrvEDMA_InstallCallBack(eDRVEDMA_CHANNEL_1, 
						eDRVEDMA_WAR,
						edmaCallback,
						pfnOldcallback);	
	
}	

//	u32SampleRate = 16000;	
INT32 InitializeUAC(UINT32 u32SampleRate)
{	
	
	PFN_ADC_CALLBACK 	pfnOldCallback;
	INT32 i;
	
     
#if 1
//	sysCheckPllConstraint(FALSE);						
//	sysSetPllClock(eSYS_APLL, 	//Another PLL,	
//					184320);		//UINT32 u32PllKHz, 	

	sysSetTimerReferenceClock (TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);	
	sysEnableCache(CACHE_WRITE_BACK);	
	
	outp32(REG_APLLCON, 0x12A7);
//	sysCheckPllConstraint(TRUE);
	
	audio_Open(eSYS_APLL, u32SampleRate);
	adc_disableInt(eADC_AUD_INT);//(PVOID)pfnRecordCallback, 0);
	adc_installCallback(eADC_AUD_INT,
					pfnRecordCallback,	/* Invalid if EDMA enable */
					&pfnOldCallback);
//	InitEDMA(E_AUD_BUF);						
#endif
#if 0
    for (i=0; i<128; i++) 				
        g_pi16AudioBuf[i] = i;     					
#endif        
    g_u32SampleCount = u32SampleRate/1000;
    g_u32RecorderByte = g_u32SampleCount << 1;   //	
    sysprintf("g_u32RecorderByte %d\n",g_u32RecorderByte);
	return 0;
}

void StartUAC(void)
{	
    g_u32IgnoreNo = 0;
	bPlaying = FALSE;
	bOver = FALSE;
	s_i16RecOutPos = s_i16RecInPos = 0;   
	s_i16RecInSample = 0;
#if 1	
	InitEDMA(E_AUD_BUF);	
	adc_StartRecord(); 
	DrvEDMA_CHEnablelTransfer(eDRVEDMA_CHANNEL_1);
#endif	
	
}

void StopUAC(void)
{
#if 1
	adc_disableInt(eADC_AUD_INT);
	adc_StopRecord();   
//	DrvEDMA_Close();
#endif	
	bPlaying = FALSE;
	bOver = TRUE;
    g_u32IgnoreNo = 0;
	s_i16RecOutPos = s_i16RecInPos = 0; 
	s_i16RecInSample = 0;	
}

/* Send data to the interrupt of Isochronous In */	  
void Send_AudioOneMSPacket(PUINT32 pu32Address, PUINT32 pu32Length)
{
	PUINT32 pu32Buf;
	int volatile i;
//	sysprintf("ISR audioOneSP\n");
#if 1    
 
    if ( bOver == TRUE )
    {
       bPlaying = FALSE;
    }
    else
       bPlaying = TRUE;
       
	if ( s_i16RecInSample >= (g_u32SampleCount << 4) )
	{
	   if (s_i16RecOutPos + g_u32RecorderByte > IN_DATA_BUF_NUM )
	   {
		   /* 1X */
//    	   UAC_SendOneAudioPacket((BYTE *)&s_ai16RecBuf[s_i16RecOutPos], g_u32RecorderByte);
//           uvcdSDRAM_USB_Transfer(EP_C, &g_pi16AudioBuf[s_i16RecOutPos] , g_u32RecorderByte);
		*pu32Address = (UINT32)&g_pi16AudioBuf[s_i16RecOutPos];
		*pu32Length	= g_u32RecorderByte;
       	   s_i16RecOutPos += g_u32SampleCount;
	       s_i16RecInSample -= g_u32SampleCount;
	   }
	   else
	   {
		   /* 2X */
//     	   UAC_SendOneAudioPacket((BYTE *)&s_ai16RecBuf[s_i16RecOutPos], (g_u32RecorderByte << 1));
//           uvcdSDRAM_USB_Transfer(EP_C, &g_pi16AudioBuf[s_i16RecOutPos] , g_u32RecorderByte << 1);
		*pu32Address = (UINT32)&g_pi16AudioBuf[s_i16RecOutPos];
		*pu32Length	= g_u32RecorderByte << 1;
       	   s_i16RecOutPos += g_u32RecorderByte;
	       s_i16RecInSample -= g_u32RecorderByte;
	   }

 	}
    else if ( s_i16RecInSample >= g_u32SampleCount )
	{   
//       UAC_SendOneAudioPacket((BYTE *)&s_ai16RecBuf[s_i16RecOutPos], g_u32RecorderByte);
//       uvcdSDRAM_USB_Transfer(EP_C, &g_pi16AudioBuf[s_i16RecOutPos] , g_u32RecorderByte);
		*pu32Address = (UINT32)&g_pi16AudioBuf[s_i16RecOutPos];
		*pu32Length	= g_u32RecorderByte;
       s_i16RecOutPos += g_u32SampleCount;
	   s_i16RecInSample -= g_u32SampleCount;
    }
    else
    {
//     UAC_SendOneAudioPacket(0, 0);  // send 0 data.
//       uvcdSDRAM_USB_Transfer(EP_C, NULL, 0);
        *pu32Length	= 0; //g_u32RecorderByte;    //??????
    }
	if (s_i16RecOutPos ==  IN_DATA_BUF_NUM)
   	{
		s_i16RecOutPos = 0;
	}
#else	
  //  uvcdSDRAM_USB_Transfer(EP_C, &g_pi16AudioBuf[s_i16RecOutPos] , g_u32RecorderByte);
        if ( s_i16RecInSample >= g_u32SampleCount )
        {
		*pu32Address = (UINT32)&g_pi16AudioBuf[s_i16RecOutPos];
		*pu32Length	= g_u32RecorderByte;
		s_i16RecOutPos += g_u32SampleCount;
	   s_i16RecInSample -= g_u32SampleCount;		
		if (s_i16RecOutPos ==  IN_DATA_BUF_NUM)
   		{
		s_i16RecOutPos = 0;
		}		
	}
	else
	{
	        *pu32Length	= 0;    
	}

#endif    		
}  
 

