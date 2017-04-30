#ifndef APP_H
  #define APP_H

#include <string.h>
#include <mqx.h>
#include <bsp.h>
#include <bsp_prv.h>
#include <mem_prv.h>
#include <mqx_prv.h>
#include <name.h>
#include <name_prv.h>
#include <message.h>
#include <msg_prv.h>
#include <mutex.h>
#include <partition.h>
#include <partition_prv.h>
#include <sem.h>
#include <sem_prv.h>
#include <event.h>
#include <event_prv.h>
#include <log.h>
#include <lwevent.h>
#include <lwlog.h>
#include <lwmem.h>
#include <lwmem_prv.h>
#include <klog.h>
#include <fio.h>
#include <mfs.h>
#include <shell.h>
#include <sdcard.h>
#include <spi.h>
#include <part_mgr.h>
#include "K66BLEZ1_INIT_SYS.h"
#include "K66BLEZ1_DMA.h"
#include "SEGGER_RTT.h"
#include "Debug_io.h"
#include "Main.h"


#define BIT(n) (1u << n)
#define LSHIFT(v,n) (((unsigned int)(v) << n))
#define DELAY_ms(x)     Delay_m7(25714*x-1)    // 999.95*N  мкс при частоте 180 МГц

extern void Delay_m7(int cnt); // Задержка на (cnt+1)*7 тактов . Передача нуля недопускается


#define dbg_printf printf

_mqx_int TimeManInit(void);
void     Get_time_counters(HWTIMER_TIME_STRUCT *t);
uint32_t   Eval_meas_time(HWTIMER_TIME_STRUCT t1,HWTIMER_TIME_STRUCT t2);
uint32_t   Get_usage_time(void);

#endif
