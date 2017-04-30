#ifndef __K66BLEZ_DAC_CONTROL
  #define __K66BLEZ_DAC_CONTROL


void     Init_DAC0(void);
void     Init_DAC1(void);
void     Set_DAC0_val(uint16_t val);
uint16_t Pin_DAC0_state(void);
void     Set_DAC1_val(uint16_t val);
uint16_t Pin_DAC1_state(void);


#endif



