#ifndef U3HB01_SENSLESS_BLDC_H
#define U3HB01_SENSLESS_BLDC_H

typedef struct
{
  uint16_t  rot_dir;  
  uint16_t  step_duration;
  uint16_t  modulation;  
  uint16_t  align_modulation;
  uint16_t  align_step_duration;
  uint16_t  align_idle_step_duration;
} 
T_BLDC_async_ctrl_params;

#define BLDC_ASYNC_MAP_SZ 6

void                          BLDC_async_application(void);
void                          BLDC_switch_sector(uint32_t sector);
uint32_t                      BLDC_start_async_mode(void);
T_BLDC_async_ctrl_params*     BLDC_sensless_get_cbl(void);
const T_BLDC_pars_map*        BLDC_async_get_map(uint32_t  *psz);
void                          BLDC_sensless_asynchronous_control(void);


#endif // MDC01_SENSLESS_BLDC_H



