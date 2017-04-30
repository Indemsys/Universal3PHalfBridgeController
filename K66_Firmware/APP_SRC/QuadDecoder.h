#ifndef __QUADDECODER
  #define __QUADDECODER






typedef struct
{
  uint32_t           enabled;
  uint32_t           smpl_cnt;
  uint32_t           min_t;
  uint32_t           max_t;
  MQX_TICK_STRUCT    ticktime_prev; // Метка времени для наблюдения за статистикой прерываний
  MQX_TICK_STRUCT    ticktime;      // Метка времени для наблюдения за статистикой прерываний

} T_qdec_diagnostic;


typedef struct
{
  volatile int32_t    tacho_counter;      // Счетчик тахоимпульсов энкодера. Значение эквивалентно позиции платформы в тахоимпульсах
  volatile int32_t    dir_sign;           // Направление тахометра
  volatile uint32_t   idle_cnt;           // Счетчик таймаута между тахоимпульсами
  volatile int32_t    puls_len;           // Интервал времени в микросекундах между тахоимпульсами
  volatile int32_t    rpm;                // Скорость вращения в оборотах в минуту
} T_qdec_cbl;

typedef void (*T_app_qdec_isr)(int32_t pulse_cnt);


void                Init_QuadratureDecoder(void);
void                QDEC_increment_idle_cnt(void);
void                QDEC_set_positioning_calback(T_app_qdec_isr isr);

T_qdec_cbl*         QDEC_get_cbl(void);
T_qdec_diagnostic*  QDEC_get_diagnostic(void);

void                QDEC_get_ticks(MQX_TICK_STRUCT *tickt_prev, MQX_TICK_STRUCT *tickt);
void                QDEC_diagnostic_restart(void);


int32_t             Get_speed(void);
int32_t             QDEC_get_counter(void);
void                QDEC_set_couter(int32_t pos);
int32_t             QDEC_get_dir_sign(void);
void                QDEC_reset_counter(void);
int32_t             QDEC_get_tacho_interval(void);

#endif
