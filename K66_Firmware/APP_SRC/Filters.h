#ifndef FILTERS_H
#define FILTERS_H


typedef struct 
{
  uint8_t  en;
  uint16_t arr[3];

} T_median_filter_uint16;

typedef struct 
{
  uint8_t  en;
  uint32_t arr[3];

} T_median_filter_uint32;



#define RUN_AVERAGE_FILTER_LEN (8)          // Длина фильтра скользящего среднего
#define RUN_AVERAGE_FILTER_INDX_MASK 0x07
typedef struct 
{
  uint8_t  en;
  uint32_t head;
  float    acc;
  float    arr[RUN_AVERAGE_FILTER_LEN];

} T_run_average_float;



uint16_t MedianFilter_3uint16(uint16_t inp, T_median_filter_uint16 *fltr);
uint32_t MedianFilter_3uint32(uint32_t inp, T_median_filter_uint32 *fltr);
float    RunAverageFilter_float(float inp, T_run_average_float *fltr);

#endif // FILTERS_H



