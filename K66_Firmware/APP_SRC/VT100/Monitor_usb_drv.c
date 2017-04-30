// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.06.23
// 11:31:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "usb_error.h"

#define  VCOM_PORT_NUM 2

static int Mnudrv1_init(void **pcbl, void *pdrv);
static int Mnudrv1_send_buf(const void *buf, unsigned int len);
static int Mnudrv1_wait_ch(unsigned char *b, int ticks);
static int Mnudrv1_printf(const char  *fmt_ptr, ...);
static int Mnudrv1_deinit(void *pcbl);

static T_monitor_driver vcom_driver1 =
{
  MN_USB_VCOM_DRIVER1,
  Mnudrv1_init,
  Mnudrv1_send_buf,
  Mnudrv1_wait_ch,
  Mnudrv1_printf,
  Mnudrv1_deinit,
};

#ifdef USB_2VCOM
static int Mnudrv2_init(void **pcbl, void *pdrv);
static int Mnudrv2_send_buf(const void *buf, unsigned int len);
static int Mnudrv2_wait_ch(unsigned char *b, int ticks);
static int Mnudrv2_printf(const char  *fmt_ptr, ...);
static int Mnudrv2_deinit(void *pcbl);

static T_monitor_driver vcom_driver2 =
{
  MN_USB_VCOM_DRIVER2,
  Mnudrv2_init,
  Mnudrv2_send_buf,
  Mnudrv2_wait_ch,
  Mnudrv2_printf,
  Mnudrv2_deinit,
};
#endif


#define MAX_STR_SZ  128
typedef struct
{
  uint8_t          str[MAX_STR_SZ];
} T_vcom_drv_cbl;



/*------------------------------------------------------------------------------
  »нициализаци€ драйвера.
  ¬ызываетс€ из задачи терминала при ее старте


 \param pcbl  -  указатель на указатель на внутреннюю управл€ющую структуру драйвера
 \param pdrv   - указатель  на структуру драйвера типа T_monitor_driver

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv1_init(void **pcbl, void *pdrv)
{
  _mqx_uint result;
  uint8_t   b;

  T_vcom_drv_cbl *p;

  // ¬ыдел€ем пам€ть дл€ управл€ющей структуры драйвера
  p = (T_vcom_drv_cbl *)_mem_alloc_zero(sizeof(T_vcom_drv_cbl));



  if (((T_monitor_driver *)pdrv)->driver_type == MN_USB_VCOM_DRIVER1)
  {
    *pcbl = p; //  ”станавливаем в управл€ющей структуре драйвера задачи указатель на управл€ющую структуру драйвера
  }
  else
  {
    LOG("Incorrect driver type", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    _mem_free(p);
    return MQX_ERROR;
  }

  // ∆дем подключени€ по  Virtual COM
  return Mnudrv1_wait_ch(&b, INT_MAX);

}


/*------------------------------------------------------------------------------

 \param pcbl - указатель на внутреннюю управл€ющую структуру драйвера

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv1_deinit(void *pcbl)
{
  // ќсвобождаем пам€ть упрал€ющей структуры
  _mem_free(pcbl);
  return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnudrv_get_USB_vcom_driver1(void)
{
  return &vcom_driver1;
}
/*-------------------------------------------------------------------------------------------------------------
  ¬ывод буфера  строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnudrv1_send_buf(const void *buf, unsigned int len)
{
  uint32_t res;

  res =  VCom1_USB_send_data((uint8_t *)buf, len, 100);

  if (res == MQX_OK) VCom1_USB_wait_sending_end(100);

  return res;
}


/*------------------------------------------------------------------------------

 \param b
 \param timeout

 \return int возвращает MQX_OK в случае состо€вшегос€ приема байта
 ------------------------------------------------------------------------------*/
static int Mnudrv1_wait_ch(unsigned char *b, int ticks)
{
  return VCom1_USB_get_data(b, ticks);
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnudrv1_printf(const char  *fmt_ptr, ...)
{
  _mqx_int          res;
  va_list           ap;
  T_vcom_drv_cbl     *p;
  T_monitor_cbl     *pvt100_cb;

  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  p = (T_vcom_drv_cbl *)(pvt100_cb->pdrvcbl);

  va_start(ap, fmt_ptr);
  res = vsnprintf((char*)p->str, MAX_STR_SZ - 1, fmt_ptr, ap);
  va_end(ap);
  if (res < 0) return MQX_ERROR;

  res = Mnudrv1_send_buf(p->str, res);

  return res;
}

#ifdef USB_2VCOM
/*------------------------------------------------------------------------------
  »нициализаци€ драйвера.
  ¬ызываетс€ из задачи терминала при ее старте


 \param pcbl  -  указатель на указатель на внутреннюю управл€ющую структуру драйвера
 \param pdrv   - указатель  на структуру драйвера типа T_monitor_driver

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv2_init(void **pcbl, void *pdrv)
{
  _mqx_uint result;
  uint8_t   b;

  T_vcom_drv_cbl *p;

  // ¬ыдел€ем пам€ть дл€ управл€ющей структуры драйвера
  p = (T_vcom_drv_cbl *)_mem_alloc_zero(sizeof(T_vcom_drv_cbl));



  if (((T_monitor_driver *)pdrv)->driver_type == MN_USB_VCOM_DRIVER2)
  {
    *pcbl = p; //  ”станавливаем в управл€ющей структуре драйвера задачи указатель на управл€ющую структуру драйвера
  }
  else
  {
    LOG("Incorrect driver type", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    _mem_free(p);
    return MQX_ERROR;
  }

  // ∆дем подключени€ по  Virtual COM
  return Mnudrv2_wait_ch(&b, INT_MAX);

}


/*------------------------------------------------------------------------------

 \param pcbl - указатель на внутреннюю управл€ющую структуру драйвера

 \return int
 ------------------------------------------------------------------------------*/
static int Mnudrv2_deinit(void *pcbl)
{
  // ќсвобождаем пам€ть упрал€ющей структуры
  _mem_free(pcbl);
  return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver* Mnudrv_get_USB_vcom_driver2(void)
{
  return &vcom_driver2;
}



/*-------------------------------------------------------------------------------------------------------------
  ¬ывод буфера  строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnudrv2_send_buf(const void *buf, unsigned int len)
{
  uint32_t res;

  res =  VCom2_USB_send_data((uint8_t *)buf, len, 100);

  if (res == MQX_OK) VCom2_USB_wait_sending_end(100);

  return res;
}

/*------------------------------------------------------------------------------

 \param b
 \param timeout

 \return int возвращает MQX_OK в случае состо€вшегос€ приема байта
 ------------------------------------------------------------------------------*/
static int Mnudrv2_wait_ch(unsigned char *b, int ticks)
{
  return VCom2_USB_get_data(b, ticks);
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnudrv2_printf(const char  *fmt_ptr, ...)
{
  _mqx_int          res;
  va_list           ap;
  T_vcom_drv_cbl     *p;
  T_monitor_cbl     *pvt100_cb;

  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  p = (T_vcom_drv_cbl *)(pvt100_cb->pdrvcbl);

  va_start(ap, fmt_ptr);
  res = vsnprintf((char*)p->str, MAX_STR_SZ - 1, fmt_ptr, ap);
  va_end(ap);
  if (res < 0) return MQX_ERROR;

  res = Mnudrv2_send_buf(p->str, res);

  return res;
}

#endif
