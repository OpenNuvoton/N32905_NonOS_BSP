/**************************************************************************//**
 * @file     W55FA93_KPI.h
 * @version  V3.00
 * @brief    N3290x series keypad driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wblib.h"
#include "W55FA93_reg.h"

#define KPI_NONBLOCK	0
#define KPI_BLOCK		1

extern void kpi_init(void);
extern int kpi_open(unsigned int src);
extern void kpi_close(void);
extern int kpi_read(unsigned char mode);
