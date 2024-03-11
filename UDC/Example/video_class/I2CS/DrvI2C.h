/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/

#ifndef _DRVI2C_H_
#define _DRVI2C_H_
#include "wbtypes.h"

#ifdef  __cplusplus
extern "C"
{
#endif
#ifndef _W55FA93_REG_H
#include "W55FA93_reg.h"
#endif

#define 	ERRCODE			UINT32 

// #define Error Code
//E_DRVI2C_WRITE_FAIL		Transmitter doesn't receive a ACK 
//E_DRVI2C_PIN_UNAVAILABLE	Pins conflict
#define E_DRVI2C_WRITE_FAIL         	0xFFFF0001	//_SYSINFRA_ERRCODE(TRUE, MODULE_ID_DRVI2C, 0)
#define E_DRVI2C_PIN_UNAVAILABLE    	0xFFFF0002	//_SYSINFRA_ERRCODE(TRUE, MODULE_ID_DRVI2C, 1)


// Acknowledgement type
#define DrvI2C_Ack_No		0
#define DrvI2C_Ack_Have		1

#define _DRVI2C_SCK_SETIN(PortIndex, PinMask)		outp32(REG_GPIOA_OMD+(PortIndex),inp32(REG_GPIOA_OMD+(PortIndex))&(~PinMask)) //PortIndex<<4 == PortIndex*0x10
#define _DRVI2C_SDA_SETIN(PortIndex, PinMask)		outp32(REG_GPIOA_OMD+(PortIndex),inp32(REG_GPIOA_OMD+(PortIndex))&(~PinMask))
#define _DRVI2C_SCK_SETOUT(PortIndex, PinMask)		outp32(REG_GPIOA_OMD+(PortIndex),inp32(REG_GPIOA_OMD+(PortIndex))|PinMask)
#define _DRVI2C_SDA_SETOUT(PortIndex, PinMask)		outp32(REG_GPIOA_OMD+(PortIndex),inp32(REG_GPIOA_OMD+(PortIndex))|PinMask)

#define _DRVI2C_SCK_SETHIGH(PortIndex, PinMask)	outp32(REG_GPIOA_DOUT+(PortIndex),inp32(REG_GPIOA_DOUT+(PortIndex))|PinMask)
#define _DRVI2C_SCK_SETLOW(PortIndex, PinMask)		outp32(REG_GPIOA_DOUT+(PortIndex),inp32(REG_GPIOA_DOUT+(PortIndex))&(~PinMask))
#define _DRVI2C_SDA_SETHIGH(PortIndex, PinMask)	outp32(REG_GPIOA_DOUT+(PortIndex),inp32(REG_GPIOA_DOUT+(PortIndex))|PinMask)
#define _DRVI2C_SDA_SETLOW(PortIndex, PinMask)		outp32(REG_GPIOA_DOUT+(PortIndex),inp32(REG_GPIOA_DOUT+(PortIndex))&(~PinMask))

#define _DRVI2C_SCK_GETVALUE(PortIndex, PinMask) 	(inp32(REG_GPIOA_PIN+(PortIndex))&PinMask)
#define _DRVI2C_SDA_GETVALUE(PortIndex, PinMask)  	(inp32(REG_GPIOA_PIN+(PortIndex))&PinMask)

typedef void (*PFN_DRVI2C_TIMEDELY)(UINT32);
typedef struct
{
	UINT32 u32SCKPortIndex;
	UINT32 u32SCKPinMask;
	UINT32 u32SDAPortIndex;
	UINT32 u32SDAPinMask;
} S_I2C_Channel;

typedef enum
{
	eDRVGPIO_GPIOA = 0x00,
	eDRVGPIO_GPIOB = 0x10,
	eDRVGPIO_GPIOC = 0x20,
	eDRVGPIO_GPIOD = 0x30
}E_DRVGPIO_PORT;
typedef enum
{
	eDRVGPIO_PIN0 = BIT0,
	eDRVGPIO_PIN1 = BIT1,
	eDRVGPIO_PIN2 = BIT2,
	eDRVGPIO_PIN3 = BIT3,
	eDRVGPIO_PIN4 = BIT4,
	eDRVGPIO_PIN5 = BIT5,
	eDRVGPIO_PIN6 = BIT6,
	eDRVGPIO_PIN7 = BIT7,
	eDRVGPIO_PIN8 = BIT8,
	eDRVGPIO_PIN9 = BIT9,
	eDRVGPIO_PIN10 = BIT10,
	eDRVGPIO_PIN11 = BIT11,
	eDRVGPIO_PIN12 = BIT12,
	eDRVGPIO_PIN13 = BIT13,
	eDRVGPIO_PIN14 = BIT14,
	eDRVGPIO_PIN15 = BIT15	
}E_DRVGPIO_BIT;


// Define function

ERRCODE
DrvI2C_Open(
    UINT32 u32SCKPortIndex,
    UINT32 u32SCKPinMask,
    UINT32 u32SDAPortIndex,
    UINT32 u32SDAPinMask,
    PFN_DRVI2C_TIMEDELY	pfntimedelay
);

void DrvI2C_Close(void);

void DrvI2C_SendStart(void);

void DrvI2C_SendStop(void);

ERRCODE
DrvI2C_WriteByte(
    UINT8 u8Data,
    UINT8 u8AckType,
    UINT8 u8Length
);

UINT32
DrvI2C_ReadByte(
	UINT8 u8AckType,
	UINT8 u8Length
);


UINT8
DrvI2C_Slave_ReadByte(
    UINT8 u8AckType
);

UINT32
DrvI2C_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif









