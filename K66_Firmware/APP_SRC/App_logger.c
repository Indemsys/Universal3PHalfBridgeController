#include "App.h"


#define       FILE_LOG_MSG_CNT  64


T_app_log_record       app_log[EVENT_LOG_SIZE];

typedef struct
{
  // Переменные лога событий
  uint32_t       event_log_head;
  uint32_t       event_log_tail;

  unsigned int sync_err;
  unsigned int overl_err;

} T_app_log_cbl;

#define MAX_LOG_FILENAME_SZ 32
#define LOG_FILE_NAME DISK_NAME"AppLog.txt"
typedef struct
{
  char         log_file_name[MAX_LOG_FILENAME_SZ + 1];
  unsigned int queue_err;
  unsigned int alloc_err;

} T_file_log_cbl;



static T_app_log_cbl    log_cbl;
static LWSEM_STRUCT     log_sem;

static T_file_log_cbl   flog_cbl; // Управляющая структура файлового лога

static uint32_t file_log_queue[sizeof(LWMSGQ_STRUCT)/sizeof(uint32_t)+ FILE_LOG_MSG_CNT * sizeof(void*)];

static uint8_t  pend_params_saving;
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint AppLogg_init(void)
{
  _mqx_uint res = MQX_OK;
  if (_lwsem_create(&log_sem, 1) != MQX_OK) res = MQX_ERROR;

  // Создаем очередь сообщений для лога на SD карту
  if  (_lwmsgq_init((void *)file_log_queue, FILE_LOG_MSG_CNT, sizeof(void *)) != MQX_OK) res = MQX_ERROR;


  return res;
}

/*------------------------------------------------------------------------------
  Функция передачи лога в задачу сохранения лога в файл


 \return _mqx_uint
 ------------------------------------------------------------------------------*/
static _mqx_uint _write_log_to_file(char *str, const char *func_name, unsigned int line_num, unsigned int severity)
{
  T_app_log_record *plrec;
  // Выделяем память для структуры с содержимым полей сообщения

  plrec = (T_app_log_record *)_mem_alloc_system_zero(sizeof(T_app_log_record));
  if (plrec != NULL)
  {
    plrec->line_num = line_num;
    plrec->severity = severity;
    strncpy(plrec->msg, str, LOG_STR_MAX_SZ);
    strncpy(plrec->func_name, func_name, EVNT_LOG_FNAME_SZ);
    _time_get(&(plrec->time));

    if (_lwmsgq_send((void *)file_log_queue, (uint32_t *)&plrec, 0) != MQX_OK)
    {
      // Если послать не удалось освобождаем память сообщения
      _mem_free(plrec);
      flog_cbl.queue_err++;
      return MQX_ERROR;
    }
    return MQX_OK;
  }
  else
  {
    flog_cbl.alloc_err++;
    return MQX_ERROR;
  }
}

/*------------------------------------------------------------------------------
  Запись сообщения в таблицу лога и в другие места назначения


 \param str         : сообщение
 \param func_name   : имя функции
 \param line_num    : номер строки
 \param severity    : важность сообщения
 ------------------------------------------------------------------------------*/
static void _applog_write(char *str, const char *func_name, unsigned int line_num, unsigned int severity)
{
  TIME_STRUCT t;
  int         head;
  int         tail;
#ifdef APP_TO_RTT_LOG
  char          rtt_log_str[LOG_STR_MAX_SZ + 1];
#endif


  if (_lwsem_wait_ticks(&log_sem, 4) == MQX_OK)
  {
#ifdef MQX_MFS
    if (wvar.en_log_file)
    {
      _write_log_to_file(str, func_name, line_num, severity);
    }
#endif
    head = log_cbl.event_log_head;
    _time_get(&app_log[head].time);
    strncpy(app_log[head].msg, str, LOG_STR_MAX_SZ - 1);
    strncpy(app_log[head].func_name, func_name, EVNT_LOG_FNAME_SZ - 1);
    app_log[head].line_num = line_num;
    app_log[head].severity = severity;
    // Сдвигаем указатель головы лога
    head++;
    if (head >= EVENT_LOG_SIZE) head = 0;
    log_cbl.event_log_head = head;

    tail = log_cbl.event_log_tail;
    // Если голова достигла хвоста, то сдвигает указатель хвоста
    if (head == tail)
    {
      tail++;
      if (tail >= EVENT_LOG_SIZE) tail = 0;
      log_cbl.event_log_tail = tail;
      log_cbl.overl_err++;
    }
#ifdef APP_TO_RTT_LOG
    {
      snprintf(rtt_log_str, LOG_STR_MAX_SZ, "%05d.%03d %s (%s %d)\r\n",
               app_log[head].time.SECONDS, app_log[head].time.MILLISECONDS,
               app_log[head].msg,
               app_log[head].func_name,
               app_log[head].line_num);

      SEGGER_RTT_WriteString(0, rtt_log_str);
    }
#endif
    //PCLnk_set_log_flag();
    _lwsem_post(&log_sem); //  Освобождаем семафор
  }
  else
  {
    log_cbl.sync_err++;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Зарегистрировать запрос на сохранение параметров 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void AppLog_pend_saving_params(void)
{
  pend_params_saving = 1;
}


/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 ------------------------------------------------------------------------------*/
void LOG(const char *str, const char *name, unsigned int line_num, unsigned int severity)
{
  char *s;
  s = _mem_alloc(128);
  if (s != NULL)
  {
    snprintf(s, 127, str);
    _applog_write(s, name, line_num, severity);
    _mem_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 \param v1
 ------------------------------------------------------------------------------*/
void LOG1(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1)
{
  char *s;
  s = _mem_alloc(128);
  if (s != NULL)
  {
    snprintf(s, 127, str, v1);
    _applog_write(s, name, line_num, severity);
    _mem_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 \param v1
 \param v2
 ------------------------------------------------------------------------------*/
void LOG2(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2)
{
  char *s;
  s = _mem_alloc(128);
  if (s != NULL)
  {
    snprintf(s, 127, str, v1, v2);
    _applog_write(s, name, line_num, severity);
    _mem_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param str
 \param name
 \param line_num
 \param severity
 \param v1
 \param v2
 \param v3
 ------------------------------------------------------------------------------*/
void LOG3(const char *str, const char *name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2, uint32_t v3)
{
  char *s;
  s = _mem_alloc(128);
  if (s != NULL)
  {
    snprintf(s, 127, str, v1, v2, v3);
    _applog_write(s, name, line_num, severity);
    _mem_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param name
 \param line_num
 \param severity
 \param fmt_ptr
 ------------------------------------------------------------------------------*/
void LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  char *s;
  va_list           ap;

  s = _mem_alloc(128);

  if (s != NULL)
  {
    va_start(ap, fmt_ptr);
    vsnprintf(s, 127, (char *)fmt_ptr, ap);
    va_end(ap);
    _applog_write(s, name, line_num, severity);
    _mem_free(s);
  }
}

/*------------------------------------------------------------------------------



 \param name
 \param line_num
 \param severity
 \param fmt_ptr
 ------------------------------------------------------------------------------*/
void VERBOSE_LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  char *s;
  va_list           ap;

  if (wvar.en_verbose_log)
  {
    s = _mem_alloc(128);
    if (s != NULL)
    {
      va_start(ap, fmt_ptr);
      vsnprintf(s, 127, (char *)fmt_ptr, ap);
      va_end(ap);
      _applog_write(s, name, line_num, severity);
      _mem_free(s);
    }
  }
}
/*------------------------------------------------------------------------------
  Получить структуру записи лога от хвоста
  Возвращает 0 если записей в логе нет

 \param rec

 \return int
 ------------------------------------------------------------------------------*/
int AppLog_get_tail_record(T_app_log_record *rec)
{
  int res = 0;
  _lwsem_wait_ticks(&log_sem, 4);

  if (log_cbl.event_log_head != log_cbl.event_log_tail)
  {
    memcpy(rec, &app_log[log_cbl.event_log_tail], sizeof(T_app_log_record));
    log_cbl.event_log_tail++;
    if (log_cbl.event_log_tail >= EVENT_LOG_SIZE) log_cbl.event_log_tail = 0;
    res = 1;
  }

  _lwsem_post(&log_sem); //  Освобождаем семафор
  return res;
}

/*------------------------------------------------------------------------------



 \param pvt100_cb
 ------------------------------------------------------------------------------*/
void AppLogg_monitor_output(T_monitor_cbl *pvt100_cb)
{
  uint32_t   i;
  uint32_t   n;
  uint32_t   reqn;
  uint32_t   outn;
  uint8_t    b;

  pvt100_cb->_printf(VT100_CLEAR_AND_HOME);
  pvt100_cb->_printf("Events log. <R> - exit, <D> - print all log\n\r");
  pvt100_cb->_printf("--------------------------------------------\n\r");

  do
  {
    VT100_set_cursor_pos(3, 0);


    // Вывод последних 22-х строк лога

    // Определяем количестово строк в логе
    if (log_cbl.event_log_head >= log_cbl.event_log_tail)
    {
      reqn = log_cbl.event_log_head - log_cbl.event_log_tail;
    }
    else
    {
      reqn = EVENT_LOG_SIZE - (log_cbl.event_log_tail - log_cbl.event_log_head);
    }

    // Определяем количество выводимых на экран строк лога
    if (reqn < EVENT_LOG_DISPLAY_ROW)
    {
      outn = reqn;
    }
    else
    {
      outn = EVENT_LOG_DISPLAY_ROW;
    }

    // Вычисляем индекс строки в логе с которой начинается вывод
    if (log_cbl.event_log_head >= log_cbl.event_log_tail)
    {
      n = log_cbl.event_log_head - outn;
    }
    else
    {
      if (outn > log_cbl.event_log_head)
      {
        n = EVENT_LOG_SIZE - (outn - log_cbl.event_log_head);
      }
      else
      {
        n = log_cbl.event_log_head - outn;
      }
    }





    for (i = 0; i < EVENT_LOG_DISPLAY_ROW; i++)
    {
      if (i < outn)
      {

        pvt100_cb->_printf(VT100_CLL_FM_CRSR"%05d.%03d %s (%s %d)\n\r",
                           app_log[n].time.SECONDS, app_log[n].time.MILLISECONDS,
                           app_log[n].msg,
                           app_log[n].func_name,
                           app_log[n].line_num);
        n++;
        if (n >= EVENT_LOG_SIZE) n = 0;
      }
      else
      {
        pvt100_cb->_printf(VT100_CLL_FM_CRSR"\n\r");
      }
    }



    if (pvt100_cb->_wait_char(&b, 200) == MQX_OK)
    {
      switch (b)
      {
      case 'D':
      case 'd':
        // Вывод всего лога
        if (log_cbl.event_log_head >= log_cbl.event_log_tail)
        {
          reqn = log_cbl.event_log_head - log_cbl.event_log_tail;
        }
        else
        {
          reqn = EVENT_LOG_SIZE - (log_cbl.event_log_tail - log_cbl.event_log_head);
        }

        pvt100_cb->_printf("\n\r");
        n = log_cbl.event_log_tail;
        for (i = 0; i < reqn; i++)
        {
          pvt100_cb->_printf("%05d.%03d %s (%s %d)\n\r",
                             app_log[n].time.SECONDS, app_log[n].time.MILLISECONDS,
                             app_log[n].msg,
                             app_log[n].func_name,
                             app_log[n].line_num);
          n++;
          if (n >= EVENT_LOG_SIZE) n = 0;
        }
        pvt100_cb->_printf("\n\r");
        pvt100_cb->_wait_char(&b, 2000);

        break;
      case 'R':
      case 'r':
        return;
      case 'C':
      case 'c':
        log_cbl.event_log_head = 0;
        log_cbl.event_log_tail = 0;
        break;
      }

    }
  }
  while (1);
}

#define TEMP_STR_SZ 128
/*------------------------------------------------------------------------------
  Задача записи лога в файл


 \param initial_data
 ------------------------------------------------------------------------------*/
void Task_file_log(unsigned int initial_data)
{
  MQX_FILE_PTR       f;
  T_app_log_record  *plrec;
  char              *tstr;
  int               res;

  strncpy(flog_cbl.log_file_name, LOG_FILE_NAME, MAX_LOG_FILENAME_SZ);

  tstr = (char *)_mem_alloc_zero(TEMP_STR_SZ + 1);
  if (tstr == NULL)
  {
    LOG("Memory allocation error. Task stopped.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
  }


  do
  {
    // Ждем появления сообщения для задачи
    if (_lwmsgq_receive((void *)file_log_queue, (uint32_t *)&plrec, LWMSGQ_RECEIVE_BLOCK_ON_EMPTY, 2, 0) == MQX_OK)
    {


      f = _io_fopen(flog_cbl.log_file_name, "a");
      if (f == NULL)
      {
        LOG("Log file opening error. Task stopped.", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
        _mem_free(plrec);
        return;
      }

      do
      {
        DATE_STRUCT date;
        // Запись в файл лога
        _time_to_date(&(plrec->time), &date);

        res = snprintf(tstr, TEMP_STR_SZ, "%04d.%02d.%02d %02d:%02d:%02d.%03d |", date.YEAR, date.MONTH, date.DAY, date.HOUR, date.MINUTE, date.SECOND, plrec->time.MILLISECONDS);
        if (res > 0) _io_write(f, tstr, res);
        res = snprintf(tstr, TEMP_STR_SZ, "%02d | %-36s | %5d |", plrec->severity, plrec->func_name, plrec->line_num);
        if (res > 0) _io_write(f, tstr, res);
        res = snprintf(tstr, TEMP_STR_SZ, " %s\r\n", plrec->msg);
        if (res > 0) _io_write(f, tstr, res);
        _mem_free(plrec);

        if (flog_cbl.alloc_err != 0)
        {
          TIME_STRUCT time;
          _time_get(&time);
          res = snprintf(tstr, TEMP_STR_SZ, "%04d.%02d.%02d %02d:%02d:%02d.%03d |", date.YEAR, date.MONTH, date.DAY, date.HOUR, date.MINUTE, date.SECOND, time.MILLISECONDS);
          if (res > 0) _io_write(f, tstr, res);
          res = snprintf(tstr, TEMP_STR_SZ, "ERROR: Allocation fault (%d)\r\n", flog_cbl.alloc_err);
          if (res > 0) _io_write(f, tstr, res);

          flog_cbl.alloc_err = 0;
        }
        if (flog_cbl.queue_err != 0)
        {
          TIME_STRUCT time;
          _time_get(&time);
          res = snprintf(tstr, TEMP_STR_SZ, "%04d.%02d.%02d %02d:%02d:%02d.%03d |", date.YEAR, date.MONTH, date.DAY, date.HOUR, date.MINUTE, date.SECOND, time.MILLISECONDS);
          if (res > 0) _io_write(f, tstr, res);
          res = snprintf(tstr, TEMP_STR_SZ, "ERROR: Messages queue fault (%d)\r\n", flog_cbl.queue_err);
          if (res > 0) _io_write(f, tstr, res);
          flog_cbl.queue_err = 0;
        }
        // Продолжаем опращивать очередь пока не извлечем из нее все сообщения
        if (_lwmsgq_receive((void *)file_log_queue, (uint32_t *)&plrec, LWMSGQ_TIMEOUT_FOR, 10, 0) != MQX_OK) break;

      }
      while (1);

      _io_fclose(f);
    }
    else
    {
      if (pend_params_saving!=0) 
      {
        pend_params_saving = 0;
        Save_Params_to_INIfile();
      }
    }
  }
  while (1);

}
