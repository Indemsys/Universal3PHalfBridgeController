// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2015.11.11
// 08:20:38
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

volatile unsigned int wdt_snapshot;
/*------------------------------------------------------------------------------
  Прерывание от watchdog-а

 \param user_isr_ptr
 ------------------------------------------------------------------------------*/
static void WDT_isr(void *user_isr_ptr)
{
  wdt_snapshot = WatchDog_get_timeout_reg();
}

/*------------------------------------------------------------------------------
  Процедура прерывания от watchdog

 ------------------------------------------------------------------------------*/
void ivINT_Watchdog(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  wdt_snapshot = WatchDog_get_timeout_reg();

  WDOG->UNLOCK = 0xC520; // Откроем доступ на запись в регитры управления WDOG
  WDOG->UNLOCK = 0xD928;
  __no_operation();
  WDOG->STCTRLL = BIT(15);
}
/*------------------------------------------------------------------------------
  Инициализируем watchdog

    Разрешаем работу WDT.
    Тактируем от LPO (1 kHz).
    Сброс будет генерироваться без предварительного прерывания
    Включаем режим окна.

 \param wdt_timeout - время (в мс )по истечении которого watchdog выполнит сброс системы
 \param wdt_win     - время (в мс) до истечения котолрого нельзя делать refresh watchdog-а
 ------------------------------------------------------------------------------*/
void WatchDog_init(unsigned int wdt_timeout, unsigned int wdt_win)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  _int_disable();

  _bsp_int_init(INT_Watchdog, WDT_ISR_PRIO, 0, TRUE);

  WDOG->UNLOCK = 0xC520; // Откроем доступ на запись в регитры управления WDOG
  WDOG->UNLOCK = 0xD928;
  __no_operation();

  WDOG->STCTRLH = 0
                  + LSHIFT(0x01, 14) // DISTESTWDOG | Allows the WDOG’s functional test mode to be disabled permanently| 0 WDOG functional test mode is not disabled.
                  + LSHIFT(0x00, 12) // BYTESEL[1:0]| This 2-bit field select the byte to be tested ...                | 00 Byte 0 selected
                  + LSHIFT(0x00, 11) // TESTSEL     | Selects the test to be run on the watchdog timer                 | 0 Quick test
                  + LSHIFT(0x00, 10) // TESTWDOG    | Puts the watchdog in the functional test mode                    |
                  + LSHIFT(0x01, 8)  // Reserved    |
                  + LSHIFT(0x01, 7)  // WAITEN      | Enables or disables WDOG in wait mode.                           | 1 WDOG is enabled in CPU wait mode.
                  + LSHIFT(0x01, 6)  // STOPEN      | Enables or disables WDOG in stop mode                            | 1 WDOG is enabled in CPU stop mode.
                  + LSHIFT(0x00, 5)  // DBGEN       | Enables or disables WDOG in Debug mode                           | 0 WDOG is disabled in CPU Debug mode.
                  + LSHIFT(0x01, 4)  // ALLOWUPDATE | Enables updates to watchdog write once registers                 | 1 WDOG write once registers can be unlocked for updating
                  + LSHIFT(0x01, 3)  // WINEN       | Enable windowing mode.                                           | 0 Windowing mode is disabled.
                  + LSHIFT(0x01, 2)  // IRQRSTEN    | Used to enable the debug breadcrumbs feature                     | 0 WDOG time-out generates reset only.
                  + LSHIFT(0x00, 1)  // CLKSRC      | Selects clock source for the WDOG                                | 1 WDOG clock sourced from alternate clock source
                  + LSHIFT(0x01, 0)  // WDOGEN      | Enables or disables the WDOG’s operation                         | 1 WDOG is enabled.
  ;
  // Предделитель устанавливаем в 0
  WDOG->PRESC = 0;

  // Программируем тайм-аут WDT
  WDOG->TOVALH = (wdt_timeout >> 16);
  WDOG->TOVALL = wdt_timeout & 0xFFFF;

  // Программируем тайм-аут WDT
  WDOG->WINH = (wdt_win >> 16);
  WDOG->WINL = wdt_win & 0xFFFF;


  _int_enable();

}

/*------------------------------------------------------------------------------
  Сброс таймера WDT
 ------------------------------------------------------------------------------*/
void WatchDog_refresh(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;

  // На время сброса WDT должны быть запрещены все прерываний и RTOS и уровня ядра
  __disable_interrupt();
  WDOG->REFRESH = 0xA602;
  WDOG->REFRESH = 0xB480;
  __enable_interrupt();
}

/*------------------------------------------------------------------------------
  Получить счетчик сбросов сгенерированных WDT
 ------------------------------------------------------------------------------*/
unsigned short WatchDog_get_counter(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;
  return WDOG->RSTCNT;
}

/*------------------------------------------------------------------------------
  Получить счетчик таймаута WDT
 ------------------------------------------------------------------------------*/
unsigned int WatchDog_get_timeout_reg(void)
{
  WDOG_MemMapPtr WDOG = WDOG_BASE_PTR;
  return (WDOG->TMROUTH << 16) | WDOG->TMROUTL ;
}
