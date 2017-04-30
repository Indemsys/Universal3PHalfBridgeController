// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017.04.18
// 9:04:17
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


static  PIDParams pid_pars;
static  real32_T  smpl_time;
static  float     target_value;



/*-----------------------------------------------------------------------------------------------------
 
 \param tgv         - целевая величина, к ней ведется регулирование
 \param p_pid_pars  - указатель на структуру с коэффициентами PID-а
 \param ts          - период сэмплирования в секундах 
-----------------------------------------------------------------------------------------------------*/
void U3HB01_PID_init(float tgv, PIDParams *p_pid_pars, real32_T  ts)
{
  target_value = tgv;

  rtDW.Del2_DSTATE    = 0;
  rtDW.Integr1_states = 0;
  rtDW.Integr2_states = 0;

  pid_pars.Kp         = p_pid_pars->Kp;
  pid_pars.Ki         = p_pid_pars->Ki;
  pid_pars.Kd         = p_pid_pars->Kd;
  pid_pars.Kn         = p_pid_pars->Kn;
  pid_pars.OutUpLim   = p_pid_pars->OutUpLim;
  pid_pars.OutLoLim   = p_pid_pars->OutLoLim;
  pid_pars.OutRateLim = p_pid_pars->OutRateLim;

  smpl_time = ts;
}

/*-----------------------------------------------------------------------------------------------------
 
 \param tgv         - целевая величина, к ней ведется регулирование
-----------------------------------------------------------------------------------------------------*/
void U3HB01_PID_update(float tgv, PIDParams *p_pid_pars)
{
  target_value = tgv;

  pid_pars.Kp         = p_pid_pars->Kp;
  pid_pars.Ki         = p_pid_pars->Ki;
  pid_pars.Kd         = p_pid_pars->Kd;
  pid_pars.Kn         = p_pid_pars->Kn;
  pid_pars.OutUpLim   = p_pid_pars->OutUpLim;
  pid_pars.OutLoLim   = p_pid_pars->OutLoLim;
  pid_pars.OutRateLim = p_pid_pars->OutRateLim;
}

/*-----------------------------------------------------------------------------------------------------
   Вычисление выходной величины по алгоритму PID
-----------------------------------------------------------------------------------------------------*/
float U3HB01_PID_step(float measured_value)
{
  float pid_output;
  float e;

  e = target_value - measured_value;
  PIDController_custom(e, smpl_time, &pid_pars, &pid_output);
  return pid_output;
}
