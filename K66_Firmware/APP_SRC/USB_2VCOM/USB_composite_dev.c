// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-05
// 10:07:44
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"



composite_device_struct_t                 g_composite_device;

extern usb_desc_request_notify_struct_t   desc_callback;




/*-----------------------------------------------------------------------------------------------------
 
 \param p_vcom_cbl 
-----------------------------------------------------------------------------------------------------*/
static void Vitrual_com_init(uint32_t indx, T_usb_cdc_vcom_struct *p_vcom_cbl)
{
  uint32_t i;
  p_vcom_cbl->port_indx = indx;

  p_vcom_cbl->configured = FALSE;

  p_vcom_cbl->out_pack_sz  = FS_DIC_BULK_OUT_ENDP_PACKET_SIZE;
  p_vcom_cbl->in_pack_sz   = FS_DIC_BULK_IN_ENDP_PACKET_SIZE;
  p_vcom_cbl->device_speed = USB_SPEED_FULL; //USB_SPEED_HIGH;


  p_vcom_cbl->tx_enabled = FALSE;
  p_vcom_cbl->rx_buf = OS_Mem_alloc_uncached_align(DATA_BUFF_SIZE * IN_BUF_QUANTITY, 32);
  p_vcom_cbl->rx_head = 0;
  p_vcom_cbl->rx_tail = p_vcom_cbl->rx_head;
  p_vcom_cbl->rx_full = 0;
  for (i = 0; i < IN_BUF_QUANTITY; i++)
  {
    // Подготавдиваем массив управляющих структур для приема пакетов
    p_vcom_cbl->rx_pack_cbl[i].buff = &(p_vcom_cbl->rx_buf[DATA_BUFF_SIZE * i]);
    p_vcom_cbl->rx_pack_cbl[i].len = 0;
    p_vcom_cbl->rx_pack_cbl[i].pos = 0;
  }
  _lwevent_create(&(p_vcom_cbl->os_flags), LWEVENT_AUTO_CLEAR); // Все события автоматически сбрасываемые

}

/*------------------------------------------------------------------------------



 \param initial_data
 ------------------------------------------------------------------------------*/
uint32_t Composite_USB_device_init(void)
{
  usb_status             status;
  class_config_struct_t *p_class_callbacks;


  Vitrual_com_init(0, Vcom1_USB_get_cbl());
  Vitrual_com_init(1, Vcom2_USB_get_cbl());

  p_class_callbacks = &g_composite_device.composite_device_config_list[CDC_VCOM1_INTERFACE_INDEX];

  p_class_callbacks->composite_application_callback.callback = VCom1_USB_App_Device_Callback;
  p_class_callbacks->composite_application_callback.arg = &g_composite_device.cdc_vcom1;
  p_class_callbacks->class_specific_callback.callback = (usb_class_specific_handler_func) VCom1_USB_App_Class_Callback;
  p_class_callbacks->class_specific_callback.arg = &g_composite_device.cdc_vcom1;
  p_class_callbacks->desc_callback_ptr = &desc_callback;
  p_class_callbacks->type = USB_CLASS_CDC;

  p_class_callbacks = &g_composite_device.composite_device_config_list[CDC_VCOM2_INTERFACE_INDEX];

  p_class_callbacks->composite_application_callback.callback = VCom2_USB_App_Device_Callback;
  p_class_callbacks->composite_application_callback.arg = &g_composite_device.cdc_vcom2;
  p_class_callbacks->class_specific_callback.callback = (usb_class_specific_handler_func) VCom2_USB_App_Class_Callback;
  p_class_callbacks->class_specific_callback.arg = &g_composite_device.cdc_vcom2;
  p_class_callbacks->desc_callback_ptr = &desc_callback;
  p_class_callbacks->type = USB_CLASS_CDC;

  g_composite_device.composite_device_config_callback.count = COMPOSITE_CFG_MAX;
  g_composite_device.composite_device_config_callback.class_app_callback = g_composite_device.composite_device_config_list;

  // Переносим во внутренности стека указатель на composite_device_config_list и возвращаем себе обратно composite_device с типом composite_handle_t 
  status = USB_Composite_Init(CONTROLLER_ID, &g_composite_device.composite_device_config_callback, &g_composite_device.composite_device);

  // В перемееной g_composite_device сохраняем хэндлеры классов полученные в функции USB_Composite_Init

  g_composite_device.cdc_vcom1 = (cdc_handle_t) g_composite_device.composite_device_config_list[CDC_VCOM1_INTERFACE_INDEX].class_handle;
  g_composite_device.cdc_vcom2 = (cdc_handle_t) g_composite_device.composite_device_config_list[CDC_VCOM2_INTERFACE_INDEX].class_handle;

  Vcom1_USB_get_cbl()->class_handle = g_composite_device.cdc_vcom1;
  Vcom2_USB_get_cbl()->class_handle = g_composite_device.cdc_vcom2;

  USB_Debug_Printf_Composite_Init(status, &g_composite_device);
  return  status;

}


