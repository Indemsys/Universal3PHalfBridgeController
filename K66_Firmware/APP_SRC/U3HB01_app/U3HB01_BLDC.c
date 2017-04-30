// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016-12-15
// 11:37:51
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

extern  float  Door_PID_step_execute(float target_s, float real_s);


/*-----------------------------------------------------------------------------------------------------
 Записать в отладочные переменные нулевые уровни выходных сигналов  
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void _Clear_motor_debug_vars(void)
{
  mot_PWM_lev = 0;
  bldcs_cbl.a_out_level = 0;
  bldcs_cbl.b_out_level = 0;
  bldcs_cbl.c_out_level = 0;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void BLDC_Free_Run(void)
{
  FTM3_OUTMASK = 0x3F;     // Отключаем все выходы
  FTM3_SWOCTRL = 0;        // Отлючаем програмное управление ключами

  mot_PWM_lev = 0;
  FTM3_set_CnV(Get_modulation_from_percent(0));
  DRV8305_Gates_disable(); // Отключаем управление выходами у драйвера мотора  DRV8305
  g_bldc_active = 0;
  g_bldc_forcebrake = 0;

  _Clear_motor_debug_vars();
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void BLDC_break(void)
{
  // Программно устанавливаем ключи на торможение
  FTM3_SWOCTRL = 0
                 + LSHIFT(0, 15) // CH7OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(0, 14) // CH6OCV | 1 The software output control forces 1 to the channel output.
                 + LSHIFT(1, 13) // CH5OCV | 1 The software output control forces 1 to the channel output.   Нижний ключ канала  C
                 + LSHIFT(0, 12) // CH4OCV | 1 The software output control forces 1 to the channel output.   Верхний ключ канала C
                 + LSHIFT(1, 11) // CH3OCV | 1 The software output control forces 1 to the channel output.   Нижний ключ канала  B
                 + LSHIFT(0, 10) // CH2OCV | 1 The software output control forces 1 to the channel output.   Верхний ключ канала B
                 + LSHIFT(1, 9)  // CH1OCV | 1 The software output control forces 1 to the channel output.   Нижний ключ канала  A
                 + LSHIFT(0, 8)  // CH0OCV | 1 The software output control forces 1 to the channel output.   Верхний ключ канала A
                 + LSHIFT(0, 7)  // CH7OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(0, 6)  // CH6OC  | 1 The channel output is affected by software output control.
                 + LSHIFT(1, 5)  // CH5OC  | 1 The channel output is affected by software output control.    Нижний ключ канала  C
                 + LSHIFT(1, 4)  // CH4OC  | 1 The channel output is affected by software output control.    Верхний ключ канала C
                 + LSHIFT(1, 3)  // CH3OC  | 1 The channel output is affected by software output control.    Нижний ключ канала  B
                 + LSHIFT(1, 2)  // CH2OC  | 1 The channel output is affected by software output control.    Верхний ключ канала B
                 + LSHIFT(1, 1)  // CH1OC  | 1 The channel output is affected by software output control.    Нижний ключ канала  A
                 + LSHIFT(1, 0)  // CH0OC  | 1 The channel output is affected by software output control.    Верхний ключ канала A
  ;
  FTM3_OUTMASK = 0x00;     // Включаем все выходы, чтобы выполнялась программная установка выходов

  mot_PWM_lev = 0;
  FTM3_set_CnV(Get_modulation_from_percent(0));
  DRV8305_Gates_enable();
  g_bldc_active = 0;
  g_bldc_forcebrake = 0;

  _Clear_motor_debug_vars();
}


