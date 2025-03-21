/**************************************************************************//**
 * @file     common.c
 * @brief    Functions for JPEG Driver
 *           - JPEG Driver Callback functions 
 *           - JPEG Header Parser function
 *           - JPEG Output file name function  
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

#define WIDTH 320
#define HEIGHT 240

//#define WIDTH 176
//#define HEIGHT 144


//#define WIDTH 128
//#define HEIGHT 96


CHAR g_u8String[100];
UINT32 g_u32StringIndex;

/*-----------------------------------------------------------------------*/
/*  Header Decode Complete Callback function                             */
/*-----------------------------------------------------------------------*/            
BOOL JpegDecHeaderComplete(void)                  
{	
	JPEG_INFO_T jpegInfo;

	jpegGetInfo(&jpegInfo);		
	
	return TRUE;	/* Return TRUE to continue Decode operation, Otherwise, Stop Decode operation */
}

/* Decode Input Wait Callback function */   
BOOL JpegDecInputWait(UINT32 u32Address,UINT32 u32Size)
{
	INT32 i32Size;
	/* Half bitstream in the Buffer (u32Address) has been decoded and User needs to put  remaining bitstream (u32Size) to the Buffer (u32Address) */	
	/* The JPEG engine is decoding the Bitstream in the other Buffer at the same time */
	if(g_u32BitstreamSize > g_u32IpwUsedSize)
	{
		i32Size = g_u32BitstreamSize - g_u32IpwUsedSize;
		
		if(i32Size > u32Size)
			i32Size = u32Size;
			
		memcpy((char *)(u32Address),(char *)(((UINT32)g_pu8JpegBuffer | 0x80000000) + g_u32IpwUsedSize), i32Size);	
		g_u32IpwUsedSize += i32Size;
		sysprintf("\tDec Input Wait - Buffer to fill %dBytes Bitstream to 0x%X & Already put %3d Bytes Bitstream in Buffer \n", i32Size, u32Address, g_u32IpwUsedSize);	
	}	
		
	return TRUE;	/* Return TRUE to continue Decode operation, Otherwise, Stop Decode operation */
}

VOID JpedInitDecOutputWaitBuffer(UINT32 u32Width,UINT32 u32Height, UINT32 u32jpegFormat)
{
	UINT32 u32MCUX, u32MCUY, u32MUC_Width,u32MCU_Height,u32MCU_Space,u32TMP;	
	UINT32 u32j,u32k, u32sizeCount = 0;	
	
	sysprintf("\n\t<Prepare Buffer for Decode Output Wait>\n");
	
	switch(u32jpegFormat)
	{
		case JPEG_DEC_YUV420:				
 			u32MCUX = u32Width / 16;
 			if(u32Width % 16)
 				u32MCUX++;	 			
		 	
 			u32MCUY = u32Height / 16;
 			if(u32Height % 16)
 				u32MCUY++;		
 				
 			u32MUC_Width = 16 * u32MCUX;
 			u32MCU_Height = 16 * u32MCUY;	
 			u32MCU_Line = u32MUC_Width * 16;	
 			u32MCU_Space = u32MUC_Width * (16+1);			 				 				
 			sysprintf("\tJPEG Format YUV420, MCUX %d, MUCY %d Width %d Height %d \n", u32MCUX,u32MCUY, u32MUC_Width,u32MCU_Height );	
			break;
		case JPEG_DEC_YUV422:
 			u32MCUX = u32Width / 16;
 			if(u32Width % 16)
 				u32MCUX++;	 			
		 	
 			u32MCUY = u32Height / 8;
 			if(u32Height % 8)
 				u32MCUY++;		
 			u32MUC_Width = 16 * u32MCUX;
 			u32MCU_Height = 8 * u32MCUY;	
 			u32MCU_Line = u32MUC_Width * 8; 
 			u32MCU_Space = u32MUC_Width * (8+1);						
			sysprintf("\tJPEG Format YUV422, MCUX %d, MUCY %d Width %d Height %d \n", u32MCUX,u32MCUY, u32MUC_Width,u32MCU_Height );	
			break;
		case JPEG_DEC_YUV444:
 			u32MCUX = u32Width / 8;
 			if(u32Width % 8)
 				u32MCUX++;	 			
		 	
 			u32MCUY = u32Height / 8;
 			if(u32Height % 8)
 				u32MCUY++;		
 			u32MUC_Width = 8 * u32MCUX;
 			u32MCU_Height = 8 * u32MCUY;
 			u32MCU_Line = u32MUC_Width * 8;	 	
 			u32MCU_Space = u32MUC_Width * (8+1);						
			sysprintf("\tJPEG Format YUV44, MCUX %d, MUCY %d Width %d Height %d \n", u32MCUX,u32MCUY, u32MUC_Width,u32MCU_Height );	
			break;		
	}		
	
	switch(g_u32DecFormat)
	{
		case JPEG_DEC_PRIMARY_PACKET_YUV422:
		case JPEG_DEC_PRIMARY_PACKET_RGB555:	
		case JPEG_DEC_PRIMARY_PACKET_RGB565:
			u32MCU_Space = u32MCU_Space * 2;	
			u32MCU_Line = u32MCU_Line * 2;	
			u32TotalSize = u32MUC_Width * u32MCU_Height * 2;
			break;
		case JPEG_DEC_PRIMARY_PACKET_RGB888:
			u32MCU_Space = u32MCU_Space * 4;	
			u32MCU_Line = u32MCU_Line * 4;	
			u32TotalSize = u32MUC_Width * u32MCU_Height * 4;				
			break;		
	}
	sysprintf("\tMCU Line Size is 0x%X\n",u32MCU_Line);	
	sysprintf("\tTotal Size is 0x%X\n",u32TotalSize);
	
	/* Buffer allocate (Only for Test) */
	u32j = 1;
	u32TMP = u32TotalSize;
	while(1)
	{
		if(u32TMP > u32j * u32MCU_Line)
			u32TMP = u32TMP - u32j * u32MCU_Line; 	
		else
		{
			u32j++;
			break;
		}
		u32j++;
	}
	sysprintf("\tNeed to trigger %d times\n",u32j);
	u32sizeCount = 0;
	for(u32k = 0;u32k<u32j;u32k++)
	{
		apBuffer[u32k] = malloc(sizeof(UINT8) * u32MCU_Line * (u32k + 1) + 0x03);
		memset(apBuffer[u32k], 0, u32MCU_Space);
        sysprintf("\tBuffer %d address 0x%08X size 0x%08X\n",u32k , ((UINT32)apBuffer[u32k] + 0x03) & ~0x03, u32MCU_Line * (u32k + 1));
		u32sizeCount = u32sizeCount + u32MCU_Line * (u32k + 1);
		
	}
	sysprintf("\tExpected Memory size [0x%08X ~0x%08X]\n",u32sizeCount - u32MCU_Line * (u32j+1) , u32sizeCount);
			
}		

/************************************************************************************/	
/* Function		: ParsingJPEG														*/		
/* Description	: Check the information as follows									*/
/*					(1) The baseline mode and simple file format test.				*/
/*					(2) Get the Quantization Table Number and first table data      */		
/* Input		: JPEG_Buffer	->	The JPEG bit-stream starting address			*/
/*				  Length		->	The JPEG bit-stream length						*/
/* Output		: >=0	->	the Quantization Table Number							*/
/*				   -1	->	Not the Baseline Mode (ERR_MODE) 						*/
/*				   -2	->	Wrong file format (ERR_FORMAT)							*/
/************************************************************************************/
INT32 ParsingJPEG(PUINT8 JPEG_Buffer,UINT32 Length, PUINT32 pu32Width, PUINT32 pu32Height, PUINT32 u32Format,BOOL bPrimary)
{
	//	HByte,LByte		:	For JPEG Marker decode
	//	MLength			:	Length of Marker (all data in the marker)
	//	index			:	The address index of the JPEG bit-stream
	//	QT_Count		:	Quantization Table Counter

    unsigned char HByte,LByte;
    unsigned short int MLength;		
    int index;	
    int HuffTable = 0xF;	
    int HuffCount = 0;
    int HuffIndex = 0;
	PUINT8 pu8Addr;	
	int HuffmanIndex[4] = {0};
	int HuffmanSize[5] = {0};
	INT32 i32Result = ERR_SUCCESS;	
	BOOL bWorkaround = FALSE;
	index = 0;			

	while(index < Length)
    {
		HByte = JPEG_Buffer[index++];

        if(HByte == 0xFF)
        {
			LByte = JPEG_Buffer[index++];
			switch(LByte)		//May be a Marker
            {
				case 0xD8:		//SOI Marker (Start Of Image)
						break;
				case 0xC1:case 0xC2:case 0xC3:case 0xC5:
                case 0xC6:case 0xC7:case 0xC8:case 0xC9:
                case 0xCA:case 0xCB:case 0xCD:case 0xCE:
                case 0xCF:
						return ERR_MODE;				/* SOF Marker(Not Baseline Mode) */
						break;
                case 0xDB:								/* DQT Marker (Define Quantization Table) */
						if(index + 1 > Length)
							return ERR_FORMAT;			/* Wrong file format */
                        HByte = JPEG_Buffer[index++];
                        LByte = JPEG_Buffer[index++];
						MLength = (HByte << 8) + LByte;
                        index += MLength - 2;			/* Skip DQT Data */			
						
						if(index > Length)
							return ERR_FORMAT;			/* Wrong file format */
                        break;
                case 0xC0:      						/* SOF Marker (Baseline mode Start Of Frame) */
                {
                		UINT16 u16Height,u16Width,end_index,Nf,Ci,HSF[3],VSF[3],i;
                		
						if(index + 1 > Length)
							return ERR_FORMAT;			/* Wrong file format */
						HByte = JPEG_Buffer[index++];
                        LByte = JPEG_Buffer[index++];
						MLength = (HByte << 8) + LByte;
						end_index = index + MLength - 2;
						index++;						
						HByte = JPEG_Buffer[index++];
                        LByte = JPEG_Buffer[index++];						
						u16Height = (HByte << 8) + LByte;	/* Get Image Height */						
						HByte = JPEG_Buffer[index++];
                        LByte = JPEG_Buffer[index++];						
						u16Width = (HByte << 8) + LByte;	/* Get Width Height */	
						
						sysprintf("\tWidth %d, Height %d\n", u16Width, u16Height);	
						*pu32Width = u16Width;
						*pu32Height	= u16Height;
						Nf = JPEG_Buffer[index++];
						//sysprintf("Component  %d\n", Nf);	
						if(Nf != 3)
							return ERR_MODE;
							
						for(i=0;i<Nf;i++)					/* Get Sampling factors */
                        {
                            Ci = JPEG_Buffer[index++];                           
                            HByte = JPEG_Buffer[index++];
                            HSF[i]=HByte/16;
                            VSF[i]=HByte%16;
                            index++;
                           
                   		}			
						/*for(i=0;i<Nf;i++)
                        {
                        	sysprintf("Component %d, Hi %d, Vi %d\n",i,HSF[i],VSF[i]);                          
                   		}*/
                   		
               		
                   		if(HSF[1] == VSF[1] == HSF[2] == VSF[2] == 1)
	               		{
    	               		if(HSF[0] == 2 && VSF[0] == 2)
        	           		{
            	       			//sysprintf("YUV420\n");
            	       			*u32Format = JPEG_DEC_YUV420;
                	   		}
                	   		else if (HSF[0] == 2 && VSF[0] == 1)
        	           		{
            	       			//sysprintf("YUV422\n");
            	       			*u32Format = JPEG_DEC_YUV422;
            	       			if((u16Height % 16) <= 8)
            	       				i32Result = ERR_ONLY_NORMAL;
                	   		}	                   			
                   			else if (HSF[0] == VSF[0] == 1)
        	           		{
            	       			//sysprintf("YUV444\n");
            	       			*u32Format = JPEG_DEC_YUV444;
            	       			if((u16Height % 16) <= 8)
            	       				i32Result = ERR_ONLY_NORMAL;            	       			
                	   		}	                   			
                	   		else
                	   			return ERR_MODE;	                   			
                   		}				
	               		else
    	           			return ERR_MODE;
    	           	
                   			
						if(end_index != index)
							return ERR_FORMAT;			/* Wrong file format */		
							
						if(index > Length)
							return ERR_FORMAT;			/* Wrong file format */
						
						break;
				}
				case 0xDA:      						/* Scan Header */
				{
						int i,Ns,Td[3],Ta[3],end_index,Tda[3];
						if(index + 1 > Length)
							return ERR_FORMAT;			/* Wrong file format */
                        HByte = JPEG_Buffer[index++];
                        LByte = JPEG_Buffer[index++];
						MLength = (HByte << 8) + LByte;
						
						end_index = index + MLength -2;
						
                        Ns = JPEG_Buffer[index++];
                       // sysprintf("Ns %d\n", Ns);
                        for(i=0;i<Ns;i++)
                        {
                            index++;
                            Tda[i] = index;
                            HByte = JPEG_Buffer[index++];
                            Td[i]=HByte/16;
                            Ta[i]=HByte%16;
                        }				
                        if(bWorkaround)		
                        {
		                  /*  for(i=0;i<Ns;i++)
								sysprintf("Component %d Td %d Ta %d\n", i ,Td[i],Ta[i]);*/
							if(Td[0] == 1 && Td[1] == 0 && Td[2] == 0)
							{
								JPEG_Buffer[Tda[0]] =  JPEG_Buffer[Tda[0]] & ~0xF0;
								JPEG_Buffer[Tda[1]] =  JPEG_Buffer[Tda[1]] | 0x10;
								JPEG_Buffer[Tda[2]] =  JPEG_Buffer[Tda[2]] | 0x10;
								JPEG_Buffer[HuffmanIndex[0] - 1] = 0x01; 
								JPEG_Buffer[HuffmanIndex[1] - 1] = 0x00; 
								
							}
							if (Ta[0] == 1 && Ta[1] == 0 && Ta[2] == 0)
							{
								JPEG_Buffer[Tda[0]] =  JPEG_Buffer[Tda[0]] & ~0x0F;
								JPEG_Buffer[Tda[1]] =  JPEG_Buffer[Tda[1]] | 0x01;
								JPEG_Buffer[Tda[2]] =  JPEG_Buffer[Tda[2]] | 0x01;	
								JPEG_Buffer[HuffmanIndex[2] - 1] = 0x11; 
								JPEG_Buffer[HuffmanIndex[3] - 1] = 0x10; 												
							}						
						}
	                    index += 3;
	                    
						if(end_index != index)						
							return ERR_FORMAT;			/* Wrong file format */		                    
	                   
						if(index > Length)
							return ERR_FORMAT;			/* Wrong file format */
						if(bWorkaround)
						{
		                    index =0;	                 				

							pu8Addr = (PUINT8)JPEG_Buffer;		
		                    
		                    if(HuffCount == 2 || HuffCount == 3)
		                    {				
		                    	UINT32 u32Length = 0;	
		                    	
		                    	if(HuffTable != 0xA)
		                    	{	                    	
									pu8Addr[256] = 0x00;		/* Clear Original 0xFF */
									pu8Addr[257] = 0x00; 	   	/* Clear Original 0xD8 */                 			
									pu8Addr[index++] = 0xFF;
									pu8Addr[index++] = 0xD8; 	                    	
									pu8Addr[index++] = 0xFF;
									pu8Addr[index++] = 0xC4;								
									
									index +=2;
									
									for(i = 0; i < 4; i++)
									{
										if(HuffTable & (1 << i))
										{
											switch(i)
											{
												case 0:
													pu8Addr[index++] = 0x00;
													u32Length += HuffmanSize[1] + 1;
													memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[1], HuffmanSize[1]);
													index += HuffmanSize[1];											
													break;
												case 1:			
													pu8Addr[index++] = 0x01;
													u32Length += HuffmanSize[0] + 1;
													memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[0], HuffmanSize[0]);
													index += HuffmanSize[0];
																								
													break;
												case 2:
													pu8Addr[index++] = 0x10;
													u32Length += HuffmanSize[3] + 1;											
													memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[3], HuffmanSize[3]);
													index += HuffmanSize[3];																						
													break;
												case 3:				
													pu8Addr[index++] = 0x11;	
													u32Length += HuffmanSize[2] + 1;													
													memcpy(pu8Addr + index, pu8Addr + HuffmanIndex[2], HuffmanSize[2]);
													index += HuffmanSize[2];			
													break;										
											}															
										}						
									}
									u32Length+=2;
									pu8Addr[4] =(u32Length >> 8) & 0xFF;
									pu8Addr[5] = u32Length & 0xFF;							 
			                    }
		                   	}
		                }
	                    return i32Result;	
						break;
				}
				case 0xC4:      //DHT Marker (Define Huffman Table)
				{
						int Li,i;
						if(index + 1 > Length)
							return ERR_FORMAT;				/* Wrong file format */
                        HByte = JPEG_Buffer[index++];
                        LByte = JPEG_Buffer[index++];
						MLength = (HByte << 8) + LByte - 2;
						if((index + MLength) > Length)
							return ERR_FORMAT;				/* Wrong file format */
							
                        do
                        {                        	
                            switch(JPEG_Buffer[index++])	/* Tc & Th */
                            {
                            	case 0x00:
                            		HuffmanIndex[0] = index;
                            		HuffTable ^= 0x1;
                            		HuffIndex = 0;
                            		break;
                            	case 0x01:
                            		HuffmanIndex[1] = index;
                            		HuffTable ^= 0x2;
                            		HuffIndex = 1;
                            		break;
                            	case 0x10:
                            		HuffmanIndex[2] = index;
                            		HuffTable ^= 0x4;
                            		HuffIndex = 2;
                            		break;
                            	case 0x11:
                            		HuffmanIndex[3] = index;
                            		HuffTable ^= 0x8;
                            		HuffIndex = 3;
                            		break;							
                            }   
                            HuffCount++;        
                                          
                            Li=0;
                            for(i=1;i<=16;i++)
                            {

                            	Li+=JPEG_Buffer[index++];
                            }        
                            HuffmanSize[HuffIndex] = Li + 16;       
                            MLength=MLength-17 - Li;
                            index = index + Li;

                    	}while(MLength!=0);		
				
                        break;
                }        
				case 0xE0:case 0xE1:case 0xE2:case 0xE3:
				case 0xE4:case 0xE5:case 0xE6:case 0xE7:
				case 0xE8:case 0xE9:case 0xEA:case 0xEB:
				case 0xEC:case 0xED:case 0xEE:case 0xEF:
				case 0xFE:
				{
                		/* Application Marker && Comment */
						int tmp;
						if(index + 1 > Length)
							return ERR_FORMAT;			/* Wrong file format */
						tmp = index - 2;
						HByte = JPEG_Buffer[index++];
						LByte = JPEG_Buffer[index++];
						MLength = (HByte << 8) +LByte;	
						if(bPrimary)																						
	                        index += MLength - 2;			/* Skip Application or Comment Data	*/			
                        break;
                }
			}
		}
	}

	return ERR_FORMAT;		//Wrong file format
}

       
/*-----------------------------------------------------------------------*/
/*  Jpeg Output File Name Function				                         */
/*-----------------------------------------------------------------------*/
CHAR	decodePath[50];
CHAR	g_szOutputString[15];         
          
CHAR *intToStr(UINT32 u32quotient)
{
    UINT32 u32i = 0,
           u32reverseStringElementNo,
           u32remainder;
    CHAR szReverseString[15];
         
    do
    {
        u32remainder = u32quotient % 10;
        szReverseString[u32i] = (char)(u32remainder + 0x30);
        u32quotient /= 10;
        u32i++;
    } while(u32quotient != 0);
    
    u32reverseStringElementNo = u32i - 1;
    
    for(u32i = 0; u32i <= u32reverseStringElementNo; u32i++)
        g_szOutputString[u32i] = szReverseString[u32reverseStringElementNo - u32i];

    g_szOutputString[u32i] = '\0';
    
    return g_szOutputString;
}      
      
UINT32 GetData(VOID)
{
	UINT8 u8Data;
	UINT32 u32Data;
	u8Data = '0';
	u32Data = 0;
	
	while(1)
	{	
		u8Data = sysGetChar();
		
		if(u8Data >= 0x30 && u8Data <= 0x39)
		{
			u8Data = u8Data - 0x30;
			sysprintf("%d", u8Data);
			u32Data = u32Data * 10 + u8Data;  
		
		}
		if(u8Data == 0x0D)
		{
			sysprintf("\n");
			break;		
		}
		
	}
	return u32Data;
}

VOID GetString(VOID)
{	
	UINT8 u8Data;
	
	strcpy(g_u8String, "C:\\");			
	
	g_u32StringIndex = 3;
	
	while(1)
	{	
		u8Data = sysGetChar();		
	
		if(u8Data == 0x0D)
		{
			sysprintf("\n");
			g_u8String[g_u32StringIndex++] = 0x00;
			break;		
		}
		else
		{
			g_u8String[g_u32StringIndex] = u8Data;
			sysprintf("%c",g_u8String[g_u32StringIndex++]);
		}
		
	}
	g_u32StringIndex -= 5;
	return;
}






















