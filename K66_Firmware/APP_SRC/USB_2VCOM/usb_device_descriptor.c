#include "App.h"


extern composite_device_struct_t   g_composite_device;
//----------------------------------------------------------------------------------------------------------
// Конфигурация для 1-го интерфейса Virtual COM
//----------------------------------------------------------------------------------------------------------

usb_ep_struct_t cdc_vcom1_cic_ep[CIC_ENDP_COUNT] = {
  #if CIC_NOTIF_ELEM_SUPPORT
  {
    CDC_VCOM1_CIC_INTERRUPT_IN_ENDPOINT, USB_INTERRUPT_PIPE, USB_SEND, CDC_VCOM1_CIC_NOTIF_PACKET_SIZE
  }
  #endif
};

usb_ep_struct_t cdc_vcom1_dic_ep[DIC_ENDP_COUNT] =
{
  #if DATA_CLASS_SUPPORT
  {
    CDC_VCOM1_DIC_BULK_IN_ENDPOINT, USB_BULK_PIPE, USB_SEND, CDC_VCOM1_DIC_BULK_IN_PACKET_SIZE
  },
  {
    CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, USB_BULK_PIPE, USB_RECV, DIC_BULK_OUT_ENDP_PACKET_SIZE
  }
  #endif
};

static usb_if_struct_t cdc_vcom1_interfaces[CDC_VCOM1_INTERFACE_COUNT] =
{
  {
    0, // Индекс интерфейса в дескрипторе 
    {
      CIC_ENDP_COUNT,
      cdc_vcom1_cic_ep
    }
  },
  {
    1, // Индекс интерфейса в дескрипторе 
    {
      DIC_ENDP_COUNT,
      cdc_vcom1_dic_ep
    }
  }
};

//static usb_interfaces_struct_t cdc_vcom1_interface_list[USB_DEVICE_CONFIGURATION_COUNT] =
//{
//  {
//    CDC_VCOM1_INTERFACE_COUNT,
//    cdc_vcom1_interfaces
//  },
//};

//----------------------------------------------------------------------------------------------------------
// Конфигурация для 2-го интерфейса Virtual COM
//----------------------------------------------------------------------------------------------------------

usb_ep_struct_t cdc_vcom2_cic_ep[CIC_ENDP_COUNT] = {
  #if CIC_NOTIF_ELEM_SUPPORT
  {
    CDC_VCOM2_CIC_INTERRUPT_IN_ENDPOINT, USB_INTERRUPT_PIPE, USB_SEND, CDC_VCOM2_CIC_NOTIF_PACKET_SIZE
  }
  #endif
};

usb_ep_struct_t cdc_vcom2_dic_ep[DIC_ENDP_COUNT] =
{
  #if DATA_CLASS_SUPPORT
  {
    CDC_VCOM2_DIC_BULK_IN_ENDPOINT, USB_BULK_PIPE, USB_SEND, CDC_VCOM2_DIC_BULK_IN_PACKET_SIZE
  },
  {
    CDC_VCOM2_DIC_BULK_OUT_ENDPOINT, USB_BULK_PIPE, USB_RECV, DIC_BULK_OUT_ENDP_PACKET_SIZE
  }
  #endif
};

static usb_if_struct_t cdc_vcom2_interfaces[CDC_VCOM2_INTERFACE_COUNT] =
{
  {
    2,  // Индекс интерфейса в дескрипторе 
    {
      CIC_ENDP_COUNT,
      cdc_vcom2_cic_ep
    }
  },
  {
    3, // Индекс интерфейса в дескрипторе 
    {
      DIC_ENDP_COUNT,
      cdc_vcom2_dic_ep
    }
  }
};

//static usb_interfaces_struct_t cdc_vcom2_interface_list[USB_DEVICE_CONFIGURATION_COUNT] =
//{
//  {
//    CDC_VCOM2_INTERFACE_COUNT,
//    cdc_vcom2_interfaces
//  },
//};

//----------------------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------------------

static usb_class_struct_t usb_dec_class[USB_CDC_CLASS_MAX] =
{
  {
    USB_CLASS_CDC,
    {
      CDC_VCOM1_INTERFACE_COUNT,
      cdc_vcom1_interfaces
    },
  },
  {
    USB_CLASS_CDC,
    {
      CDC_VCOM2_INTERFACE_COUNT,
      cdc_vcom2_interfaces
    },
  }
};

static usb_composite_info_struct_t usb_composite_info =
{
  2,
  usb_dec_class,
};

uint8_t g_device_descriptor[DEVICE_DESCRIPTOR_SIZE] =
{

  DEVICE_DESCRIPTOR_SIZE,             /* "Device Descriptor Size */
  USB_DEVICE_DESCRIPTOR,              /* "Device" Type of descriptor */
  USB_uint_16_low(BCD_USB_VERSION),
  USB_uint_16_high(BCD_USB_VERSION),  /* BCD USB version  */
  DEVICE_DESC_DEVICE_CLASS,           /* Device Class is indicated in the interface descriptors */
  DEVICE_DESC_DEVICE_SUBCLASS,        /* Device Subclass is indicated in the interface descriptors  */
  DEVICE_DESC_DEVICE_PROTOCOL,        /* Device Protocol  */
  CONTROL_MAX_PACKET_SIZE,            /* Max Packet size */
  0xA2, 0x15,                         /* Vendor ID */
  0x00, 0x09,                         /* Product ID */
  0x02, 0x00,                         /* BCD Device version */
  0x01,                               /* Manufacturer string index */
  0x02,                               /* Product string index */
  0x00,                               /* Serial number string index */
  DEVICE_DESC_NUM_CONFIG_SUPPOTED     /* Number of configurations */
};

uint8_t g_config_descriptor[CONFIG_DESC_SIZE] =
{
  CONFIG_ONLY_DESC_SIZE,                /*  Configuration Descriptor Size - always 9 bytes*/
  USB_CONFIG_DESCRIPTOR,                /* "Configuration" type of descriptor */
  USB_uint_16_low(CONFIG_DESC_SIZE),
  USB_uint_16_high(CONFIG_DESC_SIZE),   /*  Total length of the Configuration descriptor */
  4,                                    // Количество интерфейсов поддерживаемых конфигурацией 2+2 (2 одного cdc и 2 другого cdc)
  0x01,                                 /*  Configuration Value */
  0x00,                                 /*  Configuration Description String Index*/
  (USB_DESC_CFG_ATTRIBUTES_D7_POS) |
  (USBCFG_DEV_SELF_POWER << USB_DESC_CFG_ATTRIBUTES_SELF_POWERED_SHIFT) |
  (USBCFG_DEV_REMOTE_WAKEUP << USB_DESC_CFG_ATTRIBUTES_REMOTE_WAKEUP_SHIFT), /*  Attributes.support RemoteWakeup and self power */
  USB_DEVICE_MAX_POWER,                /*  Current draw from bus */

  // IAD  Interface Association Descriptor
  0x08, // bLength: Interface Descriptor size
  0x0B, // bDescriptorType: IAD
  0x00, // bFirstInterface
  0x02, // bInterfaceCount
  0x02, // bFunctionClass: CDC
  0x02, // bFunctionSubClass
  0x01, // bFunctionProtocol
  0x02, // iFunction


  //***********************************
  // Communication Interface Descriptor
  //***********************************
  0x09,                           // Размер дескриптора 
  0x04,                           // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x00,                           // Индекс интерфейса
  0x00,                           // bAlternateSetting */
  0x01,                           // Количество конечгых точек в интерфейсе 
  0x02,                           // Класс - CDC_VCOM_CIC
  0x02,                           // Субкласс 
  0x00,                           // Протокол
  0x03,                           // Индекс стороки описывающей интерфейс

  /* CDC Class-Specific descriptor */
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x00,                           //
  0x10,                           //
  0x01,                           // USB Class Definitions for CDC spec release number in BCD 

  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x01,
  0x01,                           // D0(if set): device handles call management itself D1(if set): process commands multiplexed over the data interface*/
  0x01,                           //  Indicates multiplexed commands are handled via data interface */
                                  // 
  0x04,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x02,                           // 
  0x06,                           // Device supports request send break, device supports request combination o set_line_coding, set_control_line_state, get_line_coding and the notification serial state */
                                  // 
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x06,                           // 
  0x00,                           // The interface number of the Communications or Data Class interface  
  0x01                            // Interface number of subordinate interface in the Union 


  // Notification Endpoint descriptor
  #if CIC_NOTIF_ELEM_SUPPORT      /*Endpoint descriptor */
  , /* Comma Added if NOTIF ELEM IS TO BE ADDED */
  0x07,                           // Размер дескриптора 
  0x05,                           // Тип дескриптора. END POINT 
  CDC_VCOM1_CIC_INTERRUPT_IN_ENDPOINT | (USB_SEND << 7),
  0x03,                           // Тип конечной точки - INTERRUPT PIPE
  USB_uint_16_low(CDC_VCOM1_CIC_NOTIF_PACKET_SIZE),
  USB_uint_16_high(CDC_VCOM1_CIC_NOTIF_PACKET_SIZE),
  CIC_NOTIF_ENDP_INTERVAL
  #endif

  //***********************************
  // Data Interface Descriptor */
  //***********************************

  #if DATA_CLASS_SUPPORT
  ,                                 
  0x09,                             // Размер дескриптора 
  0x04,                             // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x01,                             // Индекс интерфейса
  0x00,                             // bAlternateSetting */
  0x02,                             // Количесство конечных точек в интерфейсе
  0x0A,                             // DATA Interface Class */
  0x00,                             // Data Interface SubClass Code */
  0x00,                             // Протокол
  0x04,                             // Interface Description String Index*/

  /*Bulk IN Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM1_DIC_BULK_IN_ENDPOINT | (USB_SEND << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(CDC_VCOM1_DIC_BULK_IN_PACKET_SIZE),
  USB_uint_16_high(CDC_VCOM1_DIC_BULK_IN_PACKET_SIZE),
  0x00, /* This value is ignored for Bulk ENDPOINT */

  /*Bulk OUT Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM1_DIC_BULK_OUT_ENDPOINT | (USB_RECV << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(DIC_BULK_OUT_ENDP_PACKET_SIZE),
  USB_uint_16_high(DIC_BULK_OUT_ENDP_PACKET_SIZE),
  0x00, /* This value is ignored for Bulk ENDPOINT */
  #endif

  //******************************************************************************************
  // Объявление второго Virtual COM port
  //******************************************************************************************


  // IAD  Interface Association Descriptor
  0x08, // bLength: Interface Descriptor size
  0x0B, // bDescriptorType: IAD
  0x02, // bFirstInterface
  0x02, // bInterfaceCount
  0x02, // bFunctionClass: CDC
  0x02, // bFunctionSubClass
  0x01, // bFunctionProtocol
  0x02, // iFunction


  //***********************************
  // Communication Interface Descriptor
  //***********************************
  0x09,                           // Размер дескриптора 
  0x04,                           // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x02,                           // Индекс интерфейса
  0x00,                           // bAlternateSetting */
  0x01,                           // Количество конечгых точек в интерфейсе 
  0x02,                           // Класс - CDC_VCOM_CIC
  0x02,                           // Субкласс 
  0x00,                           // Протокол
  0x03,                           // Индекс стороки описывающей интерфейс

  /* CDC Class-Specific descriptor */
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x00,                           //
  0x10,                           //
  0x01,                           // USB Class Definitions for CDC spec release number in BCD 

  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x01,
  0x01,                           // D0(if set): device handles call management itself D1(if set): process commands multiplexed over the data interface*/
  0x01,                           //  Indicates multiplexed commands are handled via data interface */
                                  // 
  0x04,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x02,                           // 
  0x06,                           // Device supports request send break, device supports request combination o set_line_coding, set_control_line_state, get_line_coding and the notification serial state */
                                  // 
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x06,                           // 
  0x02,                           // The interface number of the Communications or Data Class interface  
  0x03                            // Interface number of subordinate interface in the Union 


  #if CIC_NOTIF_ELEM_SUPPORT      /*Endpoint descriptor */
  , /* Comma Added if NOTIF ELEM IS TO BE ADDED */
  0x07,                           // Размер дескриптора 
  0x05,                           // Тип дескриптора. END POINT 
  CDC_VCOM2_CIC_INTERRUPT_IN_ENDPOINT | (USB_SEND << 7),
  0x03,                           // Тип конечной точки - INTERRUPT PIPE
  USB_uint_16_low(CDC_VCOM2_CIC_NOTIF_PACKET_SIZE),
  USB_uint_16_high(CDC_VCOM2_CIC_NOTIF_PACKET_SIZE),
  CIC_NOTIF_ENDP_INTERVAL
  #endif

  //***********************************
  // Data Interface Descriptor */
  //***********************************

  #if DATA_CLASS_SUPPORT
  ,
  0x09,                             // Размер дескриптора 
  0x04,                             // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x03,                             // Индекс интерфейса
  0x00,                             // bAlternateSetting */
  0x02,                             // Количесство конечных точек в интерфейсе
  0x0A,                             // DATA Interface Class */
  0x00,                             // Data Interface SubClass Code */
  0x00,                             // Протокол
  0x04,                             // Interface Description String Index*/

  /*Bulk IN Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM2_DIC_BULK_IN_ENDPOINT | (USB_SEND << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(CDC_VCOM2_DIC_BULK_IN_PACKET_SIZE),
  USB_uint_16_high(CDC_VCOM2_DIC_BULK_IN_PACKET_SIZE),
  0x00, /* This value is ignored for Bulk ENDPOINT */

  /*Bulk OUT Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM2_DIC_BULK_OUT_ENDPOINT | (USB_RECV << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(DIC_BULK_OUT_ENDP_PACKET_SIZE),
  USB_uint_16_high(DIC_BULK_OUT_ENDP_PACKET_SIZE),
  0x00, /* This value is ignored for Bulk ENDPOINT */

  #endif
};

#if HIGH_SPEED
uint8_t g_device_qualifier_descriptor[DEVICE_QUALIFIER_DESCRIPTOR_SIZE] =
{
  DEVICE_QUALIFIER_DESCRIPTOR_SIZE,      /* Device Qualifier Descriptor Size */
  USB_DEVQUAL_DESCRIPTOR,                /* Type of Descriptor */
  USB_uint_16_low(BCD_USB_VERSION),      /*  BCD USB version  */
  USB_uint_16_high(BCD_USB_VERSION),
  DEVICE_DESC_DEVICE_CLASS,              /* bDeviceClass */
  DEVICE_DESC_DEVICE_SUBCLASS,           /* bDeviceSubClass */
  DEVICE_DESC_DEVICE_PROTOCOL,           /* bDeviceProtocol */
  CONTROL_MAX_PACKET_SIZE,               /* bMaxPacketSize0 */
  DEVICE_OTHER_DESC_NUM_CONFIG_SUPPOTED, /* bNumConfigurations */
  0x00                                   /* Reserved : must be zero */
};



uint8_t g_other_speed_config_descriptor[OTHER_SPEED_CONFIG_DESCRIPTOR_SIZE] =
{
  CONFIG_ONLY_DESC_SIZE,                /*  Configuration Descriptor Size - always 9 bytes*/
  USB_CONFIG_DESCRIPTOR,                /* "Configuration" type of descriptor */
  USB_uint_16_low(CONFIG_DESC_SIZE),
  USB_uint_16_high(CONFIG_DESC_SIZE),   /*  Total length of the Configuration descriptor */
  4,                                    // Количество интерфейсов поддерживаемых конфигурацией 2+2 (2 одного cdc и 2 другого cdc)
  0x01,                                 /*  Configuration Value */
  0x00,                                 /*  Configuration Description String Index*/
  (USB_DESC_CFG_ATTRIBUTES_D7_POS) |
  (USBCFG_DEV_SELF_POWER << USB_DESC_CFG_ATTRIBUTES_SELF_POWERED_SHIFT) |
  (USBCFG_DEV_REMOTE_WAKEUP << USB_DESC_CFG_ATTRIBUTES_REMOTE_WAKEUP_SHIFT), /*  Attributes.support RemoteWakeup and self power */
  USB_DEVICE_MAX_POWER,                /*  Current draw from bus */

  // IAD  Interface Association Descriptor
  0x08, // bLength: Interface Descriptor size
  0x0B, // bDescriptorType: IAD
  0x00, // bFirstInterface
  0x02, // bInterfaceCount
  0x02, // bFunctionClass: CDC
  0x02, // bFunctionSubClass
  0x01, // bFunctionProtocol
  0x02, // iFunction


  //***********************************
  // Communication Interface Descriptor
  //***********************************
  0x09,                           // Размер дескриптора 
  0x04,                           // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x00,                           // Индекс интерфейса
  0x00,                           // bAlternateSetting */
  0x01,                           // Количество конечгых точек в интерфейсе 
  0x02,                           // Класс - CDC_VCOM_CIC
  0x02,                           // Субкласс 
  0x00,                           // Протокол
  0x03,                           // Индекс стороки описывающей интерфейс

  /* CDC Class-Specific descriptor */
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x00,                           //
  0x10,                           //
  0x01,                           // USB Class Definitions for CDC spec release number in BCD 

  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x01,
  0x01,                           // D0(if set): device handles call management itself D1(if set): process commands multiplexed over the data interface*/
  0x01,                           //  Indicates multiplexed commands are handled via data interface */
                                  // 
  0x04,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x02,                           // 
  0x06,                           // Device supports request send break, device supports request combination o set_line_coding, set_control_line_state, get_line_coding and the notification serial state */
                                  // 
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x06,                           // 
  0x00,                           // The interface number of the Communications or Data Class interface  
  0x01                            // Interface number of subordinate interface in the Union 


  // Notification Endpoint descriptor
  #if CIC_NOTIF_ELEM_SUPPORT      /*Endpoint descriptor */
  , /* Comma Added if NOTIF ELEM IS TO BE ADDED */
  0x07,                           // Размер дескриптора 
  0x05,                           // Тип дескриптора. END POINT 
  CDC_VCOM1_CIC_INTERRUPT_IN_ENDPOINT | (USB_SEND << 7),
  0x03,                           // Тип конечной точки - INTERRUPT PIPE
  USB_uint_16_low(CDC_VCOM1_CIC_NOTIF_PACKET_SIZE),
  USB_uint_16_high(CDC_VCOM1_CIC_NOTIF_PACKET_SIZE),
  CIC_NOTIF_ENDP_INTERVAL
  #endif

  //***********************************
  // Data Interface Descriptor */
  //***********************************

  #if DATA_CLASS_SUPPORT
  ,                                 
  0x09,                             // Размер дескриптора 
  0x04,                             // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x01,                             // Индекс интерфейса
  0x00,                             // bAlternateSetting */
  0x02,                             // Количесство конечных точек в интерфейсе
  0x0A,                             // DATA Interface Class */
  0x00,                             // Data Interface SubClass Code */
  0x00,                             // Протокол
  0x04,                             // Interface Description String Index*/

  /*Bulk IN Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM1_DIC_BULK_IN_ENDPOINT | (USB_SEND << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(512),
  USB_uint_16_high(512),
  0x00, /* This value is ignored for Bulk ENDPOINT */

  /*Bulk OUT Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM1_DIC_BULK_OUT_ENDPOINT | (USB_RECV << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(512),
  USB_uint_16_high(512),
  0x00, /* This value is ignored for Bulk ENDPOINT */
  #endif

  //******************************************************************************************
  // Объявление второго Virtual COM port
  //******************************************************************************************


  // IAD  Interface Association Descriptor
  0x08, // bLength: Interface Descriptor size
  0x0B, // bDescriptorType: IAD
  0x02, // bFirstInterface
  0x02, // bInterfaceCount
  0x02, // bFunctionClass: CDC
  0x02, // bFunctionSubClass
  0x01, // bFunctionProtocol
  0x02, // iFunction


  //***********************************
  // Communication Interface Descriptor
  //***********************************
  0x09,                           // Размер дескриптора 
  0x04,                           // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x02,                           // Индекс интерфейса
  0x00,                           // bAlternateSetting */
  0x01,                           // Количество конечгых точек в интерфейсе 
  0x02,                           // Класс - CDC_VCOM_CIC
  0x02,                           // Субкласс 
  0x00,                           // Протокол
  0x03,                           // Индекс стороки описывающей интерфейс

  /* CDC Class-Specific descriptor */
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x00,                           //
  0x10,                           //
  0x01,                           // USB Class Definitions for CDC spec release number in BCD 

  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x01,
  0x01,                           // D0(if set): device handles call management itself D1(if set): process commands multiplexed over the data interface*/
  0x01,                           //  Indicates multiplexed commands are handled via data interface */
                                  // 
  0x04,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x02,                           // 
  0x06,                           // Device supports request send break, device supports request combination o set_line_coding, set_control_line_state, get_line_coding and the notification serial state */
                                  // 
  0x05,                           // Размер дескриптора 
  0x24,                           // Тип дескриптора. SC_INTERFACE
  0x06,                           // 
  0x02,                           // The interface number of the Communications or Data Class interface  
  0x03                            // Interface number of subordinate interface in the Union 


  #if CIC_NOTIF_ELEM_SUPPORT      /*Endpoint descriptor */
  , /* Comma Added if NOTIF ELEM IS TO BE ADDED */
  0x07,                           // Размер дескриптора 
  0x05,                           // Тип дескриптора. END POINT 
  CDC_VCOM2_CIC_INTERRUPT_IN_ENDPOINT | (USB_SEND << 7),
  0x03,                           // Тип конечной точки - INTERRUPT PIPE
  USB_uint_16_low(CDC_VCOM2_CIC_NOTIF_PACKET_SIZE),
  USB_uint_16_high(CDC_VCOM2_CIC_NOTIF_PACKET_SIZE),
  CIC_NOTIF_ENDP_INTERVAL
  #endif

  //***********************************
  // Data Interface Descriptor */
  //***********************************

  #if DATA_CLASS_SUPPORT
  ,
  0x09,                             // Размер дескриптора 
  0x04,                             // Тип дескриптора. ДЕСКРИПТОР ИНТЕРФЕЙСА  
  0x03,                             // Индекс интерфейса
  0x00,                             // bAlternateSetting */
  0x02,                             // Количесство конечных точек в интерфейсе
  0x0A,                             // DATA Interface Class */
  0x00,                             // Data Interface SubClass Code */
  0x00,                             // Протокол
  0x04,                             // Interface Description String Index*/

  /*Bulk IN Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM2_DIC_BULK_IN_ENDPOINT | (USB_SEND << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(512),
  USB_uint_16_high(512),
  0x00, /* This value is ignored for Bulk ENDPOINT */

  /*Bulk OUT Endpoint descriptor */
  0x07,                             // Размер дескриптора 
  0x05,                             // Тип дескриптора. END POINT 
  CDC_VCOM2_DIC_BULK_OUT_ENDPOINT | (USB_RECV << 7),
  0x02,                             // Тип конечной точки - BULK PIPE
  USB_uint_16_low(512),
  USB_uint_16_high(512),
  0x00, /* This value is ignored for Bulk ENDPOINT */

  #endif
};

#endif

uint8_t USB_STR_0[USB_STR_0_SIZE + USB_STR_DESC_SIZE] =
{
  sizeof(USB_STR_0),
  USB_STRING_DESCRIPTOR,
  0x09,
  0x04 /*equivalent to 0x0409*/
};
// Строка описания производителя
uint8_t USB_STR_1[USB_STR_1_SIZE + USB_STR_DESC_SIZE] =
{
  sizeof(USB_STR_1),
  USB_STRING_DESCRIPTOR,
  'I', 0,
  'N', 0,
  'D', 0,
  'E', 0,
  'M', 0,
  'S', 0,
  'Y', 0,
  'S', 0
};

uint8_t USB_STR_2[USB_STR_2_SIZE + USB_STR_DESC_SIZE] =
{
  sizeof(USB_STR_2),
  USB_STRING_DESCRIPTOR,
  'M', 0,
  'C', 0,
  'U', 0,
  ' ', 0,
  'K', 0,
  '6', 0,
  '6', 0,
  'B', 0,
  'L', 0,
  'E', 0,
  'Z', 0,
  ' ', 0,
  'v', 0,
  '1', 0,
  '.', 0,
  '0', 0,
};

uint8_t USB_STR_3[USB_STR_3_SIZE + USB_STR_DESC_SIZE] =
{
  sizeof(USB_STR_3),
  USB_STRING_DESCRIPTOR,
  'V', 0,
  'C', 0,
  'O', 0,
  'M', 0,
  '1', 0,
  ' ', 0,
  'C', 0,
  'I', 0,
  'C', 0,
};

uint8_t USB_STR_4[USB_STR_4_SIZE + USB_STR_DESC_SIZE] =
{
  sizeof(USB_STR_4),
  USB_STRING_DESCRIPTOR,
  'V', 0,
  'C', 0,
  'O', 0,
  'M', 0,
  '1', 0,
  ' ', 0,
  'D', 0,
  'I', 0,
  'C', 0,
};

uint8_t USB_STR_n[USB_STR_n_SIZE + USB_STR_DESC_SIZE] =
{
  sizeof(USB_STR_n),
  USB_STRING_DESCRIPTOR,
  'B', 0,
  'A', 0,
  'D', 0,
  ' ', 0,
  'S', 0,
  'T', 0,
  'R', 0,
  'I', 0,
  'N', 0,
  'G', 0
};


#if HIGH_SPEED
uint8_t *g_std_descriptors[USB_MAX_STD_DESCRIPTORS + 1] =
{
  NULL,
  g_device_descriptor,
  g_config_descriptor,
  NULL, /* string */
  NULL, /* Interface */
  NULL, /* Endpoint */
  g_device_qualifier_descriptor,
  g_other_speed_config_descriptor
};
uint8_t g_std_desc_size[USB_MAX_STD_DESCRIPTORS + 1] =
{
  0,
  DEVICE_DESCRIPTOR_SIZE,
  CONFIG_DESC_SIZE,
  0, /* string */
  0, /* Interface */
  0, /* Endpoint */
  DEVICE_QUALIFIER_DESCRIPTOR_SIZE,
  OTHER_SPEED_CONFIG_DESCRIPTOR_SIZE
};


#else
uint8_t *g_std_descriptors[USB_MAX_STD_DESCRIPTORS + 1] =
{
  NULL,
  g_device_descriptor,
  g_config_descriptor,
  NULL, /* string */
  NULL, /* Interface */
  NULL, /* Endpoint */
  NULL, /* Device Qualifier */
  NULL  /* other speed config*/
};

uint8_t g_std_desc_size[USB_MAX_STD_DESCRIPTORS + 1] =
{
  0,
  DEVICE_DESCRIPTOR_SIZE,
  CONFIG_DESC_SIZE,
  0, /* string */
  0, /* Interface */
  0, /* Endpoint */
  0, /* Device Qualifier */
  0 /* other speed config */
};

#endif

uint8_t g_string_desc_size[USB_MAX_STRING_DESCRIPTORS + 1] =
{
  sizeof(USB_STR_0),
  sizeof(USB_STR_1),
  sizeof(USB_STR_2),
  sizeof(USB_STR_3),
  sizeof(USB_STR_4),
  sizeof(USB_STR_n)
};

uint8_t *g_string_descriptors[USB_MAX_STRING_DESCRIPTORS + 1] =
{
  USB_STR_0,
  USB_STR_1,
  USB_STR_2,
  USB_STR_3,
  USB_STR_4,
  USB_STR_n
};

usb_language_t usb_lang[USB_MAX_LANGUAGES_SUPPORTED] =
{
  {
    (uint16_t)0x0409,
    g_string_descriptors,
    g_string_desc_size,
  },
};

usb_all_languages_t g_languages =
{
  USB_STR_0,
  sizeof(USB_STR_0),
  USB_MAX_LANGUAGES_SUPPORTED,
  usb_lang
};

static uint8_t g_valid_config_values[USB_MAX_CONFIG_SUPPORTED + 1] = { 0, 1 };
static uint8_t g_alternate_interface[USB_MAX_SUPPORTED_INTERFACES];


/*
*************************************************************************

  @name  USB_Desc_Get_Descriptor

  @brief The function returns the corresponding descriptor

  @param handle:        handle
  @param type:          type of descriptor requested
  @param str_num:       string index for string descriptor
  @param index:         string descriptor language Id
  @param descriptor:    output descriptor pointer
  @param size:          size of descriptor returned

  @return USB_OK                              When Success
          USBERR_INVALID_REQ_TYPE             when Error
**************************************************************************
*/
uint8_t USB_Desc_Get_Descriptor(uint32_t handle, uint8_t type, uint8_t str_num, uint16_t index, uint8_t **descriptor, uint32_t *size)
{
  uint8_t res = USB_OK;

  USB_Debug_Printf_Get_Descriptor(handle, type, str_num, index);

  /* string descriptors are handled separately */
  if (type == USB_STRING_DESCRIPTOR)
  {
    if (index == 0)
    {
      /* return the string and size of all languages */
      *descriptor = (uint8_t *)g_languages.languages_supported_string;
      *size = g_languages.languages_supported_size;
    }
    else
    {
      uint8_t lang_id = 0;
      uint8_t lang_index = USB_MAX_LANGUAGES_SUPPORTED;

      for (; lang_id < USB_MAX_LANGUAGES_SUPPORTED; lang_id++)
      {
        /* check whether we have a string for this language */
        if (index == g_languages.usb_language[lang_id].language_id)
        { /* check for max descriptors */
          if (str_num < USB_MAX_STRING_DESCRIPTORS)
          { /* setup index for the string to be returned */
            lang_index = str_num;
          }
          break;
        }
      }

      /* set return val for descriptor and size */
      *descriptor = (uint8_t *)g_languages.usb_language[lang_id].lang_desc[str_num];
      *size = g_languages.usb_language[lang_id].lang_desc_size[lang_index];
    }
  }
  else if (type < USB_MAX_STD_DESCRIPTORS + 1)
  {
    // Здесь перенастраиваем размеры буферов конечных точек на максимальный размер в соответствии с текущей скоростью интерфейса
    // Поскольку после события USB_DEV_EVENT_BUS_RESET скорость могла быть определена не корректно, так как в последствии скорость интерфейса
    // изменяется после обрабоки прерывания по флагу EHCI_STS_PORT_CHANGE
    if (type == USB_DEVICE_DESCRIPTOR)
    {
      //USB_Desc_Set_Speed(handle,USB_SPEED_HIGH);
    }

    /* set return val for descriptor and size*/
    *descriptor = (uint8_t *)g_std_descriptors[type];

    /* if there is no descriptor then return error */
    if (*descriptor != NULL)
    {
      *size = g_std_desc_size[type];
      if (*size == 141)
      {
        res = USB_OK;
      }
    }
    else
    {
      res = USBERR_INVALID_REQ_TYPE;
    }
  }
  else /* invalid descriptor */
  {
    res = USBERR_INVALID_REQ_TYPE;
  }
  USB_Debug_Printf_Descriptor(res, (uint32_t)*descriptor, (uint32_t)*size);

  return res;
}

/**************************************************************************//*!
 *
 * @name  USB_Desc_Get_Interface
 *
 * @brief The function returns the alternate interface
 *
 * @param handle:         handle
 * @param interface:      interface number
 * @param alt_interface:  output alternate interface
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Desc_Get_Interface(uint32_t handle, uint8_t interface, uint8_t *alt_interface)
{
  USB_Debug_Printf_Get_Interface(handle, interface);
  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* get alternate interface*/
    *alt_interface = g_alternate_interface[interface];
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Desc_Set_Interface
 *
 * @brief The function sets the alternate interface
 *
 * @param handle:         handle
 * @param interface:      interface number
 * @param alt_interface:  input alternate interface
 *
 * @return USB_OK                              When Success
 *         USBERR_INVALID_REQ_TYPE             when Error
 *****************************************************************************/
uint8_t USB_Desc_Set_Interface(uint32_t handle, uint8_t interface, uint8_t alt_interface)
{
  USB_Debug_Printf_Set_Interface(handle, interface);

  /* if interface valid */
  if (interface < USB_MAX_SUPPORTED_INTERFACES)
  {
    /* set alternate interface*/
    g_alternate_interface[interface] = alt_interface;
    return USB_OK;
  }

  return USBERR_INVALID_REQ_TYPE;
}

/**************************************************************************//*!
 *
 * @name  USB_Desc_Valid_Configation
 *
 * @brief The function checks whether the configuration parameter
 *        input is valid or not
 *
 * @param handle          handle
 * @param config_val      configuration value
 *
 * @return TRUE           When Valid
 *         FALSE          When Error
 *****************************************************************************/
bool USB_Desc_Valid_Configation(uint32_t handle, uint16_t config_val)
{
  uint8_t loop_index = 0;

  USB_Debug_Printf_Valid_Configation(handle, config_val);

  /* check with only supported val right now */
  while (loop_index < (USB_MAX_CONFIG_SUPPORTED + 1))
  {
    if (config_val == g_valid_config_values[loop_index])
    {
      return TRUE;
    }
    loop_index++;
  }
  return FALSE;
}

/**************************************************************************//*!
 *
 * @name  USB_Desc_Remote_Wakeup
 *
 * @brief The function checks whether the remote wakeup is supported or not
 *
 * @param handle:        handle
 *
 * @return USBCFG_DEV_REMOTE_WAKEUP (TRUE) - if remote wakeup supported
 *****************************************************************************/
bool USB_Desc_Remote_Wakeup(uint32_t handle)
{
  USB_Debug_Printf_Remote_Wakeup(handle);

  return USBCFG_DEV_REMOTE_WAKEUP;
}

/**************************************************************************//*!
 *
 * @name  USB_Set_Configation
 *
 * @brief The function checks whether the configuration parameter
 *        input is valid or not
 *
 * @param handle          handle
 * @param config_val      configuration value
 *
 * @return TRUE           When Valid
 *         FALSE          When Error
 *****************************************************************************/
uint8_t USB_Set_Configation(cdc_handle_t handle, uint8_t config)
{
  uint32_t i;

  USB_Debug_Printf_Set_Configation(handle, config);

  for (i = 0; i < usb_composite_info.count; i++)
  {
    switch (usb_composite_info.class_handle[i].type)
    {
    case USB_CLASS_CDC:
      //usb_dec_class[i].interfaces = usb_cdc_configuration[config - 1];
      break;
    default:
      break;
    }
  }
  return USB_OK;
}


/**************************************************************************//*!
 *
 * @name  USB_Desc_Get_Entity
 *
 * @brief The function retrieves the entity specified by type.
 *
 * @param handle            handle
 *
 * @return USB_OK  - if success
 *****************************************************************************/
uint8_t USB_Desc_Get_Entity(cdc_handle_t handle, entity_type etype, uint32_t *object)
{
  USB_Debug_Printf_Get_Entity(handle, etype);

  switch (etype)
  {
  case USB_CLASS_INFO:
    break;
  case USB_CLASS_INTERFACE_INDEX_INFO:
    *object = 0xff;
    for (int i = 0; i < COMPOSITE_CFG_MAX; i++)
    {
      if (handle == (uint32_t)g_composite_device.cdc_vcom1)
      {
        *object = (uint32_t)CDC_VCOM1_INTERFACE_INDEX;
        break;
      }
      else if (handle == (uint32_t)g_composite_device.cdc_vcom2)
      {
        *object = (uint32_t)CDC_VCOM2_INTERFACE_INDEX;
        break;
      }
    }
    break;
  case USB_COMPOSITE_INFO:
    *object = (uint32_t)&usb_composite_info;
    break;
//  case USB_MSC_LBA_INFO:
//    {
//      Disk_USB_App_Class_Callback(USB_MSC_DEVICE_GET_INFO, USB_REQ_VAL_INVALID, NULL, (uint32_t *)&usb_msc_lba_info_struct, NULL);
//      *object = (unsigned long)&usb_msc_lba_info_struct;
//      break;
//    }

  default :
    break;
  }
  return USB_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Установка скорости для класса устройства  
  Вызывается для каждого класса в композитном устройстве отдельно 
 
  Но здесь обновляются данные о размерах буферов всех контрольных точек в дескрипторе устройства
  И обновляются поля в структурах всех классов.
 
  Таким образом вызов функци для каждого класса дублирует действия.  
  Возможно надо исправить, но отрицательных последствий от такого поведения не обнаружено. 
 
 \param handle  - хэндл класса 
 \param speed 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
uint8_t USB_Desc_Set_Speed(uint32_t handle, uint16_t speed)
{
  descriptor_union_t ptr1, ptr2;

  #if DATA_CLASS_SUPPORT
  uint16_t bulk_in = 0;
  uint16_t bulk_out = 0;
  #endif
  #if CIC_NOTIF_ELEM_SUPPORT
  uint16_t interrupt_size = 0;
  uint8_t interrupt_interval = 0;
  #endif


  USB_Debug_Printf_Set_Speed(handle, speed);

  if (USB_SPEED_HIGH == speed)
  {
    #if DATA_CLASS_SUPPORT
    bulk_in = HS_DIC_BULK_IN_ENDP_PACKET_SIZE;
    bulk_out = HS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
    #endif
    #if CIC_NOTIF_ELEM_SUPPORT
    interrupt_size = HS_CIC_NOTIF_ENDP_PACKET_SIZE;
    interrupt_interval = HS_CIC_NOTIF_ENDP_INTERVAL;
    #endif
  }
  else
  {
    #if DATA_CLASS_SUPPORT
    bulk_in = FS_DIC_BULK_IN_ENDP_PACKET_SIZE;
    bulk_out = FS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
    #endif
    #if CIC_NOTIF_ELEM_SUPPORT
    interrupt_size = FS_CIC_NOTIF_ENDP_PACKET_SIZE;
    interrupt_interval = FS_CIC_NOTIF_ENDP_INTERVAL;
    #endif
  }

  ptr1.pntr = g_config_descriptor;                        // Указатель на начало дескриптора устройства
  ptr2.pntr = g_config_descriptor + CONFIG_DESC_SIZE;     // Указатель на конец дескриптора устройства

  // Проходим по всей структуре дескрипторов устройства от начала до конца
  while (ptr1.word < ptr2.word)
  {
    if (ptr1.common->bDescriptorType == USB_DESC_TYPE_EP)
    {
      // Здесь мы нашли дескриптор конечной точки для прерываний

      if ((CDC_VCOM1_CIC_INTERRUPT_IN_ENDPOINT == (ptr1.ndpt->bEndpointAddress & 0x7F)) || (CDC_VCOM2_CIC_INTERRUPT_IN_ENDPOINT == (ptr1.ndpt->bEndpointAddress & 0x7F)))
      {
        // Здесь мы нашли дескриптор конечной точки
        #if CIC_NOTIF_ELEM_SUPPORT
        ptr1.ndpt->wMaxPacketSize[0] = USB_uint_16_low(interrupt_size);
        ptr1.ndpt->wMaxPacketSize[1] = USB_uint_16_high(interrupt_size);
        ptr1.ndpt->iInterval = interrupt_interval;
        #endif
      }
      else
      {
        // Здесь мы нашли все остальные дескрипторы конечных точеки
        #if DATA_CLASS_SUPPORT
        if (ptr1.ndpt->bEndpointAddress & 0x80)
        {
          // Здесь конечные точки типа IN
          ptr1.ndpt->wMaxPacketSize[0] = USB_uint_16_low(bulk_in);
          ptr1.ndpt->wMaxPacketSize[1] = USB_uint_16_high(bulk_in);
        }
        else
        {
          // Здесь конечные точки типа OUT
          ptr1.ndpt->wMaxPacketSize[0] = USB_uint_16_low(bulk_out);
          ptr1.ndpt->wMaxPacketSize[1] = USB_uint_16_high(bulk_out);
        }
        #endif
      }
    }
    ptr1.word += ptr1.common->bLength;
  }

  Vcom1_change_ep_sz(bulk_in, bulk_out, interrupt_size);
  Vcom2_change_ep_sz(bulk_in, bulk_out, interrupt_size);

  return USB_OK;
}

usb_desc_request_notify_struct_t desc_callback =
{
  USB_Desc_Get_Descriptor,
  USB_Desc_Get_Interface,
  USB_Desc_Set_Interface,
  USB_Set_Configation,
  USB_Desc_Get_Entity
};

/* EOF */
