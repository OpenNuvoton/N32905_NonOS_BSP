/**
  ******************************************************************************
  * @file    /Gneiss_Sample_Code_LLD.h
  * @author  Winbond FAE Steam Lin
  * @version V1.1.0
  * @date    09-December-2015
  * @brief   This code provide the low level RPMC hardware operate function based on STM32F205.
  *            
  * COPYRIGHT 2015 Winbond Electronics Corporation.
*/ 
#include "wblib.h"
#include "Secureic.h"
unsigned int RPMC_ReadCounterData(void);
unsigned int RPMC_ReadRPMCstatus(unsigned int checkall);
unsigned int RPMC_WrRootKey(unsigned int cadr,unsigned char *rootkey);
unsigned int RPMC_UpHMACkey(unsigned int cadr,unsigned char *rootkey,unsigned char *hmac4,unsigned char *hmackey);
unsigned int RPMC_IncCounter(unsigned int cadr,unsigned char *hmackey,unsigned char *input_tag);
unsigned char RPMC_Challenge(unsigned int cadr,unsigned char *hmackey,unsigned char *input_tag);
void RPMC_ReqCounter(unsigned int cadr, unsigned char *hmackey,unsigned char *tag);
INT32 RPMC_ReadJEDECID(PUINT8 data);
INT16 RPMC_ReadUID(PUINT8 data);

/************************ (C) COPYRIGHT Winbond Electronics Corporation *****END OF FILE****/