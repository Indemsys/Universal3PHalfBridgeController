// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 15:30:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define   _APP_GLOBALS_
// 2016-12-06
#include   "App.h"



T_median_filter_uint16 drv_temp_filter;
T_median_filter_uint16 mot_temp_filter;
T_median_filter_uint16 int_temp_filter;
T_median_filter_uint16 dc_volt_filter;




/*-----------------------------------------------------------------------------------------------------
 
 \param val 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float U3HB01_Convert_int_temp(uint16_t val)
{
  return (25 - ((val * VREF / ADC_PREC) - 0.719) / 0.001715);
}
/*-----------------------------------------------------------------------------------------------------
 
 \param val 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float U3HB01_Convert_ext_temp(uint16_t val)
{
  #define   B1  (3988.0)
  #define   r0  (10000.0)
  float r;
  r = val * 10000.0 / (0xFFF - val);
  r = (25 + 273.15) / ((log(r / r0) * (25 + 273.15)) / B1 + 1) - 273.15;

  return r;
}

/*-----------------------------------------------------------------------------------------------------
  Расчет температуры на термисторе мотора 
 
 \param val 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float U3HB01_Convert_ext_mot_temp(uint16_t val)
{
  #define   B1  (3988.0)
  #define   r0  (10000.0)
  float r;
  r = val * 10000.0 / (0xFFF - val);
  r = (25 + 273.15) / ((log(r / r0) * (25 + 273.15)) / B1 + 1) - 273.15;

  return r;
}
/*-----------------------------------------------------------------------------------------------------
 
 \param val 
 
 \return float 
-----------------------------------------------------------------------------------------------------*/
float U3HB01_Convert_vdc_temp(uint16_t val)
{
  return (val * VREF / ADC_PREC) / 0.0449;
}

/*-----------------------------------------------------------------------------------------------------
  Обработка аналоговых сигналов после окончания преобразования ADC
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_ADC_processing(void)
{
  // Фильтрация сигналов

  // Фильтруем медианным фильтром сигналы с внешнего и внутреннего ADC
  adcs.drv_temp_filtered = MedianFilter_3uint16(adcs.smpl_TEMP, &drv_temp_filter);
  adcs.mot_temp_filtered = MedianFilter_3uint16(adcs.smpl_EXTTEMP, &mot_temp_filter);
  adcs.int_temp_filtered = MedianFilter_3uint16(adcs.smpl_Temper3, &int_temp_filter);
  adcs.dc_volt_filtered  = MedianFilter_3uint16(adcs.smpl_UVDD, &dc_volt_filter);
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return _mqx_uint 
-----------------------------------------------------------------------------------------------------*/
_mqx_uint U3HB01_is_move_possible(void)
{
  // Проверить статус драйвера мотора DRV8305
  if (DRV8305_read_status_regs() != MQX_OK)  return MQX_ERROR;
  if (g_app_errors != 0) return MQX_ERROR;
  return MQX_OK;
}


/*-----------------------------------------------------------------------------------------------------
 Выбираем задачу которую будем выполнять  
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t U3HB01_Task_selector(void)
{

  SPISysBus_init(1); // Инициализация системнй SPI шины связывающей драйвер DRV8305, платы энкодеров и проч. с микроконтроллером

  switch (wvar.dev_task)
  {
  case 0:
#ifdef DOORCTL_APP
    U3HB01_Door_control(); break;
#endif
  case 1:
    U3HB01_Solenoid_control(); break;
  case 2:
    U3HB01_Rotation_control(); break;
  default:
    U3HB01_Rotation_control(); break;
  }

  return 0;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return char* 
-----------------------------------------------------------------------------------------------------*/
char* GetFirmwareDescr(void)
{

#if   APP_PROFILE == SB200AD49K
  return "SB200AD_49K" ;
#elif APP_PROFILE == SB200AD92K
  return "SB200AD_92K" ;
#elif APP_PROFILE == RB150AD49K
  return "RB150AD_49K" ;
#elif APP_PROFILE == RB150AD92K
  return "RB150AD_92K" ;
#elif APP_PROFILE == SB200AD92KV
  return "SB200AD_92K_V" ;
#elif APP_PROFILE == U3HB01
  return "U3HB01" ;
#endif


}
