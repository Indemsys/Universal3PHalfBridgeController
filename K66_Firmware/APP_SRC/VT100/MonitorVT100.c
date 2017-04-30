#include "App.h"
//#include "SFFS_test.h"
#include "MFS_test.h"

#define CNTLQ      0x11
#define CNTLS      0x13
#define DEL        0x7F
#define BACKSPACE  0x08
//#define CR         0x0D  конфликт имен с регистрами периферии
#define LF         0x0A
#define ESC        0x1B

#define COL        80   /* Maximum column size       */

#define EDSTLEN    20


static void Do_watchdog_test(uint8_t keycode);
static void Do_malloc_test(uint8_t keycode);
static void Do_show_event_log(uint8_t keycode);
static void Do_date_time_set(uint8_t keycode);
#ifdef SFFS_TEST
static void Do_SFFS_test(uint8_t keycode);
#endif
#ifdef MFS_TEST
static void Do_MFS_test(uint8_t keycode);
#endif
#ifdef MQX_SHELL
static void Do_Shell(uint8_t keycode);
#endif

static void Do_VBAT_RAM_diagnostic(uint8_t keycode);
static void Do_Reset(uint8_t keycode);

extern const T_VT100_Menu MENU_MAIN;
extern const T_VT100_Menu MENU_PARAMETERS;
extern const T_VT100_Menu MENU_SPEC;

#ifdef U3HB01_APP
extern const T_VT100_Menu MENU_DMC01;
#endif

static int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b);



/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

      Пункты имеющие свое подменю распологаются на следующем уровне вложенности
      Их функция подменяет в главном цикле обработчик нажатий по умолчанию и должна
      отдавать управление периодически в главный цикл

      Пункты не имеющие функции просто переходят на следующий уровень подменю

      Пункты не имеющие подменю полностью захватывают управление и на следующий уровень не переходят

      Пункты без подменю и функции означают возврат на предыдущий уровень
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

//-------------------------------------------------------------------------------------
const T_VT100_Menu_item MENU_MAIN_ITEMS[] =
{
  { '1', Do_Params_editor, (void *)&MENU_PARAMETERS },
#ifdef U3HB01_APP
  { '2', 0,(void *)&MENU_DMC01 },
#endif
  { '3', Do_date_time_set, 0 },
  { '4', Do_show_event_log, 0 },
#ifdef MQX_SHELL
  { '5', Do_Shell, 0 },
#endif
  { '6', 0, (void *)&MENU_SPEC },
  { '7', Do_Reset, 0  },
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_MAIN             =
{
  "U3HB01 Controller",

  "\033[5C MAIN MENU \r\n"
  "\033[5C <1> - Adjustable parameters and settings\r\n"
#ifdef U3HB01_APP
  "\033[5C <2> - Application control\r\n"
#endif
  "\033[5C <3> - Setup date and time\r\n"

  "\033[5C <4> - Log\r\n"
#ifdef MQX_SHELL
  "\033[5C <5> - Shell\r\n"
#endif
  "\033[5C <6> - Special menu\r\n"
  "\033[5C <7> - Reset\r\n",
  MENU_MAIN_ITEMS,
};

//-------------------------------------------------------------------------------------
const T_VT100_Menu      MENU_PARAMETERS       =
{
  "",
  "",
  0
};

const T_VT100_Menu_item MENU_SPEC_ITEMS[] =
{
  { '1', Do_VBAT_RAM_diagnostic, 0 },

#ifdef SFFS_TEST
  { '2', Do_SFFS_test, 0 },
#endif
#ifdef MFS_TEST
  { '3', Do_MFS_test, 0 },
#endif

  { '7', Do_malloc_test, 0 },
  { '8', Do_watchdog_test, 0 },
  { 'R', 0, 0 },
  { 'M', 0, (void *)&MENU_MAIN },
  { 0 }
};

const T_VT100_Menu      MENU_SPEC  =
{
  "MONITOR Ver.160722",
  "\033[5C SPECIAL MENU \r\n"
  "\033[5C <1> - VBAT RAM diagnostic\r\n"
#ifdef SFFS_TEST
  "\033[5C <2> - SFFS test\r\n"
#endif
#ifdef MFS_TEST
  "\033[5C <3> - MFS test\r\n"
#endif

  "\033[5C <7> - Malloc test\r\n"
  "\033[5C <8> - Watchdog test\r\n"
  "\033[5C <R> - Display previous menu\r\n"
  "\033[5C <M> - Display main menu\r\n",
  MENU_SPEC_ITEMS,
};


const char              *_days_abbrev[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char              *_months_abbrev[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/*-------------------------------------------------------------------------------------------------------------
   Процедура ожидания корректной последовательности данных для подтверждения входа с терминала
   Используется для фильтации шумов на длинном кабеле
-------------------------------------------------------------------------------------------------------------*/
const uint8_t             entry_str[] = "12345678";

static void Entry_check(void)
{
  uint32_t indx;
  uint8_t  b;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  pvt100_cb->_printf("Press this sequence to enter - %s:\r\n", entry_str);
  indx = 0;
  while (1)
  {
    if (pvt100_cb->_wait_char(&b, 200) == MQX_OK)
    {
      if (b != 0)
      {
        if (b == entry_str[indx])
        {
          indx++;
          if (indx == (sizeof(entry_str) - 1))
          {
            return;
          }
        }
        else
        {
          indx = 0;
        }
      }
    }
  }
}


/*-------------------------------------------------------------------------------------------------------------
  Вывод текущего времени
-------------------------------------------------------------------------------------------------------------*/
static void Print_RTC_time(uint32_t rtc_time)
{
  TIME_STRUCT ts;
  DATE_STRUCT tm;
  T_monitor_cbl *pvt100_cb;
  pvt100_cb = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  ts.SECONDS = rtc_time;
  ts.MILLISECONDS = 0;
  if (_time_to_date(&ts, &tm) == FALSE)
  {
    pvt100_cb->_printf(VT100_CLR_LINE"Current time is      : - - - - - -");
    return;
  }
  // Вывод времени без дня недели
  pvt100_cb->_printf(VT100_CLR_LINE"Current time is      : %s %2d %d %.2d:%.2d:%.2d", _months_abbrev[tm.MONTH - 1], tm.DAY, tm.YEAR, tm.HOUR, tm.MINUTE, tm.SECOND);
}

/*-------------------------------------------------------------------------------------------------------------
   Задача монитора
-------------------------------------------------------------------------------------------------------------*/
void Task_VT100(uint32_t initial_data)
{
  uint8_t    b;
  uint32_t rtc_time;

  T_monitor_cbl *mcbl;
  T_monitor_driver *drv;

  // Выделим память для управляющей структуры задачи
  mcbl = (T_monitor_cbl *)_mem_alloc_zero(sizeof(T_monitor_cbl));
  if (mcbl == NULL) return;
  _task_set_environment(_task_get_id(), mcbl);

  // Определим указатели функций ввода-вывода
  drv = (T_monitor_driver *)_task_get_parameter();
  mcbl->pdrv = drv;
  mcbl->_printf    = drv->_printf;
  mcbl->_wait_char = drv->_wait_char;
  mcbl->_send_buf  = drv->_send_buf;

  if (drv->_init(&mcbl->pdrvcbl, drv) != MQX_OK) return; // Выходим из задачи если не удалось инициализировать драйвер.


//  if ((app_vars.pc_monitor_mode == 1))
//  {
//    if (((app_vars.wifi_pc_monitor == 1) && (drv->driver_type == MN_TCP_DRIVER)) ||
//        ((app_vars.wifi_pc_monitor == 0) && (drv->driver_type == MN_STD_SERIAL_DRIVER)))
//    {
//      PC_monitor(mcbl);
//    }
//  }

  // Очищаем экран
  mcbl->_printf(VT100_CLEAR_AND_HOME);


  //Entry_check();
  Goto_main_menu();

  do
  {
    if (mcbl->_wait_char(&b, 200) == MQX_OK)
    {
      if (b != 0)
      {
        if ((b == 0x1B) && (mcbl->Monitor_func != Edit_func))
        {
          mcbl->_printf(VT100_CLEAR_AND_HOME);
          Entry_check();
          Goto_main_menu();
        }
        else
        {
          if (mcbl->Monitor_func) mcbl->Monitor_func(b);  // Обработчик нажатий главного цикла
        }
      }
    }


    if (mcbl->menu_trace[mcbl->menu_nesting] == &MENU_MAIN)
    {
      unsigned int uid[4];
      // Вывод на дисплей счетчиков
      //VT100_set_cursor_pos(20, 0);

      VT100_set_cursor_pos(23, 0);
      _bsp_get_unique_identificator(uid);
      mcbl->_printf("UID                  : %08X %08X %08X %08X", uid[0], uid[1], uid[2], uid[3]);
      VT100_set_cursor_pos(24, 0);
      mcbl->_printf("Firmware             : %s  (%s)", GetFirmwareDescr() , wvar.ver);
      VT100_set_cursor_pos(25, 0);
      _rtc_get_time(&rtc_time);
      Print_RTC_time(rtc_time);
    }


  }
  while (1);

}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Set_monitor_func(void (*func)(unsigned char))
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  mcbl->Monitor_func = func;
}



/*-------------------------------------------------------------------------------------------------------------
  Вывести на экран текущее меню
  1. Вывод заголовока меню
  2. Вывод содержания меню
-------------------------------------------------------------------------------------------------------------*/
void Display_menu(void)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  mcbl->_printf(VT100_CLEAR_AND_HOME);

  if (mcbl->menu_trace[mcbl->menu_nesting] == 0) return;

  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header, 1, Find_str_center((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_header));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  VT100_send_str_to_pos((uint8_t *)mcbl->menu_trace[mcbl->menu_nesting]->menu_body, 3, 0);
  mcbl->_printf("\r\n");
  mcbl->_printf(DASH_LINE);

}
/*-------------------------------------------------------------------------------------------------------------
  Поиск в текущем меню пункта вызываемого передаваемым кодом
  Параметры:
    b - код команды вазывающей пункт меню
  Возвращает:
    Указатель на соответствующий пункт в текущем меню
-------------------------------------------------------------------------------------------------------------*/
int32_t Lookup_menu_item(T_VT100_Menu_item **item, uint8_t b)
{
  int16_t i;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  if (isalpha(b) != 0) b = toupper(b);

  i = 0;
  do
  {
    *item = (T_VT100_Menu_item *)mcbl->menu_trace[mcbl->menu_nesting]->menu_items + i;
    if ((*item)->but == b) return (1);
    if ((*item)->but == 0) break;
    i++;
  }
  while (1);

  return (0);
}


/*----------------------------------------------------------------------------
 *      Line Editor
 *---------------------------------------------------------------------------*/
static int Get_string(char *lp, int n)
{
  int  cnt = 0;
  char c;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  do
  {
    if (mcbl->_wait_char((unsigned char *)&c, 200) == MQX_OK)
    {
      switch (c)
      {
      case CNTLQ:                          /* ignore Control S/Q             */
      case CNTLS:
        break;

      case BACKSPACE:
      case DEL:
        if (cnt == 0)
        {
          break;
        }
        cnt--;                             /* decrement count                */
        lp--;                              /* and line VOID*               */
        /* echo backspace                 */
        mcbl->_printf("\x008 \x008");
        break;
      case ESC:
        *lp = 0;                           /* ESC - stop editing line        */
        return (MQX_ERROR);
      default:
        mcbl->_printf("*");
        *lp = c;                           /* echo and store character       */
        lp++;                              /* increment line VOID*         */
        cnt++;                             /* and count                      */
        break;
      }
    }
  }
  while (cnt < n - 1 && c != 0x0d);      /* check limit and line feed      */
  *lp = 0;                                 /* mark end of string             */
  return (MQX_OK);
}

/*-------------------------------------------------------------------------------------------------------------
  Ввод кода для доступа к специальному меню
-------------------------------------------------------------------------------------------------------------*/
int Enter_special_code(void)
{
  char str[32];
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  if (mcbl->g_access_to_spec_menu != 0)
  {
    return MQX_OK;
  }
  mcbl->_printf(VT100_CLEAR_AND_HOME"Enter access code>");
  if (Get_string(str, 31) == MQX_OK)
  {
    if (strcmp(str, "4321\r") == 0)
    {
      mcbl->g_access_to_spec_menu = 1;
      return MQX_OK;
    }
  }

  return MQX_ERROR;
}
/*-------------------------------------------------------------------------------------------------------------
  Поиск пункта меню по коду вызова (в текущем меню)
  и выполнение соответствующей ему функции
  Параметры:
    b - код команды вазывающей пункт меню

-------------------------------------------------------------------------------------------------------------*/
void Execute_menu_func(uint8_t b)
{
  T_VT100_Menu_item *menu_item;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  if (Lookup_menu_item(&menu_item, b) != 0)
  {
    // Нашли соответствующий пункт меню
    if (menu_item->psubmenu != 0)
    {
      // Если присутствует субменю, то вывести его

      if ((T_VT100_Menu *)menu_item->psubmenu == &MENU_SPEC)
      {
        if (Enter_special_code() != MQX_OK)
        {
          Display_menu();
          return;
        }
      }

      mcbl->menu_nesting++;
      mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)menu_item->psubmenu;

      Display_menu();
      // Если есть функция у пункта меню, то передать ей обработчик нажатий в главном цикле и выполнить функцию.
      if (menu_item->func != 0)
      {
        mcbl->Monitor_func = (T_menu_func)(menu_item->func); // Установить обработчик нажатий главного цикла на функцию из пункта меню
        menu_item->func(0);                // Выполнить саму функцию меню
      }
    }
    else
    {
      if (menu_item->func == 0)
      {
        // Если нет ни субменю ни функции, значит это пункт возврата в предыдущее меню
        // Управление остается в главном цикле у обработчика по умолчанию
        Return_to_prev_menu();
        Display_menu();
      }
      else
      {
        // Если у пункта нет своего меню, то перейти очистить экран и перейти к выполению  функции выбранного пункта
        mcbl->_printf(VT100_CLEAR_AND_HOME);
        menu_item->func(0);
        // Управление возвращается в главный цикл обработчику по умолчанию
        Display_menu();
      }
    }

  }
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Goto_main_menu(void)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  mcbl->menu_nesting = 0;
  mcbl->menu_trace[mcbl->menu_nesting] = (T_VT100_Menu *)&MENU_MAIN;
  Display_menu();
  mcbl->Monitor_func = Execute_menu_func; // Назначение функции
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Return_to_prev_menu(void)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  if (mcbl->menu_nesting > 0)
  {
    mcbl->menu_trace[mcbl->menu_nesting] = 0;
    mcbl->menu_nesting--;
  }
  mcbl->Monitor_func = Execute_menu_func; // Назначение функции
}



/*-------------------------------------------------------------------------------------------------------------
  Очистка экрана монитора
-------------------------------------------------------------------------------------------------------------*/
void VT100_clr_screen(void)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf(VT100_CLEAR_AND_HOME);
}

/*-------------------------------------------------------------------------------------------------------------
     Установка курсора в заданную позицию
     Счет начинается с 0
-------------------------------------------------------------------------------------------------------------*/
void VT100_set_cursor_pos(uint8_t row, uint8_t col)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf("\033[%.2d;%.2dH", row, col);
}

/*-------------------------------------------------------------------------------------------------------------
     Вывод строки в заданную позицию
-------------------------------------------------------------------------------------------------------------*/
void VT100_send_str_to_pos(uint8_t *str, uint8_t row, uint8_t col)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf("\033[%.2d;%.2dH", row, col);
  mcbl->_send_buf(str, strlen((char *)str));
}

/*-------------------------------------------------------------------------------------------------------------
    Находим позицию начала строки для расположения ее по центру экрана
-------------------------------------------------------------------------------------------------------------*/
uint8_t Find_str_center(uint8_t *str)
{
  int16_t l = 0;
  while (*(str + l) != 0) l++; // Находим длину строки
  return (COLCOUNT - l) / 2;
}



/*-------------------------------------------------------------------------------------------------------------
  Прием строки
-------------------------------------------------------------------------------------------------------------*/
int32_t Mon_input_line(char *buf, int buf_len, int row, char *instr)
{

  int   i;
  uint8_t b;
  int   res;
  uint8_t bs_seq[] = { 0x08, 0x020, 0x08, 0 };
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  i = 0;
  VT100_set_cursor_pos(row, 0);
  mcbl->_printf(VT100_CLL_FM_CRSR);
  mcbl->_printf(">");

  if (instr != 0)
  {
    i = strlen(instr);
    if (i < buf_len)
    {
      mcbl->_printf(instr);
    }
    else i = 0;
  }

  do
  {
    if (mcbl->_wait_char(&b, 20000) != MQX_OK)
    {
      res = MQX_ERROR;
      goto exit_;
    };

    if (b == 0x08)
    {
      if (i > 0)
      {
        i--;
        mcbl->_send_buf(bs_seq, sizeof(bs_seq));
      }
    }
    else if (b != 0x0D && b != 0x0A && b != 0)
    {
      mcbl->_send_buf(&b, 1);
      buf[i] = b;           /* String[i] value set to alpha */
      i++;
      if (i >= buf_len)
      {
        res = MQX_ERROR;
        goto exit_;
      };
    }
  }
  while ((b != 0x0D) && (i < COL));

  res = MQX_OK;
  buf[i] = 0;                     /* End of string set to NUL */
exit_:

  VT100_set_cursor_pos(row, 0);
  mcbl->_printf(VT100_CLL_FM_CRSR);

  return (res);
}

/*------------------------------------------------------------------------------
 Вывод дампа области памяти


 \param addr       - выводимый начальный адрес дампа
 \param buf        - указатель на память
 \param buf_len    - количество байт
 \param sym_in_str - количество выводимых байт в строке дампа

 \return int32_t
 ------------------------------------------------------------------------------*/
void Mon_print_dump(uint32_t addr, void *buf, uint32_t buf_len, uint8_t sym_in_str)
{

  uint32_t   i;
  uint32_t   scnt;
  uint8_t    *pbuf;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  pbuf = (uint8_t *)buf;
  scnt = 0;
  for (i = 0; i < buf_len; i++)
  {
    if (scnt == 0)
    {
      mcbl->_printf("%08X: ", addr);
    }

    mcbl->_printf("%02X ", pbuf[i]);

    addr++;
    scnt++;
    if (scnt >= sym_in_str)
    {
      scnt = 0;
      mcbl->_printf("\r\n");
    }
  }

  if (scnt != 0)
  {
    mcbl->_printf("\r\n");
  }
}




/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void I2C_print_statictic(MQX_FILE_PTR fd)
{
  _mqx_int              result;
  I2C_STATISTICS_STRUCT stats;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf("Get statistics ... ");
  result = ioctl(fd, IO_IOCTL_I2C_GET_STATISTICS, (void *)&stats);
  if (result == I2C_OK)
  {
    mcbl->_printf("\n\r");
    mcbl->_printf("  Interrupts:  %d\n\r", stats.INTERRUPTS);
    mcbl->_printf("  Rx packets:  %d\n\r", stats.RX_PACKETS);
    mcbl->_printf("  Tx packets:  %d\n\r", stats.TX_PACKETS);
    mcbl->_printf("  Tx lost arb: %d\n\r", stats.TX_LOST_ARBITRATIONS);
    mcbl->_printf("  Tx as slave: %d\n\r", stats.TX_ADDRESSED_AS_SLAVE);
    mcbl->_printf("  Tx naks:     %d\n\r", stats.TX_NAKS);
  }
  else
  {
    mcbl->_printf("ERROR\n\r");
  }
  mcbl->_printf("\n\r");
}



/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint Edit_integer_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv)
{
  char   str[32];
  char   buf[32];
  uint32_t tmpv;
  sprintf(str, "%d", *value);
  if (Mon_input_line(buf, 31, row, str) == MQX_OK)
  {
    if (sscanf(buf, "%d", &tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
      return MQX_OK;
    }
  }
  return MQX_ERROR;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
_mqx_uint Edit_integer_hex_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv)
{
  char   str[32];
  char   buf[32];
  uint32_t tmpv;
  sprintf(str, "%08X", *value);
  if (Mon_input_line(buf, 31, row, str) == MQX_OK)
  {
    if (sscanf(buf, "%x", &tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
      return MQX_OK;
    }
  }
  return MQX_ERROR;
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void Edit_float_val(uint32_t row, float *value, float minv, float maxv)
{
  char   str[32];
  char   buf[32];
  float tmpv;
  sprintf(str, "%f", *value);
  if (Mon_input_line(buf, 31, row, str) == MQX_OK)
  {
    if (sscanf(buf, "%f", &tmpv) == 1)
    {
      if (tmpv > maxv) tmpv = maxv;
      if (tmpv < minv) tmpv = minv;
      *value = tmpv;
    }
  }
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static uint8_t BCD2ToBYTE(uint8_t value)
{
  uint32_t tmp;
  tmp = ((value & 0xF0) >> 4) * 10;
  return(uint8_t)(tmp + (value & 0x0F));
}
/*-----------------------------------------------------------------------------------------------------
  Установка даты и времени
-----------------------------------------------------------------------------------------------------*/
static void Do_date_time_set(uint8_t keycode)
{
  unsigned char      i, k, b;
  uint8_t              buf[EDSTLEN];
  TIME_STRUCT        mqx_time;
  DATE_STRUCT        date_time;
  uint32_t           rtc_time;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  mcbl->_printf(VT100_CLEAR_AND_HOME);

  VT100_send_str_to_pos("SYSTEM TIME SETTING", 1, 30);
  VT100_send_str_to_pos("\033[5C <M> - Display main menu, <Esc> - Exit \r\n", 2, 10);
  VT100_send_str_to_pos("Print in form [YY.MM.DD dd HH.MM.SS]:  .  .        :  :  ", SCR_ITEMS_VERT_OFFS, 1);

  mcbl->beg_pos = 38;
  k = 0;
  mcbl->curr_pos = mcbl->beg_pos;
  VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
  mcbl->_printf((char *)VT100_CURSOR_ON);

  for (i = 0; i < EDSTLEN; i++) buf[i] = 0;

  do
  {
    if (mcbl->_wait_char(&b, 200) == MQX_OK)
    {
      switch (b)
      {
      case 0x08:  // Back Space
        if (mcbl->curr_pos > mcbl->beg_pos)
        {
          mcbl->curr_pos--;
          k--;
          switch (k)
          {
          case 2:
          case 5:
          case 8:
          case 11:
          case 14:
          case 17:
            k--;
            mcbl->curr_pos--;
            break;
          }

          VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          mcbl->_printf((char *)" ");
          VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          buf[k] = 0;
        }
        break;
      case 0x7E:  // DEL
        mcbl->curr_pos = mcbl->beg_pos;
        k = 0;
        for (i = 0; i < EDSTLEN; i++) buf[i] = 0;
        VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->beg_pos);
        mcbl->_printf((char *)"  .  .        :  :  ");
        VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->beg_pos);
        break;
      case 0x1B:  // ESC
        mcbl->_printf((char *)VT100_CURSOR_OFF);
        return;
      case 'M':  //
      case 'm':  //
        mcbl->_printf((char *)VT100_CURSOR_OFF);
        return;

      case 0x0D:  // Enter
        mcbl->_printf((char *)VT100_CURSOR_OFF);

        date_time.YEAR     = BCD2ToBYTE((buf[0] << 4) + buf[1]) + 2000;
        date_time.MONTH    = BCD2ToBYTE((buf[3] << 4) + buf[4]);
        date_time.DAY      = BCD2ToBYTE((buf[6] << 4) + buf[7]);
        date_time.HOUR     = BCD2ToBYTE((buf[12] << 4) + buf[13]);
        date_time.MINUTE   = BCD2ToBYTE((buf[15] << 4) + buf[16]);
        date_time.SECOND   = BCD2ToBYTE((buf[18] << 4) + buf[19]);
        date_time.MILLISEC = 0;

        if (_time_from_date(&date_time, &mqx_time) == FALSE)
        {
          mcbl->_printf("\r\nCannot convert date_time!");
          _time_delay(3000);
          return;
        }
        rtc_time = mqx_time.SECONDS;
        _time_set(&mqx_time);      // Установка времени RTOS
        _rtc_set_time(rtc_time);   // Установка времени RTC
        return;
      default:
        if (isdigit(b))
        {
          if (k < EDSTLEN)
          {
            uint8_t str[2];
            str[0] = b;
            str[1] = 0;
            mcbl->_printf((char *)str);
            buf[k] = b - 0x30;
            mcbl->curr_pos++;
            k++;
            switch (k)
            {
            case 2:
            case 5:
            case 8:
            case 11:
            case 14:
            case 17:
              k++;
              mcbl->curr_pos++;
              break;
            }
            VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS, mcbl->curr_pos);
          }
        }
        break;

      } // switch
    }
  }
  while (1);
}



/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_show_event_log(uint8_t keycode)
{
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  AppLogg_monitor_output(mcbl);
}


#ifdef SFFS_TEST
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_SFFS_test(uint8_t keycode)
{
  uint8_t             b;
  unsigned int      en_format = 0;
  unsigned int      en_erase = 0;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  mcbl->_printf(VT100_CLEAR_AND_HOME);
  do
  {
    mcbl->_printf(" ---+++===  Serial Flash System Test ===+++---\n\r");
    mcbl->_printf("Press 'A' to start test, Press 'S' to get statistic, 'R'- exit.\n\r");
    mcbl->_printf("'F'- format before test, 'E' - erase after test\n\r");
    mcbl->_printf(DASH_LINE);
    mcbl->_printf("Format = %d, Erase = %d\n\r", en_format, en_erase);
    mcbl->_printf(DASH_LINE);

    if (mcbl->_wait_char(&b, 2000) == MQX_OK)
    {
      switch (b)
      {
      case 'F':
      case 'f':
        en_format ^= 1;
        break;
      case 'E':
      case 'e':
        en_erase ^= 1;
        break;
      case 'A':
      case 'a':
        SFFS_test(en_format, en_erase, 0);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'S':
      case 's':
        SFFS_test(en_format, en_erase, 1);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'R':
      case 'r':
        return;
      default:
        break;

      }
    }
  }
  while (1);

}
#endif

#ifdef MFS_TEST

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static unsigned int  Print_MFS_test_header(T_monitor_cbl *mcbl, T_mfs_test *p)
{
  mcbl->_printf(VT100_CLEAR_AND_HOME);
  mcbl->_printf(" ===  MFS System Test ===\n\r");
  mcbl->_printf("Press 'A'- test 1, 'B'- test 2, 'D'- test 3, 'R'- exit.\n\r");
  mcbl->_printf("'Q' - existing files read test\n\r");
  mcbl->_printf(DASH_LINE);
  mcbl->_printf("<F>ormat = %d, <E>rase = %d, Read <I>terat. = %d, Files <C>nt= %d, File s<Z>. = %d\n\r", p->en_format, p->en_erase, p->read_cicles, p->files_cnt, p->file_sz);
  mcbl->_printf(DASH_LINE);
  return 6;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_MFS_test(uint8_t keycode)
{
  uint8_t             b;
  uint8_t             row;

  T_mfs_test cbl;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  cbl.en_erase = 0;
  cbl.en_format = 0;
  cbl.file_sz = 10000;
  cbl.files_cnt = 10;
  cbl.read_cicles = 1;
  row = Print_MFS_test_header(mcbl, &cbl);

  do
  {

    if (mcbl->_wait_char(&b, 2) == MQX_OK)
    {
      switch (b)
      {
      case 'F':
      case 'f':
        cbl.en_format ^= 1;
        Print_MFS_test_header(mcbl, &cbl);
        break;
      case 'E':
      case 'e':
        cbl.en_erase ^= 1;
        Print_MFS_test_header(mcbl, &cbl);
        break;
      case 'I':
      case 'i':
        Print_MFS_test_header(mcbl, &cbl);
        Edit_integer_val(row + 1, &cbl.read_cicles, 1, 10000);
        Print_MFS_test_header(mcbl, &cbl);
        break;
      case 'C':
      case 'c':
        Print_MFS_test_header(mcbl, &cbl);
        Edit_integer_val(row + 1, &cbl.files_cnt, 1, 1000000000);
        Print_MFS_test_header(mcbl, &cbl);
        break;
      case 'Z':
      case 'z':
        Print_MFS_test_header(mcbl, &cbl);
        Edit_integer_val(row + 1, &cbl.file_sz, 1, 50000);
        Print_MFS_test_header(mcbl, &cbl);
        break;
      case 'A':
      case 'a':
        cbl.info = 0;
        cbl.write_test = 1;
        MFS_test1(&cbl);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'B':
      case 'b':
        MFS_test2(&cbl);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'D':
      case 'd':
        MFS_test3(&cbl);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'Q':
      case 'q':
        cbl.info = 0;
        cbl.write_test = 0;
        MFS_test1(&cbl);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'S':
      case 's':
        cbl.info = 1;
        cbl.write_test = 1;
        MFS_test1(&cbl);
        mcbl->_printf("\n\r\n\r");
        break;
      case 'R':
      case 'r':
        return;
      default:
        break;

      }
    }
  }
  while (1);

}
#endif


#ifdef MQX_SHELL
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_Shell(uint8_t keycode)
{
  uint8_t             b;
  _task_id            task_id;
  T_monitor_driver    *drv;
  T_monitor_cbl       *mcbl;

  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  drv = (T_monitor_driver *)_task_get_parameter();

  mcbl->_printf(VT100_CLEAR_AND_HOME);

  if (drv->driver_type == MN_STD_SERIAL_DRIVER)
  {
    task_id = _task_create(0, SHELL_IDX, 0);
    // Переходим к задаче
    if (task_id != MQX_NULL_TASK_ID)
    {
      while (1)
      {
        if (_task_get_td(task_id) == NULL) return;
        _time_delay(100);
      }
    }
  }
  else
  {
    mcbl->_printf("Unable open service with this driver.\n\r");
    mcbl->_printf("Press any key to continue...\n\r");
    while (mcbl->_wait_char(&b, 2000) != MQX_OK);
  }
}
#endif


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_VBAT_RAM_diagnostic(uint8_t keycode)
{
  uint8_t  b;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());

  do
  {
    mcbl->_printf(VT100_CLEAR_AND_HOME);
    mcbl->_printf("VBAT RAM diagnostic.\n\r");
    mcbl->_printf("Press 'R' to exit. \n\r");
    mcbl->_printf(DASH_LINE);
    Mon_print_dump((uint32_t)VBAT_RAM_ptr, VBAT_RAM_ptr, sizeof(T_VBAT_RAM), 8);

    if (mcbl->_wait_char(&b, 2000) == MQX_OK)
    {
      switch (b)
      {
      case 'S':
      case 's':
        K66BLEZ1_VBAT_RAM_sign_data();
        break;
      case 'V':
      case 'v':
        if (K66BLEZ1_VBAT_RAM_validation() == MQX_OK)
        {
          mcbl->_printf("Data Ok.\n\r");
        }
        else
        {
          mcbl->_printf("Data incorrect!.\n\r");
        }
        _time_delay(1000);
        break;
      case 'R':
      case 'r':
        return;
      }
    }
  }
  while (1);
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static void Do_watchdog_test(uint8_t keycode)
{
  uint8_t  b;
  T_monitor_cbl *mcbl;
  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf(VT100_CLEAR_AND_HOME"Misc. test.\n\r");
  mcbl->_printf("Press 'R' to exit. \n\r");

//   Тестирование кодирования и декодирования Base64
//
//   {
//   int i, n;
//   char plainbuf[16];
//   char encbuf[32];
//   char encbuf2[48];
//
//
//   mcbl->_printf("Plain buf: \n\r");
//   for (i = 0; i < sizeof(plainbuf); i++)
//   {
//     plainbuf[i] = rand();
//     mcbl->_printf("%02X ", plainbuf[i]);
//   }
//   mcbl->_printf("\r\nEncoded buf len: %d\n\r", Base64encode_len(sizeof(plainbuf)));
//
//   n = Base64encode(encbuf, plainbuf, sizeof(plainbuf));
//   mcbl->_printf("Encoded: %d\n\r",n);
//   mcbl->_printf("\r\nEncoded buf: %s\n\r", encbuf);
//   Base64encode(encbuf2, encbuf, n-1);
//   mcbl->_printf("\r\nSecond encoded buf: %s\n\r", encbuf2);
//
//   n = Base64decode_len(encbuf);
//   mcbl->_printf("\r\n\r\nDecoded buf len: %d\n\r", n);
//   n = Base64decode(plainbuf, encbuf);
//   mcbl->_printf("Decoded %d\n\r",i);
//   mcbl->_printf("Decoded buf: \n\r");
//   for (i = 0; i < n; i++)
//   {
//     mcbl->_printf("%02X ", plainbuf[i]);
//   }
//   mcbl->_printf("\r\n");
// }


// Тестирование работы watchdog
  {
    unsigned int i;
    _int_disable();
    for (i = 0; i < 1000000000ul; i++)
    {
      __no_operation();
    }
    _int_enable();
  }


  do
  {
    if (mcbl->_wait_char(&b, 200) == MQX_OK)
    {
      switch (b)
      {
      case 'R':
      case 'r':
        return;
      }
    }
  }
  while (1);
}

#define PT_ARR_SZ 2000
void *pr_arr[PT_ARR_SZ];
/*-----------------------------------------------------------------------------------------------------
  Тестирование быстродействия функции malloc
   
-----------------------------------------------------------------------------------------------------*/
static void Do_malloc_test(uint8_t keycode)
{
  uint8_t  b;
  T_monitor_cbl *mcbl;

  uint32_t i, n;
  uint32_t sz;
  uint32_t tot;
  void *pt;


  mcbl = (T_monitor_cbl *)_task_get_environment(_task_get_id());
  mcbl->_printf(VT100_CLEAR_AND_HOME"Malloc test. 1 - init fill, 2 - free all\n\r");
  mcbl->_printf("Press 'R' to exit. \n\r");

  

    tot = 0;
    n = 0;
    

  


  do
  {
    if (mcbl->_wait_char(&b, 200) == MQX_OK)
    {
      switch (b)
      {
      case '1':
        // Начальное заполнение
        for (i=0;i<PT_ARR_SZ;i++)
        {
          // Начальное заполнение
          sz = rand()&0xFF;
           _int_disable();
          Set_LED_voltage(1,0);
          pt = malloc(sz);
          Set_LED_voltage(0,0);
           _int_enable();
          _time_delay(10);
          if (pt!=NULL)
          {
            pr_arr[i] = pt;
            tot += sz;
            n++; 
          }
          else
          {
            break;
          }
        }
        mcbl->_printf("..................................... \n\r");
        mcbl->_printf("Allocated %d blocks, Total size = %d \n\r", n, tot );
        break;
      case '2':
        // Полное освобождение 
        for (i=0;i<n;i++)
        {
          // Начальное заполнение
           _int_disable();
          Set_LED_voltage(1,0);
          free(pr_arr[i]);
          Set_LED_voltage(0,0);
           _int_enable();
          _time_delay(10);
        }
        tot = 0;
        n = 0;
        mcbl->_printf("..................................... \n\r");
        mcbl->_printf("Freed %d blocks, Total size = %d \n\r", n, tot );
        break;

      case 'R':
      case 'r':
        return;
      }
    }
  }
  while (1);
}

/*------------------------------------------------------------------------------
 Сброс системы
 ------------------------------------------------------------------------------*/
static void Do_Reset(uint8_t keycode)
{
  Reset_system();
}
