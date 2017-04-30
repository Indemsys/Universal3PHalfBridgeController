// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.04.04
// 09:39:38
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


#define   RTT_STR_SZ 512
char     rtt_str[RTT_STR_SZ];

/*------------------------------------------------------------------------------
  Выводим форматированную строку в RTT
 \param BufferIndex
 \param sFormat
 ------------------------------------------------------------------------------*/
void RTT_printf(const char *fmt, ...)
{
  int32_t n;
  va_list           ap;
  va_start(ap, fmt);
  n = vsnprintf(rtt_str, RTT_STR_SZ, (char *)fmt, ap);
  va_end(ap);
  SEGGER_RTT_Write(0, rtt_str, n);
}



