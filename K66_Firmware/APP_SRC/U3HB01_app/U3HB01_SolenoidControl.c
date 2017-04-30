// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-02-28
// 14:05:04
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define FMCMD_SOLENOID_ON   30
#define FMCMD_SOLENOID_OFF  31


static LWEVENT_STRUCT slnd_lwev;

float                 slnd_pwr;
uint8_t               slnd_en;

/*-----------------------------------------------------------------------------------------------------
 
 \param flags 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_Slnd_post_evt(_mqx_uint evts)
{
  _lwevent_set(&slnd_lwev, evts);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void MDC01_Solenoid_ON(void)
{
  uint32_t mod;

  slnd_en = 1;
  mod = Get_modulation_from_percent(slnd_pwr);
  mot_PWM_lev = (int16_t)slnd_pwr;
  FTM3_set_CnV(mod);

  // Программно устанавливаем ключи
  FTM3_SWOCTRL = 0
                 + LSHIFT(0, 15) // CH7OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 14) // CH6OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(1, 13) // CH5OCV | 1 The software output control forces 1 to the channel output.   Нижний ключ канала  C
                 + LSHIFT(0, 12) // CH4OCV | 1 The software output control forces 1 to the channel output.   Верхний ключ канала C
                 + LSHIFT(0, 11) // CH3OCV | 1 The software output control forces 1 to the channel output.   Нижний ключ канала  B
                 + LSHIFT(0, 10) // CH2OCV | 1 The software output control forces 1 to the channel output.   Верхний ключ канала B
                 + LSHIFT(1, 9)  // CH1OCV | 1 The software output control forces 1 to the channel output.   Нижний ключ канала  A
                 + LSHIFT(0, 8)  // CH0OCV | 1 The software output control forces 1 to the channel output.   Верхний ключ канала A
                 + LSHIFT(0, 7)  // CH7OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 6)  // CH6OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(1, 5)  // CH5OC  | 1 The channel output is affected by software output control.    Нижний ключ канала  C
                 + LSHIFT(1, 4)  // CH4OC  | 1 The channel output is affected by software output control.    Верхний ключ канала C
                 + LSHIFT(1, 3)  // CH3OC  | 1 The channel output is affected by software output control.    Нижний ключ канала  B
                 + LSHIFT(0, 2)  // CH2OC  | 1 The channel output is affected by software output control.    Верхний ключ канала B
                 + LSHIFT(1, 1)  // CH1OC  | 1 The channel output is affected by software output control.    Нижний ключ канала  A
                 + LSHIFT(1, 0)  // CH0OC  | 1 The channel output is affected by software output control.    Верхний ключ канала A
  ;
  FTM3_OUTMASK = 0;    // Отключаем маскирование выходов. Все выходы становятся активными
  FTM3_INVCTRL = 0;    // Отключаем инверсию

  BLDC_set_PWM_sync_signal();

  DRV8305_Gates_enable();

}
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void MDC01_Solenoid_OFF(void)
{
  slnd_en = 0;

  BLDC_break();
  BLDC_set_PWM_sync_signal();

}

/*-----------------------------------------------------------------------------------------------------
 
 \param app_command 
 \param res 
 
 \return int32_t 
-----------------------------------------------------------------------------------------------------*/
static int32_t U3HB01_Solenoid_freemaster_cmd_man(uint16_t app_command, uint8_t  *res)
{
  int32_t ret = 0;

  switch (app_command)
  {
  case  FMCMD_SOLENOID_ON:
    U3HB01_Slnd_post_evt(EVT_SOLENOID_ON);
    res = 0;
    break;
  case  FMCMD_SOLENOID_OFF:
    U3HB01_Slnd_post_evt(EVT_SOLENOID_OFF);
    res = 0;
    break;
  default:
    ret = -1;
    *res = 0;
    break;
  }
  return ret;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t U3HB01_Solenoid_control(void)
{
  // Инициализируем объект синхронизации задачи
  if (_lwevent_create(&slnd_lwev, LWEVENT_AUTO_CLEAR) != MQX_OK) return RES_ERROR;

  Set_adc_isr_filtering_callback(U3HB01_ADC_processing); // Устанавливаем процедуру фильтрации сигналов с ADC

  U3HB01_Set_freemaster_cmd_man(U3HB01_Solenoid_freemaster_cmd_man);

  slnd_pwr = 25;

  FTM3_init_motor_PWM();
  _time_delay(10);      // Даем время на установдение регистров таймера по событию c компараторов PWM
  BLDC_break();         // Включаем драйвера с закорачиванием обмоток мотора на землю, активизируем измерители тока чтобы провести калибровку
  BLDC_set_PWM_sync_signal();

  if (DRV8305_init() != MQX_OK)
  {
    return MQX_ERROR;
  }
  _time_delay(10);      // Даем время на включение драйверов

  DRV8305_calibrate();  // Калибруем смещение у усилителей измерителей тока

  for (;;)
  {

    // Ждем любого флага в течении одного тика
    if (_lwevent_wait_ticks(&slnd_lwev, 0xFFFFFFFF, FALSE, 1) == MQX_OK)
    {
      _mqx_uint evts;
      evts = _lwevent_get_signalled();
      if (evts & EVT_SOLENOID_ON)
      {
        MDC01_Solenoid_ON();
      }
      else if (evts & EVT_SOLENOID_OFF)
      {
        MDC01_Solenoid_OFF();
      }

    }
    else
    {
      // Пересчитаем аналоговые сигналы в значения с плавающей точкой
      int_temp   = U3HB01_Convert_int_temp(adcs.int_temp_filtered);
      drv_temp   = U3HB01_Convert_ext_temp(adcs.drv_temp_filtered);
      motor_temp = U3HB01_Convert_ext_mot_temp(adcs.mot_temp_filtered);
      dc_voltage = U3HB01_Convert_vdc_temp(adcs.dc_volt_filtered);
    }
  }

}


