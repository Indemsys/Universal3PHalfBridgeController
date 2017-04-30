#ifndef U3HB01_HALL_BLDC_H
#define U3HB01_HALL_BLDC_H


#define CW_MOTOR_ROTATION    0
#define CCW_MOTOR_ROTATION   1

// Все должно быть типа uint16_t
typedef struct
{
  uint16_t  rot_dir;              // Направление вращения
  uint16_t  modulation;           // Текущая величина модуляции PWM сигнала, регулируется в зависимости от скорости  
  uint8_t   dir_inv;              // Флаг инверсии направления направления, изменяет направление вращения мотора и направление счета позиции

  uint8_t   commutation_step;       
  // Состояния выходных сигналов предназначенные для формирования отладочного вывода в FreeMaster
  int8_t    a_out_level;
  int8_t    b_out_level;
  int8_t    c_out_level;
    
} 
T_BLDC_sync_ctrl_params;


#define BLDC_SYNC_MAP_SZ 2


const T_BLDC_pars_map*   BLDC_sync_get_map(uint32_t  *psz);
uint32_t                 BLDC_start_rotation(uint32_t direction, float pwr);
void                     BLDC_force_break(float pwr);
void                     BLDC_commutation_by_hall_sens(uint32_t hst);
uint32_t                 BLDC_overload_observer(void);
void                     BLDC_set_PWM_sync_signal(void);

#endif // MDC01_HALL_BLDC_H



