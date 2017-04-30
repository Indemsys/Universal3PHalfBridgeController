#ifndef __K66BLEZ_ADC
  #define __K66BLEZ_ADC



#define ADC_AVER_4   1
#define ADC_AVER_8   2
#define ADC_AVER_16  3
#define ADC_AVER_32  4



typedef struct
{
  ADC_MemMapPtr ADC;                           //  Указатель на модуль ADC
  unsigned short volatile     *results_vals;   //  Указатель на буффер с результатами ADC
  unsigned char const         *config_vals;    //  Указатель на массив констант содержащих конфигурацию ADC
  uint8_t                       results_channel; //  Номер канала DMA пересылающий резульат ADC.     Более приоритетный. По умолчанию канал с большим номером имеет больший приоритет.
  uint8_t                       config_channel;  //  Номер канала DMA пересылающий конфигурацию ADC. Менее приоритетный
  uint8_t                       req_src;         //  Номер источника запросов для конфигурирования мультиплексора DMAMUX
} T_init_ADC_DMA_cbl;


typedef struct
{
  // Результаты калибровки модулей ADC
  int32_t adc0_cal_res;
  int32_t adc1_cal_res;
} T_ADC_state;


typedef struct
{
  uint16_t smpl_SNS_IA   ; 
  uint16_t smpl_U_A      ; 
  uint16_t smpl_U_B      ; 
  uint16_t smpl_UVDD     ; 
  uint16_t smpl_TEMP     ; 
                         
  uint16_t smpl_SNS_IB   ; 
  uint16_t smpl_SNS_IC   ; 
  uint16_t smpl_U_C      ; 
  uint16_t smpl_EXTTEMP  ; 
  uint16_t smpl_Temper3  ; 

  uint16_t  drv_temp_filtered;
  uint16_t  mot_temp_filtered;
  uint16_t  int_temp_filtered;
  uint16_t  dc_volt_filtered;
}
T_ADC_res;




typedef struct
{
  const char   *name;
  float        (*int_converter)(int);
  float        (*flt_converter)(float);

} T_vals_scaling;

typedef void(*T_adc_isr_callback)(void); 

void ADC_config_start_DMA(void);

void ADC_calibr_config_start(void);
void ADC_switch_on_all(void);
int  ADC_calibrating(ADC_MemMapPtr ADC);


T_ADC_state *ADC_get_state(void);

void Get_ADC_samples(T_ADC_res **pp_adc_res);
void Copy_to_ADC_res(uint8_t n, uint16_t *buf);


void Set_adc_isr_current_meas_callback(T_adc_isr_callback func);
void Set_adc_isr_filtering_callback(T_adc_isr_callback func);
void Set_adc_isr_asyncs_ctrl_callback(T_adc_isr_callback func);

#endif
