#include "App.h"


extern usb_ep_struct_t cdc_vcom1_cic_ep[CIC_ENDP_COUNT];
extern usb_ep_struct_t cdc_vcom1_dic_ep[DIC_ENDP_COUNT];


static uint8_t g_line_coding[LINE_CODING_SIZE] =
{
  /*e.g. 0x00,0x10,0x0E,0x00 : 0x000E1000 is 921600 bits per second */
  (LINE_CODE_DTERATE_IFACE >> 0) & 0x000000FF,
  (LINE_CODE_DTERATE_IFACE >> 8) & 0x000000FF,
  (LINE_CODE_DTERATE_IFACE >> 16) & 0x000000FF,
  (LINE_CODE_DTERATE_IFACE >> 24) & 0x000000FF,
  LINE_CODE_CHARFORMAT_IFACE,
  LINE_CODE_PARITYTYPE_IFACE,
  LINE_CODE_DATABITS_IFACE
};

static uint8_t g_abstract_state[COMM_FEATURE_DATA_SIZE] =
{
  (STATUS_ABSTRACT_STATE_IFACE >> 0) & 0x00FF,
  (STATUS_ABSTRACT_STATE_IFACE >> 8) & 0x00FF
};

static uint8_t g_country_code[COMM_FEATURE_DATA_SIZE] =
{
  (COUNTRY_SETTING_IFACE >> 0) & 0x00FF,
  (COUNTRY_SETTING_IFACE >> 8) & 0x00FF
};


T_usb_cdc_vcom_struct      vcom1_cbl;


/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
 \param coding_data 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Vcom_Get_Line_Coding(uint32_t handle, uint8_t interface, uint8_t **coding_data)
{
  /* get line coding data*/
  *coding_data = g_line_coding;

  USB_Debug_Printf_Get_Line_Coding(&vcom1_cbl, handle, interface, coding_data);
  return USB_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
 \param coding_data 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Vcom_Set_Line_Coding(uint32_t handle, uint8_t interface, uint8_t **coding_data)
{
  uint8_t count;
  /* set line coding data*/
  for (count = 0; count < LINE_CODING_SIZE; count++)
  {
    g_line_coding[count] = *((*coding_data + USB_SETUP_PKT_SIZE) + count);
  }
  USB_Debug_Printf_Set_Line_Coding(&vcom1_cbl, handle, interface, (uint8_t*)g_line_coding);
  return USB_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
 \param feature_data 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Vcom_Get_Abstract_State(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
  USB_Debug_Printf_Get_Abstract_State(&vcom1_cbl, handle, interface);

  /* get line coding data*/
  *feature_data = g_abstract_state;
  return USB_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
 \param feature_data 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Vcom_Get_Country_Setting(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
  USB_Debug_Printf_Get_Country_Setting(&vcom1_cbl, handle, interface);

  /* get line coding data*/
  *feature_data = g_country_code;
  return USB_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
 \param feature_data 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Vcom_Set_Abstract_State(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
  uint8_t count;

  USB_Debug_Printf_Set_Abstract_State(&vcom1_cbl, handle, interface);

  /* set Abstract State Feature*/
  for (count = 0; count < COMM_FEATURE_DATA_SIZE; count++)
  {
    g_abstract_state[count] = *(*feature_data + count);
  }
  return USB_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param handle 
 \param interface 
 \param feature_data 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Vcom_Set_Country_Setting(uint32_t handle, uint8_t interface, uint8_t **feature_data)
{
  uint8_t count;

  USB_Debug_Printf_Set_Country_Setting(&vcom1_cbl, handle, interface);

  for (count = 0; count < COMM_FEATURE_DATA_SIZE; count++)
  {
    g_country_code[count] = *(*feature_data + count);
  }
  return USB_OK;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param bulk_in 
 \param bulk_out 
 \param interrupt_size 
-----------------------------------------------------------------------------------------------------*/
void Vcom1_change_ep_sz(uint16_t bulk_in, uint16_t bulk_out, uint16_t interrupt_size)
{

  #if CIC_NOTIF_ELEM_SUPPORT
  cdc_vcom1_cic_ep[0].size = interrupt_size;
  #endif

  #if DATA_CLASS_SUPPORT
  for (int i = 0; i < DIC_ENDP_COUNT; i++)
  {
    if (USB_SEND == cdc_vcom1_dic_ep[i].direction)
    {
      cdc_vcom1_dic_ep[i].size = bulk_in;
      vcom1_cbl.in_pack_sz = bulk_in;
    }
    else
    {
      cdc_vcom1_dic_ep[i].size = bulk_out;
      vcom1_cbl.out_pack_sz = bulk_out;
    }
  }
  #endif
}

/*-----------------------------------------------------------------------------------------------------
 
 Функция вызывается в контексте задачи с именем USB_DEV (функцимя _usb_dev_task_stun -> _usb_dev_task)
 
 \param event_type 
 \param val 
 \param arg 
-----------------------------------------------------------------------------------------------------*/
void VCom1_USB_App_Device_Callback(uint8_t event_type, void *val, void *arg)
{
  uint32_t handle;


  USB_Debug_Printf_Device_Callback(&vcom1_cbl, event_type);

  handle = *((uint32_t *)arg);
  if (event_type == USB_DEV_EVENT_BUS_RESET)
  {
    vcom1_cbl.configured = FALSE;

    if (USB_OK == USB_Class_CDC_Get_Speed(handle, &vcom1_cbl.device_speed))
    {
      USB_Desc_Set_Speed(handle, vcom1_cbl.device_speed);
    }
  }
  else if (event_type == USB_DEV_EVENT_CONFIG_CHANGED)
  {
    // Указаваем конечной  точке DIC_BULK_OUT_ENDPOINT начать принимать  данные
    USB_Class_CDC_Recv_Data(handle, CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, vcom1_cbl.out_pack_sz);

    vcom1_cbl.configured = TRUE;
  }
  else if (event_type == USB_DEV_EVENT_ERROR)
  {
    /* add user code for error handling */
  }
  else if (event_type == USB_DEV_EVENT_ENUM_COMPLETE)
  {
    // Приходит после события USB_DEV_EVENT_CONFIG_CHANGED
  }
  return;
}

/*-----------------------------------------------------------------------------------------------------
 
  Функция вызывается в контексте задачи с именем USB_DEV (функцимя _usb_dev_task_stun -> _usb_dev_task)
 
 \param event 
 \param value 
 \param data 
 \param size 
 \param arg 
 
 \return uint8_t 
-----------------------------------------------------------------------------------------------------*/
uint8_t VCom1_USB_App_Class_Callback(uint8_t event, uint16_t value, uint8_t **data, uint32_t *size, void *arg)
{
  cdc_handle_t handle;
  uint8_t error = USB_OK;
  int32_t res;

  USB_Debug_Printf_Class_Callback(&vcom1_cbl, event, size);

  handle = *((cdc_handle_t *)arg);
  switch (event)
  {
  case GET_LINE_CODING:
    error = _Vcom_Get_Line_Coding(handle, value, data);
    break;
  case GET_ABSTRACT_STATE:
    error = _Vcom_Get_Abstract_State(handle, value, data);
    break;
  case GET_COUNTRY_SETTING:
    error = _Vcom_Get_Country_Setting(handle, value, data);
    break;
  case SET_LINE_CODING:
    error = _Vcom_Set_Line_Coding(handle, value, data);
    break;
  case SET_ABSTRACT_STATE:
    error = _Vcom_Set_Abstract_State(handle, value, data);
    break;
  case SET_COUNTRY_SETTING:
    error = _Vcom_Set_Country_Setting(handle, value, data);
    break;
  case USB_APP_CDC_DTE_ACTIVATED:
    if (vcom1_cbl.configured == TRUE)
    {
      // Происходит когда открывают порт на PC
      vcom1_cbl.tx_enabled = TRUE;
      _lwevent_set(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN);
    }
    break;
  case USB_APP_CDC_DTE_DEACTIVATED:
    if (vcom1_cbl.configured == TRUE)
    {
      // Происходит когда закрывают порт на PC
      vcom1_cbl.tx_enabled = FALSE;
      _lwevent_clear(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN);  // Объявляем готовность к передаче
    }
    break;
  case USB_DEV_EVENT_DATA_RECEIVED:
    {
      if (vcom1_cbl.configured == TRUE)
      {
        // Событие приема автоматически разрешает работу порта на передачу
        if (vcom1_cbl.tx_enabled == FALSE)
        {
          vcom1_cbl.tx_enabled = TRUE;
          _lwevent_set(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN);
        }

        // Если *size = 0xFFFFFFFF то данные не приняты
        if ((*size != 0xFFFFFFFF) && (*size != 0))
        {
          uint32_t tmp_indx;
          // Пакет принят

          vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].len = *size;
          vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].pos = 0;


          //  Перевести указатель на следующий свободный для приема буфер
          tmp_indx =  vcom1_cbl.rx_head;
          tmp_indx++;
          if (tmp_indx >= IN_BUF_QUANTITY) tmp_indx = 0;



          if (tmp_indx != vcom1_cbl.rx_tail)
          {
            // Если есть еще свободный буффер в очереди то назначить прием в него
            vcom1_cbl.rx_head = tmp_indx;
            res = USB_Class_CDC_Recv_Data(handle, CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, vcom1_cbl.out_pack_sz);
            if (res != USB_OK) vcom1_cbl.rx_errors++;
          }
          else
          {
            // Буферов свободных для пиема не обнаружено.
            // Сигнализируем флагом о том что не смогли зарегистрировать режим прием для конечной точки
            vcom1_cbl.rx_full = 1;
          }
          _lwevent_set(&vcom1_cbl.os_flags, VCOM_FLAG_RECEIVED); // Установливаем флаг выполненного приема
        }
        else
        {
          // Заново взводим прием на текущй буфер поскольку была ошибка приема
          res = USB_Class_CDC_Recv_Data(handle, CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, vcom1_cbl.out_pack_sz);
          if (res != USB_OK) vcom1_cbl.rx_errors++;
        }
      }
    }
    break;
  case USB_DEV_EVENT_SEND_COMPLETE:
    {
      _lwevent_set(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN);

      if ((size != NULL) && (*size != 0) && (!(*size % vcom1_cbl.in_pack_sz)))
      {
        /* If the last packet is the size of endpoint, then send also zero-ended packet,
         ** meaning that we want to inform the host that we do not have any additional
         ** data, so it can flush the output.
         */
        USB_Class_CDC_Send_Data(vcom1_cbl.class_handle, CDC_VCOM1_DIC_BULK_IN_ENDPOINT, NULL, 0);
      }
      else if ((vcom1_cbl.configured == TRUE) && (vcom1_cbl.tx_enabled == TRUE))
      {
        if ((*data != NULL) || ((*data == NULL) && (*size == 0)))
        {
          /* User: add your own code for send complete event */
          /* Schedule buffer for next receive event */

        }
      }
    }
    break;
  case USB_APP_CDC_SERIAL_STATE_NOTIF:
    {
      /* User: add your own code for serial_state notify event */
    }
    break;
  case USB_APP_CDC_CARRIER_ACTIVATED:
    {

    }
    break;
  default:
    {
      error = USBERR_INVALID_REQ_TYPE;
      break;
    }

  }

  return error;
}

/*-----------------------------------------------------------------------------------------------------
 Ожидаем окончания процедуры отправки данных
 Функци необходима в случае использования единственного буфера данных вызывающей задачей
 
 \param ticks 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t VCom1_USB_wait_sending_end(uint32_t ticks)
{
  if (_lwevent_wait_ticks(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN, FALSE, ticks) != MQX_OK)
  {
    return MQX_ERROR;
  }
  _lwevent_set(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN);
  return MQX_OK;
}

/*------------------------------------------------------------------------------



 \param buff
 \param size
 \param ticks

 \return uint32_t MQX_OK
 ------------------------------------------------------------------------------*/
uint32_t VCom1_USB_send_data(uint8_t *buff, uint32_t size, uint32_t ticks)
{
  uint32_t res;
  if (ticks != 0)
  {
    if (_lwevent_wait_ticks(&vcom1_cbl.os_flags, VCOM_FLAG_TRANSMIT_EN, FALSE, ticks) != MQX_OK)
    {
      vcom1_cbl.tx_errors++;
      return MQX_ERROR;
    }
  }
  if (USB_Class_CDC_Send_Data(vcom1_cbl.class_handle, CDC_VCOM1_DIC_BULK_IN_ENDPOINT, buff, size) != USB_OK)
  {
    vcom1_cbl.tx_errors++;
    return MQX_ERROR;
  }

  return MQX_OK;
}

/*------------------------------------------------------------------------------
  Получить данные из виртуального порта в буфер buff, количество байт данных - size
  В случае невозможности выполнения происходит возврат из функиции без задержки

 \param buff
 \param size

 \return uint32_t. Возвращает MQX_OK в случае выполнения функции и MQX_ERROR в случае невыполнения функции

 ------------------------------------------------------------------------------*/
uint32_t VCom1_USB_get_data(uint8_t *b, uint32_t ticks)
{
  int32_t  res;
  uint32_t taili = vcom1_cbl.rx_tail;

  // Если индексы буферов равны то это значит отсутствие принятых пакетов
  if (vcom1_cbl.rx_tail == vcom1_cbl.rx_head)
  {
    if (ticks != 0)
    {
      if (_lwevent_wait_ticks(&vcom1_cbl.os_flags, VCOM_FLAG_RECEIVED, FALSE, ticks) != MQX_OK) return MQX_ERROR;
    }
    else return MQX_ERROR;
  }


  *b = vcom1_cbl.rx_pack_cbl[taili].buff[vcom1_cbl.rx_pack_cbl[taili].pos];
  vcom1_cbl.rx_pack_cbl[taili].pos++;

  // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
  if (vcom1_cbl.rx_pack_cbl[taili].pos >= vcom1_cbl.rx_pack_cbl[taili].len)
  {
    // Смещаем указатель хвоста очереди приемных буфферов
    // Появляется место для движения головы очереди приемных буфферов

    if ((taili + 1) >= IN_BUF_QUANTITY) taili = 0;
    else taili++;
    vcom1_cbl.rx_tail = taili;

    if (vcom1_cbl.rx_full == 1) // Если очередь была заполнена, то посылаем запрос приема , так как в обработчике собыйтий запрос приема был пропущен
    {
      vcom1_cbl.rx_full = 0;

      if ((vcom1_cbl.rx_head + 1) >= IN_BUF_QUANTITY) vcom1_cbl.rx_head = 0;
      else vcom1_cbl.rx_head++;

      res = USB_Class_CDC_Recv_Data(vcom1_cbl.class_handle, CDC_VCOM1_DIC_BULK_OUT_ENDPOINT, vcom1_cbl.rx_pack_cbl[vcom1_cbl.rx_head].buff, vcom1_cbl.out_pack_sz);
      if (res != USB_OK) vcom1_cbl.rx_errors++;
    }
  }
  return MQX_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return T_usb_cdc_vcom_struct* 
-----------------------------------------------------------------------------------------------------*/
T_usb_cdc_vcom_struct* Vcom1_USB_get_cbl(void)
{
  return &vcom1_cbl;
}

