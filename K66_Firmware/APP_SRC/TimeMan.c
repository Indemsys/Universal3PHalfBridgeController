#include "App.h"

HWTIMER hwtimer1;
unsigned int tmodulo;
unsigned int tperiod;
#define HWTIMER1_FREQUENCY 1

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
_mqx_int TimeManInit(void)
{
  if (MQX_OK != hwtimer_init(&hwtimer1, &BSP_HWTIMER1_DEV, BSP_HWTIMER1_ID, (BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX + 1)))
  {
      return MQX_ERROR;
  }
  hwtimer_set_freq(&hwtimer1, BSP_HWTIMER1_SOURCE_CLK, HWTIMER1_FREQUENCY);

  tmodulo = hwtimer_get_modulo(&hwtimer1);
  tperiod = hwtimer_get_period(&hwtimer1);

  hwtimer_start(&hwtimer1);

  return MQX_OK;
}

/*-------------------------------------------------------------------------------------------------------------
  Получить значение времени
-------------------------------------------------------------------------------------------------------------*/
void Get_time_counters(HWTIMER_TIME_STRUCT *t)
{
  hwtimer_get_time(&hwtimer1, t);
}

/*-------------------------------------------------------------------------------------------------------------
  Вычисление разницы во времени в мкс
-------------------------------------------------------------------------------------------------------------*/
uint32_t Eval_meas_time(HWTIMER_TIME_STRUCT t1,HWTIMER_TIME_STRUCT t2)
{
  unsigned int t;
  unsigned long long  tt;

  t2.TICKS = t2.TICKS - t1.TICKS;
  if ( t2.SUBTICKS < t1.SUBTICKS )
  {
    t2.TICKS = t2.TICKS - 1;
    t2.SUBTICKS = t1.SUBTICKS - t2.SUBTICKS;
  }
  else
  {
    t2.SUBTICKS = t2.SUBTICKS - t1.SUBTICKS;
  }
  t = t2.TICKS * tperiod;
  tt = (unsigned long long)t2.SUBTICKS * (unsigned long long)tperiod;
  t = t + tt/tmodulo;
  return t;
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
uint32_t  Get_usage_time(void)
{
  HWTIMER_TIME_STRUCT t1,t2;

  Get_time_counters(&t1);
  DELAY_ms(100);
  //us_Delay(2079999u);
  Get_time_counters(&t2);

  return Eval_meas_time(t1,t2);
}


