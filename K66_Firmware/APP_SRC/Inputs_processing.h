#ifndef INPUTS_PROCESSING_H
  #define INPUTS_PROCESSING_H


void      Inputs_create_sync_obj(void);
void      Inputs_set_ADC_evt(void);
_mqx_uint Inputs_wait_for_ADC_event(void);
_mqx_uint Inputs_wait_for_changed_event(uint32_t ticks);


#endif // INPUTS_PROCESSING_H



