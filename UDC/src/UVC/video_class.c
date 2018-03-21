/****************************************************************************
 *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wblib.h"
#include "w55fa93_reg.h"
#include "usbd.h"
#include "videoclass.h"

#define HSHB_MODE
#define DATA_CODE	"20180316"

#define USB_VID		0x0416		/* Vendor ID */ 
#define USB_PID		0x9393		/* Product ID */

#ifdef HSHB_MODE
	#define	MAX_PACKET_SIZE_HS     	0x384
	#define	HSHB					0x800
#else
	#define	MAX_PACKET_SIZE_HS     	0x3E8
	#define	HSHB					0x0
#endif	

#define     MAX_PACKET_SIZE_FS     0x3E8

volatile int max_packet_size = MAX_PACKET_SIZE_HS;
    
/* length of descriptors */
#define UVC_DEVICE_DSCPT_LEN		0x12
#define UVC_CONFIG_DSCPT_LEN		0x20C
#define UVC_STR0_DSCPT_LEN			0x04
#define UVC_QUALIFIER_DSCPT_LEN		0x0a
#define UVC_HOSCONFIG_DSCPT_LEN		0x49

__align(4) volatile UVC_INFO_T 		uvcInfo;
__align(4) volatile UVC_STATUS_T 	uvcStatus;
__align(4) volatile UVC_PU_INFO_T	uvcPuInfo = {0,	70, 7, -10,10, 1, 0, 20, 2, -30, 30, 0, 0, 40, 4, 0, 50, 5, 1, 5, 3, 0, 2, 2};

/* for USB */
extern USB_CMD_T	_usb_cmd_pkt;
extern volatile USBD_INFO_T usbdInfo;

volatile UINT8 g_u8UVC_FID = 0;
volatile UINT8 g_u8UVC_PD = 0;


/* Processing Unit Control Selectors */


__align(4) static DEVICEDESCRIPTOR UVC_DeviceDescriptor = {
 0x12,      //bLength
 0x01,      //bDescriptorType
 0x0200,    //bcdUSB 
 0xEF,      //bDeviceClass 
 0x02,      //bDeviceSubClass
 0x01,      //bDeviceProtocol
 0x40,      //bMaxPacketSize0
 USB_VID,    //idVendor
 USB_PID,    //idProduct
 0x0100,     //bcdDevice
 0x01,      //iManufacturer
 0x02,      //iProduct
 0x03,      //iSerialNumber
 0x01       //bNumConfigurations
};
#ifdef UVC_FORMAT_BOTH
__align(4) static VIDEOCLASS UVC_ConfigurationBlock = {    
#elif defined UVC_FORMAT_YUV
__align(4) static VIDEOCLASS_YUV UVC_ConfigurationBlock = {    
#elif defined UVC_FORMAT_MJPEG
__align(4) static VIDEOCLASS_MJPEG UVC_ConfigurationBlock = {    
#endif
{	0x09,  	//bLength
    0x02,   //bDescriptorType
#ifdef UVC_FORMAT_BOTH
    0x020C, //wTotalLength
#elif defined UVC_FORMAT_YUV
    0x0158, //wTotalLength
#elif defined UVC_FORMAT_MJPEG
    0x0148, //wTotalLength
#endif    
    0x02,   //bNumInterfaces
    0x01,   //bConfigurationValue
    0x00,   //iConfiguration
  	0xC0,   //bmAttributes
  	0x00    //bMaxPower
},
// Standard Video Interface Collection IAD(interface Association Descriptor)//
{
    0x08,   //bLength
    0x0B,   //bDescriptorType
    0x00,   //bFirstInterface
    0x02,   //bInterfaceCount
    0x0E,   //bFunctionClass
    0x03,   //bFunctionSubClass
    0x00,   //bFunctionProtocol
    0x02   //iFunction
},    
// Standard VideoControl Interface Descriptor
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x00,   //bInterfceNumber
    0x00,   //bAlternateSetting
    0x01,   //bNumEndpoints		
    0x0E,   //bInterfaceClass
    0x01,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x02    //iInterface
},   
// Class-specific VideoControl Interface Descriptor
{
    0x0D,       //bLength
    0x24,       //bDescriptorType
    0x01,       //bDescriptorSubType
    0x0100,     //bcdUVC
    0x0032,     //wTotalLength
    0x005B8D80, //dwClockFrequency
    0x01,       //bInCollection
    0x01        //baInterfaceNr
},  
// Output Terminal Descriptor 
{
    0x09,   //bLength
    0x24,   //bDescriptorType
    0x03,   //bDescriptorSubType
    0x03,   //bTerminalID
    0x0101, //wTerminalType
    0x00,   //bAssocTerminal
    0x05,   //bSourceID
    0x00    //iTerminal
},

// Input Terminal Descriptor (Camera)
{
    0x11,   //bLength
    0x24,   //bDescriptorType
    0x02,   //bDescriptorSubType
    0x01,   //bTerminalID
    0x0201, //wTerminalType
    0x00,   //bAssocTerminal
    0x00,   //iTerminal
    0x0000, //wObjectiveFocalLengthMin
    0x0000, //wObjectiveFocalLengthMax
    0x0000, //wOcularFocalLength
    0x02,   //bControlSize
    0x0000  //bmControls
},

// Processing Uint Descriptor 
{
    0x0B,   //bLength
    0x24,   //bDescriptorType
    0x05,   //bDescriptorSubType
    0x05,   //bUnitID
    0x01,   //bSourceID
    0x0000, //wMaxMultiplier
    0x02,   //bControlSize
    0x053F, //bmControls
    0x00    //iProcessing    
},

// Standard Interrupt Endpoint Descriptor
{
    0x07,   //bLength
    0x05,   //bDescriptorType
    0x83,   //bEndpointAddress
    0x03,   //bmAttributes
    0x0010, //wMaxPacketSize
    0x06    //bInterval
},

// Class-specific Interrupt Endpoint Descriptor
{
    0x05,   //bLength
    0x25,   //bDescriptorType
    0x03,   //bDescriptorSubType
    0x0010  //wMaxPacketSize
},

// Standard VideoStreaming Interface Descriptor
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x01,   //bInterfceNumber
    0x00,   //bAlternateSetting
    0x00,   //bNumEndpoints
    0x0E,   //bInterfaceClass
    0x02,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x00    //iInterface
}, 

// Class-specific VideoStreaming Header Descriptor
{   
#ifdef UVC_FORMAT_BOTH
    0x0F,   //bLength
#else   
    0x0E,   //bLength
#endif
    0x24,   //bDescriptorType
    0x01,   //bDescriptorSubType
#ifdef UVC_FORMAT_BOTH
    0x02,   //bNumFormats           // Modified Here   
    0x018B, //wTotalLength
#elif defined UVC_FORMAT_YUV
    0x01,   //bNumFormats           // Modified Here   
    0x00D7, //wTotalLength
#elif defined UVC_FORMAT_MJPEG
    0x01,   //bNumFormats           // Modified Here   
    0x00C7, //wTotalLength
#endif       
    0x81,   //bEndpointAddress
    0x00,   //bmInfo
    0x03,   //bTerminalLink
    0x02,   //bStillCaptureMethod    
    0x01,   //bTriggerSupport
    0x00,   //bTriggerUsage
    0x01,   //bControlSize
#ifdef UVC_FORMAT_BOTH    
    0x00,   //bmaControls
#endif    
    0x00    //bmaControls   
},
#ifndef UVC_FORMAT_MJPEG
// Uncompressed Video YUV422
// Class-specific VideoStreaming Format Descriptor
{
    0x1B,   //bLength
    0x24,   //bDescriptorType
    0x04,   //bDescriptorSubType
    0x01,   //bFormatIndex
    0x03,   //bNumFrameDescriptors      // Modified Here
	0x59,0x55,0x59,0x32,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71,
// 	0x7b,0xeb,0x36,0xe4,0x52,0x4f,0x11,0xce,0x95,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70,
    0x10,
    0x01,   //bDefaultFrameIndex
    0x00,   //bAspectRatioX
    0x00,   //bAspectRatioY
    0x00,   //bmInterlaceFlags    
    0x00    //bCopyProtect
},

// Class-specific VideoStreaming Frame 1 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x05,           //bDescriptorSubType
    0x01,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0280,         //wWidth->640
    0x01E0,         //wHeight->480
    0x00096000,     //dwMinBitRate
    0x01194000,     //dwMaxBitRate
    0x00096000,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},


// Class-specific VideoStreaming Frame 3 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x05,           //bDescriptorSubType
    0x02,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0140,         //wWidth->320
    0x00F0,         //wHeight->240
    0x00025800,     //dwMinBitRate
    0x00465000,     //dwMaxBitRate
    0x00025800,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific VideoStreaming Frame 5 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x05,           //bDescriptorSubType
    0x03,           //bFrameIndex           // Modified Here   
    0x00,           //bmCapabilities    
    0x0A0,          //wWidth->160
    0x078,          //wHeight->120
    0x00009600,     //dwMinBitRate
    0x00119400,     //dwMaxBitRate
    0x00009600,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific Still Image Frame Descriptor
{
    0x12,           //bLength
    0x24,           //bDescriptorType
    0x03,           //bDescriptorSubType
    0x00,  			//bEndpointAddress
    0x03,           //bNumImageSizePatterns      
    0x0280,         //wWidth    
    0x01E0,         //wHeight            
    0x0140,         //wWidth        
    0x00F0,         //wHeight                    
    0x00A0,         //wWidth    
    0x0078,         //wHeight 
    0x00            //bNumCompressionPtn           
},
#endif
#if defined UVC_FORMAT_BOTH || defined UVC_FORMAT_MJPEG
/* MJPEG */
// Class-specific VideoStreaming Format Descriptor
{
    0x0B,   //bLength
    0x24,   //bDescriptorType
    0x06,   //bDescriptorSubType
    0x02,   //bFormatIndex
    0x03,   //bNumFrameDescriptors      // Modified Here
    0x01,   //bmFlags
    0x01,   //bDefaultFrameIndex
    0x00,   //bAspectRatioX
    0x00,   //bAspectRatioY
    0x00,   //bmInterlaceFlags
    0x00,   //bCopyProtect
},

// Class-specific VideoStreaming Frame 1 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x07,           //bDescriptorSubType
    0x01,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0280,         //wWidth->640
    0x01E0,         //wHeight->480
    0x00096000,     //dwMinBitRate
    0x01194000,     //dwMaxBitRate
    0x00096000,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific VideoStreaming Frame 5 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x07,           //bDescriptorSubType
    0x02,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0140,         //wWidth->320
    0x00F0,         //wHeight->240
    0x00025800,     //dwMinBitRate
    0x00465000,     //dwMaxBitRate
    0x00025800,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific VideoStreaming Frame 7 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x07,           //bDescriptorSubType
    0x03,           //bFrameIndex           // Modified Here   
    0x00,           //bmCapabilities    
    0x0A0,          //wWidth->160
    0x078,          //wHeight->120
    0x00009600,     //dwMinBitRate
    0x00119400,     //dwMaxBitRate
    0x00009600,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific Still Image Frame Descriptor
{
    0x12,           //bLength
    0x24,           //bDescriptorType
    0x03,           //bDescriptorSubType
    0x00,  			//bEndpointAddress
    0x03,           //bNumImageSizePatterns          
    0x0280,         //wWidth    
    0x01E0,         //wHeight     
    0x0120,			//wWidth 	    
    0x00F0,         //wHeight                    
    0x00A0,         //wWidth    
    0x0078,         //wHeight                       
    0x00            //bNumCompressionPtn           
},
#endif
/* Color Matching Descriptor */
{
    0x06,           //bLength
    0x24,           //bDescriptorType
    0x0D,           //bDescriptorSubType
    0x01,           //bColorPrimaries
    0x01,           //bTransferCharacteristics
    0x04            //bMatrixCoefficients
},
// Standard VideoStreaming Interface Descriptor  (Num 1, Alt 3)
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x01,   //bInterfceNumber
    0x01,   //bAlternateSetting
    0x01,   //bNumEndpoints    
    0x0E,   //bInterfaceClass
    0x02,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x00    //iInterface
}, 

// Standard VideoStreaming Iso Video Data Endpoint Descriptor
{
    0x07,   //bLength
    0x05,   //bDescriptorType
    0x81,   //bEndpointAddress
    0x05,   //bmAttributes
    MAX_PACKET_SIZE_HS | HSHB, //wMaxPacketSize 
    0x01    //bInterval
},
// Standard VideoStreaming Interface Descriptor  (Num 1, Alt 4)
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x01,   //bInterfceNumber
    0x02,   //bAlternateSetting
    0x01,   //bNumEndpoints    
    0x0E,   //bInterfaceClass
    0x02,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x00    //iInterface
}, 

// Standard VideoStreaming Iso Video Data Endpoint Descriptor
{
    0x07,   //bLength
    0x05,   //bDescriptorType
    0x81,   //bEndpointAddress
    0x05,   //bmAttributes
    MAX_PACKET_SIZE_HS | HSHB, //wMaxPacketSize   
    0x01    //bInterval
}
};


#ifdef UVC_FORMAT_BOTH
__align(4) static VIDEOCLASS UVC_ConfigurationBlock_FS = {    
#elif defined UVC_FORMAT_YUV
__align(4) static VIDEOCLASS_YUV UVC_ConfigurationBlock_FS = {    
#elif defined UVC_FORMAT_MJPEG
__align(4) static VIDEOCLASS_MJPEG UVC_ConfigurationBlock_FS = {    
#endif
{	0x09,  	//bLength
    0x02,   //bDescriptorType
#ifdef UVC_FORMAT_BOTH
    0x020C, //wTotalLength
#elif defined UVC_FORMAT_YUV
    0x0158, //wTotalLength
#elif defined UVC_FORMAT_MJPEG
    0x0148, //wTotalLength
#endif    
    0x02,   //bNumInterfaces
    0x01,   //bConfigurationValue
    0x00,   //iConfiguration
  	0xC0,   //bmAttributes
  	0x00    //bMaxPower
},
// Standard Video Interface Collection IAD(interface Association Descriptor)//
{
    0x08,   //bLength
    0x0B,   //bDescriptorType
    0x00,   //bFirstInterface
    0x02,   //bInterfaceCount
    0x0E,   //bFunctionClass
    0x03,   //bFunctionSubClass
    0x00,   //bFunctionProtocol
    0x02   //iFunction
},    
// Standard VideoControl Interface Descriptor
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x00,   //bInterfceNumber
    0x00,   //bAlternateSetting
    0x01,   //bNumEndpoints		
    0x0E,   //bInterfaceClass
    0x01,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x02    //iInterface
},   
// Class-specific VideoControl Interface Descriptor
{
    0x0D,       //bLength
    0x24,       //bDescriptorType
    0x01,       //bDescriptorSubType
    0x0100,     //bcdUVC
    0x0032,     //wTotalLength
    0x005B8D80, //dwClockFrequency
    0x01,       //bInCollection
    0x01        //baInterfaceNr
},  
// Output Terminal Descriptor 
{
    0x09,   //bLength
    0x24,   //bDescriptorType
    0x03,   //bDescriptorSubType
    0x03,   //bTerminalID
    0x0101, //wTerminalType
    0x00,   //bAssocTerminal
    0x05,   //bSourceID
    0x00    //iTerminal
},

// Input Terminal Descriptor (Camera)
{
    0x11,   //bLength
    0x24,   //bDescriptorType
    0x02,   //bDescriptorSubType
    0x01,   //bTerminalID
    0x0201, //wTerminalType
    0x00,   //bAssocTerminal
    0x00,   //iTerminal
    0x0000, //wObjectiveFocalLengthMin
    0x0000, //wObjectiveFocalLengthMax
    0x0000, //wOcularFocalLength
    0x02,   //bControlSize
    0x0000  //bmControls
},

// Processing Uint Descriptor 
{
    0x0B,   //bLength
    0x24,   //bDescriptorType
    0x05,   //bDescriptorSubType
    0x05,   //bUnitID
    0x01,   //bSourceID
    0x0000, //wMaxMultiplier
    0x02,   //bControlSize
    0x053F, //bmControls
    0x00    //iProcessing    
},

// Standard Interrupt Endpoint Descriptor
{
    0x07,   //bLength
    0x05,   //bDescriptorType
    0x83,   //bEndpointAddress
    0x03,   //bmAttributes
    0x0010, //wMaxPacketSize
    0x06    //bInterval
},

// Class-specific Interrupt Endpoint Descriptor
{
    0x05,   //bLength
    0x25,   //bDescriptorType
    0x03,   //bDescriptorSubType
    0x0010  //wMaxPacketSize
},

// Standard VideoStreaming Interface Descriptor
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x01,   //bInterfceNumber
    0x00,   //bAlternateSetting
    0x00,   //bNumEndpoints
    0x0E,   //bInterfaceClass
    0x02,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x00    //iInterface
}, 

// Class-specific VideoStreaming Header Descriptor
{   
#ifdef UVC_FORMAT_BOTH
    0x0F,   //bLength
#else   
    0x0E,   //bLength
#endif
    0x24,   //bDescriptorType
    0x01,   //bDescriptorSubType
#ifdef UVC_FORMAT_BOTH
    0x02,   //bNumFormats           // Modified Here   
    0x018B, //wTotalLength
#elif defined UVC_FORMAT_YUV
    0x01,   //bNumFormats           // Modified Here   
    0x00D7, //wTotalLength
#elif defined UVC_FORMAT_MJPEG
    0x01,   //bNumFormats           // Modified Here   
    0x00C7, //wTotalLength
#endif       
    0x81,   //bEndpointAddress
    0x00,   //bmInfo
    0x03,   //bTerminalLink
    0x02,   //bStillCaptureMethod    
    0x01,   //bTriggerSupport
    0x00,   //bTriggerUsage
    0x01,   //bControlSize
#ifdef UVC_FORMAT_BOTH    
    0x00,   //bmaControls
#endif    
    0x00    //bmaControls   
},
#ifndef UVC_FORMAT_MJPEG
// Uncompressed Video YUV422
// Class-specific VideoStreaming Format Descriptor
{
    0x1B,   //bLength
    0x24,   //bDescriptorType
    0x04,   //bDescriptorSubType
    0x01,   //bFormatIndex
    0x03,   //bNumFrameDescriptors      // Modified Here
	0x59,0x55,0x59,0x32,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71,
// 	0x7b,0xeb,0x36,0xe4,0x52,0x4f,0x11,0xce,0x95,0x53,0x00,0x20,0xaf,0x0b,0xa7,0x70,
    0x10,
    0x01,   //bDefaultFrameIndex
    0x00,   //bAspectRatioX
    0x00,   //bAspectRatioY
    0x00,   //bmInterlaceFlags    
    0x00    //bCopyProtect
},

// Class-specific VideoStreaming Frame 1 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x05,           //bDescriptorSubType
    0x01,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0280,         //wWidth->640
    0x01E0,         //wHeight->480
    0x00096000,     //dwMinBitRate
    0x01194000,     //dwMaxBitRate
    0x00096000,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},


// Class-specific VideoStreaming Frame 3 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x05,           //bDescriptorSubType
    0x02,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0140,         //wWidth->320
    0x00F0,         //wHeight->240
    0x00025800,     //dwMinBitRate
    0x00465000,     //dwMaxBitRate
    0x00025800,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific VideoStreaming Frame 5 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x05,           //bDescriptorSubType
    0x03,           //bFrameIndex           // Modified Here   
    0x00,           //bmCapabilities    
    0x0A0,          //wWidth->160
    0x078,          //wHeight->120
    0x00009600,     //dwMinBitRate
    0x00119400,     //dwMaxBitRate
    0x00009600,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific Still Image Frame Descriptor
{
    0x12,           //bLength
    0x24,           //bDescriptorType
    0x03,           //bDescriptorSubType
    0x00,  			//bEndpointAddress
    0x03,           //bNumImageSizePatterns      
    0x0280,         //wWidth    
    0x01E0,         //wHeight            
    0x0140,         //wWidth        
    0x00F0,         //wHeight                    
    0x00A0,         //wWidth    
    0x0078,         //wHeight 
    0x00            //bNumCompressionPtn           
},
#endif
#if defined UVC_FORMAT_BOTH || defined UVC_FORMAT_MJPEG
/* MJPEG */
// Class-specific VideoStreaming Format Descriptor
{
    0x0B,   //bLength
    0x24,   //bDescriptorType
    0x06,   //bDescriptorSubType
    0x02,   //bFormatIndex
    0x03,   //bNumFrameDescriptors      // Modified Here
    0x01,   //bmFlags
    0x01,   //bDefaultFrameIndex
    0x00,   //bAspectRatioX
    0x00,   //bAspectRatioY
    0x00,   //bmInterlaceFlags
    0x00,   //bCopyProtect
},

// Class-specific VideoStreaming Frame 1 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x07,           //bDescriptorSubType
    0x01,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0280,         //wWidth->640
    0x01E0,         //wHeight->480
    0x00096000,     //dwMinBitRate
    0x01194000,     //dwMaxBitRate
    0x00096000,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific VideoStreaming Frame 5 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x07,           //bDescriptorSubType
    0x02,           //bFrameIndex           // Modified Here
    0x00,           //bmCapabilities    
    0x0140,         //wWidth->320
    0x00F0,         //wHeight->240
    0x00025800,     //dwMinBitRate
    0x00465000,     //dwMaxBitRate
    0x00025800,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific VideoStreaming Frame 7 Descriptor
{
    0x32,           //bLength
    0x24,           //bDescriptorType
    0x07,           //bDescriptorSubType
    0x03,           //bFrameIndex           // Modified Here   
    0x00,           //bmCapabilities    
    0x0A0,          //wWidth->160
    0x078,          //wHeight->120
    0x00009600,     //dwMinBitRate
    0x00119400,     //dwMaxBitRate
    0x00009600,     //dwMaxVideoFrameBufSize
    0x00051615,     //dwDefaultFrameInterval
    0x06,           //bFrameIntervalType
    0x00051615,     //dwMinFrameInterval
    0x0007A120,     //dwMinFrameInterval
    0x000A2C2A,     //dwMinFrameInterval
    0x000F4240,     //dwMinFrameInterval
    0x001E8480,     //dwMinFrameInterval
    0x00989680      //dwMinFrameInterval

},

// Class-specific Still Image Frame Descriptor
{
    0x12,           //bLength
    0x24,           //bDescriptorType
    0x03,           //bDescriptorSubType
    0x00,  			//bEndpointAddress
    0x03,           //bNumImageSizePatterns          
    0x0280,         //wWidth    
    0x01E0,         //wHeight     
    0x0120,			//wWidth 	    
    0x00F0,         //wHeight                    
    0x00A0,         //wWidth    
    0x0078,         //wHeight                       
    0x00            //bNumCompressionPtn           
},
#endif
/* Color Matching Descriptor */
{
    0x06,           //bLength
    0x24,           //bDescriptorType
    0x0D,           //bDescriptorSubType
    0x01,           //bColorPrimaries
    0x01,           //bTransferCharacteristics
    0x04            //bMatrixCoefficients
},
// Standard VideoStreaming Interface Descriptor  (Num 1, Alt 3)
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x01,   //bInterfceNumber
    0x01,   //bAlternateSetting
    0x01,   //bNumEndpoints    
    0x0E,   //bInterfaceClass
    0x02,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x00    //iInterface
}, 

// Standard VideoStreaming Iso Video Data Endpoint Descriptor
{
    0x07,   //bLength
    0x05,   //bDescriptorType
    0x81,   //bEndpointAddress
    0x05,   //bmAttributes
    MAX_PACKET_SIZE_HS | HSHB, //wMaxPacketSize 
    0x01    //bInterval
},
// Standard VideoStreaming Interface Descriptor  (Num 1, Alt 4)
{
    0x09,   //bLength
    0x04,   //bDescriptorType
    0x01,   //bInterfceNumber
    0x02,   //bAlternateSetting
    0x01,   //bNumEndpoints    
    0x0E,   //bInterfaceClass
    0x02,   //bInterfaceSubClass
    0x00,   //bInterfaceProtocol
    0x00    //iInterface
}, 

// Standard VideoStreaming Iso Video Data Endpoint Descriptor
{
    0x07,   //bLength
    0x05,   //bDescriptorType
    0x81,   //bEndpointAddress
    0x05,   //bmAttributes
    MAX_PACKET_SIZE_HS | HSHB, //wMaxPacketSize   
    0x01    //bInterval
}
};
    
__align(4) static UINT32 UVC_QualifierDescriptor[3] = 
{
	0x0200060a, 0x400102EF, 0x00000001
};

__align(4) static UINT32 UVC_HOSConfigurationBlock[10] =
{
	0x00270709, 0x80000101, 0x00040932, 0x05080300, 0x05070050,
	0x00400281, 0x02050701, 0x01004002, 0x03830507, 0x00010040, 
};

__align(4) static UINT32 UVC_StringDescriptor0[] = 
{
	0x04090304
};

__align(4) UINT8 UVC_StringDescriptor1[] = 
{
	0x16, 0x03,
	'U', 0x00, 'S', 0x00, 'B', 0x00, ' ', 0x00, 'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00,
	'c', 0x00, 'e', 0x00				
};

__align(4) UINT8 UVC_StringDescriptor2[] = 
{
	0x2E, 0x03,
	'W', 0x00, '5', 0x00, '5', 0x00, 'F', 0x00, 'A', 0x00, '9', 0x00, '3', 0x00, ' ', 0x00,
	'U', 0x00, 'S', 0x00, 'B', 0x00, ' ', 0x00, 'U', 0x00, 'V', 0x00, 'C', 0x00, ' ', 0x00,
	'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00
};

__align(4) UINT8 UVC_StringDescriptor3[] = 
{
	0x16, 0x03,
	'0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '5', 0x00, '5', 0x00, 'F', 0x00, 'A', 0x00,
	'9', 0x00, '3', 0x00				
};


VOID uvcdPU_Control(INT16 Value)
{
	uvcInfo.u8ErrCode = 0;
	switch(_usb_cmd_pkt.wValue)
	{
		case PU_BACKLIGHT_COMPENSATION_CONTROL:
				if(Value >= uvcPuInfo.PU_BACKLIGHT_COMPENSATION_MIN && Value <= uvcPuInfo.PU_BACKLIGHT_COMPENSATION_MAX)
				{
					uvcInfo.CapFilter.Backlight = Value;
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_BACKLIGHT_COMPENSATION_CONTROL,Value);
					return;
				}
				break;			
		case PU_BRIGHTNESS_CONTROL:
				if(Value >= uvcPuInfo.PU_BRIGHTNESS_MIN && Value <= uvcPuInfo.PU_BRIGHTNESS_MAX)
				{
					uvcInfo.CapFilter.Brightness = Value;	
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_BRIGHTNESS_CONTROL,Value);					
					return;
				}
				break;			
		case PU_CONTRAST_CONTROL:
			  	if(Value >= uvcPuInfo.PU_CONTRAST_MIN && Value <= uvcPuInfo.PU_CONTRAST_MAX)
				{				
					uvcInfo.CapFilter.Contrast = Value;
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_CONTRAST_CONTROL,Value);	
					return;
				}
			 	break;			
		case PU_HUE_CONTROL:
			  	if(Value >= uvcPuInfo.PU_HUE_MIN && Value <= uvcPuInfo.PU_HUE_MAX)
			  	{			
					uvcInfo.CapFilter.Hue = Value;
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_HUE_CONTROL,Value);	
					return;
				}
			 	break;			
		case PU_SATURATION_CONTROL:
				if(Value >= uvcPuInfo.PU_SATURATION_MIN && Value <= uvcPuInfo.PU_SATURATION_MAX)
			  	{
					uvcInfo.CapFilter.Saturation = Value;	
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_SATURATION_CONTROL,Value);	
					return;
				}
			 	break;			
		case PU_SHARPNESS_CONTROL:
			  	if(Value >= uvcPuInfo.PU_SHARPNESS_MIN && Value <= uvcPuInfo.PU_SHARPNESS_MAX)
				{				
					uvcInfo.CapFilter.Sharpness = Value;
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_SHARPNESS_CONTROL,Value);	
					return;
				}
			 	break;			
		case PU_GAMMA_CONTROL:
			  	if(Value >= uvcPuInfo.PU_GAMMA_MIN && Value <= uvcPuInfo.PU_GAMMA_MAX)
			  	{				
					uvcInfo.CapFilter.Gamma = Value;
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_GAMMA_CONTROL,Value);	
					return;
				}
				
			 	break;
		case PU_POWER_LINE_FREQUENCY_CONTROL:
			  	if(Value >= uvcPuInfo.PU_POWER_LINE_FREQUENCY_MIN && Value <= uvcPuInfo.PU_POWER_LINE_FREQUENCY_MAX)
			  	{				
					uvcInfo.CapFilter.POWER_LINE_FREQUENCY = Value;
					if(uvcInfo.PU_CONTROL!=NULL)
						uvcInfo.PU_CONTROL(PU_POWER_LINE_FREQUENCY_CONTROL,Value);	
					return;
				}
				
			 	break;			 	
	}
	uvcInfo.u8ErrCode = EC_Out_Of_Range;
	outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
	outp32(CEP_CTRL_STAT,0x02);
	return;    
}

UINT32 uvcdPU_Info(UINT32 Req,UINT32 Unit)
{
	uvcInfo.u8ErrCode = 0;
	switch(Req)
   	{
		case GET_CUR:
			    switch(Unit)
			    {		    
			    	case PU_BACKLIGHT_COMPENSATION_CONTROL:
			    		 return uvcInfo.CapFilter.Backlight;
//			    		 break;
			    	case PU_BRIGHTNESS_CONTROL:
			    		 return uvcInfo.CapFilter.Brightness;
//			    		 break;
					case PU_CONTRAST_CONTROL:
						 return uvcInfo.CapFilter.Contrast;
//						 break; 		 
					case PU_HUE_CONTROL:
						 return uvcInfo.CapFilter.Hue;					 
//						 break;   
			   		case PU_SATURATION_CONTROL:
	 		   			 return uvcInfo.CapFilter.Saturation;
//			   			 break;
			   		case PU_SHARPNESS_CONTROL:
			   			 return uvcInfo.CapFilter.Sharpness;
//			   			 break;
			   		case PU_GAMMA_CONTROL:
			   			 return uvcInfo.CapFilter.Gamma;
//			   			 break;
			   		case PU_POWER_LINE_FREQUENCY_CONTROL:
					  	 return uvcInfo.CapFilter.POWER_LINE_FREQUENCY;
//					  	 break;
			   		default:
						uvcInfo.u8ErrCode = EC_Invalid_Control;			
						outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
						outp32(CEP_CTRL_STAT,0x02); 	
						break;
			   	}
			   	break;
		case GET_MIN:			 
			    switch(Unit)
			    {		    
			    	case PU_BACKLIGHT_COMPENSATION_CONTROL:
			    		 return uvcPuInfo.PU_BACKLIGHT_COMPENSATION_MIN;
//			    		 break;
			    	case PU_BRIGHTNESS_CONTROL:
			    		 return uvcPuInfo.PU_BRIGHTNESS_MIN;
//			    		 break;
					case PU_CONTRAST_CONTROL:
						 return uvcPuInfo.PU_CONTRAST_MIN;
//						 break; 		 
					case PU_HUE_CONTROL:
						 return uvcPuInfo.PU_HUE_MIN;					 
//						 break;   
			   		case PU_SATURATION_CONTROL:
	 		   			 return uvcPuInfo.PU_SATURATION_MIN;
//			   			 break;
			   		case PU_SHARPNESS_CONTROL:
			   			 return uvcPuInfo.PU_SHARPNESS_MIN;
//			   			 break;
			   		case PU_GAMMA_CONTROL:
			   			 return uvcPuInfo.PU_GAMMA_MIN;
//			   			 break;
			   		case PU_POWER_LINE_FREQUENCY_CONTROL:
			   			 return uvcPuInfo.PU_POWER_LINE_FREQUENCY_MIN;	
//			   			 break;		   			 
			   		default:
						uvcInfo.u8ErrCode = EC_Invalid_Control;			
						outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
						outp32(CEP_CTRL_STAT,0x02); 		
						return 6;	
//						break;			   			 
			   	}
//			   	break;   	
   		case GET_MAX:
 			    switch(Unit)
			    {		    
			    	case PU_BACKLIGHT_COMPENSATION_CONTROL:
			    		 return uvcPuInfo.PU_BACKLIGHT_COMPENSATION_MAX;
//			    		 break;
			    	case PU_BRIGHTNESS_CONTROL:
			    		 return uvcPuInfo.PU_BRIGHTNESS_MAX;
//			    		 break;
					case PU_CONTRAST_CONTROL:
						 return uvcPuInfo.PU_CONTRAST_MAX;
//						 break; 		 
					case PU_HUE_CONTROL:
						 return uvcPuInfo.PU_HUE_MAX;					 
//						 break;   
			   		case PU_SATURATION_CONTROL:
	 		   			 return uvcPuInfo.PU_SATURATION_MAX;
//			   			 break;
			   		case PU_SHARPNESS_CONTROL:
			   			 return uvcPuInfo.PU_SHARPNESS_MAX;
//			   			 break;
			   		case PU_GAMMA_CONTROL:
			   			 return uvcPuInfo.PU_GAMMA_MAX;
//			   			 break;
			   		case PU_POWER_LINE_FREQUENCY_CONTROL:
			   			 return uvcPuInfo.PU_POWER_LINE_FREQUENCY_MAX;	
//			   			 break;					   			 
			   		default:
						uvcInfo.u8ErrCode = EC_Invalid_Control;			
						outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
						outp32(CEP_CTRL_STAT,0x02); 		
						return 6;	
//						break;			   			 
			   	}
//			   	break;   	
   		case GET_RES:
   				return 1;
//   				break;
   		case GET_INFO:
 			    switch(Unit)
			    {		    
			    	case PU_BACKLIGHT_COMPENSATION_CONTROL:
			    	case PU_BRIGHTNESS_CONTROL:
					case PU_CONTRAST_CONTROL: 
					case PU_HUE_CONTROL:
			   		case PU_SATURATION_CONTROL:
			   		case PU_SHARPNESS_CONTROL:
			   		case PU_GAMMA_CONTROL:
			   		case PU_POWER_LINE_FREQUENCY_CONTROL:
			   			 return 3;
//			   			 break;
			   		default:
						uvcInfo.u8ErrCode = EC_Invalid_Control;			
						outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
						outp32(CEP_CTRL_STAT,0x02); 		
						return 0;	
//						break;			   			 
			   	}
//   				break;
   		case GET_DEF:  
   			    switch(Unit)
			    {		    
			    	case PU_BACKLIGHT_COMPENSATION_CONTROL:
			    		 return uvcPuInfo.PU_BACKLIGHT_COMPENSATION_DEF;
//			    		 break;
			    	case PU_BRIGHTNESS_CONTROL:
			    		 return uvcPuInfo.PU_BRIGHTNESS_DEF;
//			    		 break;
					case PU_CONTRAST_CONTROL:
						 return uvcPuInfo.PU_CONTRAST_DEF;
//						 break; 		 
					case PU_HUE_CONTROL:
						 return uvcPuInfo.PU_HUE_DEF;					 
//						 break;   
			   		case PU_SATURATION_CONTROL:
	 		   			 return uvcPuInfo.PU_SATURATION_DEF;
//			   			 break;
			   		case PU_SHARPNESS_CONTROL:
			   			 return uvcPuInfo.PU_SHARPNESS_DEF;
//			   			 break;
			   		case PU_GAMMA_CONTROL:
			   			 return uvcPuInfo.PU_GAMMA_DEF;
//			   			 break;
			   		case PU_POWER_LINE_FREQUENCY_CONTROL:
			   			 return uvcPuInfo.PU_POWER_LINE_FREQUENCY_DEF;
			   		default:
						uvcInfo.u8ErrCode = EC_Invalid_Control;			
						outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
						outp32(CEP_CTRL_STAT,0x02); 		
						return 6;	
//						break;			   			 
			   	}
//			   	break;  	
		case GET_LEN:
					uvcInfo.u8ErrCode = EC_Invalid_Request;			
					outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));						
					outp32(CEP_CTRL_STAT,0x02); 		
					return 6;	
//					break;					
   	} 
   	return 6;
}


VOID UVC_DMACompletion(void)
{	
	INT volatile TMP;
	TMP = inp32(EPA_DATA_CNT) & 0xFFFF;	
	
	while(TMP >= max_packet_size)
	{
		TMP = inp32(EPA_DATA_CNT)& 0xFFFF;

		if(usbdInfo.AlternateFlag == 0 || ((inp32(PHY_CTL) & Vbus_status) == 0))		
			return;									
		if(inp32(USB_IRQ_STAT) & SUS_IS)							
		{
			sysprintf("#Suspend\n");
			return;
		}
	}	
	
	outpw(HEAD_WORD0,((g_u8UVC_PD | UVC_PH_EndOfFrame | (g_u8UVC_FID &0x01)) <<8) | 0x02);//End Of Frame 				
	outpw(EPA_HEAD_CNT,0x02);  		
	outp32(EPA_RSP_SC, PK_END); /* Set Packet End */
	
	while(inp32(EPA_DATA_CNT) & 0xFFFF)	
	{
		if(usbdInfo.AlternateFlag == 0 || ((inp32(PHY_CTL) & Vbus_status) == 0))		
			return;	
		if(inp32(USB_IRQ_STAT) & SUS_IS)							
		{
			sysprintf("#Suspend\n");
			return;
		}				
	}
	g_u8UVC_FID++;  
	
	uvcStatus.bReady =TRUE;
	
}

VOID UVC_ClassDataIN(void)
{
	UINT32 volatile temp_cnt;
	int volatile i;
	UINT32 volatile *ptr;
	UINT8 volatile *pRemainder;
	UINT32 volatile Data;  

///Video Control Requests
  //Unit and Terminal Control Requests
  	//Interface Control Requests
    if( _usb_cmd_pkt.wIndex == 0x0000 && _usb_cmd_pkt.wValue == VC_REQUEST_ERROR_CODE_CONTROL )//Only send to Video Control interface ID(00)
	{
		Data = uvcInfo.u8ErrCode;					     		
		ptr = (UINT32 *)&Data;
	}
	//Processing Unit Control Requests
   	else if( _usb_cmd_pkt.wIndex == 0x0500)//Processing Unit ID(05) and Video Control interface ID(00)
   	{
		Data = uvcdPU_Info(_usb_cmd_pkt.bRequest,_usb_cmd_pkt.wValue);				
		if(uvcInfo.u8ErrCode)
		{
			usbdInfo._usbd_remlen_flag = 0;
			usbdInfo._usbd_remlen = 0;
			return;				
		}
		else				
			ptr = (UINT32 *)&Data;
	}
	//Camera Terminal Control Requests	
   	else if( _usb_cmd_pkt.wIndex == 0x0100)//Camera Terminal ID(01) and Video Control interface ID(00)
	{	
		uvcInfo.u8ErrCode = EC_Invalid_Control;			
		outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));	
		outp32(CEP_CTRL_STAT,0x02); 		
		usbdInfo._usbd_remlen_flag = 0;
		usbdInfo._usbd_remlen = 0;
		return;				
	}
///Videostreaming Requests	
	else if( _usb_cmd_pkt.wIndex == 0x0001)//VideoStreaming interface(0x01)
	{
		if( _usb_cmd_pkt.wValue == VS_PROBE_CONTROL || _usb_cmd_pkt.wValue == VS_COMMIT_CONTROL)
		{      										
			if(_usb_cmd_pkt.bRequest == GET_INFO || _usb_cmd_pkt.bRequest == GET_LEN)
			{
				if(_usb_cmd_pkt.bRequest == GET_INFO)			
					Data = 3;//Info
				else
					Data = 26;//Length		
				ptr = (UINT32 *)&Data;		
			}else
			{	
				if(uvcInfo.VSCmdCtlData.bFormatIndex != uvcInfo.u32FormatIndx || uvcInfo.VSCmdCtlData.bFrameIndex != uvcInfo.u32FrameIndx) 
				{
				 	uvcStatus.FormatIndex = uvcInfo.u32FormatIndx = uvcInfo.VSCmdCtlData.bFormatIndex;
					uvcStatus.FrameIndex = uvcInfo.u32FrameIndx = uvcInfo.VSCmdCtlData.bFrameIndex;
					
					uvcInfo.bChangeData = 1;
				}								      
				    switch(uvcInfo.VSCmdCtlData.bFrameIndex)
				    {
				    	case UVC_QQVGA://160*120(5)
				    		uvcInfo.VSCmdCtlData.dwMaxVideoFrameSize = UVC_SIZE_QQVGA;
				    		break;
				    	case UVC_QVGA://320*240(3)
					    	uvcInfo.VSCmdCtlData.dwMaxVideoFrameSize = UVC_SIZE_QVGA;
					    	break;
					    case UVC_VGA://640*480(1)
					    	uvcInfo.VSCmdCtlData.dwMaxVideoFrameSize = UVC_SIZE_VGA;
					    	break;
				   	}
				    				      //************Depend on the specificed frame*****************//  			    							    
			    				      
			  		uvcInfo.VSCmdCtlData.dwMaxPayloadTransferSize = uvcInfo.IsoMaxPacketSize[usbdInfo._usbd_intfsel]; 
			  	
			  		uvcStatus.MaxVideoFrameSize = uvcInfo.VSCmdCtlData.dwMaxVideoFrameSize;
		  			uvcInfo.VSCmdCtlData.dwMaxPayloadTransferSize = 2 * max_packet_size;
					uvcInfo.VSCmdCtlData.wCompQuality = 0x3D;
				   	ptr = (UINT32 *)&uvcInfo.VSCmdCtlData;				   	
			}	
		}
		else if( _usb_cmd_pkt.wValue == VS_STILL_PROBE_CONTROL || _usb_cmd_pkt.wValue == VS_STILL_COMMIT_CONTROL)
		{
			if(_usb_cmd_pkt.bRequest == GET_INFO || _usb_cmd_pkt.bRequest == GET_LEN)
			{
				if(_usb_cmd_pkt.bRequest == GET_INFO)			
					Data = 3;//Info
				else
					Data = 11;//Length
					
				ptr = (UINT32 *)&Data;			
			}else
			{						
			    switch(uvcInfo.VSStillCmdCtlData.bFrameIndex)
			    {
			    	case UVC_STILL_QQVGA://160*120(7)
			    		uvcInfo.VSStillCmdCtlData.dwMaxVideoFrameSize = UVC_SIZE_QQVGA;
			    		break;
			    	case UVC_STILL_QVGA://320*240(5)
				    	uvcInfo.VSStillCmdCtlData.dwMaxVideoFrameSize = UVC_SIZE_QVGA;
				    	break;
				    case UVC_STILL_VGA://640*480(3)
				    	uvcInfo.VSStillCmdCtlData.dwMaxVideoFrameSize = UVC_SIZE_VGA;
				    	break;	    
			   	}	
			   	
			   	uvcStatus.snapshotFormatIndex = uvcInfo.VSStillCmdCtlData.bFormatIndex;
			   	uvcStatus.snapshotFrameIndex = uvcInfo.VSStillCmdCtlData.bFrameIndex;			    		
				uvcStatus.snapshotMaxVideoFrameSize = uvcInfo.VSStillCmdCtlData.dwMaxPayloadTranferSize = uvcInfo.IsoMaxPacketSize[usbdInfo._usbd_intfsel];
				ptr = (UINT32 *)&uvcInfo.VSStillCmdCtlData;				    	
			}			
		
		}
		else if(_usb_cmd_pkt.wValue == VS_STILL_IMAGE_TRIGGER_CONTROL)
		{			
			if(_usb_cmd_pkt.bRequest == GET_INFO)			
				Data = 3;//Info
			else
				Data = uvcStatus.StillImage;//Trigger
			ptr = (UINT32 *)&Data;							
		}
	}  	
	if (_usb_cmd_pkt.wLength > 0x40)
	{
		usbdInfo._usbd_remlen_flag = 1;
		usbdInfo._usbd_remlen = _usb_cmd_pkt.wLength - 0x40;
		_usb_cmd_pkt.wLength = 0x40;
	}
	else if (usbdInfo._usbd_remlen != 0)
	{
	    if (usbdInfo._usbd_remlen > 0x40)
	    {
			usbdInfo._usbd_remlen_flag = 1;
			usbdInfo._usbd_remlen = usbdInfo._usbd_remlen -0x40;
		    _usb_cmd_pkt.wLength = 0x40;    			
	    }
	    else
	    {
			usbdInfo._usbd_remlen_flag = 0;
			_usb_cmd_pkt.wLength = usbdInfo._usbd_remlen;
			usbdInfo._usbd_remlen = 0;		        
	    }  	
	}
	else
	{
		usbdInfo._usbd_remlen_flag = 0;
		usbdInfo._usbd_remlen = 0;
	}

	temp_cnt = _usb_cmd_pkt.wLength / 4;

	for (i=0; i<temp_cnt; i++)
		outp32(CEP_DATA_BUF, *ptr++);
		
	temp_cnt = _usb_cmd_pkt.wLength % 4;
	
	if (temp_cnt)
	{
		pRemainder = (UINT8 *)ptr;
		for (i=0; i<temp_cnt; i++)
		{
			outpb(CEP_DATA_BUF, *pRemainder);
			pRemainder++;
		}			
	}			

	if (usbdInfo._usbd_remlen_flag)
		usbdInfo._usbd_ptr =(UINT32 *) ptr;

	outp32(IN_TRNSFR_CNT, _usb_cmd_pkt.wLength);
	return;			
}
VOID UVC_ClassDataOUT(void)
{       	  
    UINT8 volatile *bytPtr;
    UINT8 volatile bytevalue;			  
    UINT16 volatile OutLength;  
    int volatile i;
	
    ///Video Probe Control Request
   	if( _usb_cmd_pkt.wIndex == 0x0500)
	{   
		UINT16 *CmdDataPtr;
		INT16 value = 0; 
		UINT16 OutValue =0;
		
		CmdDataPtr = &OutValue;
	    	if( _usb_cmd_pkt.bRequest == SET_CUR)     	 
	        {  	
	        	if (_usb_cmd_pkt.wLength <= 0x40)
	            	OutLength = _usb_cmd_pkt.wLength;
	            else
	            	OutLength = 0x40;
	              
		        for (i = 0 ; i < _usb_cmd_pkt.wLength/4 ; i=i+1)     
	            {
	            	value = inp32(CEP_DATA_BUF);
	                *CmdDataPtr++ = value;
	                   	          
	            }    
	                   
	            if (_usb_cmd_pkt.wLength %4)
	            {
	            	bytPtr = (UINT8 *)CmdDataPtr;
	                OutLength = _usb_cmd_pkt.wLength %4;//_usb_cmd_pkt.wLength - (_usb_cmd_pkt.wLength/4*4);
	                
	                for (i=0;i<OutLength;i++)
	                {
	                	bytevalue = inpb(CEP_DATA_BUF);
	                    *bytPtr++ = bytevalue;
	                }    
	            }     	                  
				uvcdPU_Control(OutValue);             
			}						   
	}
	else if( _usb_cmd_pkt.wIndex == 0x0100)
	{
		uvcInfo.u8ErrCode = EC_Invalid_Request;
		outp32(CEP_IRQ_ENB, (inp32(CEP_IRQ_ENB) | 0x03));
		outp32(CEP_CTRL_STAT, 0x02);	
	}
	else if( _usb_cmd_pkt.wIndex == 0x0001)
	{ 
		INT32 value = 0; 
	    UINT32 *CmdDataPtr;
	    
		if(_usb_cmd_pkt.wValue == VS_STILL_IMAGE_TRIGGER_CONTROL)
			CmdDataPtr = (UINT32 *)&uvcStatus.StillImage;				
		else if(_usb_cmd_pkt.wValue == VS_PROBE_CONTROL || _usb_cmd_pkt.wValue == VS_COMMIT_CONTROL )
			CmdDataPtr = (UINT32 *)&uvcInfo.VSCmdCtlData;				
		else if(_usb_cmd_pkt.wValue == VS_STILL_PROBE_CONTROL || _usb_cmd_pkt.wValue == VS_STILL_COMMIT_CONTROL )
	    	CmdDataPtr = (UINT32 *)&uvcInfo.VSStillCmdCtlData;	
	    
		if (_usb_cmd_pkt.wLength <= 0x40)
			OutLength = _usb_cmd_pkt.wLength;
	    else
	    	OutLength = 0x40;
	        
	    for (i = 0 ; i < _usb_cmd_pkt.wLength/4 ; i=i+1)     
	    {
	    	value = inp32(CEP_DATA_BUF);
	        *CmdDataPtr++ = value;	                   	          
		}    
        if (_usb_cmd_pkt.wLength %4)
        {
	        bytPtr = (UINT8 *)CmdDataPtr;
	        OutLength = _usb_cmd_pkt.wLength %4;//_usb_cmd_pkt.wLength - (_usb_cmd_pkt.wLength/4*4);
			for (i=0;i<OutLength;i++)
	        {
	        	bytevalue = inpb(CEP_DATA_BUF);
	            *bytPtr++ = bytevalue;
	        }    
	    }     				    
	}	  	    
	if(uvcInfo.u8ErrCode)
	{
	//MSG_DEBUG("Rxd\n");
		outp32(CEP_IRQ_STAT, 0x440);
		outp32(CEP_IRQ_ENB, 0x2);		// suppkt int//enb sts completion int		    		    
	}			    	    
	else
	{
	//MSG_DEBUG("Rxd\n");
		outp32(CEP_IRQ_STAT, 0x440);
		outp32(CEP_CTRL_STAT, CEP_NAK_CLEAR);	// clear nak so that sts stage is complete
		outp32(CEP_IRQ_ENB, 0x400);		// suppkt int//enb sts completion int
	}
	return;
}




VOID uvcdHighSpeedInit(void)
{
	//sysprintf("uvcdHighSpeedInit\n");
	outp32(EPA_START_ADDR, 0x00000040);
	max_packet_size = MAX_PACKET_SIZE_HS;
	while(inp32(EPA_START_ADDR) != 0x00000040);
	
	max_packet_size = MAX_PACKET_SIZE_HS;
	
	outp32(EPA_RSP_SC, 0x00000000);		// av mode
	
	outp32(EPA_MPS, max_packet_size);		// mps 			
#ifdef HSHB_MODE				
	outp32(EPA_CFG, 0x000011f);		// iso in ep no 1
#else	
	outp32(EPA_CFG, 0x000001f);		// iso in ep no 1
#endif	
	outp32(EPA_END_ADDR, 0x000007EF);	
	outp32(OPER, SET_HISPD);	
	/* interrupt in */
	outp32(EPB_RSP_SC, 0x00000000);		// av mode
	outp32(EPB_MPS, 0x00000010);		// mps 
	outp32(EPB_CFG, 0x0000003d);		// interrupt in ep no 3
	outp32(EPB_START_ADDR, 0x000007F0);
	outp32(EPB_END_ADDR, 0x000007FF);			

}

VOID uvcdFullSpeedInit(void)
{
	//sysprintf("uvcdFullSpeedInit\n");
	outp32(EPA_START_ADDR, 0x00000100);
	max_packet_size = MAX_PACKET_SIZE_FS;
	while(inp32(EPA_START_ADDR) != 0x00000100);
	
	outp32(EPA_RSP_SC, 0x00000000);		// av mode
	
	outp32(EPA_MPS, max_packet_size);		// mps 			
				
	outp32(EPA_CFG, 0x0000001f);		// iso in ep no 1
	
	outp32(EPA_END_ADDR, 0x000005FF);	
	
	/* interrupt in */
	outp32(EPB_RSP_SC, 0x00000000);		// av mode
	outp32(EPB_MPS, 0x00000010);		// mps 
	outp32(EPB_CFG, 0x0000003d);		// interrupt in ep no 3
	outp32(EPB_START_ADDR, 0x00000600);
	outp32(EPB_END_ADDR, 0x0000071F);		
	
	
}

VOID uvcdInit(PFN_UVCD_PUCONTROL_CALLBACK* callback_func)
{

	sysprintf("N3290 UVC Library (%s)\n",DATA_CODE);
	
	usbdInfo.u32UVC = 1;
	usbdInfo.pu32DevDescriptor = (PUINT32) &UVC_DeviceDescriptor;
	usbdInfo.pu32QulDescriptor = (PUINT32) &UVC_QualifierDescriptor;
	usbdInfo.pu32HSConfDescriptor = (PUINT32) &UVC_ConfigurationBlock;
	usbdInfo.pu32FSConfDescriptor = (PUINT32) &UVC_ConfigurationBlock_FS;
	usbdInfo.pu32HOSConfDescriptor = (PUINT32) &UVC_HOSConfigurationBlock;	
			
	usbdInfo.pu32StringDescriptor[0] = (PUINT32) &UVC_StringDescriptor0;
	usbdInfo.pu32StringDescriptor[1] = (PUINT32) &UVC_StringDescriptor1;
	usbdInfo.pu32StringDescriptor[2] = (PUINT32) &UVC_StringDescriptor2;
	usbdInfo.pu32StringDescriptor[3] = (PUINT32) &UVC_StringDescriptor3;
	
#ifdef	UVC_FORMAT_BOTH		
 	UVC_ConfigurationBlock.VideoClassHeader.wTotalLength = 
		sizeof(CSHEADERDESCRIPTOR_BOTH) + sizeof(UNCOMPRESSFORMATDESCRIPTOR) +  
		sizeof(FRAMEDESCRIPTOR_1) + sizeof(FRAMEDESCRIPTOR_1) +
		sizeof(FRAMEDESCRIPTOR_1) + sizeof(CSSTILLFRAMEDESCRIPTOR) + 	   
        sizeof(CSFORMATDESCRIPTOR_MJPEG) + sizeof(FRAMEDESCRIPTOR_1) +	
		sizeof(FRAMEDESCRIPTOR_1) + sizeof(FRAMEDESCRIPTOR_1) +  
		sizeof(CSSTILLFRAMEDESCRIPTOR) + sizeof(COLORMATCHDESCRIPTOR); 	   	
	
	UVC_ConfigurationBlock.VideoClassConfig.wTotalLength = sizeof(VIDEOCLASS);
#elif defined UVC_FORMAT_YUV	
	UVC_ConfigurationBlock.VideoClassHeader.wTotalLength = 
		sizeof(CSHEADERDESCRIPTOR) + sizeof(UNCOMPRESSFORMATDESCRIPTOR) +  
		sizeof(FRAMEDESCRIPTOR_1) + sizeof(FRAMEDESCRIPTOR_1) +
		sizeof(FRAMEDESCRIPTOR_1) + sizeof(CSSTILLFRAMEDESCRIPTOR) + 	   
        sizeof(COLORMATCHDESCRIPTOR);
		
	UVC_ConfigurationBlock.VideoClassConfig.wTotalLength = sizeof(VIDEOCLASS_YUV);
#elif defined UVC_FORMAT_MJPEG
 	UVC_ConfigurationBlock.VideoClassHeader.wTotalLength = 
		sizeof(CSHEADERDESCRIPTOR) + 
        sizeof(CSFORMATDESCRIPTOR_MJPEG) + sizeof(FRAMEDESCRIPTOR_1) +	
		sizeof(FRAMEDESCRIPTOR_1) + sizeof(FRAMEDESCRIPTOR_1) +  
		sizeof(CSSTILLFRAMEDESCRIPTOR) + sizeof(COLORMATCHDESCRIPTOR); 	   	
	
	UVC_ConfigurationBlock.VideoClassConfig.wTotalLength = sizeof(VIDEOCLASS_MJPEG);
	
#endif	

	usbdInfo.u32DevDescriptorLen =  UVC_DEVICE_DSCPT_LEN;
	usbdInfo.u32HSConfDescriptorLen = UVC_ConfigurationBlock.VideoClassConfig.wTotalLength;
	usbdInfo.u32FSConfDescriptorLen = UVC_ConfigurationBlock.VideoClassConfig.wTotalLength;	
	usbdInfo.u32StringDescriptorLen[0] = UVC_STR0_DSCPT_LEN;
	usbdInfo.u32StringDescriptorLen[1] = UVC_StringDescriptor1[0] = sizeof(UVC_StringDescriptor1);
	usbdInfo.u32StringDescriptorLen[2] = UVC_StringDescriptor2[0] = sizeof(UVC_StringDescriptor2);
	usbdInfo.u32StringDescriptorLen[3] = UVC_StringDescriptor3[0] = sizeof(UVC_StringDescriptor3);
	usbdInfo.u32QulDescriptorLen =  UVC_QUALIFIER_DSCPT_LEN;
	usbdInfo.u32HOSConfDescriptorLen =  UVC_HOSCONFIG_DSCPT_LEN;
	
	/* Set Endpoint map */
	usbdInfo.i32EPA_Num = 1;	/* Endpoint 1 */
	usbdInfo.i32EPB_Num = 3;	/* Endpoint 2 */
	usbdInfo.i32EPC_Num = -1;	/* Not use */
	usbdInfo.i32EPD_Num = -1;	/* Not use */
	
	usbdInfo.pfnClassDataOUTCallBack = UVC_ClassDataOUT;
	usbdInfo.pfnClassDataINCallBack = UVC_ClassDataIN;	
	usbdInfo.pfnDMACompletion = UVC_DMACompletion;
	
	usbdInfo.pfnHighSpeedInit = uvcdHighSpeedInit;
	usbdInfo.pfnFullSpeedInit = uvcdFullSpeedInit;
		
	uvcInfo.u8ErrCode = 0;                                                
	uvcInfo.bChangeData = 0;
	uvcStatus.StillImage = 0;
	usbdInfo.AlternateFlag = 0;	 
	uvcInfo.PU_CONTROL = callback_func;	

	uvcInfo.IsoMaxPacketSize[0] = max_packet_size;
	uvcInfo.IsoMaxPacketSize[1] = max_packet_size; 
	uvcInfo.IsoMaxPacketSize[2] = max_packet_size; 	
	usbdInfo.usbdMaxPacketSize = uvcInfo.IsoMaxPacketSize[1];
	
	//Initialize Video Capture Filter Control Value               	
  	uvcInfo.CapFilter.Brightness = 1;  	
    uvcInfo.CapFilter.POWER_LINE_FREQUENCY = 2;
	uvcInfo.CapFilter.Brightness = 1;
	uvcInfo.CapFilter.Contrast = 2;
	uvcInfo.CapFilter.Hue = 3;
	uvcInfo.CapFilter.Saturation = 4;
	uvcInfo.CapFilter.Sharpness = 5;
	uvcInfo.CapFilter.Gamma = 3;
	uvcInfo.CapFilter.Backlight = 7;

    /* Initialize Video Probe and Commit Control data */
    memset((UINT8 *)&uvcInfo.VSCmdCtlData,0x0,sizeof(VIDEOSTREAMCMDDATA));
    uvcInfo.VSCmdCtlData.bmHint = 0x0100;
    uvcInfo.VSCmdCtlData.bFormatIndex = 1;
    uvcInfo.VSCmdCtlData.bFrameIndex = 1;
	uvcStatus.FormatIndex = 1;
	uvcStatus.FrameIndex = 1;    
    uvcInfo.VSCmdCtlData.dwFrameInterval = 0x051615;

	/* Initialize Still Image Command data */
	memset((UINT8 *)&uvcInfo.VSStillCmdCtlData,0x0,sizeof(VIDEOSTREAMSTILLCMDDATA));

	uvcInfo.VSStillCmdCtlData.bFormatIndex = 1;
    uvcInfo.VSStillCmdCtlData.bFrameIndex = 1;   
	uvcStatus.snapshotFormatIndex = 1;
	uvcStatus.snapshotFrameIndex = 1;
}

VOID uvcdSDRAM_USB_Transfer(UINT8 epname,UINT32 DRAM_Addr ,UINT32 Tran_Size)
{	
	if(Tran_Size != 0)
	{	
		if(uvcInfo.bChangeData ==1)
		{			
			uvcInfo.bChangeData = 0;
			if (inpw(DMA_CTRL_STS) &0x20)
			{
                outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);	/* Reset DMA */
	            outp32(DMA_CTRL_STS, 0x0000000);               
	            
			    uvcStatus.bReady =TRUE; 
		    }		
		}
		
		outp32(USB_IRQ_ENB, (USB_DMA_REQ | USB_RST_STS|BIT8));
		outp32(AHB_DMA_ADDR, DRAM_Addr);
		outp32(DMA_CNT, Tran_Size);
		
		if (epname == EP_A)
		    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000031);//DMA_Enable;DMA_Read;DMA_Ep_Address = 1		
		else    
		    outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000020);//DMA_Enable;DMA_Write;DMA_Ep_Address = 0
		    
	}

}


BOOL uvcdIsReady(void)
{
	if(!uvcStatus.bReady)
	{			
		/* Application disconnected */
		if(usbdInfo.AlternateFlag == 0 || ((inp32(PHY_CTL) & Vbus_status) == 0))	
		{
			if (inpw(DMA_CTRL_STS) &0x20)
			{
				//sysprintf("Reset DMA \n");
	            outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);
		    	outp32(DMA_CTRL_STS, 0x000000);     
		    	outp32(EPA_RSP_SC,BUF_FLUSH );
		    }
	   
		    uvcStatus.bReady =TRUE; 			
		}
	}	
	return uvcStatus.bReady;	
}


BOOL uvcdSendImage(UINT32 u32Addr, UINT32 u32transferSize, BOOL bStillImage)
{
	if(u32transferSize != 0)
	{
	    uvcStatus.bReady =FALSE;
        
        outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);	/* Reset DMA */
		outp32(DMA_CTRL_STS, 0x000000);    
		           			    
	    
		if(usbdStatus.appConnected == 1)
		{
			if(bStillImage) 
			{    		
				g_u8UVC_PD = UVC_PH_Snapshot;		/* Payload Header for Snapshot */    		       			    			
				uvcStatus.StillImage = 0; 
			}
			else 		
				g_u8UVC_PD = UVC_PH_Preview;		/* Payload Header for Preview */ 			 

			/* Start of Transfer Payload Data */					
			outpw(HEAD_WORD0,((g_u8UVC_PD | (g_u8UVC_FID &0x01)) <<8)| 0x02);   			
			outpw(EPA_HEAD_CNT,0x02);      
			
		    uvcdSDRAM_USB_Transfer(EP_A, u32Addr , u32transferSize);
	    
			return TRUE;
		}
		else
		{
			if (inpw(DMA_CTRL_STS) &0x20)
			{
				//sysprintf("Reset DMA 3\n");
	            outp32(DMA_CTRL_STS, inp32(DMA_CTRL_STS)|0x00000080);	/* Reset DMA */
		        outp32(DMA_CTRL_STS, 0x000000);                                
		    }
		    usbdInfo._usbd_DMA_In =1;   
		    uvcStatus.bReady =TRUE;
		}      
	}	 
	return FALSE;
}