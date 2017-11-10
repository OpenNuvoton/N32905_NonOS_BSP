#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "w55fa93_reg.h"
#include "wblib.h"
#include "w55fa93_sic.h"
#include "w55fa93_gnand.h"
#include "nvtfat.h"
#include "Font.h"
#include "writer.h"
//#include "w55fa93_vpost.h"

extern INT fmiMarkBadBlock(UINT32 block);
extern S_DEMO_FONT s_sDemo_Font;
extern INI_INFO_T Ini_Writer;

int font_x=0, font_y=16;
UINT32 u32SkipX;

#if 1
#define dbgprintf sysprintf
#else
#define dbgprintf(...)
#endif

//======================================================
// GNAND used
//======================================================
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

NDRV_T _nandDiskDriver1 =
{
	nandInit1,
	nandpread1,
	nandpwrite1,
	nand_is_page_dirty1,
	nand_is_valid_block1,
	nand_ioctl,
	nand_block_erase1,
	nand_chip_erase1,
	0
};

/**********************************/

__align(32) UINT8 infoBufArray[0x40000];
__align(32) UINT8 StorageBufferArray[0x50000];
__align(32) UINT8 CompareBufferArray[0x50000];
UINT32 infoBuf, StorageBuffer, CompareBuffer, BufferSize=0;
UINT32 NAND_BACKUP_BASE;
UINT8 *tmpBackPtr;

CHAR suNvtFullName[512], suNvtTargetFullName[512];
static INT hNvtFile = -1;
INT32 gCurBlock=0, gCurPage=0;
BOOL volatile bIsAbort = FALSE;
INT32 gNandLoaderSize=0;


/**********************************/
void nvtVerifyNand(UINT32 len)
{
	int volatile page_count, block_count=0, page;
	int volatile i, j;
	UINT8 *ptr = (UINT8 *)CompareBuffer;

	page_count = len / pNvtSM0->nPageSize;
	if ((len % pNvtSM0->nPageSize) != 0)
		page_count++;

	if (gCurPage > 0)
	{
		page = Minimum(pNvtSM0->uPagePerBlock - gCurPage, page_count);
		for (i=0; i<page; i++)
		{
			nvtSMpread(gCurBlock, i+gCurPage, (UINT8 *)ptr);
			ptr += pNvtSM0->nPageSize;
			page_count--;
		}
		if ((gCurPage+page) == pNvtSM0->uPagePerBlock)
		{
			gCurBlock++;
			gCurPage = 0;
		}
		else
			gCurPage += page;
	}

	block_count = page_count / pNvtSM0->uPagePerBlock;

	j=0;
	while(1)
	{
		if (j >= block_count)
			break;
		if (CheckBadBlockMark(gCurBlock) == Successful)
		{
			for (i=0; i<pNvtSM0->uPagePerBlock; i++)
			{
				nvtSMpread(gCurBlock, i, (UINT8 *)ptr);
				ptr += pNvtSM0->nPageSize;
			}
			j++;
		}
		gCurBlock++;
		gCurPage = 0;
	}

	if ((page_count % pNvtSM0->uPagePerBlock) != 0)
	{
		page_count = page_count - block_count * pNvtSM0->uPagePerBlock;
_read_:
		if (CheckBadBlockMark(gCurBlock) == Successful)
		{
			for (i=0; i<page_count; i++)
			{
				nvtSMpread(gCurBlock, i, (UINT8 *)ptr);
				ptr += pNvtSM0->nPageSize;
			}
			gCurPage = i;
		}
		else
		{
			gCurBlock++;
			goto _read_;
		}
	}
}


void nvtBackUpNand(UINT32 pageCount, UINT32 oldBlock)
{
	int volatile i, status=0;

	pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

	// block erase
_erase:
	status = nand_block_erase0(gCurBlock);
	if (status < 0)
	{
		fmiMarkBadBlock(gCurBlock);
		gCurBlock++;
		goto _erase;
	}

	for (i=0; i<pageCount; i++)
	{
		nvtSMpread(oldBlock, i, pInfo);
		status = nvtSMpwrite(gCurBlock, i, pInfo);
		if (status < 0)
		{
			fmiMarkBadBlock(gCurBlock);
			gCurBlock++;
			goto _erase;
		}
	}
}


void nvtWriteNand(UINT32 len)
{
	int volatile status = 0;
	int volatile page_count, block_count=0, page;
	int volatile i, j;
	UINT8 *ptr = (UINT8 *)StorageBuffer;

	page_count = len / pNvtSM0->nPageSize;
	if ((len % pNvtSM0->nPageSize) != 0)
		page_count++;

	if (gCurPage > 0)
	{
		page = Minimum(pNvtSM0->uPagePerBlock - gCurPage, page_count);
		for (i=0; i<page; i++)
		{
			status = nvtSMpwrite(gCurBlock, i+gCurPage, (UINT8 *)ptr);
			if (status < 0)
			{
				fmiMarkBadBlock(gCurBlock);
				gCurBlock++;
				nvtBackUpNand(i, gCurBlock-1);
			}
			ptr += pNvtSM0->nPageSize;
			page_count--;
		}
		if ((gCurPage+page) == pNvtSM0->uPagePerBlock)
		{
			gCurPage = 0;
			gCurBlock++;
		}
		else
			gCurPage += page;
	}

	// erase needed blocks
	block_count = page_count / pNvtSM0->uPagePerBlock;

	for (j=0; j<block_count; j++)
	{
		// block erase
_retry_erase:
		status = nand_block_erase0(gCurBlock);
		if (status < 0)
		{
			fmiMarkBadBlock(gCurBlock);
			gCurBlock++;
			goto _retry_erase;
		}

		// write block
		for (i=0; i<pNvtSM0->uPagePerBlock; i++)
		{
			status = nvtSMpwrite(gCurBlock, i, (UINT8 *)ptr);
			if (status < 0)
			{
				fmiMarkBadBlock(gCurBlock);
				gCurBlock++;
				nvtBackUpNand(i, gCurBlock-1);
			}
			ptr += pNvtSM0->nPageSize;
		}
		gCurBlock++;
		gCurPage = 0;
	}

	if ((page_count % pNvtSM0->uPagePerBlock) != 0)
	{
		page_count = page_count - block_count * pNvtSM0->uPagePerBlock;
		// erase block
_retry_remain:
		status = nand_block_erase0(gCurBlock);
		if (status < 0)
		{
			fmiMarkBadBlock(gCurBlock);
			gCurBlock++;
			goto _retry_remain;
		}

		// write block
		for (i=0; i<page_count; i++)
		{
			status = nvtSMpwrite(gCurBlock, i, (UINT8 *)ptr);
			if (status < 0)
			{
				fmiMarkBadBlock(gCurBlock);
				gCurBlock++;
				nvtBackUpNand(i, gCurBlock-1);
			}
			ptr += pNvtSM0->nPageSize;
		}
		gCurPage = i;
	}
}

/**********************************/
int File_Copy(CHAR *suSrcName, CHAR *suDstName)
{
	INT		hFileSrc, hFileDst, nByteCnt, nStatus, nStatusW, nByteCntW;

	hFileSrc = fsOpenFile(suSrcName, NULL, O_RDONLY);
	if (hFileSrc < 0)
		return hFileSrc;

	hFileDst = fsOpenFile(suDstName, NULL, O_CREATE);
	if (hFileDst < 0)
	{
		fsCloseFile(hFileSrc);
		return hFileDst;
	}

   	while (1)
   	{
        Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);

   		nStatus = fsReadFile(hFileSrc, (UINT8 *)StorageBuffer, BufferSize, &nByteCnt);
   		if (nStatus < 0)
        {
            if (nStatus != ERR_FILE_EOF)
            {
        	    dbgprintf("ERROR in File_Copy(): Read file %s error ! Reture = 0x%x !!\n", suSrcName, nStatus);
       		}
       		break;
   		}

		nStatusW = fsWriteFile(hFileDst, (UINT8 *)StorageBuffer, nByteCnt, &nByteCntW);
   		if (nStatusW < 0)
   			break;

    	if ((nByteCnt < BufferSize) || (nByteCnt != nByteCntW))
    		break;
	}
	fsCloseFile(hFileSrc);
	fsCloseFile(hFileDst);

	if (nStatus == ERR_FILE_EOF)
		nStatus = 0;
	return nStatus;
}


int File_Compare(CHAR *suFileNameS, CHAR *suFileNameD)
{
	INT		hFileS, hFileD;
	INT		nLenS, nLenD, nStatusS, nStatusD;

	hFileS = fsOpenFile(suFileNameS, NULL, O_RDONLY);
	if (hFileS < 0)
		return hFileS;

	hFileD = fsOpenFile(suFileNameD, NULL, O_RDONLY);
	if (hFileD < 0)
		return hFileD;

	while (1)
	{
        Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
		nStatusS = fsReadFile(hFileS, (UINT8 *)StorageBuffer, BufferSize, &nLenS);
		nStatusD = fsReadFile(hFileD, (UINT8 *)CompareBuffer, BufferSize, &nLenD);

		if ((nStatusS == ERR_FILE_EOF) && (nStatusD == ERR_FILE_EOF))
		{
			fsCloseFile(hFileS);
			fsCloseFile(hFileD);
			return 0;
		}

		if (nLenS != nLenD)
			break;

		if (memcmp((UINT8 *)StorageBuffer, (UINT8 *)CompareBuffer, nLenS))
			break;
		}

	fsCloseFile(hFileS);
	fsCloseFile(hFileD);

	return -1;
}

int nandCopyContent(CHAR *suDirName, CHAR *suTargetName)
{
	INT				nLenS, nLenD, nStatus;
	CHAR			suSrcLongName[MAX_FILE_NAME_LEN];
	CHAR			suDstLongName[MAX_FILE_NAME_LEN];
	CHAR			suSlash[6] = { '\\', 0, 0, 0 };
	FILE_FIND_T  	tFileInfo;
	CHAR 			Array1[64], FolderName[64];

	fsUnicodeStrCat(suDirName, suSlash);	/* append '\' */
	fsUnicodeStrCat(suTargetName, suSlash);	/* append '\' */

	memset((UINT8 *)&tFileInfo, 0, sizeof(tFileInfo));

	nStatus = fsFindFirst(suDirName, NULL, &tFileInfo);
	if (nStatus < 0)
	{
		fsUnicodeToAscii(suDirName,FolderName, TRUE);
		sprintf(Array1, "No %s Folder", FolderName);
		Draw_CurrentOperation(Array1,nStatus);
		return nStatus;
	}

	do
	{
		if (tFileInfo.ucAttrib & A_DIR)
		{
			if ((strcmp(tFileInfo.szShortName, "..") == 0) ||
				(strcmp(tFileInfo.szShortName, ".") == 0))
				continue;

			fsUnicodeCopyStr(suSrcLongName, suDirName);
			fsUnicodeCopyStr(suDstLongName, suTargetName);
			nLenS = fsUnicodeStrLen(suDirName);
			nLenD = fsUnicodeStrLen(suTargetName);
			if ( !((suDirName[nLenS-2] == '\\') && (suDirName[nLenS-1] == 0)) )
				fsUnicodeStrCat(suSrcLongName, suSlash);	/* append '\' */
			if ( !((suTargetName[nLenD-2] == '\\') && (suTargetName[nLenD-1] == 0)) )
				fsUnicodeStrCat(suDstLongName, suSlash);	/* append '\' */
			fsUnicodeStrCat(suSrcLongName, tFileInfo.suLongName);
			fsUnicodeStrCat(suDstLongName, tFileInfo.suLongName);

			nStatus = fsMakeDirectory(suDstLongName, NULL);
			if (nStatus < 0)
			{
				Draw_CurrentOperation("Unable to Create Directory",nStatus);
				return nStatus;
			}

			nStatus = nandCopyContent(suSrcLongName, suDstLongName);
			if (nStatus < 0)
			{
				sysprintf("===> 22 (Copy file %s fail)\n", suSrcLongName);
				bIsAbort = TRUE;
				return nStatus;
			}
		}
		else
		{
			fsUnicodeCopyStr(suSrcLongName, suDirName);
			fsUnicodeCopyStr(suDstLongName, suTargetName);
			fsUnicodeStrCat(suSrcLongName, tFileInfo.suLongName);
			fsUnicodeStrCat(suDstLongName, tFileInfo.suLongName);

			dbgprintf("Copying   file %s\n", tFileInfo.szShortName);
			sprintf(Array1, "Copying %s", tFileInfo.szShortName);
			nStatus = File_Copy(suSrcLongName, suDstLongName);
			if (nStatus < 0)
			{
				Draw_CurrentOperation(Array1,nStatus);
				return nStatus;
			}
			else
				Draw_CurrentOperation(Array1,nStatus);

			dbgprintf("Comparing file %s\n", tFileInfo.szShortName);
			sprintf(Array1, "Comparing %s", tFileInfo.szShortName);
			nStatus = File_Compare(suSrcLongName, suDstLongName);
			if (nStatus < 0)
			{
				Draw_CurrentOperation(Array1,nStatus);
				return nStatus;
			}
			Draw_CurrentOperation(Array1,nStatus);
		}

	} while (!fsFindNext(&tFileInfo));

	fsFindClose(&tFileInfo);
	return 0;
}


/**********************************/
int main()
{
	DateTime_T ltime;

	NDISK_T *ptNDisk;
	PDISK_T *pDisk_nand = NULL;
	int status=0, nReadLen;
	CHAR szNvtFullName[64];

	LDISK_T *ptLDisk;
	PDISK_T *ptPDisk;
	PARTITION_T *ptPart;
	int LogicSectorD=-1;

	char Array1[64];
	UINT  Next_Font_Height;

	/* enable cache */
	sysDisableCache();
	sysInvalidCache();
	sysEnableCache(CACHE_WRITE_BACK);

	/* check SDRAM size and buffer address */

	infoBuf = (UINT32) &infoBufArray[0] | 0x80000000;
	StorageBuffer = (UINT32)&StorageBufferArray[0] | 0x80000000;
	CompareBuffer = (UINT32)&CompareBufferArray[0] | 0x80000000;

	pInfo = (UINT8 *)((UINT32)infoBuf | 0x80000000);

    /* configure Timer0 for FMI library */
	sysSetTimerReferenceClock(TIMER0, 12000000);
	sysStartTimer(TIMER0, 100, PERIODIC_MODE);

	sysprintf("=====> W55FA93 ExNandWriter (v%d.%d) Begin [%d] <=====\n", MAJOR_VERSION_NUM, MINOR_VERSION_NUM, sysGetTicks(0));

	ltime.year = 2011;
	ltime.mon  = 10;
	ltime.day  = 31;
	ltime.hour = 8;
	ltime.min  = 40;
	ltime.sec  = 0;
	sysSetLocalTime(ltime);

	fsInitFileSystem();
	fsAssignDriveNumber('X', DISK_TYPE_SD_MMC, 0, 1);     		// SD0, single partition
#if (TARGET_Chip_Select == 0)
	fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 0, 1);     	// NAND1-1, 2 partitions
	//fsAssignDriveNumber('E', DISK_TYPE_SMART_MEDIA, 0, 2);    // NAND1-2, 2 partitions
#else
	fsAssignDriveNumber('D', DISK_TYPE_SMART_MEDIA, 1, 1);     	// NAND1-1, 1 partitions
#endif

	Draw_Init();
	font_y = g_Font_Height;
	Next_Font_Height = g_Font_Height-6;

	Draw_Font(COLOR_RGB16_WHITE,  &s_sDemo_Font, font_x, font_y, "Mount SD Card:");
	u32SkipX = 14;

	sicIoctl(SIC_SET_CLOCK, 192000, 0, 0);
	sicOpen();
	status = sicSdOpen0();
	if (status < 0)
	{
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
		sysprintf("===> 1 (No SD Card)\n");
		bIsAbort = TRUE;
		goto _end_;
	}

	// Get the NandWriter setting from INI file (NandWriter.ini)
	status = ProcessINI("X:\\ExNandWriter.ini");
	if (status < 0)
	{
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
		sysprintf("===> 1.1 (Wrong INI file)\n");
		bIsAbort = TRUE;
		goto _end_;
	}

	Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
	font_y += Next_Font_Height;

	nvtSMInit();

	sprintf(Array1, "Nand:%d(Blk)*%d(Pg)*%d(Size)",
            pNvtSM0->uBlockPerFlash, pNvtSM0->uPagePerBlock, pNvtSM0->nPageSize);
	Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, 0, 	_LCM_HEIGHT_ -1-g_Font_Height,Array1);

	BufferSize = pNvtSM0->uPagePerBlock * pNvtSM0->nPageSize;

	tmpBackPtr = malloc(BufferSize+32);
	NAND_BACKUP_BASE = (UINT32) (tmpBackPtr+31) & ~0x1F;
	if (NAND_BACKUP_BASE ==0)
	{
		sysprintf("Malloc fail for size=0x%x\n",BufferSize);
		goto _end_;
	}

	if (BufferSize >= 0x50000)
		BufferSize = 0x40000;


	Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Mount GNAND:");
	u32SkipX = 12;

	ptNDisk = (NDISK_T *)malloc(sizeof(NDISK_T));
#if (TARGET_Chip_Select == 0)
	if (GNAND_InitNAND(&_nandDiskDriver0, ptNDisk, TRUE))
#else
	if (GNAND_InitNAND(&_nandDiskDriver1, ptNDisk, TRUE))
#endif
    {
    	sysprintf("ERROR: GNAND initial fail !!\n");
    	return -1;
    }

	status = GNAND_MountNandDisk(ptNDisk);
	if (status)
	{
		Draw_CurrentOperation("Mount GNAND",status);
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
	}
	else
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
	font_y += Next_Font_Height;

	// 16 MB for 1st user partition, the others for 2nd user partitions.
	pDisk_nand = (PDISK_T *)ptNDisk->pDisk;

	/* partition and format Nand1-1 and Nand1-2 */
	sysprintf("=====> partition and format [%d] <=====\n", sysGetTicks(0));
	Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Format Nand:");
    status = fsFormatFlashMemoryCard((PDISK_T *)pDisk_nand);    // only one partition
	if (status < 0)
	{
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
		sysprintf("===> 5 (Format NAND fail)\n");
		bIsAbort = TRUE;
		goto _end_;
	}
	else
		Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
	font_y += Next_Font_Height;


	{
		UINT32 uBlockSize, uFreeSize, uDiskSize;
		/* Get disk information */
		uBlockSize=0, uFreeSize=0, uDiskSize=0;
		fsDiskFreeSpace('X', &uBlockSize, &uFreeSize, &uDiskSize);
		sysprintf("\nDisk X Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);

		uBlockSize=0, uFreeSize=0, uDiskSize=0;
		fsDiskFreeSpace('D', &uBlockSize, &uFreeSize, &uDiskSize);
		sysprintf("Disk D Size: %d Kbytes, Free Space: %d KBytes\n", (INT)uDiskSize, (INT)uFreeSize);
	}


	// Get the Start sector for D & E partition

	ptLDisk = (LDISK_T *)malloc(sizeof(LDISK_T));

	if (get_vdisk('D', &ptLDisk) <0)
	{
	  	sysprintf(" ===> 6 (vdisk fail)\n");
		bIsAbort = TRUE;
		goto _end_;
	}

	ptPDisk = ptLDisk->ptPDisk;	// get the physical disk structure pointer of NAND disk

	ptPart = ptPDisk->ptPartList;	// Get the partition of NAND disk

	while (ptPart != NULL)
	{
#if 1
		sysprintf("Driver %c -- Start sector : %d, Total sector : %d\n",
				ptPart->ptLDisk->nDriveNo, ptPart->uStartSecN, ptPart->uTotalSecN);
#endif
		if 	(ptPart->ptLDisk->nDriveNo == 'D')
		{
			LogicSectorD = ptPart->uStartSecN;
		}

		ptPart = ptPart->ptNextPart;
	}

	/*********************************/
	/* copy first partition content  */
	/*********************************/
	if (Ini_Writer.NAND1_1_FAT >= 0)
	{
		sysprintf("\n=====> copy First Partition Content [%d] <=====\n", sysGetTicks(0));

		if (Ini_Writer.NAND1_1_FAT == 0)
		{
			// Copy File Through FAT like Binary ISO
			Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying NAND1-1:");
			u32SkipX = 16;

			if (LogicSectorD == -1)
			{
				Draw_CurrentOperation("LogicSectorD :",hNvtFile);
				sysprintf("===> 7.1 (Wrong start sector for NAND1-1)\n");
				bIsAbort = TRUE;
				goto _end_;
			}

			strcpy(szNvtFullName, "X:\\NAND1-1\\content.bin");
			fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);
			hNvtFile = fsOpenFile(suNvtFullName, NULL, O_RDONLY);

			dbgprintf("Copying file %s\n", szNvtFullName);
			sprintf(Array1, "Open %s", szNvtFullName);
			if (hNvtFile < 0)
			{
				Draw_CurrentOperation("Open X:\\NAND1-1\\content.bin",hNvtFile);
				Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);

				sysprintf("===> 7.2 (Open content.bin fail)\n");
				bIsAbort = TRUE;
				goto _end_;
			}
			else
				Draw_CurrentOperation(Array1,hNvtFile);

			while(1)
			{
			    Draw_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
				status = fsReadFile(hNvtFile, (UINT8 *)StorageBuffer, BufferSize, &nReadLen);
				GNAND_write(ptNDisk, LogicSectorD,nReadLen/512 ,(UINT8 *)StorageBuffer);

				LogicSectorD += nReadLen/512;

				if (status == ERR_FILE_EOF)
					break;
				else if (status < 0)
				{
				    Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
					Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);
					sysprintf("===> 7.3 (Read content.bin fail) [0x%x]\n", status);
					bIsAbort = TRUE;
					goto _end_;
				}
			}
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
			Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
			font_y += Next_Font_Height;
		}
		else
		{
			// Copy File through FAT
			Draw_Font(COLOR_RGB16_WHITE, &s_sDemo_Font, font_x, font_y, "Copying NAND1-1:");
			u32SkipX = 16;

			strcpy(szNvtFullName, "D:");
			fsAsciiToUnicode(szNvtFullName, suNvtTargetFullName, TRUE);

			strcpy(szNvtFullName, "X:\\NAND1-1");
			fsAsciiToUnicode(szNvtFullName, suNvtFullName, TRUE);

			status = nandCopyContent(suNvtFullName, suNvtTargetFullName);
            Draw_Clear_Wait_Status(font_x+ u32SkipX*g_Font_Step, font_y);
			if (status < 0)
			{
				Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Fail);

				sysprintf("===> 7.4 (Copy files in NAND1-1 fail) [0x%x]\n", status);
				bIsAbort = TRUE;
				goto _end_;
			}
			Draw_Status(font_x+ u32SkipX*g_Font_Step, font_y, Successful);
			font_y += Next_Font_Height;
		}
	}

_end_:

	sysprintf("=====> Finish [%d] <=====\n", sysGetTicks(0));
	Draw_FinalStatus(bIsAbort);

	if (tmpBackPtr != NULL)
		free(tmpBackPtr);

	while(1);
}


