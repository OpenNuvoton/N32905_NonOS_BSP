/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/
 
#include "string.h"
#include "stdlib.h"
#include "w55fa93_reg.h"
#include "spu.h"
#include "DrvSPU.h"

//#define DBG_PRINTF(...)
//#define DBG_PRINTF printf

volatile S_CHANNEL_CTRL g_sChannelSettings;
//volatile UINT32 g_ua32UserData[6];

PFN_DRVSPU_CB_FUNC	*g_pfnUserEventCallBack[32];
PFN_DRVSPU_CB_FUNC	*g_pfnSilentEventCallBack[32];
PFN_DRVSPU_CB_FUNC	*g_pfnLoopStartEventCallBack[32];
PFN_DRVSPU_CB_FUNC	*g_pfnEndEventCallBack[32];
PFN_DRVSPU_CB_FUNC	*g_pfnEndAddressEventCallBack[32];
PFN_DRVSPU_CB_FUNC	*g_pfnThresholdAddressEventCallBack[32];
 

#define	E_SUCCESS	0
//#define	E_DRVSPU_WRONG_CHANNEL	-1
/*---------------------------------------------------------------------------------------------------------*/
/* Function: DrvSPU_IntHandler                                                                             */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*      None																		                       */
/*                                                                                                         */
/* Returns:                                                                                                */
/*      None 						                                                                       */
/*                                                                                                         */
/* Description:                                                                                            */
/*      SPU Interrupt Service Routine                                                                      */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

void DrvSPU_IntHandler(void)
{
	UINT8 ii;
	UINT32 u32Channel, u32InterruptFlag;	
	
	u32Channel = 1;
	
	for (ii=0; ii<32; ii++)
	{
		if (inp32(CH_IRQ) & u32Channel)
		{
			while(inp32(CH_CTRL) & CH_FN);
			
			// load previous channel settings		
			outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (ii << 24));		
			outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
			while(inp32(CH_CTRL) & CH_FN);
				
			u32InterruptFlag = inp32(CH_EVENT);
			outp32(CH_EVENT, inp32(CH_EVENT));				

	#if 1
			if (u32InterruptFlag & DRVSPU_USER_INT)
			{
				if ( inp32(CH_EVENT) & EV_USR_EN)
					g_pfnUserEventCallBack[ii]((UINT8*)SPU_CTRL);
			}				
	
			if (u32InterruptFlag & DRVSPU_SILENT_INT)
			{
				if ( inp32(CH_EVENT) & EV_SLN_EN)
					g_pfnSilentEventCallBack[ii]((UINT8*)SPU_CTRL);
			}				

			if (u32InterruptFlag & DRVSPU_LOOPSTART_INT)
			{
				if ( inp32(CH_EVENT) & EV_LP_EN)
					g_pfnLoopStartEventCallBack[ii]((UINT8*)SPU_CTRL);
			}				

			if (u32InterruptFlag & DRVSPU_END_INT)
			{
				if ( inp32(CH_EVENT) & EV_END_EN)
					g_pfnEndEventCallBack[ii]((UINT8*)SPU_CTRL);
			}				

			if (u32InterruptFlag & DRVSPU_ENDADDRESS_INT)				
			{
				if ( inp32(CH_EVENT) & END_EN)
					g_pfnEndAddressEventCallBack[ii]((UINT8*)((UINT32)_pucPlayAudioBuff + HALF_FRAG_SIZE));
			}				
	
			if (u32InterruptFlag & DRVSPU_THADDRESS_INT)				
			{
				if ( inp32(CH_EVENT) & TH_EN)
					g_pfnThresholdAddressEventCallBack[ii]((UINT8*)((UINT32)_pucPlayAudioBuff));
			}				

			outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
			outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_IRQ_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				
			while(inp32(CH_CTRL) & CH_FN);
				
	#else										
			DrvSPU_UploadChannelSettings(&g_sChannelSettings);	// Will be added 20090715
			if (u32InterruptFlag & DRVSPU_USER_INT)
				g_pfnUserEventCallBack[ii]((S_CHANNEL_CTRL*)&g_sChannelSettings);
	
			if (u32InterruptFlag & DRVSPU_SILENT_INT)
				g_pfnSilentEventCallBack[ii]((S_CHANNEL_CTRL*)&g_sChannelSettings);

			if (u32InterruptFlag == DRVSPU_LOOPSTART_INT)
				g_pfnLoopStartEventCallBack[ii]((S_CHANNEL_CTRL*)&g_sChannelSettings);

			if (u32InterruptFlag == DRVSPU_END_INT)
				g_pfnEndEventCallBack[ii]((S_CHANNEL_CTRL*)&g_sChannelSettings);

			if (u32InterruptFlag == DRVSPU_ENDADDRESS_INT)
				g_pfnEndAddressEventCallBack[ii]((S_CHANNEL_CTRL*)&g_sChannelSettings);
	
			if (u32InterruptFlag == DRVSPU_THADDRESS_INT)
				g_pfnThresholdAddressEventCallBack[ii]((S_CHANNEL_CTRL*)&g_sChannelSettings);
	#endif				
		}
	
		u32Channel <<= 1; 
	}

	outp32(CH_IRQ, inp32(CH_IRQ));
}	


//==========================================================================
//==========================================================================
// SPU open
ERRCODE
DrvSPU_Open(void)
{
	UINT8 ii;

	// enable SPU engine clock 
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE | HCLK4_CKE);			// enable SPU engine clock 

	// disable SPU engine 
//	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_EN);
	outp32(SPU_CTRL, 0x00);
	
	// given FIFO size = 4
	outp32(SPU_CTRL, 0x04000000);		

	// reset SPU engine 
//	outp32(SPU_CTRL, inp32(SPU_CTRL) | SPU_EN);
	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_SWRST);	
	
	outp32(SPU_CTRL, inp32(SPU_CTRL) | SPU_SWRST);
	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_SWRST);	
	
	// disable all channels
	outp32(CH_EN, 0x00);		
	
	for (ii=0; ii<32; ii++)
	{
		DrvSPU_ClearInt(ii, DRVSPU_ALL_INT);
		DrvSPU_DisableInt(ii, DRVSPU_ALL_INT);
	}
	
	return E_SUCCESS;
}

// SPU close 
void DrvSPU_Close(void)
{
	// reset SPU engine 
//	outp32(SPU_CTRL, inp32(SPU_CTRL) | SPU_EN);
	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_SWRST);	
	
	outp32(SPU_CTRL, inp32(SPU_CTRL) | SPU_SWRST);
	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_SWRST);	

	// disable SPU engine 
	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_EN);
	
	// disable SPU engine clock 
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) & ~ADO_CKE & ~SPU_CKE);		// disable SPU engine clock 
}

ERRCODE
DrvSPU_ChannelOpen(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		outp32(CH_EN, inp32(CH_EN) | (0x0001 << u32Channel));
		return E_SUCCESS;
	}
	else		
		return E_DRVSPU_WRONG_CHANNEL;	   	
}

BOOL
DrvSPU_IsChannelEnabled(
	E_DRVSPU_CHANNEL e32Channel
)
{
	UINT32 u32Channel;
	
	u32Channel = 1;
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		u32Channel <<= e32Channel;	
		if (inp32(CH_EN ) & u32Channel)
			return TRUE;
		else 
			return FALSE;
	}
	else		
		return FALSE;
}

ERRCODE 
DrvSPU_ChannelClose(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		outp32(CH_EN, inp32(CH_EN) & ~(0x0001 << u32Channel));
		return E_SUCCESS;
	}		
	else		
		return E_DRVSPU_WRONG_CHANNEL;	   	
}

ERRCODE 
DrvSPU_EnableInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag, 
	PFN_DRVSPU_CB_FUNC* pfnCallBack
)
{

	if ( (u32Channel >=eDRVSPU_CHANNEL_0) && (u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		// set new channel settings for previous channel settings						
		if (u32InterruptFlag & DRVSPU_USER_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) | EV_USR_EN);		
			g_pfnUserEventCallBack[u32Channel] = pfnCallBack;										
		}
		else if (u32InterruptFlag & DRVSPU_SILENT_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) | EV_SLN_EN);				
			g_pfnSilentEventCallBack[u32Channel] = pfnCallBack;													
		}
		else if (u32InterruptFlag & DRVSPU_LOOPSTART_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) | EV_LP_EN);						
			g_pfnLoopStartEventCallBack[u32Channel] = pfnCallBack;													
		}
		else if (u32InterruptFlag & DRVSPU_END_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) | EV_END_EN);						
			g_pfnEndEventCallBack[u32Channel] = pfnCallBack;													
		}

		else if (u32InterruptFlag & DRVSPU_ENDADDRESS_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) | END_EN);						
			g_pfnEndAddressEventCallBack[u32Channel] = pfnCallBack;													
		}

		else if (u32InterruptFlag & DRVSPU_THADDRESS_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) | TH_EN);														
			g_pfnThresholdAddressEventCallBack[u32Channel] = pfnCallBack;													
		}
		outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
		outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_IRQ_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				
		while(inp32(CH_CTRL) & CH_FN);		
		
		//DrvAIC_InstallISR(eDRVAIC_INT_LEVEL1, eDRVAIC_INT_SPU, (PVOID)DrvSPU_IntHandler, 0);			
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	 

return 0;		  
}

BOOL
DrvSPU_IsIntEnabled(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
)
{
	UINT32 u32Flag;

	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		switch (u32InterruptFlag)
		{
			case DRVSPU_USER_INT:
				u32Flag = inp32(CH_EVENT) & EV_USR_EN;					
				break;

			case DRVSPU_SILENT_INT:
				u32Flag = inp32(CH_EVENT) & EV_SLN_EN;
				break;

			case DRVSPU_LOOPSTART_INT:
				u32Flag = inp32(CH_EVENT) & EV_LP_EN;
				break;

			case DRVSPU_END_INT:
				u32Flag = inp32(CH_EVENT) & EV_END_EN;						
				break;

			case DRVSPU_ENDADDRESS_INT:
				u32Flag = inp32(CH_EVENT) & END_EN;						
				break;

			case DRVSPU_THADDRESS_INT:
				u32Flag = inp32(CH_EVENT) & TH_EN;														
				break;

			default:
				u32Flag = 0;			
				break;
		}

		if (u32Flag) return TRUE;
		else return FALSE;
	}
	 
	else
		return FALSE;
}

ERRCODE 
DrvSPU_DisableInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		// set new channel settings for previous channel settings						
		if (u32InterruptFlag & DRVSPU_USER_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) & ~EV_USR_EN);		
		}
		if (u32InterruptFlag & DRVSPU_SILENT_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) & ~EV_SLN_EN);				
		}
		if (u32InterruptFlag & DRVSPU_LOOPSTART_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) & ~EV_LP_EN);						
		}
		if (u32InterruptFlag & DRVSPU_END_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) & ~EV_END_EN);						
		}
		if (u32InterruptFlag & DRVSPU_ENDADDRESS_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) & ~END_EN);						
		}
		if (u32InterruptFlag & DRVSPU_THADDRESS_INT)
		{
			outp32(CH_EVENT, inp32(CH_EVENT) & ~TH_EN);
		}
		outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
		outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_IRQ_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				
		while(inp32(CH_CTRL) & CH_FN);
	
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

ERRCODE 
DrvSPU_ClearInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		// set new channel settings for previous channel settings						
		if (u32InterruptFlag & DRVSPU_USER_INT)
		{
			outp32(CH_EVENT, (inp32(CH_EVENT) & ~0x3F00) | EV_USR_FG);		
		}
		if (u32InterruptFlag & DRVSPU_SILENT_INT)
		{
			outp32(CH_EVENT, (inp32(CH_EVENT) & ~0x3F00) | EV_SLN_FG);				
		}
		if (u32InterruptFlag & DRVSPU_LOOPSTART_INT)
		{
			outp32(CH_EVENT, (inp32(CH_EVENT) & ~0x3F00) | EV_LP_FG);						
		}
		if (u32InterruptFlag & DRVSPU_END_INT)
		{
			outp32(CH_EVENT, (inp32(CH_EVENT) & ~0x3F00) | EV_END_FG);						
		}
		if (u32InterruptFlag & DRVSPU_ENDADDRESS_INT)
		{
			outp32(CH_EVENT, (inp32(CH_EVENT) & ~0x3F00) | END_FG);						
		}
		if (u32InterruptFlag & DRVSPU_THADDRESS_INT)
		{
			outp32(CH_EVENT, (inp32(CH_EVENT) & ~0x3F00) | TH_FG);														
		}
		
		outp32(CH_EVENT, inp32(CH_EVENT) & ~AT_CLR_EN);			// clear Auto Clear Enable bit
		
		outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
		outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_IRQ_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				
		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

BOOL 
DrvSPU_PollInt(
	UINT32 u32Channel, 
	UINT32 u32InterruptFlag
)
{
/*
	BOOL bStatus;
	
	bStatus = TRUE;
	
	if ( (u32Channel >=eDRVSPU_CHANNEL_0) && (u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		// set new channel settings for previous channel settings						
		if (u32InterruptFlag == DRVSPU_USER_INT)
		{
			bStatus = (inp32(CH_EVENT) & EV_USR_FG) ? TRUE : FALSE;		
		}
		else if (u32InterruptFlag == DRVSPU_SILENT_INT)
		{
			bStatus = (inp32(CH_EVENT) & EV_SLN_FG) ? TRUE : FALSE;				
		}
		else if (u32InterruptFlag == DRVSPU_LOOPSTART_INT)
		{
			bStatus = (inp32(CH_EVENT) & EV_LP_FG) ? TRUE : FALSE;						
		}
		else if (u32InterruptFlag == DRVSPU_END_INT)
		{
			bStatus = (inp32(CH_EVENT) & EV_END_FG) ? TRUE : FALSE;						
		}
		else if (u32InterruptFlag == DRVSPU_ENDADDRESS_INT)
		{
			bStatus = (inp32(CH_EVENT) & END_FG) ? TRUE : FALSE;						
		}
		else if (u32InterruptFlag == DRVSPU_THADDRESS_INT)
		{
			bStatus = (inp32(CH_EVENT) & TH_FG) ? TRUE : FALSE;						
		}
		else 
			return E_DRVSPU_WRONG_INTERRUPT;	   		
		
		return bStatus;
	}
	 
	else
		return FALSE;
*/
return FALSE;		
}

ERRCODE 
DrvSPU_SetPauseAddress_PCM16(
	UINT32 u32Channel, 
	UINT32 u32Address
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(PA_ADDR, u32Address);		

//		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS);				
		outp32(CH_CTRL, inp32(CH_CTRL) & ~0xFFFF);
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_PARTIAL_SETTINGS | DRVSPU_UPDATE_PAUSE_PARTIAL);				

		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT32
DrvSPU_GetPauseAddress_PCM16(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return (inp32(PA_ADDR));
	}
	else
		return 0;
}



ERRCODE 
DrvSPU_SetBaseAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(S_ADDR, u32Address);		

		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS);				
		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT32
DrvSPU_GetBaseAddress(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return (inp32(S_ADDR));
	}
	else
		return 0;
}

ERRCODE 
DrvSPU_SetThresholdAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(M_ADDR, u32Address);		

		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS);				
		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT32
DrvSPU_GetThresholdAddress(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return inp32(M_ADDR);
	}
	else
		return 0;
}

ERRCODE 
DrvSPU_SetToneAmp(
	UINT32 u32Channel, 
	UINT32 u32Amp
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(TONE_AMP, u32Amp);		

		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS);				
		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

ERRCODE 
DrvSPU_SetTonePulse(
	UINT32 u32Channel, 
	UINT32 u32Pulse
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(TONE_PULSE, u32Pulse);		

		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS);				
		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

ERRCODE 
DrvSPU_SetEndAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(E_ADDR, u32Address);		

		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS);				
		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT32
DrvSPU_GetEndAddress(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return inp32(E_ADDR);
	}
	else
		return 0;
}

UINT32
DrvSPU_GetCurrentAddress(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return inp32(CUR_ADDR);
	}
	else return 0;	   
}

UINT32
DrvSPU_GetLoopStartAddress(
	UINT32 u32Channel, 
	UINT32 u32Address
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return inp32(LP_ADDR);
	}
	else return 0;	   
}

//#ifdef OPT_DIRECT_SET_DFA
#if 1
	ERRCODE 
	DrvSPU_SetDFA(
		UINT32 u32Channel, 
		UINT16 u16DFA
	)
	{
		if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
		{
			// wait to finish previous channel settings
			while(inp32(CH_CTRL) & CH_FN);
			
			// load previous channel settings		
			outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
			outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
			while(inp32(CH_CTRL) & CH_FN);
			
			outp32(CH_PAR_2, u16DFA);		
	
			outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
			outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_DFA_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				
	
			while(inp32(CH_CTRL) & CH_FN);		
			
			return E_SUCCESS;
		}
		 
		else
			return E_DRVSPU_WRONG_CHANNEL;	   
	}

#else
	ERRCODE 
	DrvSPU_SetDFA(
		UINT32 u32Channel, 
		UINT16 u16SrcSampleRate,
		UINT16 u16OutputSampleRate
	)
	{
		UINT32 u32DFA;
		
	//	u32DFA = (u16SrcSampleRate/u16OutputSampleRate) * 1024;
		u32DFA = (u16SrcSampleRate*1024)/u16OutputSampleRate;
	
		if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
		{
			// wait to finish previous channel settings
			while(inp32(CH_CTRL) & CH_FN);
			
			// load previous channel settings		
			outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
			outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
			while(inp32(CH_CTRL) & CH_FN);
	
			outp32(CH_PAR_2, u32DFA);		
	
			outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
			outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_DFA_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				
	
			while(inp32(CH_CTRL) & CH_FN);		
			
			return E_SUCCESS;
		}
		 
		else
			return E_DRVSPU_WRONG_CHANNEL;	   
	}
#endif	

UINT16
DrvSPU_GetDFA(
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		return inp32(CH_PAR_2) & 0x1FFF;
	}
	else
		return 0;
}

// MSB 8-bit = left channel; LSB 8-bit = right channel
ERRCODE 
DrvSPU_SetPAN(
	UINT32 u32Channel, 
	UINT16 u16PAN
)
{
	UINT32 u32PAN;
	
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		u32PAN = u16PAN;
		u32PAN <<= 8;			
		u32PAN &= (PAN_L + PAN_R);
		outp32(CH_PAR_1, (inp32(CH_PAR_1) & (~(PAN_L+PAN_R))) | u32PAN);		

		outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
		outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_PAN_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				

		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT16
DrvSPU_GetPAN(
	UINT32 u32Channel
)
{
	UINT32 u32PAN;
	
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		u32PAN = inp32(CH_PAR_1);
		u32PAN >>= 8;
		return (u32PAN & 0xFFFF);
	}
	else
		return 0;
}


ERRCODE 
DrvSPU_SetSrcType(
	UINT32 u32Channel, 
	UINT8 u8DataFormat
)
{

	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		outp32(CH_PAR_1, (inp32(CH_PAR_1) & ~SRC_TYPE) | u8DataFormat);		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_UPDATE_ALL_SETTINGS );				

		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT16
DrvSPU_GetSrcType(
	UINT32 u32Channel
)
{
	UINT8 u8DataFormat;
	
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		u8DataFormat = inp32(CH_PAR_1);
		return (u8DataFormat & 0x07);
	}
	else
		return 0;
}

ERRCODE 
DrvSPU_SetChannelVolume(
	UINT32 u32Channel, 
	UINT8 u8Volume
)
{
	UINT32 u32PAN;
	
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		u32PAN = u8Volume;
		u32PAN <<= 24;
		outp32(CH_PAR_1, (inp32(CH_PAR_1) & 0x00FFFFFF) | u32PAN);		

		outp32(CH_CTRL, inp32(CH_CTRL) & ~DRVSPU_UPDATE_ALL_PARTIALS);		
		outp32(CH_CTRL, inp32(CH_CTRL) | (DRVSPU_UPDATE_VOL_PARTIAL + DRVSPU_UPDATE_PARTIAL_SETTINGS));				

		while(inp32(CH_CTRL) & CH_FN);		
		
		return E_SUCCESS;
	}
	 
	else
		return E_DRVSPU_WRONG_CHANNEL;	   
}

UINT8
DrvSPU_GetChannelVolume(
	UINT32 u32Channel
)
{
	UINT16 u32PAN;
	
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		
		u32PAN = inp32(CH_PAR_1);
		u32PAN >>= 24;
		return (u32PAN & 0xFF);
	}
	else
		return 0;
}

void DrvSPU_EqOpen(
	E_DRVSPU_EQ_BAND eEqBand,
	E_DRVSPU_EQ_GAIN eEqGain		
)
{
	switch (eEqBand)
	{
		case eDRVSPU_EQBAND_DC:
			outp32(EQGain1, (inp32(EQGain1) & (~Gaindc)) | eEqGain <<16);
			break;
	
		case eDRVSPU_EQBAND_1:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain01)) | eEqGain);
			break;
	
		case eDRVSPU_EQBAND_2:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain02)) | eEqGain <<4);
			break;

		case eDRVSPU_EQBAND_3:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain03)) | eEqGain <<8);
			break;

		case eDRVSPU_EQBAND_4:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain04)) | eEqGain <<12);
			break;

		case eDRVSPU_EQBAND_5:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain05)) | eEqGain <<16);
			break;

		case eDRVSPU_EQBAND_6:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain06)) | eEqGain <<20);
			break;

		case eDRVSPU_EQBAND_7:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain07)) | eEqGain <<24);
			break;

		case eDRVSPU_EQBAND_8:
			outp32(EQGain0, (inp32(EQGain0) & (~Gain08)) | eEqGain <<28);
			break;

		case eDRVSPU_EQBAND_9:
			outp32(EQGain1, (inp32(EQGain1) & (~Gain09)) | eEqGain);
			break;

		default:
		case eDRVSPU_EQBAND_10:
			outp32(EQGain1, (inp32(EQGain1) & (~Gain10)) | eEqGain <<4);
			break;
	}
	
	outp32(DAC_PAR, inp32(DAC_PAR) | EQU_EN | ZERO_EN);
}

void DrvSPU_EqClose(void)
{
	outp32(DAC_PAR, inp32(DAC_PAR) & (~EQU_EN) & (~ZERO_EN));
}

void DrvSPU_SetVolume(
	UINT16 u16Volume	// MSB: left channel; LSB right channel
)	
{
	//outp32(DAC_VOL, (inp32(DAC_VOL) & ~(DWA_SEL | ANA_PD | LHPVL | RHPVL)) | (u16Volume & 0x3F3F));
	outp32(DAC_VOL, (inp32(DAC_VOL) & ~(0x3F3F)) | (u16Volume & 0x3F3F));
}

UINT16 
DrvSPU_GetVolume(void)
{
	return (inp32(DAC_VOL) & 0x3F3F);
}	

void DrvSPU_StartPlay(void)
{
	outp32(SPU_CTRL, inp32(SPU_CTRL) | SPU_EN);
}

void DrvSPU_StopPlay(void)
{
	outp32(SPU_CTRL, inp32(SPU_CTRL) & ~SPU_EN);
}

void delay_loop(int PLL)	// delay 1mS for PLL input
{
	volatile int ii, jj;
	
	PLL /= 5;
	for (ii=0; ii<PLL; ii++)
	{
//		for (jj=0; jj<1000; jj++);
		for (jj=0; jj<200; jj++);
	}
}

UINT32  
DrvSPU_SetSampleRate (
	UINT32  u32SystemClock,
	UINT32  SampleRate
)
{
	UINT32 u32ClockDivider, u32RealSampleRate,u32ClkSel=0,u32DFA;
	UINT32 u32Buf, u32Buf2, u32Buf3 ;
	int ii, jj, kk;
	
	
	u32Buf2 = inp32(CH_EN);
//	outp32(CH_EN, 0x00);
	outp32(REG_AHBCLK, inp32(REG_AHBCLK) | ADO_CKE | SPU_CKE);			// enable SPU engine clock 
	
	//if (inp32(REG_CHIPCFG) & 0x01)
	//	u32SystemClock = sysGetPLLOutputKhz(eSYS_UPLL, 27000)*1000;
	//else
	//	u32SystemClock = sysGetPLLOutputKhz(eSYS_UPLL, 12000)*1000;
	
	u32ClockDivider = 	u32SystemClock / (128*SampleRate);
	
	if (u32ClockDivider > 255)
	{
	
		u32ClkSel = u32ClockDivider / 255;
		u32ClockDivider = u32ClockDivider / u32ClkSel;
	}

	u32Buf = inp32(REG_CLKDIV1);
	u32Buf &= ~ADO_S;
	u32Buf |= (0x3 << 19) | (u32ClkSel <<16);
	u32Buf &= ~ADO_N1;
	u32ClockDivider &= 0xFF;
	u32ClockDivider	--;
	u32Buf  |= u32ClockDivider<<24;	

	u32Buf3 = inp32(REG_SPU_DAC_VOL);
	jj = inp32(REG_SPU_DAC_VOL) & 0xFF;
	for (ii=0; ii<jj; ii++)
	{
		outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) - 0x0101);	// set volume to zero			
		delay_loop(240);	// delay 1mS for 240MHz PLL output
//		sysDelay(1);		
	}			
	outp32(REG_CLKDIV1, u32Buf);			
	
	for (ii=0; ii<jj; ii++)
	{
		outp32(REG_SPU_DAC_VOL, inp32(REG_SPU_DAC_VOL) + 0x0101);	// set volume to zero			
		delay_loop(240);	// delay 1mS for 240MHz PLL output		
//		sysDelay(1);		
	}			

	u32ClockDivider++;
	u32ClkSel++;
	u32RealSampleRate = u32SystemClock / (128*u32ClockDivider*u32ClkSel);
	
	u32DFA= SampleRate * 1024 / u32RealSampleRate;

	// set DFA 
	DrvSPU_SetDFA(0, u32DFA);	
	DrvSPU_SetDFA(1, u32DFA);		

//	outp32(CH_EN, u32Buf2);
	return u32RealSampleRate;
}

ERRCODE
DrvSPU_UploadChannelContents (
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		// wait to finish previous channel settings
		while(inp32(CH_CTRL) & CH_FN);
		
		// load previous channel settings		
		outp32(CH_CTRL, (inp32(CH_CTRL) & ~CH_NO) | (u32Channel << 24));		
		outp32(CH_CTRL, inp32(CH_CTRL) | DRVSPU_LOAD_SELECTED_CHANNEL);
		while(inp32(CH_CTRL) & CH_FN);
		return E_SUCCESS;
	}
	else
		return E_DRVSPU_WRONG_CHANNEL;	   	
}


ERRCODE 
DrvSPU_ChannelCtrl(
	S_CHANNEL_CTRL *psChannelCtrl
)
{
	volatile UINT32 u32Channel; 
	
	u32Channel = psChannelCtrl->u32ChannelIndex;
	
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		DrvSPU_SetChannelVolume(u32Channel, psChannelCtrl->u8ChannelVolume);	
		DrvSPU_SetPAN(u32Channel, psChannelCtrl->u16PAN);	
		DrvSPU_SetSrcType(u32Channel, psChannelCtrl->u8DataFormat);
		
#ifdef OPT_DIRECT_SET_DFA		
//		DrvSPU_SetDFA(u32Channel, psChannelCtrl->u16DFA);		
#else		
//		DrvSPU_SetDFA(u32Channel, psChannelCtrl->u16SrcSampleRate, psChannelCtrl->u16OutputSampleRate);				
#endif		
		DrvSPU_SetBaseAddress(u32Channel, psChannelCtrl->u32SrcBaseAddr);		
		DrvSPU_SetThresholdAddress(u32Channel, psChannelCtrl->u32SrcThresholdAddr);
		DrvSPU_SetEndAddress(u32Channel, psChannelCtrl->u32SrcEndAddr);
		
		return E_SUCCESS;
	}
	else
		return E_DRVSPU_WRONG_CHANNEL;	   	
}

ERRCODE
DrvSPU_ChannelPause (
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		outp32(CH_PAUSE, inp32(CH_PAUSE) | (0x0001 << u32Channel));
		return E_SUCCESS;
	}
	else		
		return E_DRVSPU_WRONG_CHANNEL;	   	
}

ERRCODE
DrvSPU_ChannelResume (
	UINT32 u32Channel
)
{
	if ( ((INT)u32Channel >=eDRVSPU_CHANNEL_0) && ((INT)u32Channel <=eDRVSPU_CHANNEL_31) )
	{
		outp32(CH_PAUSE, inp32(CH_PAUSE) & ~(0x0001 << u32Channel));
		return E_SUCCESS;
	}
	else		
		return E_DRVSPU_WRONG_CHANNEL;	   	
}

