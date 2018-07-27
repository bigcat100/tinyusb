/**************************************************************************/
/*!
    @file     usbd_desc.c
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/

#include "tusb_option.h"

#if TUSB_OPT_DEVICE_ENABLED

#define _TINY_USB_SOURCE_FILE_

#include "tusb.h"


#if CFG_TUD_DESC_AUTO

/*------------- VID/PID -------------*/
#ifndef CFG_TUD_DESC_VID
#define CFG_TUD_DESC_VID          0xCAFE
#endif

#ifndef CFG_TUD_DESC_PID

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID Generic | HID Composite | HID Mouse | HID Keyboard | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)    ( (CFG_TUD_##itf) << (n) )
#define CFG_TUD_DESC_PID    (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | \
                             _PID_MAP(HID_KEYBOARD, 2) | _PID_MAP(HID_MOUSE, 3) /*|  _PID_MAP(HID_GENERIC, 5)*/ )
#endif

/*------------- Interface Numbering -------------*/
/* The order as follows: CDC, MSC, HID
 * If a interface is not enabled, the later will take its place
 */

#define ITF_NUM_CDC           0
#define ITF_NUM_MSC           (ITF_NUM_CDC + 2*CFG_TUD_CDC)

#define ITF_NUM_HID_KBD       (ITF_NUM_MSC + CFG_TUD_MSC)
#define ITF_NUM_HID_MSE       (ITF_NUM_HID_KBD + CFG_TUD_HID_KEYBOARD)

#define ITF_TOTAL             (ITF_NUM_HID_MSE + CFG_TUD_HID_MOUSE)


/*------------- Endpoint Numbering & Size -------------*/
#define _EP_IN(x)             (0x80 | (x))
#define _EP_OUT(x)            (x)

// CDC
#define EP_CDC_NOTIF          _EP_IN (ITF_NUM_CDC+1)
#define EP_CDC_NOTIF_SIZE     8

#define EP_CDC_OUT            _EP_OUT(ITF_NUM_CDC+2)
#define EP_CDC_IN             _EP_IN (ITF_NUM_CDC+2)

// Mass Storage
#define EP_MSC_OUT            _EP_OUT(ITF_NUM_MSC+1)
#define EP_MSC_IN             _EP_IN (ITF_NUM_MSC+1)


// HID Keyboard with boot protocol
#if CFG_TUD_HID_KEYBOARD && CFG_TUD_HID_KEYBOARD_BOOT
#define EP_HID_KBD_BOOT       _EP_IN (ITF_NUM_HID_KBD+1)
#define EP_HID_KBD_BOOT_SZ    8

#endif

// HID Mouse with boot protocol
#if CFG_TUD_HID_MOUSE && CFG_TUD_HID_MOUSE_BOOT
#define EP_HID_MSE_BOOT        _EP_IN (ITF_NUM_HID_MSE+1)
#define EP_HID_MSE_BOOT_SZ     8
#endif

#if 0 // CFG_TUD_HID_BOOT_PROTOCOL

// HID composite = keyboard + mouse
#define EP_HID_COMP           _EP_IN (ITF_NUM_HID_KBD+1)
#define EP_HID_COMP_SIZE      16

#endif


// TODO HID Generic


//--------------------------------------------------------------------+
// Keyboard Report Descriptor
//--------------------------------------------------------------------+
#if CFG_TUD_HID_KEYBOARD
uint8_t const _desc_auto_hid_kbd_report[] = {
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
      // 8 bits Modifier Keys (Shfit, Control, Alt)
      HID_USAGE_MIN    ( 224                                    ),
      HID_USAGE_MAX    ( 231                                    ),
      HID_LOGICAL_MIN  ( 0                                      ),
      HID_LOGICAL_MAX  ( 1                                      ),

      HID_REPORT_COUNT ( 8                                      ),
      HID_REPORT_SIZE  ( 1                                      ),
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

      // 8 bit reserved
      HID_REPORT_COUNT ( 1                                      ),
      HID_REPORT_SIZE  ( 8                                      ),
      HID_INPUT        ( HID_CONSTANT                           ),

    // 6-byte Keycodes
    HID_USAGE_PAGE (HID_USAGE_PAGE_KEYBOARD),
      HID_USAGE_MIN    ( 0                                   ),
      HID_USAGE_MAX    ( 255                                 ),
      HID_LOGICAL_MIN  ( 0                                   ),
      HID_LOGICAL_MAX  ( 255                                 ),

      HID_REPORT_COUNT ( 6                                   ),
      HID_REPORT_SIZE  ( 8                                   ),
      HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ),

    // LED Indicator Kana | Compose | Scroll Lock | CapsLock | NumLock
    HID_USAGE_PAGE  ( HID_USAGE_PAGE_LED                   ),
      /* 5-bit Led report */
      HID_USAGE_MIN    ( 1                                       ),
      HID_USAGE_MAX    ( 5                                       ),
      HID_REPORT_COUNT ( 5                                       ),
      HID_REPORT_SIZE  ( 1                                       ),
      HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),

      /* led padding */
      HID_REPORT_COUNT ( 1                                       ),
      HID_REPORT_SIZE  ( 3                                       ),
      HID_OUTPUT       ( HID_CONSTANT                            ),
  HID_COLLECTION_END
};
#endif

//--------------------------------------------------------------------+
// Mouse Report Descriptor
//--------------------------------------------------------------------+
#if CFG_TUD_HID_MOUSE
uint8_t const _desc_auto_hid_mse_report[] = {
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE    ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
    HID_USAGE      (HID_USAGE_DESKTOP_POINTER),

    HID_COLLECTION ( HID_COLLECTION_PHYSICAL ),
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON ),
        HID_USAGE_MIN    ( 1                                      ),
        HID_USAGE_MAX    ( 3                                      ),
        HID_LOGICAL_MIN  ( 0                                      ),
        HID_LOGICAL_MAX  ( 1                                      ),

        // Left, Right, Middle, Backward, Forward mouse buttons
        HID_REPORT_COUNT ( 3                                      ),
        HID_REPORT_SIZE  ( 1                                      ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

        // 3 bit padding
        HID_REPORT_COUNT ( 1                                      ),
        HID_REPORT_SIZE  ( 5                                      ),
        HID_INPUT        ( HID_CONSTANT                           ),

      HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP ),
        /* X, Y position */
        HID_USAGE        ( HID_USAGE_DESKTOP_X                    ),
        HID_USAGE        ( HID_USAGE_DESKTOP_Y                    ),
        HID_LOGICAL_MIN  ( 0x81                                   ), /* -127 */
        HID_LOGICAL_MAX  ( 0x7f                                   ), /* 127  */

        HID_REPORT_COUNT ( 2                                      ),
        HID_REPORT_SIZE  ( 8                                      ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_RELATIVE ),

        /* mouse scroll */
        HID_USAGE       ( HID_USAGE_DESKTOP_WHEEL                ),
        HID_LOGICAL_MIN ( 0x81                                   ), /* -127 */
        HID_LOGICAL_MAX ( 0x7f                                   ), /* 127  */
        HID_REPORT_COUNT( 1                                      ),
        HID_REPORT_SIZE ( 8                                      ),
        HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ),

    HID_COLLECTION_END,
  HID_COLLECTION_END
};
#endif


/*------------------------------------------------------------------*/
/* Auto generate descriptor
 *------------------------------------------------------------------*/

// For highspeed device but currently in full speed mode
//tusb_desc_device_qualifier_t _device_qual =
//{
//    .bLength = sizeof(tusb_desc_device_qualifier_t),
//    .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
//    .bcdUSB = 0x0200,
//    .bDeviceClass =
//};

/*------------- Device Descriptor -------------*/
tusb_desc_device_t const _desc_auto_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

  #if CFG_TUD_CDC
    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
  #else
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
  #endif

    .bMaxPacketSize0    = CFG_TUD_ENDOINT0_SIZE,

    .idVendor           = CFG_TUD_DESC_VID,
    .idProduct          = CFG_TUD_DESC_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01 // TODO multiple configurations
};


/*------------- Configuration Descriptor -------------*/
typedef struct ATTR_PACKED
{
  tusb_desc_configuration_t           config;

  //------------- CDC -------------//
#if CFG_TUD_CDC
  struct ATTR_PACKED
  {
    tusb_desc_interface_assoc_t       iad;

    //CDC Control Interface
    tusb_desc_interface_t             comm_itf;
    cdc_desc_func_header_t            header;
    cdc_desc_func_call_management_t   call;
    cdc_desc_func_acm_t               acm;
    cdc_desc_func_union_t             union_func;
    tusb_desc_endpoint_t              ep_notif;

    //CDC Data Interface
    tusb_desc_interface_t             data_itf;
    tusb_desc_endpoint_t              ep_out;
    tusb_desc_endpoint_t              ep_in;
  }cdc;
#endif

  //------------- Mass Storage -------------//
#if CFG_TUD_MSC
  struct ATTR_PACKED
  {
    tusb_desc_interface_t             itf;
    tusb_desc_endpoint_t              ep_out;
    tusb_desc_endpoint_t              ep_in;
  } msc;
#endif

  //------------- HID -------------//
#if CFG_TUD_HID_KEYBOARD && CFG_TUD_HID_KEYBOARD_BOOT
  struct ATTR_PACKED
  {
    tusb_desc_interface_t             itf;
    tusb_hid_descriptor_hid_t         hid_desc;
    tusb_desc_endpoint_t              ep_in;
  } hid_kbd_boot;
#endif

#if CFG_TUD_HID_MOUSE && CFG_TUD_HID_MOUSE_BOOT
  struct ATTR_PACKED
  {
    tusb_desc_interface_t             itf;
    tusb_hid_descriptor_hid_t         hid_desc;
    tusb_desc_endpoint_t              ep_in;
  } hid_mse_boot;
#endif

#if 0 // CFG_TUD_HID_BOOT_PROTOCOL

#if CFG_TUD_HID_KEYBOARD || CFG_TUD_HID_MOUSE
  struct ATTR_PACKED
  {
    tusb_desc_interface_t             itf;
    tusb_hid_descriptor_hid_t         hid_desc;
    tusb_desc_endpoint_t              ep_in;

    #if CFG_TUD_HID_KEYBOARD
    tusb_desc_endpoint_t              ep_out;
    #endif
  } hid_composite;
#endif

#endif

} desc_auto_cfg_t;

desc_auto_cfg_t const _desc_auto_config_struct =
{
    .config =
    {
        .bLength             = sizeof(tusb_desc_configuration_t),
        .bDescriptorType     = TUSB_DESC_CONFIGURATION,

        .wTotalLength        = sizeof(desc_auto_cfg_t),
        .bNumInterfaces      = ITF_TOTAL,

        .bConfigurationValue = 1,
        .iConfiguration      = 0x00,
        .bmAttributes        = TUSB_DESC_CONFIG_ATT_BUS_POWER,
        .bMaxPower           = TUSB_DESC_CONFIG_POWER_MA(100)
    },

#if CFG_TUD_CDC
    // IAD points to CDC Interfaces
    .cdc =
    {
      .iad =
      {
          .bLength           = sizeof(tusb_desc_interface_assoc_t),
          .bDescriptorType   = TUSB_DESC_INTERFACE_ASSOCIATION,

          .bFirstInterface   = ITF_NUM_CDC,
          .bInterfaceCount   = 2,

          .bFunctionClass    = TUSB_CLASS_CDC,
          .bFunctionSubClass = CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL,
          .bFunctionProtocol = CDC_COMM_PROTOCOL_ATCOMMAND,
          .iFunction         = 0
      },

      //------------- CDC Communication Interface -------------//
      .comm_itf =
      {
          .bLength            = sizeof(tusb_desc_interface_t),
          .bDescriptorType    = TUSB_DESC_INTERFACE,
          .bInterfaceNumber   = ITF_NUM_CDC,
          .bAlternateSetting  = 0,
          .bNumEndpoints      = 1,
          .bInterfaceClass    = TUSB_CLASS_CDC,
          .bInterfaceSubClass = CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL,
          .bInterfaceProtocol = CDC_COMM_PROTOCOL_ATCOMMAND,
          .iInterface         = 4
      },

      .header =
      {
          .bLength            = sizeof(cdc_desc_func_header_t),
          .bDescriptorType    = TUSB_DESC_CLASS_SPECIFIC,
          .bDescriptorSubType = CDC_FUNC_DESC_HEADER,
          .bcdCDC             = 0x0120
      },

      .call =
      {
          .bLength            = sizeof(cdc_desc_func_call_management_t),
          .bDescriptorType    = TUSB_DESC_CLASS_SPECIFIC,
          .bDescriptorSubType = CDC_FUNC_DESC_CALL_MANAGEMENT,
          .bmCapabilities     = { 0 },
          .bDataInterface     = ITF_NUM_CDC+1,
      },

      .acm =
      {
          .bLength            = sizeof(cdc_desc_func_acm_t),
          .bDescriptorType    = TUSB_DESC_CLASS_SPECIFIC,
          .bDescriptorSubType = CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT,
          .bmCapabilities     = { // 0x02
              .support_line_request = 1,
          }
      },

      .union_func =
      {
          .bLength                  = sizeof(cdc_desc_func_union_t), // plus number of
          .bDescriptorType          = TUSB_DESC_CLASS_SPECIFIC,
          .bDescriptorSubType       = CDC_FUNC_DESC_UNION,
          .bControlInterface        = ITF_NUM_CDC,
          .bSubordinateInterface    = ITF_NUM_CDC+1,
      },

      .ep_notif =
      {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_CDC_NOTIF,
          .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
          .wMaxPacketSize   = { .size = EP_CDC_NOTIF_SIZE },
          .bInterval        = 0x10
      },

      //------------- CDC Data Interface -------------//
      .data_itf =
      {
          .bLength            = sizeof(tusb_desc_interface_t),
          .bDescriptorType    = TUSB_DESC_INTERFACE,
          .bInterfaceNumber   = ITF_NUM_CDC+1,
          .bAlternateSetting  = 0x00,
          .bNumEndpoints      = 2,
          .bInterfaceClass    = TUSB_CLASS_CDC_DATA,
          .bInterfaceSubClass = 0,
          .bInterfaceProtocol = 0,
          .iInterface         = 0x00
      },

      .ep_out =
      {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_CDC_OUT,
          .bmAttributes     = { .xfer = TUSB_XFER_BULK },
          .wMaxPacketSize   = { .size = CFG_TUD_CDC_EPSIZE },
          .bInterval        = 0
      },

      .ep_in =
      {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_CDC_IN,
          .bmAttributes     = { .xfer = TUSB_XFER_BULK },
          .wMaxPacketSize   = { .size = CFG_TUD_CDC_EPSIZE },
          .bInterval        = 0
      },
    },
#endif // cdc

#if CFG_TUD_MSC
    //------------- Mass Storage-------------//
    .msc =
    {
      .itf =
      {
          .bLength            = sizeof(tusb_desc_interface_t),
          .bDescriptorType    = TUSB_DESC_INTERFACE,
          .bInterfaceNumber   = ITF_NUM_MSC,
          .bAlternateSetting  = 0x00,
          .bNumEndpoints      = 2,
          .bInterfaceClass    = TUSB_CLASS_MSC,
          .bInterfaceSubClass = MSC_SUBCLASS_SCSI,
          .bInterfaceProtocol = MSC_PROTOCOL_BOT,
          .iInterface         = 4 + CFG_TUD_CDC
      },

      .ep_out =
      {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_MSC_OUT,
          .bmAttributes     = { .xfer = TUSB_XFER_BULK },
          .wMaxPacketSize   = { .size = CFG_TUD_MSC_EPSIZE},
          .bInterval        = 1
      },

      .ep_in =
      {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_MSC_IN,
          .bmAttributes     = { .xfer = TUSB_XFER_BULK },
          .wMaxPacketSize   = { .size = CFG_TUD_MSC_EPSIZE},
          .bInterval        = 1
      }
    },
#endif // msc

#if CFG_TUD_HID_KEYBOARD && CFG_TUD_HID_KEYBOARD_BOOT
    .hid_kbd_boot =
    {
        .itf =
        {
          .bLength            = sizeof(tusb_desc_interface_t),
          .bDescriptorType    = TUSB_DESC_INTERFACE,
          .bInterfaceNumber   = ITF_NUM_HID_KBD,
          .bAlternateSetting  = 0x00,
          .bNumEndpoints      = 1,
          .bInterfaceClass    = TUSB_CLASS_HID,
          .bInterfaceSubClass = HID_SUBCLASS_BOOT,
          .bInterfaceProtocol = HID_PROTOCOL_KEYBOARD,
          .iInterface         = 0 //4 + CFG_TUD_CDC + CFG_TUD_MSC
        },

        .hid_desc =
        {
          .bLength         = sizeof(tusb_hid_descriptor_hid_t),
          .bDescriptorType = HID_DESC_TYPE_HID,
          .bcdHID          = 0x0111,
          .bCountryCode    = HID_Local_NotSupported,
          .bNumDescriptors = 1,
          .bReportType     = HID_DESC_TYPE_REPORT,
          .wReportLength   = sizeof(_desc_auto_hid_kbd_report)
        },

        .ep_in =
        {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_HID_KBD_BOOT,
          .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
          .wMaxPacketSize   = { .size = EP_HID_KBD_BOOT_SZ },
          .bInterval        = 0x0A
        }
    },
#endif // boot keyboard

  //------------- HID Mouse -------------//
#if CFG_TUD_HID_MOUSE && CFG_TUD_HID_MOUSE_BOOT
    .hid_mse_boot =
    {
        .itf =
        {
          .bLength            = sizeof(tusb_desc_interface_t),
          .bDescriptorType    = TUSB_DESC_INTERFACE,
          .bInterfaceNumber   = ITF_NUM_HID_MSE,
          .bAlternateSetting  = 0x00,
          .bNumEndpoints      = 1,
          .bInterfaceClass    = TUSB_CLASS_HID,
          .bInterfaceSubClass = HID_SUBCLASS_BOOT,
          .bInterfaceProtocol = HID_PROTOCOL_MOUSE,
          .iInterface         = 0 // 4 + CFG_TUD_CDC + CFG_TUD_MSC + CFG_TUD_HID_KEYBOARD
        },

        .hid_desc =
        {
          .bLength         = sizeof(tusb_hid_descriptor_hid_t),
          .bDescriptorType = HID_DESC_TYPE_HID,
          .bcdHID          = 0x0111,
          .bCountryCode    = HID_Local_NotSupported,
          .bNumDescriptors = 1,
          .bReportType     = HID_DESC_TYPE_REPORT,
          .wReportLength   = sizeof(_desc_auto_hid_mse_report)
        },

        .ep_in =
        {
          .bLength          = sizeof(tusb_desc_endpoint_t),
          .bDescriptorType  = TUSB_DESC_ENDPOINT,
          .bEndpointAddress = EP_HID_MSE_BOOT,
          .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
          .wMaxPacketSize   = { .size = EP_HID_MSE_BOOT_SZ },
          .bInterval        = 0x0A
        },
    },

#endif // boot mouse

#if 0

#if CFG_TUD_HID_KEYBOARD || CFG_TUD_HID_MOUSE
    //------------- HID Keyboard + Mouse (multiple reports) -------------//
    .hid_composite =
    {
        .itf =
        {
            .bLength            = sizeof(tusb_desc_interface_t),
            .bDescriptorType    = TUSB_DESC_INTERFACE,
            .bInterfaceNumber   = ITF_NUM_HID_KBD,
            .bAlternateSetting  = 0x00,
            .bNumEndpoints      = 2,
            .bInterfaceClass    = TUSB_CLASS_HID,
            .bInterfaceSubClass = 0,
            .bInterfaceProtocol = 0,
            .iInterface         = 4 + CFG_TUD_CDC + CFG_TUD_MSC,
        },

        .hid_desc =
        {
            .bLength           = sizeof(tusb_hid_descriptor_hid_t),
            .bDescriptorType   = HID_DESC_TYPE_HID,
            .bcdHID            = 0x0111,
            .bCountryCode      = HID_Local_NotSupported,
            .bNumDescriptors   = 1,
            .bReportType       = HID_DESC_TYPE_REPORT,
            .wReportLength     = sizeof(_desc_auto_hid_composite_report)
        },

        .ep_in =
        {
            .bLength          = sizeof(tusb_desc_endpoint_t),
            .bDescriptorType  = TUSB_DESC_ENDPOINT,
            .bEndpointAddress = EP_HID_COMP,
            .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
            .wMaxPacketSize   = { .size = EP_HID_COMP_SIZE },
            .bInterval        = 0x0A
        }
    }
#endif

#endif // boot protocol
};

uint8_t const * const _desc_auto_config = (uint8_t const*) &_desc_auto_config_struct;

#endif

/*------------------------------------------------------------------*/
/* MACRO TYPEDEF CONSTANT ENUM
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/* VARIABLE DECLARATION
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/* FUNCTION DECLARATION
 *------------------------------------------------------------------*/




#endif
