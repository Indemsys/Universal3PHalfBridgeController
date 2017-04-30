// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2015.11.11
// 09:24:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern void     FMSTR_Recorder(void);
/*------------------------------------------------------------------------------
 Задача сброса watchdog и наблюдения за работоспособностью системы


 \param initial_data
 ------------------------------------------------------------------------------*/
void Task_supervisor(unsigned int initial_data)
{
  uint32_t   btn_cnt = 0;
  uint32_t   tcnt    = 0;
  _mqx_uint  tps;

  do
  {
    // Проверить нажатие кнопки
    //if (GPIOA_PDIR &  BIT(8))
    //{
    //  // Нажата кнопка подачи питания
    //  btn_cnt++;
    //  if (btn_cnt > 5)
    //  {
    //    GPIOE_PCOR = BIT(27); // Выключаем поддержку ключа питания
    //  }
    //}

    LEDS_state_automat();
    
    if (wvar.led_strip_en)
    {
      WS2812B_periodic_refresh();
    }

    tps = _time_get_ticks_per_sec();
    tcnt++;
    if (((tcnt * 1000ul) / tps) > SUPERVISOR_TIMEOUT)
    {
      WatchDog_refresh();
      tcnt = 0;
    }

    _time_delay_ticks(1);

    if (g_fmstr_rate_src == FMSTR_SMPS_SUPERVISOR_TASK)
    {
      FMSTR_Recorder(); // Вызываем функцию записи сигнала для инструмента FreeMaster
    }


  }
  while (1);
}



/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Write_start_log_rec(void)
{
  RCM_MemMapPtr RCM =  RCM_BASE_PTR;
  unsigned int uid[4];
  LOG("##################### START #####################", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);

  _bsp_get_unique_identificator(uid);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT, "UID     : %08X %08X %08X %08X", uid[0], uid[1], uid[2], uid[3]);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT, "UID16BIT: %08X",  uid[0] ^ uid[1] ^ uid[2] ^ uid[3]);
  // Записать версию
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT, "Firmware compile time: %s %s", __DATE__, __TIME__);

  {

    unsigned int v;
    // Запись причины сброса
    v = RCM->SRS0;

    if (v & BIT(7))
    {
      LOG("Reset caused by POR", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(6))
    {
      LOG("Reset caused by external reset pin", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(5))
    {
      LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT, "Reset caused by watchdog timeout (%d)", WatchDog_get_counter());
    }
    if (v & BIT(2))
    {
      LOG("Reset caused by LVD trip or POR", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(0))
    {
      LOG("Reset caused by LLWU module wakeup source", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }

    v = RCM->SRS1;
    if (v & BIT(5))
    {
      LOG("Reset caused by peripheral failure to acknowledge attempt to enter stop mode", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(4))
    {
      LOG("Reset caused by EzPort receiving the RESET command while the device is in EzPort mode", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(3))
    {
      LOG("Reset caused by host debugger system setting of the System Reset Request bit", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(2))
    {
      LOG("Reset caused by software setting of SYSRESETREQ bit", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(1))
    {
      LOG("Reset caused by core LOCKUP event", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
    if (v & BIT(0))
    {
      LOG("Reset caused by JTAG", __FUNCTION__, __LINE__, SEVERITY_DEFAULT);
    }
  }

}



