#ifndef RTOS_UTILS_H
  #define RTOS_UTILS_H

uint32_t     Measure_reference_time_interval(uint32_t ti);
void         Task_background(unsigned int initial_data);
void         Install_and_enable_isr(int num, int pri, INT_ISR_FPTR isr_ptr);
void         Install_and_enable_kernel_isr(int num, int pri, INT_KERNEL_ISR_FPTR isr_ptr);
void         Set_shar_s32val(int32_t *adr, int32_t v);
void         Set_shar_u32val(uint32_t *adr, uint32_t v);
void         Set_shar_buff(void *dest , void *src, uint32_t sz);
uint32_t     Get_shar_u32val(uint32_t *adr);
int32_t      Get_shar_s32val(int32_t *adr);

void         Get_reference_time(void);

uint32_t     Time_diff_ms(TIME_STRUCT *tlast_ptr, TIME_STRUCT *tnow_ptr);
uint32_t     Time_diff_sec(TIME_STRUCT *tlast_ptr, TIME_STRUCT *current_time);
uint32_t     Time_elapsed_ms(TIME_STRUCT *tlast_ptr);
uint32_t     Time_elapsed_sec(TIME_STRUCT *tlast_ptr);
uint32_t     Conv_ms_to_ticks(uint32_t ms);

void         Reset_system(void);


#endif // RTOS_UTILS_H



