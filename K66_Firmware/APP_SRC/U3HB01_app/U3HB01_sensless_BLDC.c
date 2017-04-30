// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-04
// 21:22:16
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


// Определения фаз коммутации являющиеся индексами в таблице  commutation_tbl
// Фазы следующие в прядке возрастания означают вращение по часовой стрелке
#define SECT_AB  1
#define SECT_AC  2
#define SECT_BC  3
#define SECT_BA  4
#define SECT_CA  5
#define SECT_CB  6

const static T_commutation_tbl commutation_tbl_cw[8]  =
 {
  // outmask,   outinv
   { 0x003F,    0x0000,   0  ,  0  ,  0 },
   { 0x0030,    0x0001,   1  , -1  ,  0 }, //  C (P3) switched off,  A inverted (high lev.) , B normal   (low. lev.), С normal   (low. lev.).  State 0
   { 0x000C,    0x0001,   1  ,  0  , -1 }, //  B (Р2) switched off,  A inverted (high lev.) , B normal   (low. lev.), С normal   (low. lev.).  State 1
   { 0x0003,    0x0002,   0  ,  1  , -1 }, //  A (Р1) switched off,  A normal   (low. lev.) , B inverted (high lev.), С normal   (low. lev.).  State 2
   { 0x0030,    0x0002,  -1  ,  1  ,  0 }, //  C (Р3) switched off,  A normal   (low. lev.) , B inverted (high lev.), С normal   (low. lev.).  State 3
   { 0x000C,    0x0004,  -1  ,  0  ,  1 }, //  B (Р2) switched off,  A normal   (low. lev.) , B normal   (low. lev.), С inverted (high lev.).  State 4
   { 0x0003,    0x0004,   0  , -1  ,  1 }, //  A (Р1) switched off,  A normal   (low. lev.) , B normal   (low. lev.), С inverted (high lev.).  State 5
   { 0x003F,    0x0000,   0  ,  0  ,  0 },
 };

const static T_commutation_tbl commutation_tbl_ccw[8] =
{
  // outmask,  outinv
  { 0x003F,    0x0000,   0  ,  0  ,  0 },
  { 0x0003,    0x0004,   0  , -1  ,  1 }, //  A (Р1) switched off,  A normal   (low. lev.) , B normal   (low. lev.), С inverted (high lev.).  State 5
  { 0x000C,    0x0004,  -1  ,  0  ,  1 }, //  B (Р2) switched off,  A normal   (low. lev.) , B normal   (low. lev.), С inverted (high lev.).  State 4
  { 0x0030,    0x0002,  -1  ,  1  ,  0 }, //  C (Р3) switched off,  A normal   (low. lev.) , B inverted (high lev.), С normal   (low. lev.).  State 3
  { 0x0003,    0x0002,   0  ,  1  , -1 }, //  A (Р1) switched off,  A normal   (low. lev.) , B inverted (high lev.), С normal   (low. lev.).  State 2
  { 0x000C,    0x0001,   1  ,  0  , -1 }, //  B (Р2) switched off,  A inverted (high lev.) , B normal   (low. lev.), С normal   (low. lev.).  State 1
  { 0x0030,    0x0001,   1  , -1  ,  0 }, //  C (P3) switched off,  A inverted (high lev.) , B normal   (low. lev.), С normal   (low. lev.).  State 0
  { 0x003F,    0x0000,   0  ,  0  ,  0 },
};


// Переменные для управления асинхронным вращением
static uint32_t                g_sector;
static uint32_t                g_state_counter;
static uint32_t                g_align_state;


static const T_BLDC_pars_map   BLDC_map[BLDC_ASYNC_MAP_SZ]            =
{
  {  "Rotating direction (0-CW, 1-CCW) ",&bldca_cbl.rot_dir,                   CW_DIR             },
  {  "Modulation level                 ",&bldca_cbl.modulation,                500                },
  {  "Step duration (PWM cicles)       ",&bldca_cbl.step_duration,             23                 },
  {  "Align modulation level           ",&bldca_cbl.align_modulation   ,       800                },
  {  "Align step duration (PWM cicles) ",&bldca_cbl.align_step_duration,       800                },
  {  "Align IDLE step duration         ",&bldca_cbl.align_idle_step_duration,  500                },
};



/*-----------------------------------------------------------------------------------------------------
  Инициализация управляющих параметров значениями по умолчанию 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
static void BLDC_init_sensless_params(void)
{
  uint32_t i;
  for (i = 0; i < BLDC_ASYNC_MAP_SZ; i++)
  {
    *BLDC_map[i].pval =  BLDC_map[i].defv;
  }
}


/*-----------------------------------------------------------------------------------------------------
   Старт вращения двигателя в асинхронном режиме
-----------------------------------------------------------------------------------------------------*/
uint32_t BLDC_start_async_mode(void)
{
  if (g_bldc_active == 0)
  {

    g_sector        = SECT_BA; // Назначаем сектор с которого начнем выравнивание
    g_state_counter = 0;
    g_align_state   = 3;

    FTM3_SWOCTRL = 0;        // Отлючаем програмное управление ключами
    DRV8305_Gates_enable();

    g_bldc_active = 1;
    g_bldc_forcebrake = 0;

    return MQX_OK;
  }
  return MQX_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
 Записать в отладочные переменные уровни выходных сигналов  
 
 \param hst 
-----------------------------------------------------------------------------------------------------*/
static void _Save_motor_outputs_cw(uint32_t hst)
{
  bldcs_cbl.a_out_level = commutation_tbl_cw[hst].mot_a_out * mot_PWM_lev;
  bldcs_cbl.b_out_level = commutation_tbl_cw[hst].mot_b_out * mot_PWM_lev;
  bldcs_cbl.c_out_level = commutation_tbl_cw[hst].mot_c_out * mot_PWM_lev;
}
/*-----------------------------------------------------------------------------------------------------
 Записать в отладочные переменные уровни выходных сигналов  
 
 \param hst 
-----------------------------------------------------------------------------------------------------*/
static void _Save_motor_outputs_ccw(uint32_t hst)
{
  bldcs_cbl.a_out_level = commutation_tbl_ccw[hst].mot_a_out * mot_PWM_lev;
  bldcs_cbl.b_out_level = commutation_tbl_ccw[hst].mot_b_out * mot_PWM_lev;
  bldcs_cbl.c_out_level = commutation_tbl_ccw[hst].mot_c_out * mot_PWM_lev;
}


/*-----------------------------------------------------------------------------------------------------
 Включить питание в указанном секторе
 Отладочная функция
 
 \param sector 
-----------------------------------------------------------------------------------------------------*/
void BLDC_switch_sector(uint32_t sector)
{
  // Не выполняем если мотор в режиме управления вращением
  if (g_bldc_active == 0)
  {
    if ((sector >= 1) && (sector <= 6))
    {
      g_sector = sector; // Назначаем сектор
      FTM3_SWOCTRL = 0;        // Отлючаем програмное управление ключами
      FTM3_OUTMASK = commutation_tbl_cw[g_sector].outmask;
      FTM3_INVCTRL = commutation_tbl_cw[g_sector].outinv;
      _Save_motor_outputs_cw(g_sector);
      FTM3_set_CnV(bldca_cbl.align_modulation);
      Get_percent_from_modulation(bldca_cbl.align_modulation);
      DRV8305_Gates_enable();
    }
    else
    {
      FTM3_OUTMASK = 0x3F; // Отключаем все выходы
      DRV8305_Gates_disable();
    }
    FTM3_SYNC |= BIT(7);
  }
}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return T_BLDC_params* 
-----------------------------------------------------------------------------------------------------*/
T_BLDC_async_ctrl_params* BLDC_sensless_get_cbl(void)
{
  return &bldca_cbl;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param psz 
 
 \return const T_BLDC_map* 
-----------------------------------------------------------------------------------------------------*/
const T_BLDC_pars_map* BLDC_async_get_map(uint32_t  *psz)
{
  if (psz != 0) *psz = BLDC_ASYNC_MAP_SZ;
  return BLDC_map;
}

/*-----------------------------------------------------------------------------------------------------
 Управление вращением без обратной сызи с датчиками
 Выполняется в прерывание уровня ядра!!!
 Нельзя использовать сервисы RTOS 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void BLDC_sensless_asynchronous_control(void)
{
  if (g_bldc_active)
  {
    if (g_state_counter == 0)
    {
      if (g_align_state != 0)
      {
        // Отработка процедуры выравнивания
        if (g_align_state == 1)
        {
          g_state_counter = bldca_cbl.align_idle_step_duration;
          FTM3_OUTMASK = 0x003F; // На последнем шаге выравнивания отключаем все ключи
          FTM3_SYNC |= BIT(7);
        }
        else
        {
          g_state_counter = bldca_cbl.align_step_duration;
          g_sector++;
          if (g_sector > 6) g_sector = 1;
          FTM3_OUTMASK = commutation_tbl_cw[g_sector].outmask;
          FTM3_INVCTRL = commutation_tbl_cw[g_sector].outinv;
          _Save_motor_outputs_cw(g_sector);
          FTM3_set_CnV(bldca_cbl.align_modulation);
          FTM3_SYNC |= BIT(7);
          Get_percent_from_modulation(bldca_cbl.align_modulation);
        }
        g_align_state--;
      }
      else
      {
        g_state_counter = bldca_cbl.step_duration;
        g_sector++;
        if (g_sector > 6) g_sector = 1;
        if (bldca_cbl.rot_dir == CW_DIR)
        {
          FTM3_OUTMASK = commutation_tbl_cw[g_sector].outmask; //
          FTM3_INVCTRL = commutation_tbl_cw[g_sector].outinv;  //
          _Save_motor_outputs_cw(g_sector);
        }
        else
        {
          FTM3_OUTMASK = commutation_tbl_ccw[g_sector].outmask; //
          FTM3_INVCTRL = commutation_tbl_ccw[g_sector].outinv;  //
          _Save_motor_outputs_ccw(g_sector);

        }
        FTM3_set_CnV(bldca_cbl.modulation);
        FTM3_SYNC |= BIT(7);
        Get_percent_from_modulation(bldca_cbl.align_modulation);
      }
    }
    else
    {
      g_state_counter--;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------
 Приложение для тестирования мотора в асинхронном режиме
 
 ToDo: Не используется 
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void BLDC_async_application(void)
{
  BLDC_init_sensless_params();
}
