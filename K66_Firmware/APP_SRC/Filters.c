// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-17
// 12:08:47
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint16_t MedianFilter_3uint16(uint16_t inp, T_median_filter_uint16 *fltr)
{
  if (fltr->en == 0) 
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1]; 
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if ( fltr->arr[1] > fltr->arr[0] )
  {
    if ( fltr->arr[2] > fltr->arr[1] ) return fltr->arr[1];
    else if ( fltr->arr[2] < fltr->arr[0] ) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if ( fltr->arr[0] < fltr->arr[2] ) return fltr->arr[0];
    else if ( fltr->arr[1] > fltr->arr[2] ) return fltr->arr[1];
    else return fltr->arr[2];
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t MedianFilter_3uint32(uint32_t inp, T_median_filter_uint32 *fltr)
{
  if (fltr->en == 0) 
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1]; 
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if ( fltr->arr[1] > fltr->arr[0] )
  {
    if ( fltr->arr[2] > fltr->arr[1] ) return fltr->arr[1];
    else if ( fltr->arr[2] < fltr->arr[0] ) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if ( fltr->arr[0] < fltr->arr[2] ) return fltr->arr[0];
    else if ( fltr->arr[1] > fltr->arr[2] ) return fltr->arr[1];
    else return fltr->arr[2];
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
float RunAverageFilter_float(float inp, T_run_average_float *fltr)
{
  uint32_t i;
  if (fltr->en == 0) 
  {
    for (i=0;i<RUN_AVERAGE_FILTER_LEN;i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp*RUN_AVERAGE_FILTER_LEN;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head = (fltr->head+1) & RUN_AVERAGE_FILTER_INDX_MASK; 
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение  
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение 
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение   
    fltr->head = head;
    return (fltr->acc/(float)RUN_AVERAGE_FILTER_LEN);
  }
}

