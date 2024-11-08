/**************************************************************************//**
 * @file     jpegDec.c
 * @brief    JPEG Decode function shows the decode control flow
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wbtypes.h"
#include "wblib.h"

#include "jpegcodec.h"
#include "NVTFAT.h"
#include "W55FA93_SIC.h"
#include "jpegSample.h"
#include "W55FA93_VPOST.h"

extern LCDFORMATEX lcdInfo;

extern CHAR g_u8String[100];
extern UINT32 g_u32StringIndex;

UINT32 g_u32xStart, g_u32xEnd, g_u32yStart, g_u32yEnd;
UINT32 g_u32WindowX, g_u32WindowY;	
UINT32 g_u32DecFormat = JPEG_DEC_PRIMARY_PACKET_RGB565;			/* Decode Output format */
PUINT8 g_pu8JpegBuffer;											/* The source bit stream data for decoding */
UINT32 g_u32JpegBuffer;
PUINT8 g_pu8DecFrameBuffer;										/* The buffer for decoding output */
   
/*-----------------------------------------------------------------------*/
/*  Decode Input Wait parameter				                             */
/*-----------------------------------------------------------------------*/        
#if defined (__GNUC__)
UINT8  g_au8DecInputWaitBuffer[DEC_IPW_BUFFERSIZE] __attribute__((aligned(32)));  /* Buffer for Decode Input Wait */
#else
UINT8 __align(32) g_au8DecInputWaitBuffer[DEC_IPW_BUFFERSIZE];	                  /* Buffer for Decode Input Wait */
#endif
UINT32 g_u32IpwUsedSize  = 0;									/* Buffer Control parameter for Decode Input Wait (bitstream size put to Bitstream Buffer )*/
UINT32 g_u32BitstreamSize = 0;									/* JPEG Bitstream Size for Decode Input Wait */
     
/*-----------------------------------------------------------------------*/
/*  Decode Output Wait parameter				                         */
/*-----------------------------------------------------------------------*/          
PUINT8 apBuffer[100];											/* Decode Output buffer pointer array */
UINT32 g_u32OpwUsedSize;										/* JPEG Bitstream Size for Decode Input Wait */
UINT32 u32MCU_Line;												/* One MCU Line data size */
UINT32 u32TotalSize;											/* Total size for JPEG Decode Ouput */
UINT32 g_u32OpwBufferIndex = 0;									/* Decode output Buffer index */

int setOutputBuffer(UINT32 u32Width, UINT32 u32Height, UINT32 u32Format)
{
	UINT32 u32FrameBuffer;		
	JPEG_INFO_T jpegInfo;
	UINT32 u32BufferSize, u32YBuffer, u32UBuffer,u32VBuffer;
	INT8	fill = 0xFF;
	UINT32 u32OutputOffset;
	UINT32 u32TargetWidth,u32TargetHeight;
	UINT16 u16Width,u16Height;	
		
	if(g_bDecPanelTest && !g_bDecWindecTest)
	{
		/* Allocate the Raw Data Buffer for Decode Operation */	
		if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_RGB888)
			u32BufferSize = PANEL_WIDTH * PANEL_HEIGHT * 4;
		else
			u32BufferSize = PANEL_WIDTH * PANEL_HEIGHT * 2;
			
		/* Allocate Raw Data Buffer for Decode Operation */	
		g_pu8DecFrameBuffer = (PUINT8)malloc(sizeof(CHAR) * (u32BufferSize + 0x03));		
		
		if(g_pu8DecFrameBuffer == NULL)
		{
			sysprintf("\tCan't allocate the buffer for decode (size 0x%X)\n",u32BufferSize);
			return FALSE;
		}
			
		u32FrameBuffer =  (((UINT32) g_pu8DecFrameBuffer + 0x03) & ~0x03) | 0x80000000;
		
		sysprintf("\tThe decoded data starts from 0x%X, Size is 0x%X\n", u32FrameBuffer,u32BufferSize);	
		
		/* DownScale size control */	 
		if(u32Width > u32Height)
		{
			if((u32Width > PANEL_WIDTH || u32Height > PANEL_HEIGHT))	
			{
				/* Set Downscale to QVGA */
				jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, TARGET_HEIGHT, TARGET_WIDTH);
				u32TargetHeight = TARGET_HEIGHT;
				u32TargetWidth = TARGET_WIDTH;	
			}
			else
			{
				u32TargetHeight = u32Height;
				u32TargetWidth = u32Width;			
			}			 
		}
		else
		{
			if((u32Width > PANEL_WIDTH || u32Height > PANEL_HEIGHT))	
			{
				UINT32 ratio;
				
				ratio = u32Height / PANEL_HEIGHT + 1;
				
				/* Set Downscale to QVGA */
				sysprintf("\tJpeg width is smaller than height -> Can't downscale to QVGA -> downscale to %dx%d\n",jpegInfo.jpeg_width / ratio,jpegInfo.jpeg_height / ratio);
				jpegIoctl(JPEG_IOCTL_SET_DECODE_DOWNSCALE, u32Height / ratio, u32Width / ratio);
				u32TargetHeight = u32Height / ratio;
				u32TargetWidth = u32Width / ratio;	
			}
			else
			{
				u32TargetHeight = u32Height;
				u32TargetWidth = u32Width;			
			}	
		}
				
		/* Set Decode Stride to Panel width */	
		jpegIoctl(JPEG_IOCTL_SET_DECODE_STRIDE, PANEL_WIDTH, 0);		
		
		/* The pixel offset for putting the image at the center of Frame Buffer */			
		u32OutputOffset = (UINT32)((PANEL_WIDTH * ((UINT32)(PANEL_HEIGHT - u32TargetHeight) / 2))) + (UINT32)((PANEL_WIDTH - u32TargetWidth) / 2);	
		
		/* Data for Clear Buffer (Just for test) */
		if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_YUV422)
			fill = 0x7F;
	
		if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_RGB888)
		{
			memset((UINT32*)u32FrameBuffer,fill, PANEL_WIDTH * PANEL_HEIGHT * 4);
			u32OutputOffset *= 4; 
		}
		else
		{
			if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_YUV422)
			{
				UINT32 i;
				for(i = 0;i<PANEL_WIDTH * PANEL_HEIGHT * 2;i++)
				{
					if(i%2)
						*((PUINT8)(u32FrameBuffer + i)) = 0x80;	
					else
						*((PUINT8)(u32FrameBuffer + i)) = 0xFF;
				}
			}
			else
				memset((UINT32*)u32FrameBuffer,fill, PANEL_WIDTH * PANEL_HEIGHT * 2);		
						
			u32OutputOffset *= 2; 
		}
		
		/* Change Raw data Output address (Let JPEG engine output data to the center of Panel Buffer) */	
		jpegIoctl(JPEG_IOCTL_SET_YADDR, (UINT32) u32FrameBuffer + u32OutputOffset, 0);
	}
	else
	{
		/* For Normal Decode buffer allocation */
    if(u32Format == JPEG_DEC_YUV422)   
    {
		    /* alignment for YUV422 raw data */
		    if(u32Width % 16)
		        u32Width = (u32Width & 0xFFFFFFF0) + 16;     
		        
		    if(u32Height  % 8)
		        u32Height  = (u32Height  & 0xFFFFFFF8) + 8; 		          
    }
    else if(u32Format == JPEG_DEC_YUV444)
		{
		    /* alignment for YUV444 raw data */
		    if(u32Width % 8)
		        u32Width = (u32Width & 0xFFFFFFF8) + 8; 
		        
		    if(u32Height  % 8)
		        u32Height  = (u32Height  & 0xFFFFFFF8) + 8; 		        
		}
		else
		{	/* alignment for YUV420 raw data */		    
		    if(u32Width % 16)
		        u32Width = (u32Width & 0xFFFFFFF0) + 16;
		    if(u32Height % 16)
		        u32Height = (u32Height & 0xFFFFFFF0) + 16;		        
		} 

		/* Raw Data Buffer for Decode Operation */	
		if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_RGB888)
			u32BufferSize = u32Width * u32Height * 4;
		else if (g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
		{
			if(u32Format == JPEG_DEC_YUV422)   
				u32BufferSize = u32Width * u32Height * 2;
			else if (u32Format == JPEG_DEC_YUV444)  
				u32BufferSize = u32Width * u32Height * 3;
			else
				u32BufferSize = u32Width * u32Height * 1.5;	
		}
		else
			u32BufferSize = u32Width * u32Height * 2;			
	
		if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
		{
		  UINT32 u32FrameBuffer; 			   	
	   	/* Allocate Raw Data Buffer for Decode Operation (Prepare 1MB for Planar output) */	
	  	/* Or user needs to get image size to allocate buffer before Decode Trigger for Planar */
	  	g_pu8DecFrameBuffer = (PUINT8)malloc(sizeof(CHAR) * (u32BufferSize + 0x03));			
		
		  if(g_pu8DecFrameBuffer == NULL)
		  {
			  sysprintf("\tCan't allocate the buffer for decode (size 0x%X)\n",PLANAR_DEC_BUFFER_SIZE);
			  return FALSE;
		  }
			
		  u32YBuffer =  (((UINT32) g_pu8DecFrameBuffer + 0x03) & ~0x03) | 0x80000000;


    	/* For Normal Decode buffer allocation */
      if(u32Format == JPEG_DEC_YUV422)   
      {	       
			  u32UBuffer = u32YBuffer + u32Width * u32Height;
			  u32VBuffer = u32UBuffer + u32Width * u32Height / 2;
      }
      else if(u32Format == JPEG_DEC_YUV444)
		  {
			  u32UBuffer = u32YBuffer + u32Width * u32Height;
			  u32VBuffer = u32UBuffer + u32Width * u32Height;
	  	}
		  else
		  {
			  u32UBuffer = u32YBuffer + u32Width * u32Height;
			  u32VBuffer = u32UBuffer + u32Width * u32Height / 4;
		  } 			
			
		  /* Set Decoded Image Address (Only Can be set before Decode Trigger for Planar;The address can set any time before existing Header Decode Complete Callback function) */
		  jpegIoctl(JPEG_IOCTL_SET_YADDR, u32YBuffer, 0);		
		  jpegIoctl(JPEG_IOCTL_SET_UADDR, u32UBuffer, 0);		
		  jpegIoctl(JPEG_IOCTL_SET_VADDR, u32VBuffer, 0);			
		  sysprintf("\tThe Y/U/V Buffer prepared for Planar format starts from 0x%X,0x%X,0x%X\n", u32YBuffer, u32UBuffer, u32VBuffer);
		}
		else
		{
			/* Allocate Raw Data Buffer for Decode Operation (Final destination address for Decode Ouptut Wait) */	
			g_pu8DecFrameBuffer = (PUINT8)malloc(sizeof(CHAR) * (u32BufferSize + 0x03));			
		
			if(g_pu8DecFrameBuffer == NULL)
			{
				sysprintf("\tCan't allocate the buffer for decode (size 0x%X)\n",u32BufferSize);
				return FALSE;
			}

			u32FrameBuffer =  (((UINT32) g_pu8DecFrameBuffer + 0x03) & ~0x03) | 0x80000000;
				
			sysprintf("\tThe allocated buffer for decoded data starts from 0x%X, Size is 0x%X\n", u32FrameBuffer,u32BufferSize);			
				
			if(g_bDecWindecStride && g_bDecWindecTest)
			{
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_RGB888)
				{
				  memset((UINT32*)u32FrameBuffer,fill, u32Width * u32Height * 4);
				}
				else
				{
				  if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_YUV422)
				  {
				    UINT32 i;
				    for(i = 0;i<u32Width * u32Height * 2;i++)
				    {
				      if(i%2)
				        *((PUINT8)(u32FrameBuffer + i)) = 0x80;	
				      else
				        *((PUINT8)(u32FrameBuffer + i)) = 0xFF;
				    }   
				  }
				  else
				    memset((UINT32*)u32FrameBuffer,fill, u32Width * u32Height * 2);		
						
				  u32OutputOffset *= 2; 
				}
			}				
			/* Set Decoded Image Address (Can be set before Decode Trigger for Packet/Planar format)*/
			jpegIoctl(JPEG_IOCTL_SET_YADDR, u32FrameBuffer, 0);	
			sysprintf("\tThe Buffer prepared for packet format starts from 0x%X\n", u32FrameBuffer);
		}
	}	
  return TRUE;
}
/*-----------------------------------------------------------------------*/
/*  Decode Function          				                             */
/*-----------------------------------------------------------------------*/  
VOID JpegDecTest (VOID)
{	
	INT 	len;
	UINT32 u32BitstreamSize;
	JPEG_INFO_T jpegInfo;
	INT nWriteLen, nStatus, nReadLen;
	INT hFile;
	CHAR	suFileName[100];	
	
	/* Decode Output Wait */
	UINT32 u32Width,u32Height,u32Format,starttime,endtime;	
  JPEG_WINDOW_DECODE_T windec;
  UINT32 u32Stride, item, u32WindowXsize, u32WindowYsize;	
/*******************************************************************/	
/* 							Read JPEG file						   */
/*******************************************************************/			
	sysprintf("Input Jpeg File name\n");
	
	GetString();
		
	strcpy(decodePath, g_u8String);
	
	fsAsciiToUnicode(decodePath, suFileName, TRUE);
	
    hFile = fsOpenFile(suFileName, NULL, O_RDONLY);
	if (hFile > 0)
		sysprintf("\tOpen file:[%s], file handle:%d\n", decodePath, hFile);
	else
	{
		sysprintf("\tFailed to open file: %s (%x)\n", decodePath, hFile);
		return;
	}	
		
	u32BitstreamSize = fsGetFileSize(hFile);
	
	sysprintf("\tBit stream  size for Decode is %d\n", u32BitstreamSize);
	
	/* Allocate the Bitstream Data Buffer for Decode Operation */	
    g_pu8JpegBuffer = (PUINT8)malloc(sizeof(CHAR) * (u32BitstreamSize + 0x03));
    g_u32JpegBuffer = (((UINT32)g_pu8JpegBuffer + 0x03) & ~0x03) | 0x80000000;
    
    nStatus = fsReadFile(hFile, (UINT8 *)g_u32JpegBuffer, u32BitstreamSize, &nReadLen);
	
	if (nStatus < 0)
		sysprintf("\tRead error!!\n");		
	
	fsCloseFile(hFile);  
	
    if(ParsingJPEG((UINT8 *)(UINT32)g_u32JpegBuffer, nReadLen, &u32Width, &u32Height, &u32Format, TRUE) == ERR_MODE)
	{
		sysprintf("\tNot Support the JPEG sampling\n");	
    	goto END_ERROR_FORMAT;
	}
    if(g_bDecWindecTest)
    {
		  u32WindowXsize = (u32Width + 15) / 16;
		
		  u32WindowYsize = (u32Height + 15) / 16;
set_again:
        sysprintf(" * Window Decode Region (X,Y) = (%d,%d)\n", u32WindowXsize, u32WindowYsize);
        if(g_bDecPanelTest)
            sysprintf("    - Only %d x %d MCU region can show on Panel\n", PANEL_WIDTH/16,PANEL_HEIGHT/16);

        sysprintf("    - Input Window Start position X : ");
        while(1)
        {
            g_u32xStart = GetData();
            if(g_u32xStart <= u32WindowXsize)
                break;
        }
        sysprintf("    - Input Window Start position Y : ");

        while(1)
        {
            g_u32yStart = GetData();
            if(g_u32yStart <= u32WindowYsize)
                break;
        }

        sysprintf("    - Input Window End position X : ");

        while(1)
        {
            g_u32xEnd = GetData();
            if(g_u32xEnd <= u32WindowXsize && g_u32xStart <= g_u32xEnd)
                break;
        }
        sysprintf("    - Input Window End position Y : ");
        while(1)
        {
            g_u32yEnd = GetData();
            if(g_u32yEnd <= u32WindowYsize && g_u32yStart <= g_u32yEnd)
                break;
        }
        sysprintf(" * Win Dec - (%d, %d) ~ (%d, %d)\n", g_u32xStart, g_u32yStart, g_u32xEnd, g_u32yEnd);
        if(g_bDecPanelTest)
        {
            if(((g_u32xEnd - g_u32xStart) > PANEL_WIDTH/16) || ((g_u32yEnd - g_u32yStart) > PANEL_HEIGHT/16))
            {
                sysprintf("    - The region is over the panel size. Please set the regio nagain\n");
                goto set_again;
            }
        }

        if(!g_bDecPanelTest)
        {
            sysprintf("0: Enable Stride\n");
            sysprintf("1: Disable Stride\n");
            item = sysGetChar();
            if(item == '0')
                g_bDecWindecStride = 1;
            else
                g_bDecWindecStride = 0;
        }
        else
            g_bDecWindecStride = 1;
    }
/*******************************************************************/	
/* 						Decode JPEG Bitstream					   */
/*******************************************************************/	    

	/* JPEG Init */
	jpegInit();
	
	if(g_bDecIpwTest)
	{   
		/*******************************************************************/	
		/* 						Decode Input Setting					   */
		/*******************************************************************/		
		sysprintf ("\tDecode Input Test - %d KB Bitstream Buffer\n",DEC_IPW_BUFFERSIZE / 1024);
		sysprintf ("\tBitstream Buffer 0 0x%x \n",(UINT32)g_au8DecInputWaitBuffer | 0x80000000);
		sysprintf ("\tBitstream Buffer 1 0x%x \n",(UINT32)g_au8DecInputWaitBuffer | 0x80000000 + DEC_IPW_BUFFERSIZE/2);
		
		/* Reset the Buffer control parameter for Decode Input Wait function */
		g_u32IpwUsedSize = 0;
		
		/* Set Bit stream Address (g_au8DecInputWaitBuffer -> 8KB Buffer) */   
    	jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR,(UINT32) g_au8DecInputWaitBuffer, 0);

		g_u32BitstreamSize = u32BitstreamSize;
		
	    /* Set Decode Input Wait mode (Maximum size for the buffer is 2046 KB) */	
		jpegIoctl(JPEG_IOCTL_SET_DECINPUTWAIT_CALBACKFUN, (UINT32) JpegDecInputWait, DEC_IPW_BUFFERSIZE);
		
		if(u32BitstreamSize < DEC_IPW_BUFFERSIZE)
			len = u32BitstreamSize;
		else
			len = DEC_IPW_BUFFERSIZE;
		
		/* Prepare the data for first decode operation (Fill Buffer 0 & 1) */	
		/* Copy bitstream from the temp buffer (g_pu8JpegBuffer) to the buffer engine used (g_au8DecInputWaitBuffer) */	
        memcpy((char *)((UINT32)g_au8DecInputWaitBuffer | 0x80000000),(char *)g_u32JpegBuffer, len);
		
		/* The bitstream size put to Bitstream Buffer */
		g_u32IpwUsedSize += len;     	
    	
	}
	else
	{
		/* Set Bit stream Address */   
    	jpegIoctl(JPEG_IOCTL_SET_BITSTREAM_ADDR,(UINT32) g_pu8JpegBuffer, 0);	
    }   
    
    
	if(setOutputBuffer(u32Width, u32Height, u32Format) == FALSE)
		goto END_ERROR_FORMAT;
          
    /* Decode mode */	
	jpegIoctl(JPEG_IOCTL_SET_DECODE_MODE, g_u32DecFormat, 0);	
	
	/* Set JPEG Header Decode End Call Back Function */
	jpegIoctl(JPEG_IOCTL_SET_HEADERDECODE_CALBACKFUN, (UINT32) JpegDecHeaderComplete, 0);	
	
  if(g_bDecWindecTest)	
	{
    if(g_bDecWindecStride)
		{
			if(g_bDecPanelTest)
         u32Stride = PANEL_WIDTH;
			else
         u32Stride = u32Width;
		}
    else
      u32Stride = 16 * (g_u32xEnd - g_u32xStart + 1);
                         
    windec.u16StartMCUX = g_u32xStart;
    windec.u16StartMCUY = g_u32yStart;
    windec.u16EndMCUX = g_u32xEnd;
    windec.u16EndMCUY = g_u32yEnd;
    windec.u32Stride = u32Stride;
    jpegIoctl(JPEG_IOCTL_WINDOW_DECODE, (UINT32)&windec, 0);	
	}
	starttime = sysGetTicks(TIMER0);
	
    /* Trigger JPEG decoder */
    jpegIoctl(JPEG_IOCTL_DECODE_TRIGGER, 0, 0);  
        
	/* Wait for complete */
	if(jpegWait())
	{
		/*******************************************************************/	
		/* 						Prepare Output file name  	   			   */
		/*******************************************************************/	
		endtime = sysGetTicks(TIMER0);
		
		g_u8String[g_u32StringIndex] = '_';
		g_u8String[g_u32StringIndex + 1] = 0x00;
		
		strcpy(decodePath, g_u8String);
  	if(g_bDecWindecTest)
			strcat(decodePath, "WINDEC_"); 
		
		if(g_bDecIpwTest)
			strcat(decodePath, "IPW_"); 
					
		if(g_bDecPanelTest)
			strcat(decodePath, "Panel_"); 			
		
		if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)	
			strcat(decodePath, "PLANAR_"); 	
		else
		{
			switch(g_u32DecFormat)
			{
				case JPEG_DEC_PRIMARY_PACKET_YUV422:
					strcat(decodePath, "PACKET_YUV422_"); 	
					break;
				case JPEG_DEC_PRIMARY_PACKET_RGB555:
					strcat(decodePath, "PACKET_RGB555_"); 	
					break;
				case JPEG_DEC_PRIMARY_PACKET_RGB565:
					strcat(decodePath, "PACKET_RGB565_"); 
					break;	
				case JPEG_DEC_PRIMARY_PACKET_RGB888:
					strcat(decodePath, "PACKET_RGB888_"); 						
					break;		
			}		
		}
		jpegGetInfo(&jpegInfo);
		sysprintf("\tJPEG Decode Complete!!\n");
		sysprintf("\t\tJpeg YuvFormat ");
		switch(jpegInfo.yuvformat)
		{
			case JPEG_DEC_YUV420:
				sysprintf("YUV420\n");
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
					strcat(decodePath, "YUV420_"); 	
				break;
			case JPEG_DEC_YUV422:
				sysprintf("YUV422\n");
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
					strcat(decodePath, "YUV422_"); 	
				break;			
			case JPEG_DEC_YUV444:
				sysprintf("YUV444\n");
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
					strcat(decodePath, "YUV444_"); 	
				break;			
			case JPEG_DEC_YUV411:
				sysprintf("YUV411\n");
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
					strcat(decodePath, "YUV411_"); 	
				break;			
			case JPEG_DEC_GRAY:
				sysprintf("GRAY\n");
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)				
					strcat(decodePath, "GRAY_"); 	
				break;			
			case JPEG_DEC_YUV422T:		
				sysprintf("YUV422T\n");
				if(g_u32DecFormat == JPEG_DEC_PRIMARY_PLANAR_YUV)
					strcat(decodePath, "YUV422T_"); 	
				break;			
		}
		sysprintf("\t\tJpeg Width = %d\n",jpegInfo.jpeg_width);			/* JPEG width in Bitstream header */
		sysprintf("\t\tJpeg Height = %d\n",jpegInfo.jpeg_height);		/* JPEG width in Bitstream header */	
		strcat(decodePath, "size");
		
		if(g_bDecPanelTest)
		{
		   	strcat(decodePath, intToStr(PANEL_WIDTH));
	   		strcat(decodePath, "x");
		   	strcat(decodePath, intToStr(PANEL_HEIGHT));		
		}
		else
		{
            jpegIoctl(JPEG_IOCTL_GET_WINDOW_DECODE_SIZE, (UINT32)&u32WindowXsize, (UINT32)&u32WindowYsize);

            if(g_bDecWindecTest)
            {
                if(g_bDecWindecStride)
                    strcat(decodePath, intToStr(u32Stride));
                else
                    strcat(decodePath, intToStr(u32WindowXsize));
                strcat(decodePath, "x");
                strcat(decodePath, intToStr(u32WindowYsize));
            }
            else
            {
                strcat(decodePath, intToStr(jpegInfo.width));
                strcat(decodePath, "x");
                strcat(decodePath, intToStr(jpegInfo.height));
            }
        }
        strcat(decodePath, ".dat");

        if(g_bDecWindecTest)
        {
            sysprintf("\t\tOutput Image Width = %d\n", u32WindowXsize);  /* Decode image width */
            sysprintf("\t\tOutput Image Height = %d\n",u32WindowYsize);  /* Decode image height */
        }
        else
		{
		  sysprintf("\t\tOutput Image Width = %d\n",jpegInfo.width);		/* Decode image width */
		  sysprintf("\t\tOutput Image Height = %d\n",jpegInfo.height);	/* Decode image height */	
		}
		
		if(jpegInfo.stride)
			sysprintf("\t\tJpeg Decode Image Stride = %d\n",jpegInfo.stride);			
			
		/*******************************************************************/	
		/* 						Get Output file size	  	   			   */
		/*******************************************************************/
					
		len = jpegInfo.image_size[0];
			
		if(g_bDecPanelTest)
		{		
			sysprintf ("\t\tOutput Raw data size from JPEG engine %d x %d\n", PANEL_WIDTH, TARGET_HEIGHT);
			
			if(g_u32DecFormat == JPEG_DEC_PRIMARY_PACKET_RGB888)
				len = PANEL_WIDTH * PANEL_HEIGHT * 4;
			else
				len = PANEL_WIDTH * PANEL_HEIGHT * 2;
			
			sysprintf ("\t\tOutput Raw data size (file) %d x %d\n", PANEL_WIDTH, PANEL_HEIGHT);
			
			vpostLCMInit(&lcdInfo, (UINT32*)((UINT32)g_pu8DecFrameBuffer | 0x80000000));		
			
		}
		else
		{
			sysprintf ("\t\tOuput Raw data size (file) %d\n", len);		
		}		
    }
    else
    {
    	sysprintf("\tJPEG Decode Error!!\n");
    	len = 0;
    	goto END;
    }

/*******************************************************************/	
/* 					   	  Write to Disk	    	    			   */
/*******************************************************************/	
    
	fsAsciiToUnicode(decodePath, suFileName, TRUE);	  
	
    hFile = fsOpenFile(suFileName, NULL, O_CREATE | O_TRUNC);
	if (hFile > 0)
		sysprintf("\tOpen file:[%s], file handle:%d\n", decodePath, hFile);
	else
	{
		sysprintf("\tFailed to open file: %s (%x)\n", decodePath, hFile);
	}

	nStatus = fsWriteFile(hFile, (UINT8 *)((UINT32)g_pu8DecFrameBuffer | 0x80000000), len, &nWriteLen);
	
	if (nStatus < 0)
		sysprintf("\tWrite error!!\n");
	
	fsCloseFile(hFile);
		
END:	
	free(g_pu8DecFrameBuffer);
END_ERROR_FORMAT:	
	free(g_pu8JpegBuffer);
	
	
	sysprintf ("\tDecode test completed!\n");	
	
	return;
}


