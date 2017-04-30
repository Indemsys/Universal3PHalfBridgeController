// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 09:59:37
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

// 2017-01-04

void     FMSTR_Recorder(void);

#define UNDEF_HALL_STATE 7
// 101 100 110 010 011 001
//  0   1   2   3   4   5
static uint32_t hall_states[8] =
{
  UNDEF_HALL_STATE, // 000  Невозможное состояние
  5, // 001  Состояние 5
  3, // 010  Состояние 3
  4, // 011  Состояние 4
  1, // 100  Состояние 1
  0, // 101  Состояние 0
  2, // 110  Состояние 2
  UNDEF_HALL_STATE, // 111  Невозможное состояние
};

// Массив для определения приращения и знака приращения по сигналу с датчика Холла
// По горизонтали состояние текущее по вертикали предыдущее
static int32_t hall_deltas[6][6] =
{
  // Состояние текущее
  // 0    1    2    3    4    5
  0,   1,   2,   3,-2,-1, //  Предыдущее состояние 0
  -1,   0,   1,   2,   3,-2, //  Предыдущее состояние 1
  -2,-1,   0,   1,   2,   3, //  Предыдущее состояние 2
  3,-2,-1,   0,   1,   2, //  Предыдущее состояние 3
  2,   3,-2,-1,   0,   1, //  Предыдущее состояние 4
  1,   2,   3,-2,-1,   0, //  Предыдущее состояние 5
};

static T_hall_isr_callback              hall_position_control_callback;
static T_hall_isr_callback              hall_speed_control_callback;
static T_hall_isr_commutation_callback  hall_commutation_callback;

static int32_t         delta = 0;
static FTM_MemMapPtr   FTM_2 = FTM2_BASE_PTR;
static uint32_t        cnv;
static uint32_t        hstate;
static uint32_t        ovrf_evt;
static float           speed;


/*-----------------------------------------------------------------------------------------------------
  Обработчик прерывания таймера FTM2
  Прерывания происходят каждый раз при изменении состояния датчиков холла и при переполнении таймера FTM2Р 
  Таймер переполняется каждые HALL_TMR_OVERFL_PERIOD_US 
   
  Прерывание уровня ядра!!!
-----------------------------------------------------------------------------------------------------*/
void Hall_sensor_kernel_isr(void)
{

  uint32_t        sc;

  sc =  FTM_2->SC; // Читаем состояние
  if (sc & BIT(7))
  {
    // Считаем переполения
    FTM_2->SC = sc & ~(BIT(7)); // Сбрасываем бит TOF
    if (hccbl.ftm2_overfl < (HALL_TMR_OVRFLL_FOR_0_SPEED + 1)) hccbl.ftm2_overfl++; // Считаем количество переполнений, но не больше заданного максимального
    ovrf_evt = 1;                 // Устанвливаем флаг переполнения
  }
  if (FTM_2->CONTROLS[1].CnSC & BIT(7))
  {
    // Расчитываем длительность зафиксированного импульса
    FTM_2->CONTROLS[1].CnSC = FTM_2->CONTROLS[1].CnSC & ~(BIT(7));  // Сбрасываем бит CHF

    cnv = FTM_2->CONTROLS[1].CnV;
    hstate = U3HB01_get_hall_state(); // Получаем бинарную маску состояния 3-х датчиков холла

    // Если в этом прерывании было зафиксировано переполение, надо решить учитывать его при расчете длительности импульса или нет
    if (ovrf_evt != 0)
    {
      if (FTM_2->CNT < cnv)
      {
        // Если текущее значение таймера меньше захваченного значения,
        //  то это означает что захват значения был до осуществлен до переполнения таймера и зафиксированное сейчас переполение учитывать не надо
        hccbl.ftm2_overfl--;
      }
    }

    // Определяем позицию

    if ((hstate < 1) || (hstate > 6))
    {
      hccbl.undef_state_errors++;
    }
    else
    {
      hstate = hall_states[hstate]; // Получаем номер состояния из маски состояния сенсоров Холла
      if (hccbl.prev_hst != UNDEF_HALL_STATE)
      {
        delta = hall_deltas[hccbl.prev_hst][hstate];
        if ((delta > 1) || (delta < -1))
        {
          hccbl.big_delta_errors++;
        }
        else if (delta == 0)
        {
          hccbl.no_change_state_errors++;
        }
        if (bldcs_cbl.dir_inv != 0)
        {
          delta = -delta;
        }
        hccbl.hall_counter += delta;

        // Измеряем длительность импульса
        hccbl.hall_pulse_len = cnv  + hccbl.ftm2_overfl *  (FTM2_CAPTURE_MOD + 1) - hccbl.prev_cnv;

      }
      hccbl.prev_hst = hstate;
    }

    hccbl.prev_cnv = cnv;
    hccbl.ftm2_overfl = 0;

    if (hall_position_control_callback != 0)
    {
      if (hall_position_control_callback() == RES_OK)  // Здесь следим за углом раскрыва двери
      {
        if (hall_commutation_callback != 0) hall_commutation_callback(hstate); // Выполняем коммутацию обмотки мотора
      }
    }
    else
    {
      if (hall_commutation_callback != 0) hall_commutation_callback(hstate); // Выполняем коммутацию обмотки мотора
    }
  }


  if (ovrf_evt != 0)
  {
    int32_t diff;

    ovrf_evt = 0;
    // Определяем на сколько позиция изменилась с прошлого момента
    diff = hccbl.prev_hall_counter - hccbl.hall_counter;
    hccbl.prev_hall_counter = hccbl.hall_counter;

    speed = diff * 1000000.0 / ((float)(HALL_TMR_OVERFL_PERIOD_US)*12.0);
    // Если вычисленная скрость превышает максимально возможную, то новое значение не принимать
    if (fabs(speed) <= MOTOR_SPEED_MAX)
    {
      // Знак скрости всегда положителен если вращаемся по заданному направлению
      if (bldcs_cbl.dir_inv > 0)
      {
        if (bldcs_cbl.rot_dir == CCW_DIR) speed = -speed;
      }
      else
      {
        if (bldcs_cbl.rot_dir == CW_DIR) speed = -speed;
      }
      // Усредняем скорость
      mot_speed_rps_raw = speed;
      if (hccbl.speed_filter_en != 0)
      {
        mot_speed_rps = RunAverageFilter_float(speed, &hccbl.speed_filter);
      }
      else
      {
        mot_speed_rps = speed;
      }
    }


    if (hall_speed_control_callback != 0) hall_speed_control_callback(); // Здесь отрабатываем PID алгоритм

    if (g_fmstr_rate_src == FMSTR_SMPS_HALL_TMR_OVERFL_ISR)
    {
      FMSTR_Recorder(); // Вызываем функцию записи сигнала для инструмента FreeMaster
    }
  }

  BLDC_set_PWM_sync_signal();

}

/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_hall_state(void)
{
  uint32_t hst;
  hst = U3HB01_get_hall_state(); // Получаем бинарную маску состояния 3-х датчиков холла
  if ((hst < 1) || (hst > 6)) hst = 1;
  hst = hall_states[hst]; // Получаем номер состояния из маски состояния сенсоров Холла
  return hst;
}

/*----------------------------------------------------------------------------------------------------- 
  Инициализация таймеров FTM1 и FTM2 для обработки cигнала с 3-х фазного Hall сенсора  
 
 \param void 
-----------------------------------------------------------------------------------------------------*/
void FTM1_FTM2_init_3ph_hal_sens_measl(void)
{
  SIM_SOPT4 |= BIT(22); // FTM2 channel 1 input capture source select | 1 - Exclusive OR of FTM2_CH1, FTM2_CH0 and FTM1_CH1.
  hccbl.prev_hst = UNDEF_HALL_STATE;
  hccbl.hall_counter  = 0;
  hccbl.prev_hall_counter = 0;
  hccbl.overfl_bound   = HALL_TMR_OVRFLL_FOR_0_SPEED;

  FTM1_FTM2_init_hall_sens_meas();
}


/*-----------------------------------------------------------------------------------------------------
 
 \param speed 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Convert_speed_RPS_to_pulse_length(float speed)
{
  float    t;
  uint32_t val;
  // Один оборот составляет 12 импульсов
  // Находим время в мкс одного импульса
  t = 1.0 / (speed * 12.0);
  val = (uint32_t)((t * FTM2_SYSCLK) / FTM2_PRESC);

  return val;
}

/*-----------------------------------------------------------------------------------------------------
 Получить счетик импульсов датчика Холла
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_Hall_pulse_counter(void)
{
  return hccbl.hall_counter;
}
/*-----------------------------------------------------------------------------------------------------
 
 \param void 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
T_hall_capture_cbl* Get_Hall_cbl(void)
{
  return &hccbl;
}


/*-----------------------------------------------------------------------------------------------------
 
 \param val 
 
 \return uint32_t 
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_Hall_state_from_mask(uint32_t val)
{
  return hall_states[val];
}


/*-----------------------------------------------------------------------------------------------------
  Установка функции обратного вызова hall_position_control_callback
  Обратный вызов осуществляется в процедуре ISR режима ядра 
 
 \param func 
-----------------------------------------------------------------------------------------------------*/
void Set_hall_isr_position_ctrl_callback(T_hall_isr_callback func)
{
  hall_position_control_callback = func;
}
/*-----------------------------------------------------------------------------------------------------
  Установка функции обратного вызова hall_speed_control_callback
  Обратный вызов осуществляется в процедуре ISR режима ядра 
 
 \param func 
-----------------------------------------------------------------------------------------------------*/
void Set_hall_isr_speed_ctrl_callback(T_hall_isr_callback func)
{
  hall_speed_control_callback = func;
}
/*-----------------------------------------------------------------------------------------------------
  Установка функции обратного вызова процедуры коммутации мотора при синхронном управлении
  Обратный вызов осуществляется в процедуре ISR режима ядра 
 
 \param func 
-----------------------------------------------------------------------------------------------------*/
void Set_hall_isr_commutation_callback(T_hall_isr_commutation_callback func)
{
  hall_commutation_callback = func;
}



