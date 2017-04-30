#include "App.h"

static int Mnsdrv_init(void **pcbl, void *pdrv);
static int Mnsdrv_send_buf(const void *buf, unsigned int len);
static int Mnsdrv_wait_ch(unsigned char *b, int timeout);
static int Mnsdrv_printf(const char  *fmt_ptr, ...);
static int Mnsdrv_deinit(void *pcbl);


static T_monitor_driver ser_std_driver=
{
   MN_STD_SERIAL_DRIVER,
   Mnsdrv_init,
   Mnsdrv_send_buf,
   Mnsdrv_wait_ch,
   Mnsdrv_printf,
   Mnsdrv_deinit,
};


typedef struct
{
  MQX_FILE_PTR fin;
  MQX_FILE_PTR fout;

} T_ser_drv_cbl;


/*-----------------------------------------------------------------------------------------------------
  ОТкрываем файл драйвера и получаем файловые указатели на основании типа драйвера
-----------------------------------------------------------------------------------------------------*/
static _mqx_int Mnsdrv_file_by_type(const int type, MQX_FILE_PTR *pfin, MQX_FILE_PTR *pfout)
{
   const char *fnm;
   FILE_PTR f;
   switch (type)
   {
   case MN_SERIAL_A_DRIVER:
      fnm = "ittya:";
      break;
   case MN_SERIAL_B_DRIVER:
      fnm = "ittyb:";
      break;
   case MN_SERIAL_C_DRIVER:
      fnm = "ittyc:";
      break;
   case MN_SERIAL_D_DRIVER:
      fnm = "ittyd:";
      break;
   case MN_SERIAL_E_DRIVER:
      fnm = "ittye:";
      break;
   case MN_SERIAL_F_DRIVER:
      fnm = "ittyf:";
      break;
   case MN_STD_SERIAL_DRIVER:
   default:
      *pfin  = stdin;
      *pfout = stdout;
      return MQX_OK;
   }

   f = fopen(fnm, 0);
   if (f == NULL) return MQX_ERROR;
   *pfin  = f;
   *pfout = f;
   return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static int Mnsdrv_init(void **pcbl, void *pdrv)
{
   _mqx_uint result;

   T_ser_drv_cbl *p;

   // Выделяем память для управляющей структуры драйвера
   p = (T_ser_drv_cbl *)_mem_alloc_zero(sizeof(T_ser_drv_cbl));

   // Открываем файл драйвера
   if (Mnsdrv_file_by_type( ((T_monitor_driver *)pdrv)->driver_type, &(p->fin) , &(p->fout) ) != MQX_OK)
   {
      return MQX_ERROR;
   }

   *pcbl = p; //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера


   return MQX_OK;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static int Mnsdrv_deinit(void *pcbl)
{

   return MQX_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_monitor_driver *Mnsdrv_get_ser_std_driver(void)
{
  return &ser_std_driver;
}

/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mnsdrv_send_buf(const void *buf, unsigned int len)
{
   int i;
   T_ser_drv_cbl *p;
   T_monitor_cbl    *pvt100_cb;
   pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
   p = (T_ser_drv_cbl*)(pvt100_cb->pdrvcbl);

   for (i = 0; i < len; i++)
   {
      _io_fputc(((char*)buf)[i], p->fout);
   }
   return MQX_OK;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnsdrv_wait_ch(unsigned char *b, int timeout)
{
  MQX_TICK_STRUCT   tickt_start;
  MQX_TICK_STRUCT   tickt;
  bool              overfl;
  T_ser_drv_cbl     *p;
  T_monitor_cbl     *pvt100_cb;

  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  p = (T_ser_drv_cbl*)(pvt100_cb->pdrvcbl);

   _time_get_ticks(&tickt_start);
   for (;;)
   {
      while (status() == 0)
      {
         _time_get_ticks(&tickt);
         if (_time_diff_milliseconds(&tickt, &tickt_start, &overfl)>timeout) return MQX_ERROR;
         _time_delay_ticks(1);
      }
      *b = _io_fgetc(p->fin);
      return MQX_OK;
   }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mnsdrv_printf(const char  *fmt_ptr, ...)
{
   _mqx_int          res;
   va_list           ap;
   T_ser_drv_cbl     *p;
   T_monitor_cbl     *pvt100_cb;

   pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());
   p = (T_ser_drv_cbl*)(pvt100_cb->pdrvcbl);


   va_start(ap, fmt_ptr);
   res = _io_vfprintf(p->fout, (char *)fmt_ptr, ap);
   va_end(ap);

   return res;
}

