// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-03-01
// 13:57:27
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

#define FMCMD_ROTATION_START         40
#define FMCMD_ROTATION_BREAKE        41
#define FMCMD_ROTATION_FORCEBREAKE   42
#define FMCMD_ROTATION_FREERUN       43


static LWEVENT_STRUCT rot_lwev;

/*-----------------------------------------------------------------------------------------------------
 
 \param flags 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_Rot_post_evt(_mqx_uint evts)
{
  _lwevent_set(&rot_lwev, evts);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _ROTC_Rotation_Start(void)
{
  rot_cbl.cmd_flags = ROT_CMD_START;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _ROTC_Rotation_Brake(void)
{
  rot_cbl.cmd_flags = ROT_CMD_BREAK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _ROTC_Rotation_ForceBrake(void)
{
  rot_cbl.cmd_flags = ROT_CMD_FORCE_BREAK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _ROTC_Rotation_FreeRun(void)
{
  rot_cbl.cmd_flags = ROT_CMD_FREERUN;
}

/*-----------------------------------------------------------------------------------------------------
  Устанавоиваем пассивное торможение закорачиванием обмоток
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Start_Break_phase(void)
{
  BLDC_break();
}

/*-----------------------------------------------------------------------------------------------------
  Включаем активное торможение устанавливая постояннй уровень напряжения на обмотках 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Start_Force_Break_phase(void)
{
  BLDC_force_break(wvar.rot_fbreak_pwm_lev);
}

/*-----------------------------------------------------------------------------------------------------
  Отключаем драйвер от обмоток, разрешаем свободное вращение двигателя 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Start_FreeRun_phase(void)
{
  BLDC_Free_Run();
}

/*-----------------------------------------------------------------------------------------------------
  Инициализируем линейное наращивание мощности на двигателе выбирая начальный люфт 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Start_rotation(void)
{
  if (g_bldc_active == 0)
  {
    rot_cbl.step_cnt   = 0;
    rot_cbl.start_pos  = hccbl.hall_counter;
    if (wvar.rot_dir == CW_MOTOR_ROTATION)
    {
      rot_cbl.end_pos = rot_cbl.start_pos + wvar.rot_steps;
    }
    else
    {
      rot_cbl.end_pos = rot_cbl.start_pos - wvar.rot_steps;
    }

    if (BLDC_start_rotation(wvar.rot_dir, wvar.rot_pwm_lev) != MQX_OK)
    {
      _Start_Break_phase();
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
  Слежение за углом поворота по датчику Холла
 
  Вызывается из Hall_sensor_kernel_isr по событию изменения состояния датчиков Холла
  Прерывание уровня ядра!!!
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Rotation_angle_control(void)
{
  if (wvar.rot_steps != 0)
  {
    if (rot_cbl.end_pos == hccbl.hall_counter)
    {
      _Start_Break_phase();
      return RES_ERROR;
    }
  }
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------
  Процедура управления скоростью путем регулировки скважности ШИМ
  Вызывается из Hall_sensor_kernel_isr по событию переполения таймера 
  Прерывание уровня ядра!!!
 
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Rotation_control(void)
{
  //..................................................................................
  // Отрабатываем команды
  //..................................................................................
  switch (rot_cbl.cmd_flags)
  {

  case ROT_CMD_BREAK:
    _Start_Break_phase();
    break;

  case ROT_CMD_FORCE_BREAK:
    _Start_Force_Break_phase();
    break;

  case ROT_CMD_FREERUN:
    _Start_FreeRun_phase();
    break;

  case ROT_CMD_START:
    _Start_rotation();
    break;
  }
  if (rot_cbl.cmd_flags != 0) rot_cbl.last_cmd_flags = rot_cbl.cmd_flags;
  rot_cbl.cmd_flags = 0;

  //..................................................................................
  // Отрабатываем состояния процесса если таковые есть
  //..................................................................................
  rot_cbl.step_cnt++;


  if (BLDC_overload_observer() != 0)
  {
    _ROTC_Rotation_Brake();
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _ROTC_load_app_params(void)
{
  hccbl.speed_filter_en                    = wvar.en_speed_fltr;
  bldcs_cbl.dir_inv                        = 0;

  ovrc_cbl.max_current                     = wvar.max_curr;
  ovrc_cbl.max_long_current                = wvar.max_long_curr;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param app_command 
 \param res 
 
 \return int32_t 
-----------------------------------------------------------------------------------------------------*/
static int32_t U3HB01_Rotation_freemaster_cmd_man(uint16_t app_command, uint8_t  *res)
{
  int32_t ret = 0;

  switch (app_command)
  {
  case  FMCMD_ROTATION_START:
    U3HB01_Rot_post_evt(EVT_ROTATION_ON);
    res = 0;
    break;
  case  FMCMD_ROTATION_BREAKE:
    U3HB01_Rot_post_evt(EVT_ROTATION_OFF);
    res = 0;
    break;

  case  FMCMD_ROTATION_FORCEBREAKE:
    U3HB01_Rot_post_evt(EVT_ROTATION_FORCEBRAKE);
    break;

  case  FMCMD_ROTATION_FREERUN:
    U3HB01_Rot_post_evt(EVT_ROTATION_FREERUN);
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
uint32_t U3HB01_Rotation_control(void)
{
  // Инициализируем объект синхронизации задачи
  if (_lwevent_create(&rot_lwev, LWEVENT_AUTO_CLEAR) != MQX_OK) return RES_ERROR;

  ovrc_cbl.overload_flags = NO_OVERLOAD;

  ExtEnc_init(); // Инициализируем работу с внешним датчиком угла поворота/энкодером

  FTM_init_PWM_DMA(FTM0_BASE_PTR); // Инициализируем PWM генератор для работы со светодиодной лентой на WS2812B

  _ROTC_load_app_params();
  U3HB01_Set_freemaster_cmd_man(U3HB01_Rotation_freemaster_cmd_man);

  Set_adc_isr_filtering_callback(U3HB01_ADC_processing); // Устанавливаем процедуру фильтрации сигналов с ADC

  FTM3_init_motor_PWM();
  _time_delay(10);      // Даем время на установдение регистров таймера по событию c компараторов PWM

  FTM1_FTM2_init_3ph_hal_sens_measl(); // Запускаем прерывания от таймеров измерителей сигналов с датчиков Холла

  BLDC_break();         // Включаем драйвера с закорачиванием обмоток мотора на землю, активизируем измерители тока чтобы провести калибровку

  BLDC_set_PWM_sync_signal();

  if (DRV8305_init() != MQX_OK)
  {
    return MQX_ERROR;
  }

  _time_delay(10);      // Даем время на включение драйверов

  DRV8305_calibrate();  // Калибруем смещение у усилителей измерителей тока

  Set_adc_isr_asyncs_ctrl_callback(0);                              // События по прерываниям от ADС не используем
  Set_hall_isr_commutation_callback(BLDC_commutation_by_hall_sens); // Установливаем вызов процедуры коммутации по датчикам Холла
  Set_hall_isr_speed_ctrl_callback(_Rotation_control);              // Установливаем вызов процедуры управления скоростью путем регулировки скважности ШИМ
  Set_hall_isr_position_ctrl_callback(_Rotation_angle_control);     // Слежение за углом поворота



  for (;;)
  {

    // Ждем любого флага в течении одного тика
    if (_lwevent_wait_ticks(&rot_lwev, 0xFFFFFFFF, FALSE, 1) == MQX_OK)
    {
      _mqx_uint evts;
      evts = _lwevent_get_signalled();
      if (evts & EVT_ROTATION_ON)
      {
        _ROTC_Rotation_Start();
      }
      else if (evts & EVT_ROTATION_OFF)
      {
        _ROTC_Rotation_Brake();
      }
      else if (evts & EVT_ROTATION_FORCEBRAKE)
      {
        _ROTC_Rotation_ForceBrake();
      }
      else if (evts & EVT_ROTATION_FREERUN)
      {
        _ROTC_Rotation_FreeRun();
      }

      if (evts & EVT_ROTATION_ENC_DAT_READY)
      {
        ExtEnc_Read_angle();
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

