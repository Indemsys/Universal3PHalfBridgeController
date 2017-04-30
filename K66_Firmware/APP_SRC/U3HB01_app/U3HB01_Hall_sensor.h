#ifndef U3HB01_HALL_SENSOR_H
#define U3HB01_HALL_SENSOR_H


#define FTM2_SYSCLK                 (60000000ul)

#define MOT_PWM_FREQ                (16000) // Частота ШИМ мотора
#define FTM_MOTOR_MOD               (FTM2_SYSCLK/(MOT_PWM_FREQ*2))
#define MOD_MARGIN                  10.0 // Процент на который ограничен максимум модуляции 

#define HALL_TMR_OVERFL_PERIOD_US   (30000ul)                 // Период переполения таймера датчика Холла в микросекундах
#define PID_PERIOD_US               HALL_TMR_OVERFL_PERIOD_US // Период выполнения PID алгоритма управления скростью мотора в микросекундах
#define PID_PERIOD_S                ((float)HALL_TMR_OVERFL_PERIOD_US/1000000.0) // Период выполнения PID алгоритма управления скростью мотора в секундах

#define LONG_CURRENT_PERIOD         (1000000ul)               // Период измерения длительного среднего тока
#define LONG_CURRENT_SMPLS_NUM      (LONG_CURRENT_PERIOD/HALL_TMR_OVERFL_PERIOD_US) // Количество отсчетов для вычисления среднего продолжительного тока


#define FTM2_PRESC                  64ul      // Предделитель системной частоты таймера FTM2
#define FTM2_PRESC_VAL              6         // Программируемая величина соответствующая заданному предделителю

#define FTM2_CAPTURE_MOD            ((((FTM2_SYSCLK/1000000ul)*HALL_TMR_OVERFL_PERIOD_US)/FTM2_PRESC)-1)  // Модуль таймера в режиме измерения скорости вращения с датчиками холла

// Мотор с редуктором без нагрузки вращается при минимальной модуляции ШИМ = 3%, скрость при этом равна ~4.8 оборотов в сек
//
#define MOTOR_SPEED_MIN              1       // Скорость вращения мотора при минимальной скорости открытия дверей. Длительность импульса в тактах таймера = 312500  (83.3 мс)

#define MOTOR_SPEED_NOMIMAL          8.0     // Скорость вращения мотора при номинальной скорости открытия дверей. Длительность импульса в тактах таймера = 39062   (10.4 мс)

#define PULS_PER_MOT_ROUND           12.0    // Количество пульсов полученных сложением по модулю 2 всех 3-х сигналов от датчиков Холла на оборот мотора

#if   (APP_PROFILE == SB200AD49K) || (APP_PROFILE == RB150AD49K)
  #define   PULS_PER_DEV_ROUND         1775    // Точнее 1774.6 Количество пульсов полученных сложением по модулю 2 всех 3-х сигналов от датчиков Холла на оборот в устройстве
  #define   MOTOR_SPEED_MAX            500.0   // 
                                               
#elif (APP_PROFILE == SB200AD92K) || (APP_PROFILE == RB150AD92K)
  #define   PULS_PER_DEV_ROUND         3332    // Количество пульсов полученных сложением по модулю 2 всех 3-х сигналов от датчиков Холла на оборот в устройстве 
  #define   MOTOR_SPEED_MAX            270.0   // 
                                               
#elif APP_PROFILE == SB200AD92KV
  #define   PULS_PER_DEV_ROUND         2208    // Количество пульсов полученных сложением по модулю 2 всех 3-х сигналов от датчиков Холла на оборот в устройстве 
  #define   MOTOR_SPEED_MAX            400.0   //

#elif APP_PROFILE == U3HB01
  #define   PULS_PER_DEV_ROUND         12      // Количество пульсов полученных сложением по модулю 2 всех 3-х сигналов от датчиков Холла на оборот в устройстве 
  #define   MOTOR_SPEED_MAX            1000000.0   
#endif

#define MAX_HALL_PULSE_LENGTH_US     (1000000ul/((uint32_t)(MOTOR_SPEED_MIN*PULS_PER_MOT_ROUND)))                   // Максимальная длительность импульса с датчика Холла в микросекундах
#define MAX_HALL_PULSE_LENGTH        ((uint32_t)((float)FTM2_SYSCLK)/(MOTOR_SPEED_MIN*PULS_PER_MOT_ROUND*(float)FTM2_PRESC))  // Максимальная длительность импульса с датчика Холла в тактах таймера
#define HALL_TIMER_OVRFL_PER_US      ((FTM2_PRESC*FTM2_CAPTURE_MOD)/(FTM2_SYSCLK/1000000ul))   // Период в микросекундах переполнения таймера датчиков Холла,  60 - частота тактирования таймера в МГц


#define HALL_TMR_OVRFLL_FOR_0_SPEED  (MAX_HALL_PULSE_LENGTH_US/HALL_TIMER_OVRFL_PER_US) // Количество переполнений таймера после которого считаем скорость нулевой

typedef uint32_t (*T_hall_isr_callback)(void);
typedef void (*T_hall_isr_commutation_callback)(uint32_t hstate);

typedef struct
{
  uint32_t             ftm2_overfl;
  uint32_t             hall_pulse_len;
  uint32_t             prev_cnv;
  uint32_t             prev_hst;
  volatile int32_t     hall_counter;
  int32_t              prev_hall_counter;
  uint32_t             undef_state_errors;
  uint32_t             big_delta_errors;
  uint32_t             no_change_state_errors;
  uint32_t             overfl_bound;

  uint8_t              speed_filter_en;
  T_run_average_float  speed_filter;

} T_hall_capture_cbl;


void                   Hall_sensor_kernel_isr(void);

void                   FTM1_FTM2_init_3ph_hal_sens_measl(void);
uint32_t               Get_Hall_pulse_counter(void);
T_hall_capture_cbl*    Get_Hall_cbl(void);
uint32_t               Get_Hall_state_from_mask(uint32_t val);
uint32_t               Convert_speed_RPS_to_pulse_length(float speed);
void                   Set_hall_isr_position_ctrl_callback(T_hall_isr_callback func);
void                   Set_hall_isr_speed_ctrl_callback(T_hall_isr_callback func);
void                   Set_hall_isr_commutation_callback(T_hall_isr_commutation_callback func);
uint32_t               Get_hall_state(void);

#endif // MDC01_HALL_SENSOR_H



