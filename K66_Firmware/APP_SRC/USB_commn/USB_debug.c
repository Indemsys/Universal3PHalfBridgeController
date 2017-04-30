// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-06
// 11:15:17
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param str_num 
 \param index 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Get_Descriptor(uint32_t handle, uint8_t type, uint8_t str_num, uint16_t index)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Desc_Get_Descriptor (handle= %08X): ", handle);
  switch (type)
  {
  case USB_DEVICE_DESCRIPTOR      :
    USB_PRINTF("USB_DEVICE_DESCRIPTOR     "); break;
  case USB_CONFIG_DESCRIPTOR      :
    USB_PRINTF("USB_CONFIG_DESCRIPTOR     "); break;
  case USB_STRING_DESCRIPTOR      :
    USB_PRINTF("USB_STRING_DESCRIPTOR     "); break;
  case USB_DESCRIPTOR_TYPE_INTERFACE       :
    USB_PRINTF("USB_IFACE_DESCRIPTOR      "); break;
  case USB_ENDPOINT_DESCRIPTOR    :
    USB_PRINTF("USB_ENDPOINT_DESCRIPTOR   "); break;
  case USB_DEVQUAL_DESCRIPTOR     :
    USB_PRINTF("USB_DEVQUAL_DESCRIPTOR    "); break;
  case USB_OTHER_SPEED_DESCRIPTOR :
    USB_PRINTF("USB_OTHER_SPEED_DESCRIPTOR"); break;
  default:
    USB_PRINTF("undefined descriptor  %d", type); break;
    break;
  }
  USB_PRINTF(" . str_num=%d, index=%d\r\n", str_num, index);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param descriptor 
 \param sz 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Descriptor(uint8_t res, uint32_t descriptor, uint32_t sz)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("...................................... Result=%d, Descriptror = %08X , size = %d\r\n", res, descriptor, sz);
  #endif
}
/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Get_Interface(uint32_t handle, uint8_t interface)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Desc_Get_Interface (%08X, %d)\r\n", handle, interface);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Set_Interface(uint32_t handle, uint8_t interface)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Desc_Set_Interface (%08X, %d)\r\n", handle, interface);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param config_val 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Valid_Configation(uint32_t handle, uint16_t config_val)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Desc_Valid_Configation (%08X, %d)\r\n", handle, config_val);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Remote_Wakeup(uint32_t handle)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Desc_Remote_Wakeup (%08X)\r\n", handle);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param config 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Set_Configation(cdc_handle_t handle, uint8_t config)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Set_Configation (%08X, %d)\r\n", handle, config);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param etype 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Get_Entity(cdc_handle_t handle, entity_type etype)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("USB_Desc_Get_Entity (%08X, %d)\r\n", handle, etype);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param speed 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Set_Speed(uint32_t handle, uint16_t speed)
{
  #ifdef USB_DEBUG_PRINT
  if (speed == USB_SPEED_HIGH)
  {
    USB_PRINTF("USB_Desc_Set_Speed handle=%08X, speed= HIGH\r\n", handle);
  }
  else if (speed == USB_SPEED_FULL)
  {
    USB_PRINTF("USB_Desc_Set_Speed handle=%08X, speed= FULL\r\n", handle);
  }
  else if (speed == USB_SPEED_LOW)
  {
    USB_PRINTF("USB_Desc_Set_Speed handle=%08X, speed= LOW\r\n", handle);
  }
  else
  {
    USB_PRINTF("USB_Desc_Set_Speed handle=%08X, speed= undefined(%d) \r\n", handle, speed);
  }
  #endif
}
#ifdef USB_2VCOM
/*-----------------------------------------------------------------------------------------------------
 
 \param status 
 \param p_g_composite_device 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Composite_Init(usb_status status, composite_device_struct_t *p_g_composite_device)
{
  #ifdef USB_DEBUG_PRINT

  cdc_handle_t hndl;
  uint32_t i;
  uint32_t n;

  USB_PRINTF("USB_Composite_Init result =  %d\r\n", status);
  USB_PRINTF("Configuration descriptor size = %d\r\n", CONFIG_DESC_SIZE);

  n = p_g_composite_device->composite_device_config_callback.count;
  if (n > 0)
  {
    USB_PRINTF("Composite dev config count = %d\r\n", n);
    for (i = 0; i < n; i++)
    {
      hndl = p_g_composite_device->composite_device_config_list[i].class_handle;
      USB_PRINTF("  Config %d: Handle = %08X\r\n", i, hndl);
    }
  }
  else
  {
    USB_PRINTF("Error! Composite dev config count = 0\r\n");
  }

  #endif
}
#endif
/*-----------------------------------------------------------------------------------------------------
 
 \param event_type 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Device_Callback(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint8_t event_type)
{
  #ifdef USB_DEBUG_PRINT

  USB_PRINTF("Port %d.  USB_App_Device_Callback: ", p_vcom1_cbl->port_indx);
  switch (event_type)
  {
  case USB_DEV_EVENT_BUS_RESET:
    USB_PRINTF("USB_DEV_EVENT_BUS_RESET             "); break;

  case USB_DEV_EVENT_CONFIG_CHANGED:
    USB_PRINTF("USB_DEV_EVENT_CONFIG_CHANGED        "); break;

  case USB_DEV_EVENT_INTERFACE_CHANGED        :
    USB_PRINTF("USB_DEV_EVENT_INTERFACE_CHANGED     "); break;

  case USB_DEV_EVENT_ENUM_COMPLETE            :
    USB_PRINTF("USB_DEV_EVENT_ENUM_COMPLETE         "); break;

  case USB_DEV_EVENT_SEND_COMPLETE            :
    USB_PRINTF("USB_DEV_EVENT_SEND_COMPLETE         "); break;

  case USB_DEV_EVENT_DATA_RECEIVED            :
    USB_PRINTF("USB_DEV_EVENT_DATA_RECEIVED         "); break;

  case USB_DEV_EVENT_ERROR                    :
    USB_PRINTF("USB_DEV_EVENT_ERROR                 "); break;

  case USB_DEV_EVENT_GET_DATA_BUFF            :
    USB_PRINTF("USB_DEV_EVENT_GET_DATA_BUFF         "); break;

  case USB_DEV_EVENT_EP_STALLED               :
    USB_PRINTF("USB_DEV_EVENT_EP_STALLED            "); break;

  case USB_DEV_EVENT_EP_UNSTALLED             :
    USB_PRINTF("USB_DEV_EVENT_EP_UNSTALLED          "); break;

  case USB_DEV_EVENT_GET_TRANSFER_SIZE        :
    USB_PRINTF("USB_DEV_EVENT_GET_TRANSFER_SIZE     "); break;

  case USB_DEV_EVENT_TYPE_SET_REMOTE_WAKEUP   :
    USB_PRINTF("USB_DEV_EVENT_TYPE_SET_REMOTE_WAKEUP"); break;

  case USB_DEV_EVENT_TYPE_CLR_REMOTE_WAKEUP   :
    USB_PRINTF("USB_DEV_EVENT_TYPE_CLR_REMOTE_WAKEUP"); break;

  case USB_DEV_EVENT_TYPE_SET_EP_HALT         :
    USB_PRINTF("USB_DEV_EVENT_TYPE_SET_EP_HALT      "); break;

  case USB_DEV_EVENT_TYPE_CLR_EP_HALT         :
    USB_PRINTF("USB_DEV_EVENT_TYPE_CLR_EP_HALT      "); break;

  case USB_DEV_EVENT_DETACH                   :
    USB_PRINTF("USB_DEV_EVENT_DETACH                "); break;

  default:
    USB_PRINTF("undefined event %d", event_type);
    break;
  }
  USB_PRINTF("\r\n");
  #endif

}

/*-----------------------------------------------------------------------------------------------------
 
 \param event 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Class_Callback(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint8_t event, uint32_t *size)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("Port %d.  USB_App_Class_Callback: ", p_vcom1_cbl->port_indx);
  switch (event)
  {
  case SEND_ENCAPSULATED_COMMAND       :
    USB_PRINTF("SEND_ENCAPSULATED_COMMAND      "); break;
  case GET_ENCAPSULATED_RESPONSE       :
    USB_PRINTF("GET_ENCAPSULATED_RESPONSE      "); break;
  case SET_COMM_FEATURE                :
    USB_PRINTF("SET_COMM_FEATURE               "); break;
  case GET_COMM_FEATURE                :
    USB_PRINTF("GET_COMM_FEATURE               "); break;
//  case CLEAR_COMM_FEATURE              :
//    USB_PRINTF("CLEAR_COMM_FEATURE             "); break;
  case SET_AUX_LINE_STATE              :
    USB_PRINTF("SET_AUX_LINE_STATE             "); break;
  case SET_HOOK_STATE                  :
    USB_PRINTF("SET_HOOK_STATE                 "); break;
  case PULSE_SETUP                     :
    USB_PRINTF("PULSE_SETUP                    "); break;
  case SEND_PULSE                      :
    USB_PRINTF("SEND_PULSE                     "); break;
  case SET_PULSE_TIME                  :
    USB_PRINTF("SET_PULSE_TIME                 "); break;
  case RING_AUX_JACK                   :
    USB_PRINTF("RING_AUX_JACK                  "); break;
  case SET_LINE_CODING                 :
    USB_PRINTF("SET_LINE_CODING                "); break;
  case GET_LINE_CODING                 :
    USB_PRINTF("GET_LINE_CODING                "); break;
  case SET_CONTROL_LINE_STATE          :
    USB_PRINTF("SET_CONTROL_LINE_STATE         "); break;
  case SEND_BREAK                      :
    USB_PRINTF("SEND_BREAK                     "); break;
  case SET_RINGER_PARAMS               :
    USB_PRINTF("SET_RINGER_PARAMS              "); break;
  case GET_RINGER_PARAMS               :
    USB_PRINTF("GET_RINGER_PARAMS              "); break;
  case SET_OPERATION_PARAM             :
    USB_PRINTF("SET_OPERATION_PARAM            "); break;
  case GET_OPERATION_PARAM             :
    USB_PRINTF("GET_OPERATION_PARAM            "); break;
  case SET_LINE_PARAMS                 :
    USB_PRINTF("SET_LINE_PARAMS                "); break;
  case GET_LINE_PARAMS                 :
    USB_PRINTF("GET_LINE_PARAMS                "); break;
  case DIAL_DIGITS                     :
    USB_PRINTF("DIAL_DIGITS                    "); break;
  case SET_UNIT_PARAMETER              :
    USB_PRINTF("SET_UNIT_PARAMETER             "); break;
  case GET_UNIT_PARAMETER              :
    USB_PRINTF("GET_UNIT_PARAMETER             "); break;
  case CLEAR_UNIT_PARAMETER            :
    USB_PRINTF("CLEAR_UNIT_PARAMETER           "); break;
  case GET_PROFILE                     :
    USB_PRINTF("GET_PROFILE                    "); break;
  case SET_ETHERNET_MULTICAST_FILTERS  :
    USB_PRINTF("SET_ETHERNET_MULTICAST_FILTERS "); break;
  case SET_ETHERNET_POW_PATTER_FILTER  :
    USB_PRINTF("SET_ETHERNET_POW_PATTER_FILTER "); break;
  case GET_ETHERNET_POW_PATTER_FILTER  :
    USB_PRINTF("GET_ETHERNET_POW_PATTER_FILTER "); break;
  case SET_ETHERNET_PACKET_FILTER      :
    USB_PRINTF("SET_ETHERNET_PACKET_FILTER     "); break;
  case GET_ETHERNET_STATISTIC          :
    USB_PRINTF("GET_ETHERNET_STATISTIC         "); break;
  case SET_ATM_DATA_FORMAT             :
    USB_PRINTF("SET_ATM_DATA_FORMAT            "); break;
  case GET_ATM_DEVICE_STATISTICS       :
    USB_PRINTF("GET_ATM_DEVICE_STATISTICS      "); break;
  case SET_ATM_DEFAULT_VC              :
    USB_PRINTF("SET_ATM_DEFAULT_VC             "); break;
  case GET_ATM_VC_STATISTICS           :
    USB_PRINTF("GET_ATM_VC_STATISTICS          "); break;
  case MDLM_SPECIFIC_REQUESTS_MASK     :
    USB_PRINTF("MDLM_SPECIFIC_REQUESTS_MASK    "); break;
  case GET_COUNTRY_SETTING             :
    USB_PRINTF("GET_COUNTRY_SETTING            "); break;
  case SET_ABSTRACT_STATE              :
    USB_PRINTF("SET_ABSTRACT_STATE             "); break;
  case SET_COUNTRY_SETTING             :
    USB_PRINTF("SET_COUNTRY_SETTING            "); break;
  case USB_APP_CDC_CARRIER_DEACTIVATED :
    USB_PRINTF("USB_APP_CDC_CARRIER_DEACTIVATED"); break;
  case USB_APP_CDC_CARRIER_ACTIVATED   :
    USB_PRINTF("USB_APP_CDC_CARRIER_ACTIVATED  "); break;
  case USB_APP_CDC_DTE_DEACTIVATED     :
    USB_PRINTF("USB_APP_CDC_DTE_DEACTIVATED    "); break;
  case USB_APP_CDC_DTE_ACTIVATED       :
    USB_PRINTF("USB_APP_CDC_DTE_ACTIVATED      "); break;
  case USB_APP_GET_LINK_SPEED          :
    USB_PRINTF("USB_APP_GET_LINK_SPEED         "); break;
  case USB_APP_GET_LINK_STATUS         :
    USB_PRINTF("USB_APP_GET_LINK_STATUS        "); break;
  case USB_APP_CDC_SERIAL_STATE_NOTIF  :
    USB_PRINTF("USB_APP_CDC_SERIAL_STATE_NOTIF "); break;
  case USB_DEV_EVENT_DATA_RECEIVED:
    USB_PRINTF("USB_DEV_EVENT_DATA_RECEIVED"); 
    if (size!=NULL)
    {
      USB_PRINTF("(%d)",*size); 
    }
    break;
  case USB_DEV_EVENT_SEND_COMPLETE:
    USB_PRINTF("USB_DEV_EVENT_SEND_COMPLETE    "); 
    if (size!=NULL)
    {
      USB_PRINTF("(%d)",*size); 
    }
    break;

  default:
    USB_PRINTF("undefined event %d", event);
    break;
  }
  USB_PRINTF("\r\n");
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Get_Line_Coding(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface, uint8_t **coding_data)
{
  #ifdef USB_DEBUG_PRINT
  uint8_t *cd;

  USB_PRINTF("Port %d.  USB_Get_Line_Coding (%08X, Intf=%d) ", p_vcom1_cbl->port_indx, handle, interface);
  cd = *coding_data;
  USB_PRINTF("Speed=%d ", (cd[3]<<24) | (cd[2]<<16) |(cd[1]<<8) | (cd[0]<<0) );
  USB_PRINTF("fmt=%d ", cd[4] );
  USB_PRINTF("parity=%d ", cd[5] );
  USB_PRINTF("bits=%d ", cd[6] );
  USB_PRINTF("\r\n");
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Set_Line_Coding(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface, uint8_t *coding_data)
{
  #ifdef USB_DEBUG_PRINT
  uint8_t *cd;

  USB_PRINTF("Port %d.  USB_Set_Line_Coding (%08X, Intf=%d) ", p_vcom1_cbl->port_indx, handle, interface);
  cd = coding_data;
  USB_PRINTF("Speed=%d ", (cd[3]<<24) | (cd[2]<<16) |(cd[1]<<8) | (cd[0]<<0) );
  USB_PRINTF("fmt=%d ", cd[4] );
  USB_PRINTF("parity=%d ", cd[5] );
  USB_PRINTF("bits=%d ", cd[6] );
  USB_PRINTF("\r\n");
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Get_Abstract_State(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("Port %d.  USB_Get_Abstract_State (%08X, Intf=%d)\r\n", p_vcom1_cbl->port_indx, handle, interface);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Get_Country_Setting(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("Port %d.  USB_Get_Country_Setting (%08X, Intf=%d)\r\n", p_vcom1_cbl->port_indx, handle, interface);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Set_Abstract_State(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("Port %d.  USB_Set_Abstract_State (%08X, Intf=%d)\r\n", p_vcom1_cbl->port_indx, handle, interface);
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
-----------------------------------------------------------------------------------------------------*/
void USB_Debug_Printf_Set_Country_Setting(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface)
{
  #ifdef USB_DEBUG_PRINT
  USB_PRINTF("Port %d.  USB_Set_Country_Setting (%08X, Intf=%d)\r\n", p_vcom1_cbl->port_indx, handle, interface);
  #endif
}
