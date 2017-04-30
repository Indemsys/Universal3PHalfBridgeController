#ifndef USB_DEBUG_H
#define USB_DEBUG_H

//#define   USB_DEBUG_PRINT // –азрешаем отладочный вывод в процедурах USB

//#define _USB_DEBUG      Ётот макрос разрешает отладочный вывод из более низких уровней стека USB, !!! ћакрос объ€вл€етс€ в диалоге настроек IDE проекта

void USB_Debug_Printf_Get_Descriptor(uint32_t handle, uint8_t type, uint8_t str_num, uint16_t index);
void USB_Debug_Printf_Descriptor(uint8_t res, uint32_t descriptor, uint32_t sz);
void USB_Debug_Printf_Get_Interface(uint32_t handle, uint8_t interface);
void USB_Debug_Printf_Set_Interface(uint32_t handle, uint8_t interface);
void USB_Debug_Printf_Valid_Configation(uint32_t handle, uint16_t config_val);
void USB_Debug_Printf_Remote_Wakeup(uint32_t handle);
void USB_Debug_Printf_Set_Configation(cdc_handle_t handle, uint8_t config);
void USB_Debug_Printf_Get_Entity(cdc_handle_t handle, entity_type etype);
void USB_Debug_Printf_Set_Speed(uint32_t handle, uint16_t speed);

#ifdef USB_2VCOM
void USB_Debug_Printf_Composite_Init(usb_status status, composite_device_struct_t *p_g_composite_device);
#endif

void USB_Debug_Printf_Device_Callback(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint8_t event_type);
void USB_Debug_Printf_Class_Callback(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint8_t event, uint32_t *size);
void USB_Debug_Printf_Get_Line_Coding(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface, uint8_t **coding_data);
void USB_Debug_Printf_Set_Line_Coding(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface, uint8_t *coding_data);
void USB_Debug_Printf_Get_Abstract_State(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface);
void USB_Debug_Printf_Set_Abstract_State(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface);
void USB_Debug_Printf_Get_Country_Setting(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface);
void USB_Debug_Printf_Set_Country_Setting(T_usb_cdc_vcom_struct *p_vcom1_cbl, uint32_t handle, uint8_t interface);

#endif // USB_DEBUG_H



