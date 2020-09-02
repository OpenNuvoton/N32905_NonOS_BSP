/**************************************************************************//**
 * @file     alone.h
 * @version  V3.00
 * @brief    Data type definition header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#define FALSE				0
#define TRUE				1

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

#define	_DWORD_(a)	*((volatile DWORD *)(a))
#define	_WORD_(a)	*((volatile WORD *)(a))
#define	_BYTE_(a)	*((volatile BYTE *)(a))
