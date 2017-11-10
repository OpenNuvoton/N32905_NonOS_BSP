/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbio.h"
#include "wbtypes.h"

#include "w55fa93_reg.h"
#include "w55fa93_sic.h"
#include "w55fa93_GNAND.h"
#include "nvtfat.h"

__align (32) UINT8 g_ram0[512*16*16*16];
__align (32) UINT8 g_ram1[512*16*16*16];

UINT32 u32SecCnt;
UINT32 u32backup[10];

extern PDISK_T *pDisk_SD0;
extern PDISK_T *pDisk_SD1;
extern PDISK_T *pDisk_SD2;


int volatile gTotalSectors_SD = 0, gTotalSectors_SM = 0;


NDRV_T _nandDiskDriver0 =
{
    nandInit0,
    nandpread0,
    nandpwrite0,
    nand_is_page_dirty0,
    nand_is_valid_block0,
    nand_ioctl,
    nand_block_erase0,
    nand_chip_erase0,
    0
};
NDISK_T *ptMassNDisk;

void initPLL(void)
{
}


int STATUS;

int main(void)
{
    int ii, jj;

    initPLL();

    /* initialize FMI (Flash memory interface controller) */

#ifndef OPT_FPGA_DEBUG
    sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);      /* clock from PLL */
#else
    sicIoctl(SIC_SET_CLOCK, 27000, 0, 0);       /* clock from FPGA clock in */
#endif

    sicOpen();

    if (sicSdOpen0()<=0)
    {
        printf("Error in initializing SD card !! \n");
        sicSdClose0();

        //--- Example code to enable/disable SD0 Card Detect feature.
        printf("SD0 card detect feature is DISABLED by default.\n");
        printf("The card status will always be INSERTED if card detect feature disabled.\n");
        sicIoctl(SIC_GET_CARD_STATUS, (INT32)&ii, 0, 0);
        printf("Check SD0 card status ..... is %s\n", ii ? "INSERTED" : "REMOVED");

        printf("Now, try to enable SD0 card detect feature ... Done!\n");
        sicIoctl(SIC_SET_CARD_DETECT, TRUE, 0, 0);  // MUST call sicIoctl() BEFORE sicSdOpen0()
        sicSdOpen0();

        ii = FALSE;
        printf("Wait SD0 card insert ... \n");
        while (ii == FALSE)
        {
            sicIoctl(SIC_GET_CARD_STATUS, (INT32)&ii, 0, 0);
        }
        printf("Card inserted !!\n");
        sicSdClose0();

        printf("Now, Try to disable SD0 card detect feature ... Done!\n");
        sicIoctl(SIC_SET_CARD_DETECT, FALSE, 0, 0); // MUST call sicIoctl() BEFORE sicSdOpen0()
        sicSdOpen0();

        printf("=== THE END ===\n");
        while(1);
    }
    else
    {
        printf("SD total sector is %4x !! --\n", pDisk_SD0->uTotalSectorN);
    }

    for(ii=0; ii<512*16*16*16; ii++)
        g_ram0[ii] = rand();
//      g_ram0[ii] = ii*(ii/512 + ii);

    while(1)
    {
        u32backup[0] = g_ram0[0];
        for (jj = 1000; jj < pDisk_SD0->uTotalSectorN -0x100; jj+=u32SecCnt)
        {
            u32SecCnt = rand()&(0xFFF);
            if (u32SecCnt==0)
                u32SecCnt = 1;

            sicSdWrite0(jj, u32SecCnt, (UINT32)g_ram0);

            memset(g_ram1, 0x11, u32SecCnt);
            sicSdRead0(jj, u32SecCnt, (UINT32)g_ram1);
            if(memcmp(g_ram0, g_ram1, u32SecCnt*512) != 0)
            {
                printf("data compare ERROR at sector No. !! -- %4x \n", jj);
                while(1);
            }
            printf("data compare OK at sector No. !! -- %4x \n", jj);
        }
    }
}
