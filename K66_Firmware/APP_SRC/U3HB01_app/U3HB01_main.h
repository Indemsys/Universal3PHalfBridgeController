#ifndef U3HB01_MAIN_H
#define U3HB01_MAIN_H

typedef struct
{

  // ѕеременные контрол€ перегрузи по току
  float        aver_curr_a;
  float        aver_curr_b;
  float        aver_curr_c;
  float        max_current;
               
  // ѕеременные контрол€ долговременной перегрузки по току
  float        long_aver_curr_a;
  float        long_aver_curr_b;
  float        long_aver_curr_c;
  uint32_t     long_curr_cnt;
  float        max_long_current;

  uint8_t      overload_flags;    // ƒиагностическа€ переменна€. ѕричина остановки движени€

} T_overcur_observer;





void      U3HB01_ADC_processing(void);
_mqx_uint U3HB01_is_move_possible(void);

float     U3HB01_Convert_int_temp(uint16_t val);
float     U3HB01_Convert_ext_temp(uint16_t val);
float     U3HB01_Convert_ext_mot_temp(uint16_t val);
float     U3HB01_Convert_vdc_temp(uint16_t val);

uint32_t  U3HB01_Task_selector(void);
char*     GetFirmwareDescr(void);

#endif // MDC01_MAIN_H



