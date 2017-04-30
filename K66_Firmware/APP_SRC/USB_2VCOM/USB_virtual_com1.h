#ifndef USB_VIRTUAL_COM1_H
#define USB_VIRTUAL_COM1_H


void                    Vcom1_change_ep_sz(uint16_t bulk_in, uint16_t bulk_out, uint16_t interrupt_size);
void                    VCom1_USB_App_Device_Callback(uint8_t event_type, void *val, void *arg);
uint8_t                 VCom1_USB_App_Class_Callback(uint8_t event, uint16_t value, uint8_t **data, uint32_t *size, void *arg);
uint32_t                VCom1_USB_wait_sending_end(uint32_t ticks);
uint32_t                VCom1_USB_send_data(uint8_t *buff, uint32_t size, uint32_t ticks);
uint32_t                VCom1_USB_get_data(uint8_t *b, uint32_t ticks);
T_usb_cdc_vcom_struct*  Vcom1_USB_get_cbl(void);


#endif


/* EOF */
