#ifndef U3HB01_PIDCONTROL_H
#define U3HB01_PIDCONTROL_H


void  U3HB01_PID_init(float tgv, PIDParams *p_pid_pars, real32_T  ts);
void  U3HB01_PID_update(float tgv, PIDParams *p_pid_pars);
float U3HB01_PID_step(float measured_value);

#endif // U3HB01_PIDCONTROL_H



