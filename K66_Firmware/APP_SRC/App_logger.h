#ifndef __APP_LOGGER
  #define __APP_LOGGER


#define  EVENT_LOG_SIZE   (64) // Должно быть степенью 2
#define  EVENT_LOG_DISPLAY_ROW  22 // Количество строк лога выводимых на экран при автообновлении

#define    LOG_STR_MAX_SZ    128
#define    EVNT_LOG_FNAME_SZ 64


// Структура хранения записи лога
typedef struct
{
  TIME_STRUCT      time;
  char             msg[LOG_STR_MAX_SZ+1];
  char             func_name[EVNT_LOG_FNAME_SZ+1];
  unsigned int     line_num;
  unsigned int     severity;
} T_app_log_record;


_mqx_uint    AppLogg_init(void);

void         LOG(const char* str, const char* name, unsigned int line_num,unsigned int severity);
void         LOG1(const char* str, const char* name, unsigned int line_num, unsigned int severity,uint32_t v1);
void         LOG2(const char* str, const char* name, unsigned int line_num, unsigned int severity, uint32_t v1,uint32_t v2);
void         LOG3(const char* str, const char* name, unsigned int line_num, unsigned int severity, uint32_t v1, uint32_t v2,uint32_t v3);
void         LOGs(const char* name, unsigned int line_num, unsigned int severity, const char* fmt_ptr, ...);
void         AppLog_pend_saving_params(void);
void         VERBOSE_LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...);
void         AppLogg_monitor_output(T_monitor_cbl *pvt100_cb);
int          AppLog_get_tail_record(T_app_log_record *rec);
void         Task_file_log(unsigned int initial_data);
#endif
