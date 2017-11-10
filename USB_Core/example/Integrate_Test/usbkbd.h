/*************************************************************************
 * Nuvoton Electronics Corporation confidential
 *
 * Copyright (c) 2008 by Nuvoton Electronics Corporation
 * All rights reserved
 *
 * FILENAME
 *     usbkbd.h
 *
 * VERSION
 *     1.0
 *
 * DESCRIPTION
 *     NUC930 USB Host keyboard driver header file
 *
 * HISTORY
 *     2008.06.24       Created
 *
 * REMARK
 *     None
 **************************************************************************/

#ifndef _USBKBD_H_
#define _USBKBD_H_

#include "usb.h"

typedef struct usb_kbd 
{
    //struct input_dev dev;
    struct usb_device  *usbdev;
#ifdef ETST_ALIGNMENT_INT
    UINT8           *newData;
#else    
    UINT8           newData[8];
#endif    
    UINT8           oldData[8];
    URB_T           urbIrq, urbLed;
    DEV_REQ_T     	dr;
    UINT8           leds, newleds;
    CHAR            name[128];
    INT             open;
} USB_KBD_T;

extern USB_KBD_T  *_my_usbkbd;

extern INT  USBKeyboardInit(VOID);
extern INT  USBKeyboardOpen(USB_KBD_T *);
extern VOID USBKeyboardClose(USB_KBD_T *);
extern VOID USBKeyboardLED(URB_T *);

#endif