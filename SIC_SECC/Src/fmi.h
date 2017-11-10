#include <stdio.h>

#include "wbio.h"

//#define _SIC_USE_INT_
//#define DEBUG
#define TIMER0  0

//#define printf	sysprintf

//#define _USE_DAT3_DETECT_

//-- function return value
#define	   Successful  0
#define	   Fail        1

#ifdef ECOS
#define sysGetTicks(TIMER0)  cyg_current_time()
//#define printf	diag_printf
#endif

// extern global variables
extern UINT32 _fmi_uFMIReferenceClock;
extern BOOL volatile _fmi_bIsSDDataReady;

#define STOR_STRING_LEN	32

/* we allocate one of these for every device that we remember */
typedef struct disk_data_t
{
    struct disk_data_t  *next;           /* next device */

    /* information about the device -- always good */
	unsigned int  totalSectorN;
	unsigned int  diskSize;			/* disk size in Kbytes */
	int           sectorSize;
    char          vendor[STOR_STRING_LEN];
    char          product[STOR_STRING_LEN];
    char          serial[STOR_STRING_LEN];
} DISK_DATA_T;


// function declaration

// SD functions
INT  fmiSDCommand(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg);
INT  fmiSDCmdAndRsp(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg, INT nCount);
INT  fmiSDCmdAndRsp2(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg, UINT *puR2ptr);
INT  fmiSDCmdAndRspDataIn(FMI_SD_INFO_T *pSD, UINT8 ucCmd, UINT32 uArg);
INT  fmiSD_Init(FMI_SD_INFO_T *pSD);
INT  fmiSelectCard(FMI_SD_INFO_T *pSD);
VOID fmiGet_SD_info(FMI_SD_INFO_T *pSD, DISK_DATA_T *_info);
INT  fmiSD_Read_in(FMI_SD_INFO_T *pSD, UINT32 uSector, UINT32 uBufcnt, UINT32 uDAddr);
INT  fmiSD_Write_in(FMI_SD_INFO_T *pSD, UINT32 uSector, UINT32 uBufcnt, UINT32 uSAddr);

