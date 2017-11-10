
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"
#include "W55fa93_vpost.h"
#include "demo.h"

extern VOID vpostEnableInt(E_DRVVPOST_INT eInt);
extern int vpostInstallCallBack(E_DRVVPOST_INT eIntSource,
						PFN_DRVVPOST_INT_CALLBACK	pfnCallback,
						PFN_DRVVPOST_INT_CALLBACK 	*pfnOldCallback);
extern VOID vpostSetFrameBuffer(UINT32 u32BufferAddress);
	
extern BOOL bIsFrameBuffer0,  bIsFrameBuffer1, bIsFrameBuffer2; 
extern UINT8 u8PacketFrameBuffer[],  u8PacketFrameBuffer1[],  u8PacketFrameBuffer2[];
static UINT32 u32BufIdx=2;

void	VPOST_InterruptServiceRiuntine()
{
	switch(u32BufIdx)
	{
		case 0: 
			if(bIsFrameBuffer1==1)	
			{/* Check Frame Buffer 1 is dirty */
				bIsFrameBuffer0 = 0;	/* Frame Buffer 0 is clean */
				u32BufIdx = 1;
				vpostSetFrameBuffer((UINT32)u8PacketFrameBuffer1);
			}
			break;		
		case 1: 
			if(bIsFrameBuffer2==1)	
			{/* Check Frame Buffer 2 is dirty */
				bIsFrameBuffer1 = 0;	/* Frame Buffer 1 is clean */
				u32BufIdx = 2;
				vpostSetFrameBuffer((UINT32)u8PacketFrameBuffer2);
			}
			break;
		case 	2:  	
			if(bIsFrameBuffer0==1)		
			{/* Check Frame Buffer 0 is dirty */
				bIsFrameBuffer2 = 0;	/* Frame Buffer 2 is clean */
				u32BufIdx = 0;
				vpostSetFrameBuffer((UINT32)u8PacketFrameBuffer);			
			}
			break;
	}	
}

void InitVPOST(UINT8* pu8FrameBuffer)
{		
	PFN_DRVVPOST_INT_CALLBACK fun_ptr;
	LCDFORMATEX lcdFormat;	
	lcdFormat.ucVASrcFormat = DRVVPOST_FRAME_YCBYCR;
	lcdFormat.nScreenWidth = OPT_LCM_WIDTH;
	lcdFormat.nScreenHeight = OPT_LCM_HEIGHT;	  
	vpostLCMInit(&lcdFormat, (UINT32*)pu8FrameBuffer);
	vpostInstallCallBack(eDRVVPOST_VINT, (PFN_DRVVPOST_INT_CALLBACK)VPOST_InterruptServiceRiuntine,  (PFN_DRVVPOST_INT_CALLBACK*)&fun_ptr);
	vpostEnableInt(eDRVVPOST_VINT);	

}	