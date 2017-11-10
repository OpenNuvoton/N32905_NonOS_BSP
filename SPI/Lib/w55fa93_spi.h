#ifndef _SPILIB_H_
#define _SPILIB_H_

#define SPI_SET_CLOCK			0

#define SPI_8BIT	8
#define SPI_16BIT	16
#define SPI_32BIT	32

typedef struct _spi_info_t
{
	INT32	nPort;
	BOOL	bIsSlaveMode;
	BOOL	bIsClockIdleHigh;
	BOOL	bIsLSBFirst;
	BOOL	bIsAutoSelect;
	BOOL	bIsActiveLow;
	BOOL	bIsTxNegative;
} SPI_INFO_T;

/* extern function */
VOID spiIoctl(INT32 spiPort, INT32 spiFeature, INT32 spicArg0, INT32 spicArg1);
INT  spiOpen(SPI_INFO_T *pInfo);
INT  spiRead(INT port, INT RxBitLen, INT len, CHAR *pDst);
INT  spiWrite(INT port, INT TxBitLen, INT len, CHAR *pSrc);
INT  spiEnable(INT32 spiPort);
INT  spiDisable(INT32 spiPort);


/* for SPI Flash */
INT  spiFlashInit(void);
INT  spiFlashEraseSector(UINT32 addr, UINT32 secCount);
INT  spiFlashEraseAll(void);
INT  spiFlashWrite(UINT32 addr, UINT32 len, UINT32 *buf);
INT  spiFlashRead(UINT32 addr, UINT32 len, UINT32 *buf);
INT  spiFlashEnter4ByteMode(void);
INT  spiFlashExit4ByteMode(void);


/* internal function */
int spiActive(int port);
int spiTxLen(int port, int count, int bitLen);

void spiSetClock(int port, int clock_by_MHz, int output_by_kHz);


#endif
