/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 1989-2008 ARC International
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the source functions for the MQX timing
*   test.
*   NOTE THAT FOR THESE TESTS, THE KERNEL TIMER IS NOT DISABLED.
*
*
*END************************************************************************/
#define FLASH_TARGET 1

#include "app.h"
#include "Benchmark_config.h"
/* Uncomment this to turn on logging */
/*#define ENABLE_KLOG*/



/*#define NO_IO*/
#ifdef NO_IO
_mqx_uint _io_init(void)
{ return MQX_OK;}
void      _io_serial_default_init(void)
{ }
_mqx_int  _io_printf(char  *fmt_ptr, ...)
{ return 0;}
_mqx_int  _io_sprintf(char  *str_ptr, char  *fmt_ptr, ...)
{ return 0;}
_mqx_int  _io_fputc(_mqx_int c, MQX_FILE_PTR p)
{ return 0;}
_mqx_int  _io_fputs(char *c, MQX_FILE_PTR p)
{ return 0;}
_mqx_int  _io_putchar(_mqx_int c)
{ return 0; }
_mqx_int  _io_puts(char *s)
{ return 0; }
#endif

const TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{

#ifdef _TASKING
  { CREATE_TEST_TASK,       create_test_task,     PSP_MINSTACKSIZE,  CREATE_TASK_PRIO,   "Create",         0 },

  { ADD_READY_TASK,         add_ready_task,       MORE_STACK + 500, MAIN_PRIO,          "Add ready",      0 },
  { ADD_READY_FIRST_TASK,   add_ready_first_task, MORE_STACK + 500, MAIN_PRIO,          "Add ready first", 0 },
#endif

  { MAIN_TASK,              main_task,            MORE_STACK + 2800, MAIN_PRIO,          "Main", MQX_AUTO_START_TASK },

#ifdef _TASKING
  { SUSPEND_TASK,           suspend_task,         MORE_STACK + 500, SUSPEND_PRIO,       "Suspend",        0 },
  { SUSPEND_FIRST_TASK,     suspend_first_task,   MORE_STACK + 500, SUSPEND_FIRST_PRIO, "Suspend first",  0 },

  { RESUME_TASK,            resume_task,          MORE_STACK + 500, RESUME_PRIO,        "Resume",         0 },
  { RESUME_LAST_TASK,       resume_last_task,     MORE_STACK + 500, RESUME_LAST_PRIO,   "Resume last",    0 },
#endif

#ifdef _MESSAGE
  { SEND_TASK,              send_task,            MORE_STACK + 500, SEND_PRIO,          "Send",           0 },
  { SEND_LAST_TASK,         send_last_task,       MORE_STACK + 500, SEND_LAST_PRIO,     "Send last",      0 },
  { RECEIVE_TASK,           receive_task,         MORE_STACK + 500, RECEIVE_PRIO,       "Receive",        0 },
  { RECEIVE_FIRST_TASK,     receive_first_task,   MORE_STACK + 500, RECEIVE_FIRST_PRIO, "Receive first",  0 },
#endif

#ifdef _SEM
  { SEM_POST_TASK,          sem_task,             MORE_STACK + 500, SEM_POST_PRIO,      "Sem post",       0 },
  { SEM_POST_LAST_TASK,     sem_last_task,        MORE_STACK + 500, SEM_POST_LAST_PRIO, "Send post last", 0 },
  { SEM_WAIT_TASK,          sem_wait_task,        MORE_STACK + 500, SEM_WAIT_PRIO,      "Sem wait",       0 },
  { SEM_WAIT_FIRST_TASK,    sem_wait_first_task,  MORE_STACK + 500, SEM_WAIT_FIRST_PRIO, "Sem wait first", 0 },
#endif

#ifdef _LWSEM
  { LWSEM_POST_TASK,        lwsem_task,           MORE_STACK + 500, LWSEM_POST_PRIO,      "Sem post",       0 },
  { LWSEM_POST_LAST_TASK,   lwsem_last_task,      MORE_STACK + 500, LWSEM_POST_LAST_PRIO, "Send post last", 0 },
  { LWSEM_WAIT_TASK,        lwsem_wait_task,      MORE_STACK + 500, LWSEM_WAIT_PRIO,      "Sem wait",       0 },
  { LWSEM_WAIT_FIRST_TASK,  lwsem_wait_first_task, MORE_STACK + 500, LWSEM_WAIT_FIRST_PRIO, "Sem wait first", 0 },
#endif

#ifdef _EVENT_MUTEX
  { EVENT_SET_TASK,         event_task,           MORE_STACK + 500, EVENT_SET_PRIO,      "Event set",       0 },
  { EVENT_SET_LAST_TASK,    event_last_task,      MORE_STACK + 500, EVENT_SET_LAST_PRIO, "Event set last",  0 },
  { EVENT_WAIT_TASK,        event_wait_task,      MORE_STACK + 500, EVENT_WAIT_PRIO,      "Event wait",       0 },
  { EVENT_WAIT_FIRST_TASK,  event_wait_first_task, MORE_STACK + 500, EVENT_WAIT_FIRST_PRIO, "Event wait first", 0 },

  { LWEVENT_SET_TASK,        lwevent_task,           MORE_STACK + 500, LWEVENT_SET_PRIO,      "Event set",       0 },
  { LWEVENT_SET_LAST_TASK,   lwevent_last_task,      MORE_STACK + 500, LWEVENT_SET_LAST_PRIO, "Event set last",  0 },
  { LWEVENT_WAIT_TASK,       lwevent_wait_task,      MORE_STACK + 500, LWEVENT_WAIT_PRIO,      "Event wait",       0 },
  { LWEVENT_WAIT_FIRST_TASK, lwevent_wait_first_task, MORE_STACK + 500, LWEVENT_WAIT_FIRST_PRIO, "Event wait first", 0 },

  { MUTEX_UNLOCK_TASK,      mutex_task,           MORE_STACK + 500, MUTEX_UNLOCK_PRIO,     "Mutex unlock",      0 },
  { MUTEX_UNLOCK_LAST_TASK, mutex_last_task,      MORE_STACK + 500, MUTEX_UNLOCK_LAST_PRIO, "Mutex unlock last", 0 },
  { MUTEX_LOCK_TASK,        mutex_lock_task,      MORE_STACK + 500, MUTEX_LOCK_PRIO,      "Mutex lock",       0 },
  { MUTEX_LOCK_FIRST_TASK,  mutex_lock_first_task, MORE_STACK + 500, MUTEX_LOCK_FIRST_PRIO, "Mutex lock first", 0 },
#endif

  { 0,                0,                  0,                0, 0,               0 }
};

/* Timer variables */
volatile uint32_t          loop_overhead;
volatile uint32_t          end_hwticks;
volatile MQX_TICK_STRUCT  start_ticks;
volatile MQX_TICK_STRUCT  end_ticks;
uint32_t(*get_hwticks)(void *);
void   *get_hwticks_param;

/* Stored results (for bsps that cannot print) */
MQX_TICK_STRUCT  start_ticks_array[TEST_RESULTS_SIZE];
MQX_TICK_STRUCT  end_ticks_array[TEST_RESULTS_SIZE];
_mqx_uint        div_array[TEST_RESULTS_SIZE];
char            *test_id[TEST_RESULTS_SIZE];
bool          is_looping[TEST_RESULTS_SIZE];
int32_t           result_ns[TEST_RESULTS_SIZE];

const char      *null_string = "";

/* Data arrays */
uint32_t          temp_array[LOOP_OVERHEAD_AVG_SIZE+2];
volatile void   *test_array[TEST_ARRAY_SIZE+SEND_QUEUE+2];
_task_id         task_array[TASK_ARRAY_SIZE];
_queue_id        qid_array[TEST_ARRAY_SIZE+SEND_QUEUE+2];

_mqx_uint  old_prio;
_mqx_uint  ints;

/* Test variables */
_pool_id                 message_pool;
#ifdef _EVENT_MUTEX
char                     event_name[] = "event.A";
MUTEX_STRUCT    mutex[TEST_ARRAY_SIZE+1];
LWEVENT_STRUCT  lwevent;
LWEVENT_STRUCT  lwevent_array[TEST_ARRAY_SIZE+1];
#endif
#ifdef _LWSEM
LWSEM_STRUCT    lwsem[TEST_ARRAY_SIZE+1];
#endif
#ifdef _PARTITION
unsigned char           big_partition[64 +(TEST_ARRAY_SIZE *(4+64))+ 64];
unsigned char           partitions[TEST_ARRAY_SIZE][64 +(1 *(4 + 64))+ 64];
_partition_id            part;
#endif
#ifdef _LWMEM
unsigned char           dummy_lwmem_pool[24];
_lwmem_pool_id           lwmem_pool_id;
LWMEM_POOL_STRUCT        lwmem_pool;
#endif
void                    *taskq;

//
//const MQX_INITIALIZATION_STRUCT  MQX_init_struct =
//{
//  /* PROCESSOR_NUMBER                */  BSP_DEFAULT_PROCESSOR_NUMBER,
//  /* START_OF_KERNEL_MEMORY          */  BSP_DEFAULT_START_OF_KERNEL_MEMORY,
//  /* END_OF_KERNEL_MEMORY            */  BSP_DEFAULT_END_OF_KERNEL_MEMORY,
//  /* INTERRUPT_STACK_SIZE            */  1024 * 2,
//  /* TASK_TEMPLATE_LIST              */(void *)MQX_template_list,
//  /* MQX_HARDWARE_INTERRUPT_LEVEL_MAX*/  BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX,
//  /* MAX_MSG_POOLS                   */  TEST_ARRAY_SIZE + 1,
//  /* MAX_MSGQS                       */  SEND_QUEUE + TEST_ARRAY_SIZE + 2,
//  /* IO_CHANNEL                      */  BSP_DEFAULT_IO_CHANNEL,
//  /* IO_OPEN_MODE                    */  BSP_DEFAULT_IO_OPEN_MODE
//};

#if DEBUG_LEVEL
/*                   Display list of tasks         */

static void list_tasks(void)
{ /* Body */
  KERNEL_DATA_STRUCT_PTR kernel_data;
  TD_STRUCT_PTR          td_ptr;
  _mqx_uint              size;

  _GET_KERNEL_DATA(kernel_data);

  td_ptr = (TD_STRUCT_PTR)((unsigned char *)kernel_data->TD_LIST.NEXT - FIELD_OFFSET(TD_STRUCT, TD_LIST_INFO));
  size   = _QUEUE_GET_SIZE(&kernel_data->TD_LIST);

  while (size && td_ptr)
  {
    printf("TD: 0x%x  ID: 0x%x Next: 0x%x\n", td_ptr, td_ptr->TASK_ID, (TD_STRUCT_PTR)((unsigned char *)td_ptr->TD_LIST_INFO.NEXT - FIELD_OFFSET(TD_STRUCT, TD_LIST_INFO)));
    size--;
    td_ptr = (TD_STRUCT_PTR)((unsigned char *)td_ptr->TD_LIST_INFO.NEXT - FIELD_OFFSET(TD_STRUCT, TD_LIST_INFO));
  } /* Endwhile */

} /* Endbody */
#endif

#ifdef _TASKING
/*TASK*-------------------------------------------------------------------
*
* Task Name    : create_test_task
* Comments     :
*    This task does nothing, it never executes
*
*END*----------------------------------------------------------------------*/

void create_test_task(uint32_t dummy)
{ /* Body */
} /* Endbody */
#endif

#ifdef _MESSAGE
/*TASK*-------------------------------------------------------------------
*
* Task Name    : send_task
* Comments     :
*    This task waits for a message, then sends it to the next task.n
*
*END*----------------------------------------------------------------------*/

void send_task(uint32_t index)
{ /* Body */
  THE_MESSAGE_PTR msg_ptr;
  _queue_id       next_qid;
  _queue_id       my_qid;

  my_qid   = _msgq_open(SEND_QUEUE + index, 0);
  next_qid = _msgq_get_id(0, SEND_QUEUE + index + 1);

  /* Modify my task priority */
  set_priority(SEND_PRIO - index);

  /* Wait for main task to start me up */
  _int_disable();
  msg_ptr = (THE_MESSAGE_PTR)_msgq_receive_ticks(MSGQ_ANY_QUEUE, 0);
  msg_ptr->HEADER.TARGET_QID = next_qid;
  _msgq_send(msg_ptr);
  _msgq_close(my_qid);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : send_last_task
* Comments     :
*   This task receives the last message, then stops the timing.
*
*END*----------------------------------------------------------------------*/

void send_last_task(uint32_t index)
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  THE_MESSAGE_PTR          msg_ptr;
  _queue_id                next_qid;
  _queue_id                my_qid;

  kernel_data = _mqx_get_kernel_data();
  my_qid   = _msgq_open(SEND_QUEUE + index, 0);
  next_qid = _msgq_get_id(0, SEND_QUEUE + index + 1);

  /* Wait for main task to start me up */
  _int_disable();
  msg_ptr = (THE_MESSAGE_PTR)_msgq_receive_ticks(MSGQ_ANY_QUEUE, 0);
  END_TIME();
  _msg_free(msg_ptr);
  _msgq_close(my_qid);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : receive_task
* Comments     :
*    This task waits for a message
*
*END*----------------------------------------------------------------------*/

void receive_task(uint32_t index)
{ /* Body */
  THE_MESSAGE_PTR msg_ptr;
  _queue_id       my_qid;

  my_qid   = _msgq_open(SEND_QUEUE + index, 0);

  /* Modify my task priority */
  set_priority(RECEIVE_PRIO - index);

  /* Wait for main task to start me up */
  _int_disable();
  _task_block();
  msg_ptr = (THE_MESSAGE_PTR)_msgq_receive_ticks(MSGQ_ANY_QUEUE, 0);
  _msg_free(msg_ptr);
  _msgq_close(my_qid);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : receive_first_task
* Comments     :
*    This task waits for a message, first starting the timer
*
*END*----------------------------------------------------------------------*/

void receive_first_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT        *kernel_data;
  THE_MESSAGE_PTR msg_ptr;
  _queue_id       my_qid;

  kernel_data = _mqx_get_kernel_data();
  my_qid      = _msgq_open(SEND_QUEUE + index, 0);

  /* Wait for main task to start me up */
  _int_disable();
  _task_block();
  START_GET_TIME();
  msg_ptr = (THE_MESSAGE_PTR)_msgq_receive_ticks(MSGQ_ANY_QUEUE, 0);
  _msg_free(msg_ptr);
  _msgq_close(my_qid);
  _task_destroy(0);

} /* Endbody */
#endif

#ifdef _SEM
/*TASK*-------------------------------------------------------------------
*
* Task Name    : sem_task
* Comments     :
*    This task waits for a semaphore, then posts it
*
*END*----------------------------------------------------------------------*/

void sem_task
(
 uint32_t index
 )
{ /* Body */
  void   *sem_ptr;

  /* Modify my task priority */
  set_priority(SEM_POST_PRIO - index);

  _sem_open("sem.t", &sem_ptr);
  _int_disable();
  _sem_wait_ticks(sem_ptr, 0);
  _sem_post(sem_ptr);
  _sem_close(sem_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : sem_last_task
* Comments     :
*    This task waits for a semaphore, then stops the timer.
*
*END*----------------------------------------------------------------------*/

void sem_last_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  void                  *sem_ptr;

  kernel_data = _mqx_get_kernel_data();
  _sem_open("sem.t", &sem_ptr);
  _int_disable();
  _sem_wait_ticks(sem_ptr, 0);
  END_TIME();
  _sem_post(sem_ptr);
  _sem_close(sem_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : sem_wait_task
* Comments     :
*    This task waits for a semaphore
*
*END*----------------------------------------------------------------------*/

void sem_wait_task
(
 uint32_t index
 )
{ /* Body */
  void   *sem_ptr;

  _sem_open("sem.t", &sem_ptr);
  _int_disable();
  _task_block();
  _sem_wait_ticks(sem_ptr, 0);
  _sem_post(sem_ptr);
  _sem_close(sem_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : sem_wait_first_task
* Comments     :
*    This task waits for a semaphore, first starting the timer
*
*END*----------------------------------------------------------------------*/

void sem_wait_first_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  void                  *sem_ptr;

  kernel_data = _mqx_get_kernel_data();

  _sem_open("sem.t", &sem_ptr);
  _int_disable();
  _task_block();
  START_GET_TIME();
  _sem_wait_ticks(sem_ptr, 0);
  _sem_post(sem_ptr);
  _sem_close(sem_ptr);
  _task_destroy(0);

} /* Endbody */
#endif

#ifdef _LWSEM
/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwsem_task
* Comments     :
*    This task waits for a light weight semaphore, then posts it
*
*END*----------------------------------------------------------------------*/

void lwsem_task
(
 uint32_t index
 )
{ /* Body */
  LWSEM_STRUCT_PTR lwsem_ptr = &lwsem[0];

  /* Modify my task priority */
  set_priority(LWSEM_POST_PRIO - index);

  _int_disable();
  _lwsem_wait(lwsem_ptr);
  _lwsem_post(lwsem_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwsem_last_task
* Comments     :
*    This task waits for a light weight semaphore, then stops the timer.
*
*END*----------------------------------------------------------------------*/

void lwsem_last_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  LWSEM_STRUCT_PTR lwsem_ptr = &lwsem[0];

  kernel_data = _mqx_get_kernel_data();
  _int_disable();
  _lwsem_wait(lwsem_ptr);
  END_TIME();
  _lwsem_post(lwsem_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwsem_wait_task
* Comments     :
*    This task waits for a light weight semaphore
*
*END*----------------------------------------------------------------------*/

void lwsem_wait_task
(
 uint32_t index
 )
{ /* Body */
  LWSEM_STRUCT_PTR lwsem_ptr = &lwsem[index];

  _int_disable();
  _task_block();
  _lwsem_wait(lwsem_ptr);
  _lwsem_post(lwsem_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwsem_wait_first_task
* Comments     :
*    This task waits for a light weight semaphore, first starting the timer
*
*END*----------------------------------------------------------------------*/

void lwsem_wait_first_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  LWSEM_STRUCT_PTR lwsem_ptr = &lwsem[index];

  kernel_data = _mqx_get_kernel_data();

  _int_disable();
  _task_block();
  START_GET_TIME();
  _lwsem_wait(lwsem_ptr);
  _lwsem_post(lwsem_ptr);
  _task_destroy(0);

} /* Endbody */
#endif

#ifdef _EVENT_MUTEX
/*TASK*-------------------------------------------------------------------
*
* Task Name    : event_task
* Comments     :
*    This task waits for a event, then posts it
*
*END*----------------------------------------------------------------------*/

void event_task
(
 uint32_t index
 )
{ /* Body */
  void   *event_ptr;
  void   *event2_ptr;

  /* Modify my task priority */
  set_priority(EVENT_SET_PRIO - index);

  event_name[6] = 'A' + index;
  _event_open(event_name, &event_ptr);
  event_name[6] = 'A' + index + 1;
  _event_open(event_name, &event2_ptr);
  _int_disable();
  _event_wait_any_ticks(event_ptr, 1, 0);
  _event_set(event2_ptr, 1);
  _event_close(event_ptr);
  _event_close(event2_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : event_last_task
* Comments     :
*    This task waits for an event, then stops the timer.
*
*END*----------------------------------------------------------------------*/

void event_last_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  void                  *event_ptr;

  kernel_data = _mqx_get_kernel_data();

  event_name[6] = 'A' + index;
  _event_open(event_name, &event_ptr);
  _int_disable();
  _event_wait_any_ticks(event_ptr, 1, 0);
  END_TIME();
  _event_close(event_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : event_wait_task
* Comments     :
*    This task waits for an event
*
*END*----------------------------------------------------------------------*/

void event_wait_task
(
 uint32_t index
 )
{ /* Body */
  void   *event_ptr;

  event_name[6] = 'A';
  _event_open(event_name, &event_ptr);
  _int_disable();
  _task_block();
  _event_wait_any_ticks(event_ptr, MAX_MQX_UINT, 0);
  _event_close(event_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : event_wait_first_task
* Comments     :
*    This task waits for an event, first starting the timer
*
*END*----------------------------------------------------------------------*/

void event_wait_first_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  void                  *event_ptr;

  kernel_data = _mqx_get_kernel_data();

  event_name[6] = 'A';
  _event_open(event_name, &event_ptr);
  _int_disable();
  _task_block();
  START_GET_TIME();
  _event_wait_any_ticks(event_ptr, MAX_MQX_UINT, 0);
  _event_close(event_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwevent_task
* Comments     :
*    This task waits for a lwevent, then posts it
*
*END*----------------------------------------------------------------------*/

void lwevent_task
(
 uint32_t index
 )
{ /* Body */
  LWEVENT_STRUCT_PTR lwevent1_ptr = &lwevent_array[index];
  LWEVENT_STRUCT_PTR lwevent2_ptr = &lwevent_array[index + 1];

  /* Modify my task priority */
  set_priority(LWEVENT_SET_PRIO - index);

  _int_disable();
  _lwevent_wait_ticks(lwevent1_ptr, 1, FALSE, 0);
  _lwevent_set(lwevent2_ptr, 1);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwevent_last_task
* Comments     :
*    This task waits for an lwevent, then stops the timer.
*
*END*----------------------------------------------------------------------*/

void lwevent_last_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  LWEVENT_STRUCT_PTR lwevent1_ptr = &lwevent_array[index];

  kernel_data = _mqx_get_kernel_data();
  _int_disable();
  _lwevent_wait_ticks(lwevent1_ptr, 1, FALSE, 0);
  END_TIME();
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwevent_wait_task
* Comments     :
*    This task waits for an lwevent
*
*END*----------------------------------------------------------------------*/

void lwevent_wait_task
(
 uint32_t index
 )
{ /* Body */
  LWEVENT_STRUCT_PTR lwevent1_ptr = &lwevent_array[index];

  _int_disable();
  _task_block();
  _lwevent_wait_ticks(lwevent1_ptr, MAX_MQX_UINT, FALSE, 0);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : lwevent_wait_first_task
* Comments     :
*    This task waits for an lwevent, first starting the timer
*
*END*----------------------------------------------------------------------*/

void lwevent_wait_first_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  LWEVENT_STRUCT_PTR lwevent1_ptr = &lwevent_array[index];

  kernel_data = _mqx_get_kernel_data();

  _int_disable();
  _task_block();
  START_GET_TIME();
  _lwevent_wait_ticks(lwevent1_ptr, MAX_MQX_UINT, FALSE, 0);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : mutex_task
* Comments     :
*    This task locks a mutex, then unlocks it
*
*END*----------------------------------------------------------------------*/

void mutex_task
(
 uint32_t index
 )
{ /* Body */
  MUTEX_STRUCT_PTR mutex1_ptr = &mutex[index];
  MUTEX_STRUCT_PTR mutex2_ptr = &mutex[index + 1];

  /* Modify my task priority */
  set_priority(MUTEX_UNLOCK_PRIO - (_mqx_uint)index);

  _int_disable();
  _mutex_lock((void *)mutex2_ptr);
  _mutex_lock((void *)mutex1_ptr);
  _mutex_unlock((void *)mutex2_ptr);
  _mutex_unlock((void *)mutex1_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : mutex_last_task
* Comments     :
*    This task locks a mutex, then stops the timer.
*
*END*----------------------------------------------------------------------*/

void mutex_last_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  MUTEX_STRUCT_PTR mutex1_ptr = &mutex[index];

  kernel_data = _mqx_get_kernel_data();

  _int_disable();
  _mutex_lock((void *)mutex1_ptr);
  END_TIME();
  _mutex_unlock((void *)mutex1_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : mutex_lock_task
* Comments     :
*    This task locks a mutex
*
*END*----------------------------------------------------------------------*/

void mutex_lock_task
(
 uint32_t index
 )
{ /* Body */
  MUTEX_STRUCT_PTR mutex1_ptr = &mutex[index];

  _int_disable();
  _task_block();
  _mutex_lock((void *)mutex1_ptr);
  _mutex_unlock((void *)mutex1_ptr);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : mutex_lock_first_task
* Comments     :
*    This task locks a mutex, first starting the timer
*
*END*----------------------------------------------------------------------*/

void mutex_lock_first_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  MUTEX_STRUCT_PTR mutex1_ptr = &mutex[index];

  kernel_data = _mqx_get_kernel_data();

  _int_disable();
  _task_block();
  START_GET_TIME();
  _mutex_lock((void *)mutex1_ptr);
  _mutex_unlock((void *)mutex1_ptr);
  _task_destroy(0);

} /* Endbody */
#endif

/*NOTIFIER*-----------------------------------------------------------------
*
* Notifier Name : test_notifier
* Comments      :
*   This is the interrupt notifier for the interrupt being tested.
*
*END*----------------------------------------------------------------------*/

void test_notifier
(
 void
 )
{ /* Body */
  /* Dont do anything */
} /* Endbody */

#ifdef _TASKING
/*TASK*-------------------------------------------------------------------
*
* Task Name    : add_ready_task
* Comments     :
*    This task requests add readies his parent.
*
*END*----------------------------------------------------------------------*/

void add_ready_task
(
 uint32_t dummy
 )
{ /* Body */

  while (TRUE)
  {
    _task_block();
  } /* Endwhile */

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : add_ready_first_task
* Comments     :
*    This task starts the timer, then blocks.
*
*END*----------------------------------------------------------------------*/

void add_ready_first_task
(
 uint32_t dummy
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;

  kernel_data = _mqx_get_kernel_data();
  while (TRUE)
  {
    START_GET_TIME();
    _task_block();
  } /* Endwhile */

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : suspend_task
* Comments     :
*    This task suspends onto the task queue.
*
*END*----------------------------------------------------------------------*/

void suspend_task
(
 uint32_t dummy
 )
{ /* Body */

  _taskq_suspend(taskq);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : suspend_first_task
* Comments     :
*    This task starts the timer, then suspends.
*
*END*----------------------------------------------------------------------*/

void suspend_first_task
(
 uint32_t dummy
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;

  kernel_data = _mqx_get_kernel_data();
  START_TIME();
  _taskq_suspend(taskq);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : resume_task
* Comments     :
*    This task suspends onto the task queue, then resumes 1 task.
*
*END*----------------------------------------------------------------------*/

void resume_task
(
 uint32_t index
 )
{ /* Body */

  set_priority(RESUME_PRIO - (_mqx_uint)index);
  _taskq_suspend(taskq);
  _taskq_resume(taskq, MQX_TASK_QUEUE_RESUME_ONE);
  _task_destroy(0);

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : resume_last_task
* Comments     :
*    This task suspends on the taskq, then collects the final time.
*
*END*----------------------------------------------------------------------*/

void resume_last_task
(
 uint32_t index
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;

  kernel_data = _mqx_get_kernel_data();
  _taskq_suspend(taskq);
  END_TIME();
  _task_destroy(0);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : Analogous_clock_notifier
* Return value  : none
* Comments      :
*    This function performs the same amount of work that the clock notifier
* does under MQX.  It is used to determine the amount of overhead during
* the performance tests.
*
*END*----------------------------------------------------------------------*/

typedef struct test_time  {
  uint32_t         T;
  MQX_TICK_STRUCT TIME;
  QUEUE_STRUCT    QUEUE;
  void           *TIMER_COMPONENT_ISR;
} TEST_TIME_STRUCT;

TEST_TIME_STRUCT Test_time_struct = { 0 };

void analogous_clock_notifier
(
 void
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  TD_STRUCT_PTR                     td_ptr;
  _mqx_uint                         count;
  TEST_TIME_STRUCT                 *test_ptr;

  kernel_data = _mqx_get_kernel_data();

  /* Update the current time. */
  test_ptr = &Test_time_struct;
  PSP_INC_TICKS(&test_ptr->TIME);

  _INT_DISABLE();

  /* Check if the currently running task is a time slice task
  ** and if his time has expired, put him at the end of his queue
  */
  td_ptr = kernel_data->ACTIVE_PTR;
  #if MQX_HAS_TIME_SLICE
  if ( td_ptr->FLAGS & MQX_TIME_SLICE_TASK )
  {
    /* NOT taken */
    null_function();
  } /* Endif */
  #endif

  /*
  ** Check for tasks on the timeout queue, and wake the appropriate
  ** ones up.  The timeout queue is a time-priority queue.
  */
  count = _QUEUE_GET_SIZE(&test_ptr->QUEUE);
  if ( count )
  {
    /* NOT taken */
    null_function();
  } /* Endif */

  _INT_ENABLE();

  if ( test_ptr->TIMER_COMPONENT_ISR != NULL )
  {
    null_function();
  } /* Endif */

} /* Endbody */
#endif

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : null_function
* Return value  : none
* Comments      :
*    This function does nothing
*
*END*----------------------------------------------------------------------*/

volatile int null_function_probe;
void null_function
(
 void
 )
{ /* Body */
  null_function_probe = 0;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : check_created_tasks
* Return value  : none
* Comments      :
*    This function checks to make sure all tasks were created
*
*END*----------------------------------------------------------------------*/

static void check_created_tasks
(
 uint32_t test_number
 )
{ /* Body */
  _mqx_uint i;

  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    if ( task_array[i] == 0 )
    {
      printf("TASK CREATE FAILED FOR TEST %d, index %d\n", test_number, i);
      break;
    } /* Endif */
  } /* Endfor */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : check_operation
* Return value  : none
* Comments      :
*    This function checks to make sure all operations succeeded
*
*END*----------------------------------------------------------------------*/

static void check_operation
(
 uint32_t test_number
 )
{ /* Body */
  _mqx_uint i;

  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    if ( test_array[i] == 0 )
    {
      printf("OPERATION FAILED FOR TEST %d, index %d\n", test_number, i);
      break;
    } /* Endif */
  } /* Endfor */

} /* Endbody */


/*TASK*-------------------------------------------------------------------
*
* Task Name    : main_task
* Comments     :
*   Sets the time, and starts the sending tasks.
*   Calculate the time it takes for the test.
*
*END*----------------------------------------------------------------------*/


void main_task
(
 uint32_t dummy
 )
{ /* Body */
  volatile KERNEL_DATA_STRUCT      *kernel_data;
  THE_MESSAGE_PTR          msg_ptr;
  _queue_id                my_qid;
  _pool_id                 pool;
  _mqx_uint                i, j;
  _mqx_uint                config;
  _mqx_uint                config2;

  _int_install_unexpected_isr();

  kernel_data = _mqx_get_kernel_data();
  get_hwticks = kernel_data->GET_HWTICKS;
  get_hwticks_param = kernel_data->GET_HWTICKS_PARAM;

#ifndef NO_IO
  printf("\n# MQX timing test.\n");
#endif

  DEBUGM(printf("\nhwticks per tick %d, ticks per sec %d\n",
                kernel_data->HW_TICKS_PER_TICK, kernel_data->TICKS_PER_SECOND);
         )

  memset(start_ticks_array,0xFFFFFFFF,sizeof(start_ticks_array));
  memset(end_ticks_array, 0xFFFFFFFF, sizeof(end_ticks_array));
  for (i = 0; i < TEST_RESULTS_SIZE; i++)
  {
    test_id[i] = (char *)null_string;
    is_looping[i] = FALSE;
  } /* Endfor */

  /* Create all components */
#ifdef _EVENT_MUTEX
  _event_create_component(TEST_ARRAY_SIZE + 1, 0, 0);
  _mutex_create_component();
  _lwevent_create(&lwevent, 0);
  _lwevent_destroy(&lwevent);
#endif
#ifdef _SEM
  _sem_create_component(TEST_ARRAY_SIZE, 0, 0);
#endif
#ifdef _MESSAGE
  _msg_create_component();
  my_qid = _msgq_open(MAIN_QUEUE, 0);
#endif
#ifdef _LWMEM
  _lwmem_create_pool(&lwmem_pool, partitions, 24);
#endif


  /* =================== CALIBRATION ====================== */
  /*
  ** run consecutive tests and average results but throw away the
  ** first result since loop may not have been in cache
  */

  store_heading(HEADING_System_Parameters, "System Parameters");


  for (j = 0; j < (LOOP_OVERHEAD_AVG_SIZE + 1); j++)
  {
    START_TIME();
    for (i = 0; i < TEST_ARRAY_SIZE; ++i)
    {
      null_function();
      test_array[i] = (void *)i;
    } /* Endfor */
    END_TIME();
    temp_array[j] = end_ticks.HW_TICKS - start_ticks.HW_TICKS;
  } /* Endfor */
  loop_overhead = 0;
  for (j = 1; j < (LOOP_OVERHEAD_AVG_SIZE + 1); j++)
  {
    loop_overhead += temp_array[j];
  } /* Endfor */
  loop_overhead /= LOOP_OVERHEAD_AVG_SIZE;

  printf("# Loop calibration = %lu hardware ticks\n", loop_overhead);

  DEBUGM(printf("Testmem result 0x%x\n", _mem_test());
         )
  DEBUGM(print_memory_pool();
         )

#ifdef _TASKING
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i] = (void *)i;
    analogous_clock_notifier();
  } /* Endfor */
  END_TIME();
  store_result(RESULT_System_Timer_Tick_Overhead, "System Timer/Tick Overhead", 1, TRUE);
#endif
  DEBUGM(printf("Testmem result 0x%x\n", _mem_test());
         )
  DEBUGM(print_memory_pool();
         )

  /* =================== MEMORY MANAGEMENT ================ */

  store_heading(HEADING_Memory_Management,"Memory Management");

#ifdef _TASKING
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]= _mem_alloc(sizeof(MEMBLOCK_STRUCT));
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Allocate_Memory);
  store_result(RESULT_Allocate_Memory, "Allocate Memory", 1, TRUE);

  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _mem_free((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Free_Memory, "Free Memory", 1, TRUE);

  DEBUGM(printf("Testmem result 0x%x\n", _mem_test());
         )
  DEBUGM(print_memory_pool();
         )

  /* =================== TASK MANAGEMENT ================== */

  store_heading(HEADING_Task_Management,"Task Management");

  /* Modify my task priority to be higher */
  set_priority(MAIN_PRIO - 1);
  _mem_zero((void *)task_array, sizeof(task_array));
  START_TIME();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    null_function();
    DEBUGTASKERR(list_tasks());
    task_array[i]= _task_create(0, CREATE_TEST_TASK, (uint32_t)i);
  } /* Endfor */
  END_TIME();
  check_created_tasks(RESULT_Create_Task);
  store_result(RESULT_Create_Task, "Create Task", 1, TRUE);

  DEBUGM(_klog_show_stack_usage();
         )

  START_TIME();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    null_function();
    DEBUGTASKERR(list_tasks());
    _task_destroy((_task_id)task_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Delete_Task, "Delete Task", 1, TRUE);

  _mem_zero((void *)task_array, sizeof(task_array));
  _mem_zero((void *)test_array, sizeof(test_array));
  task_array[0]= _task_create(0, ADD_READY_FIRST_TASK, (uint32_t)0);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    null_function();
    task_array[i]= _task_create(0, ADD_READY_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Ready_Task);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    test_array[i]= _task_get_td(task_array[i]);
  } /* Endfor */
  for (i = TASK_ARRAY_SIZE; i < TEST_ARRAY_SIZE; ++i)
  {
    test_array[i]= task_array;
  }
  check_operation(RESULT_Ready_Task);
  /* Let the tasks run */
  _time_delay_ticks(20);
  START_TIME();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    null_function();
    _task_ready((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Ready_Task, "Ready Task", 1, TRUE);


  /* Wait for a timer tick to expire by POLLING */
  WAIT_FOR_TICK(i);
  _int_disable();
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Block_Task, "Block Task", 1, FALSE);
  store_result(RESULT_Context_Switch, "Context Switch", 1, FALSE);
  _time_delay_ticks(4);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_destroy(task_array[i]);
  } /* Endfor */
  _time_delay_ticks(10);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]=(void *)i;
    _sched_yield();
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Yield_Task, "Yield Task", 1, TRUE);

  /* =================== TASK QUEUES ================== */

  store_heading(HEADING_Task_Queues, "Task Queues");

  _mem_zero((void *)task_array, sizeof(task_array));
  taskq = _taskq_create(0);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, SUSPEND_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Suspend_Task);
  START_TIME();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    null_function();
    _taskq_suspend_task(task_array[i], taskq);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Suspend_Task, "Suspend Task", 1, TRUE);

  START_TIME();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    null_function();
    _taskq_resume(taskq, MQX_TASK_QUEUE_RESUME_ONE);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Resume_Task, "Resume Task", 1, TRUE);
  _time_delay_ticks(4);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_destroy(task_array[i]);
  } /* Endfor */


  _mem_zero((void *)task_array, sizeof(task_array));
  task_array[0]= _task_create(0, SUSPEND_FIRST_TASK, (uint32_t)0);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, SUSPEND_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Suspend_Task_Context_Switch);
  /* Wait for a timer tick to expire by POLLING */
  WAIT_FOR_TICK(i);
  _int_disable();
  _sched_yield();
  END_TIME();
  store_result(RESULT_Suspend_Task_Context_Switch, "Suspend Task Context Switch", 1, FALSE);
  _time_delay_ticks(4);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_destroy(task_array[i]);
  } /* Endfor */


  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, RESUME_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, RESUME_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Resume_Task_Context_Switch);
  START_TIME();
  _taskq_resume(taskq, MQX_TASK_QUEUE_RESUME_ONE);
  _int_enable();
  store_result(RESULT_Resume_Task_Context_Switch, "Resume Task Context Switch", 1, FALSE);
#endif

  /* =================== PARTITIONS ================== */

#ifdef _PARTITION
  store_heading(HEADING_Partitions, "Partitions");

  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]= _partition_create_at((void *)&partitions[i][0], 64 + (1 * (4 + 64)), 1);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Partition_Create);
  store_result(RESULT_Partition_Create, "Partition Create", 1, TRUE);


  _mem_zero((void *)test_array, sizeof(test_array));
  part = _partition_create_at((void *)&big_partition[0], 64 + (TEST_ARRAY_SIZE * (4 + 64)), 1);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]= _partition_alloc(part);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Partition_Allocate_Block);
  store_result(RESULT_Partition_Allocate_Block, "Partition Allocate Block", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _partition_free((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Partition_Free_Block, "Partition Free Block", 1, TRUE);
#endif

  /* =================== SEMAPHORES ======================= */

#ifdef _SEM
  store_heading(HEADING_Semaphores, "Semaphores");

  if ( _sem_create("sem.t", TEST_ARRAY_SIZE, 0) != MQX_OK )
  {
    printf("sem create failed test 36\n");
  } /* Endif */
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _sem_open("sem.t", (void **)&test_array[i]);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Open_Semaphore);
  store_result(RESULT_Open_Semaphore, "Open Semaphore", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _sem_wait_ticks((void *)test_array[0], 0);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Wait_Semaphore_ticks, "Wait Semaphore (ticks)", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _sem_post((void *)test_array[0]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_Semaphore, "Set Semaphore", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _sem_close((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Close_Semaphore, "Close Semaphore", 1, TRUE);
  _sem_destroy("sem.t", TRUE);


  if ( _sem_create_fast(0, 1, 0) != MQX_OK )
  {
    printf("sem create fast failed test 37\n");
  } /* Endif */
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _sem_open_fast(0, (void **)&test_array[i]);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Open_Semaphore_Fast);
  store_result(RESULT_Open_Semaphore_Fast, "Open Semaphore Fast", 1, TRUE);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _sem_close((void *)test_array[i]);
  } /* Endfor */


  if ( _sem_create("sem.t", 1, 0) != MQX_OK )
  {
    printf("Sem create failed test 40\n");
  } /* Endif */
  _sem_open("sem.t", (void **)&test_array[0]);
  if ( test_array[0] == 0 )
  {
    printf("Sem open failed test 40\n");
  } /* Endif */
  _sem_wait_ticks((void *)test_array[0], 0);
  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, SEM_POST_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, SEM_POST_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Set_Semaphore_Context_Switch);
  START_TIME();
  _sem_post((void *)test_array[0]);
  _int_enable();
  store_result(RESULT_Set_Semaphore_Context_Switch, "Set Semaphore Context Switch", 1, FALSE);
  _sem_close((void *)test_array[0]);
  _sem_destroy("sem.t", TRUE);


  if ( _sem_create("sem.t", TEST_ARRAY_SIZE, 0) != MQX_OK )
  {
    printf("Sem create failed test 39\n");
  } /* Endif */
  _sem_open("sem.t", (void **)&test_array[0]);
  if ( test_array[0] == 0 )
  {
    printf("Sem open failed test 39\n");
  } /* Endif */
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _sem_wait_ticks((void *)test_array[0], 0);
  } /* Endfor */
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, SEM_POST_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Set_Semaphore_Ready_Task);
  set_priority(SEM_POST_LAST_PRIO);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _sem_post((void *)test_array[0]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_Semaphore_Ready_Task, "Set Semaphore Ready Task", 1, TRUE);
  _sem_close((void *)test_array[0]);
  set_priority(MAIN_PRIO);
  _time_delay_ticks(2);
  _sem_destroy("sem.t", TRUE);


  _mem_zero((void *)task_array, sizeof(task_array));
  if ( _sem_create("sem.t", 1, 0) != MQX_OK )
  {
    printf("Sem create failed test 42\n");
  } /* Endif */
  _sem_open("sem.t", (void **)&test_array[0]);
  if ( test_array[0] == 0 )
  {
    printf("Sem open failed test 42\n");
  } /* Endif */
  _sem_wait_ticks((void *)test_array[0], 0);
  task_array[0]= _task_create(0, SEM_WAIT_FIRST_TASK, 0);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, SEM_WAIT_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Wait_Semaphore_ticks_Block_Task);
  set_priority(MAIN_PRIO - 2);
  _int_disable();
  _time_delay_ticks(2);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Wait_Semaphore_ticks_Block_Task, "Wait Semaphore (ticks) Block Task", 1, FALSE);
  _sem_post((void *)test_array[0]);
  _time_delay_ticks(2);
  _sem_destroy("sem.t", TRUE);
#endif

  /* =================== LIGHT WEIGHT SEMAPHORES ======================= */

#ifdef _LWSEM
  store_heading(HEADING_Light_Weight_Semaphores, "Light Weight Semaphores");

  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    test_array[i]= &lwsem[i];
  } /* Endfor */
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwsem_create((LWSEM_STRUCT_PTR)test_array[i], TEST_ARRAY_SIZE);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Open_Light_Weight_Semaphore, "Open LW-Semaphore", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwsem_wait((LWSEM_STRUCT_PTR) & lwsem[0]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Wait_Light_Weight_Semaphore, "Wait LW-Semaphore", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwsem_post((LWSEM_STRUCT_PTR) & lwsem[0]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_Light_Weight_Semaphore, "Set LW-Semaphore", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwsem_destroy((LWSEM_STRUCT_PTR)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Close_Light_Weight_Semaphore, "Close LW-Semaphore", 1, TRUE);


  _mem_zero((void *)task_array, sizeof(task_array));
  _lwsem_create((LWSEM_STRUCT_PTR) & lwsem[0], 1);
  _lwsem_wait((LWSEM_STRUCT_PTR) & lwsem[0]);
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, LWSEM_POST_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, LWSEM_POST_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Set_Light_Weight_Semaphore_Context_Switch);
  START_TIME();
  _lwsem_post((LWSEM_STRUCT_PTR) & lwsem[0]);
  _int_enable();
  store_result(RESULT_Set_Light_Weight_Semaphore_Context_Switch, "Set LW-Semaphore Context Switch", 1, FALSE);
  _time_delay_ticks(2);
  _lwsem_destroy((LWSEM_STRUCT_PTR) & lwsem[0]);


  _time_delay_ticks(2);
  _mem_zero((void *)task_array, sizeof(task_array));
  _lwsem_create((LWSEM_STRUCT_PTR) & lwsem[0], 1);
  _lwsem_wait((LWSEM_STRUCT_PTR) & lwsem[0]);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, LWSEM_POST_TASK, 0);
  } /* Endfor */
  check_created_tasks(RESULT_Set_Light_Weight_Semaphore_Ready_Task);
  set_priority(LWSEM_POST_LAST_PRIO);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwsem_post((LWSEM_STRUCT_PTR) & lwsem[0]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_Light_Weight_Semaphore_Ready_Task, "Set LW-Semaphore Ready Task", 1, TRUE);
  set_priority(MAIN_PRIO);
  _time_delay_ticks(2);
  _lwsem_destroy((LWSEM_STRUCT_PTR) & lwsem[0]);


  _mem_zero((void *)task_array, sizeof(task_array));
  _lwsem_create((LWSEM_STRUCT_PTR) & lwsem[0], 1);
  _lwsem_wait((LWSEM_STRUCT_PTR) & lwsem[0]);
  task_array[0]= _task_create(0, LWSEM_WAIT_FIRST_TASK, 0);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, LWSEM_WAIT_TASK, 0);
  } /* Endfor */
  check_created_tasks(RESULT_Wait_Light_Weight_Semaphore_Block_Task);
  set_priority(MAIN_PRIO - 2);
  _int_disable();
  _time_delay_ticks(2);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Wait_Light_Weight_Semaphore_Block_Task, "Wait LW-Semaphore Block Task", 1, FALSE);
  _lwsem_post((LWSEM_STRUCT_PTR) & lwsem[0]);
  _time_delay_ticks(2);
  _lwsem_destroy((LWSEM_STRUCT_PTR) & lwsem[0]);
#endif

  /* =================== EVENTS =========================== */

#ifdef _EVENT_MUTEX
  store_heading(HEADING_Events, "Events");

  if ( _event_create("event.t") != MQX_OK )
  {
    printf("Event create failed test 44\n");
  } /* Endif */
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _event_open("event.t", (void **)&test_array[i]);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Open_Event);
  store_result(RESULT_Open_Event, "Open Event", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _event_close((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Close_Event, "Close Event", 1, TRUE);


  _event_open("event.t", (void **)&test_array[0]);
  if ( test_array[0] == 0 )
  {
    printf("Event open failed, test 49\n");
  } /* Endif */
  _event_set((void *)test_array[0], MAX_MQX_UINT);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _event_wait_any_ticks((void *)test_array[0], 0x1, 0);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Wait_Event_ticks, "Wait Event (ticks)", 1, TRUE);

  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _event_set((void *)test_array[0], 1);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_Event, "Set Event", 1, TRUE);
  _event_clear((void *)test_array[0], MAX_MQX_UINT);
  _event_destroy("event.t");

  if ( _event_create_fast(0) != MQX_OK )
  {
    printf("Event create fast failed test 45\n");
  } /* Endif */
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _event_open_fast(0, (void **)&test_array[i]);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Open_Event_Fast);
  store_result(RESULT_Open_Event_Fast, "Open Event Fast", 1, TRUE);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _event_close((void *)test_array[i]);
  } /* Endfor */
  _event_destroy_fast(0);


  _mem_zero((void *)test_array, sizeof(test_array));
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    event_name[6]= 'A' + i;
    _event_create(event_name);
    _event_open(event_name, (void **)&test_array[i]);
  } /* Endfor */
  check_operation(RESULT_Set_Event_Context_Switch);
  event_name[6]= 'A' + TEST_ARRAY_SIZE;
  _event_create(event_name);
  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, EVENT_SET_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, EVENT_SET_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Set_Event_Context_Switch);
  START_TIME();
  _event_set((void *)test_array[0], 1);
  _int_enable();
  store_result(RESULT_Set_Event_Context_Switch, "Set Event Context Switch", 1, FALSE);
  _event_clear((void *)test_array[0], MAX_MQX_UINT);
  _time_delay_ticks(2);


  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _event_clear((void *)test_array[i], MAX_MQX_UINT);
    task_array[i]= _task_create(0, EVENT_SET_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Set_Event_Ready_Task);
  set_priority(EVENT_SET_LAST_PRIO);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _event_set((void *)test_array[i], 1);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_Event_Ready_Task, "Set Event Ready Task", 1, TRUE);
  set_priority(MAIN_PRIO);


  _mem_zero((void *)task_array, sizeof(task_array));
  task_array[0]= _task_create(0, EVENT_WAIT_FIRST_TASK, 0);
  _event_clear((void *)test_array[0], MAX_MQX_UINT);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    _event_clear((void *)test_array[i], MAX_MQX_UINT);
    task_array[i]= _task_create(0, EVENT_WAIT_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Wait_Event_ticks_Block_Task);
  set_priority(MAIN_PRIO - 2);
  _int_disable();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Wait_Event_ticks_Block_Task, "Wait Event (ticks) Block Task", 1, FALSE);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _event_set((void *)test_array[i], MAX_MQX_UINT);
  } /* Endfor */
  _time_delay_ticks(2);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _event_close((void *)test_array[i]);
    event_name[6]= 'A' + i;
    _event_destroy(event_name);
  } /* Endif */
  _time_delay_ticks(2);

  /* =================== MUTEXES ========================== */

  store_heading(HEADING_Mutexes, "Mutexes");
  for (i = 0; i < TEST_ARRAY_SIZE + 1; ++i)
  {
    test_array[i]= &mutex[i];
  } /* Endfor */
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _mutex_init((void *)test_array[i], NULL);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Open_Mutex, "Open Mutex", 1, TRUE);

  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _mutex_destroy((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Close_Mutex, "Close Mutex", 1, TRUE);

  for (i = 0; i < TEST_ARRAY_SIZE + 1; ++i)
  {
    _mutex_init((void *)test_array[i], NULL);
  } /* Endfor */
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _mutex_lock((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Lock_Mutex, "Lock Mutex", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _mutex_unlock((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Unlock_Mutex, "Unlock Mutex", 1, TRUE);


  _mutex_lock((void *)&mutex[0]);
  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, MUTEX_UNLOCK_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, MUTEX_UNLOCK_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Unlock_Mutex_Context_Switch);
  START_TIME();
  _mutex_unlock((void *)&mutex[0]);
  _int_enable();
  store_result(RESULT_Unlock_Mutex_Context_Switch, "Unlock Mutex Context Switch", 1, FALSE);
  _time_delay_ticks(2);


  for (i = 1; i < TEST_ARRAY_SIZE + 1; ++i)
  {
    _mutex_lock((void *)test_array[i]);
  } /* Endfor */
  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, MUTEX_UNLOCK_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Unlock_Mutex_Ready_Task);
  _time_delay_ticks(2);
  set_priority(MUTEX_UNLOCK_LAST_PRIO);
  START_TIME();
  for (i = 1; i < TEST_ARRAY_SIZE + 1; ++i)
  {
    null_function();
    _mutex_unlock((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Unlock_Mutex_Ready_Task, "Unlock Mutex Ready Task", 1, TRUE);
  set_priority(MAIN_PRIO);
  _time_delay_ticks(2);


  _mem_zero((void *)task_array, sizeof(task_array));
  task_array[0]= _task_create(0, MUTEX_LOCK_FIRST_TASK, 0);
  _mutex_lock((void *)&mutex[0]);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    _mutex_lock((void *)test_array[i]);
    task_array[i]= _task_create(0, MUTEX_LOCK_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Lock_Mutex_Block_Task);
  set_priority(MAIN_PRIO - 2);
  _int_disable();
  _time_delay_ticks(2);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Lock_Mutex_Block_Task, "Lock Mutex Block Task", 1, FALSE);
  _time_delay_ticks(2);
  for (i = 0; i < TEST_ARRAY_SIZE + 1; ++i)
  {
    _mutex_unlock((void *)test_array[i]);
    _mutex_destroy((void *)test_array[i]);
  } /* Endfor */
  _time_delay_ticks(2);
#endif

  /* =================== POOL MANAGEMENT ================== */

#ifdef _MESSAGE
  store_heading(HEADING_Message_Pools, "Message Pools");

  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]=(void *)_msgpool_create(sizeof(MESSAGE_HEADER_STRUCT), 1, 1, 1);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Create_Message_Pool);
  store_result(RESULT_Create_Message_Pool, "Create Message Pool", 1, TRUE);

  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _msgpool_destroy((_pool_id)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Delete_Message_Pool, "Delete Message Pool", 1, TRUE);

  pool = _msgpool_create(sizeof(MESSAGE_HEADER_STRUCT), TEST_ARRAY_SIZE, 0, 0);
  if ( pool == 0 )
  {
    printf("Could not create message pool test 27\n");
  } /* Endif */
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]= _msg_alloc(pool);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Allocate_Message);
  store_result(RESULT_Allocate_Message, "Allocate Message", 1, TRUE);

  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _msg_free((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Free_Message, "Free Message", 1, TRUE);
  _msgpool_destroy(pool);

  /* =================== MESSAGE PASSING ================== */

  store_heading(HEADING_Message_Passing, "Message Passing");
  _mem_zero((void *)qid_array, sizeof(qid_array));
  START_TIME();
  for (i = SEND_QUEUE; i < TEST_ARRAY_SIZE + SEND_QUEUE; ++i)
  {
    null_function();
    qid_array[i]= _msgq_open(i, 0);
  } /* Endfor */
  END_TIME();
  for (i = SEND_QUEUE; i < TEST_ARRAY_SIZE + SEND_QUEUE; ++i)
  {
    if ( qid_array[i] == 0 )
    {
      printf("Message queue not created, test 18, index %d\n", i);
      break;
    } /* Endif */
  } /* Endfor */
  store_result(RESULT_Open_Message_Queue, "Open Message Queue", 1, TRUE);


  START_TIME();
  for (i = SEND_QUEUE; i < TEST_ARRAY_SIZE + SEND_QUEUE; ++i)
  {
    null_function();
    _msgq_close(qid_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Close_Message_Queue, "Close Message Queue", 1, TRUE);


  pool = _msgpool_create(sizeof(MESSAGE_HEADER_STRUCT), TEST_ARRAY_SIZE,
                         0, 0);
  if ( pool == 0 )
  {
    printf("Could not create message pool test 19\n");
  } /* Endif */
  _mem_zero((void *)test_array, sizeof(test_array));
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    msg_ptr = _msg_alloc(pool);
    if ( msg_ptr == NULL )
    {
      break;
    } /* Endif */
    msg_ptr->HEADER.TARGET_QID = my_qid;
    test_array[i]= msg_ptr;
  } /* Endwhile */
  check_operation(RESULT_Send_Message);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _msgq_send((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Send_Message, "Send Message", 1, TRUE);


  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]= _msgq_receive_ticks(MSGQ_ANY_QUEUE, 0);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Receive_Message_ticks);
  store_result(RESULT_Receive_Message_ticks, "Receive Message (ticks)", 1, TRUE);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _msg_free((void *)test_array[i]);
  } /* Endfor */
  _msgpool_destroy(pool);


  _mem_zero((void *)task_array, sizeof(task_array));
  pool = _msgpool_create(sizeof(MESSAGE_HEADER_STRUCT), 1, 0, 0);
  msg_ptr = _msg_alloc(pool);
  msg_ptr->HEADER.TARGET_QID = _msgq_get_id(0, SEND_QUEUE);
  /* Following tasks will be all high priority */
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, SEND_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, SEND_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Send_Message_Context_Switch);
  START_TIME();
  _msgq_send(msg_ptr);
  _int_enable();
  store_result(RESULT_Send_Message_Context_Switch, "Send Message Context Switch", 1, FALSE);
  _msgpool_destroy(pool);
  _time_delay_ticks(2);


  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, RECEIVE_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Send_Message_Ready_Task);
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  /* Modify my task priority to be higher */
  set_priority(SEND_LAST_PRIO);
  pool = _msgpool_create(sizeof(MESSAGE_HEADER_STRUCT), TEST_ARRAY_SIZE,
                         0, 0);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    msg_ptr = _msg_alloc(pool);
    msg_ptr->HEADER.TARGET_QID = _msgq_get_id(0, SEND_QUEUE + i);
    test_array[i]= msg_ptr;
  } /* Endwhile */
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _msgq_send((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Send_Message_Ready_Task, "Send Message Ready Task", 1, TRUE);
  set_priority(MAIN_PRIO);
  _time_delay_ticks(2);


  _mem_zero((void *)task_array, sizeof(task_array));
  task_array[0]= _task_create(0, RECEIVE_FIRST_TASK, 0);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    task_array[i]= _task_create(0, RECEIVE_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Receive_Message_ticks_Block_Task);
  /* Modify my task priority to let them run, then put me higher again */
  set_priority(RECEIVE_FIRST_PRIO);
  _int_disable();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  /*Let them run */
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Receive_Message_ticks_Block_Task, "Receive Message (ticks) Block Task", 1, FALSE);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    msg_ptr = _msg_alloc(pool);
    msg_ptr->HEADER.TARGET_QID = _msgq_get_id(0, SEND_QUEUE + i);
    _msgq_send(msg_ptr);
  } /* Endwhile */
  _time_delay_ticks(2);
  DEBUGM(_klog_show_stack_usage();
         )
#endif

  /* =================== LWEVENTS =========================== */

#ifdef _EVENT_MUTEX
  store_heading(HEADING_LWEvents,"Light Weight Events");

  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    test_array[i]= &lwevent_array[i];
  } /* Endfor */
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwevent_create((LWEVENT_STRUCT *)test_array[i], 0);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Open_LWEvent, "Open LW-Event", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwevent_destroy((LWEVENT_STRUCT *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Close_LWEvent, "Close LW-Event", 1, TRUE);

  _lwevent_create(&lwevent, 0);
  _lwevent_set(&lwevent, 1);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwevent_wait_ticks(&lwevent, 1, FALSE, 0);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Wait_LWEvent_ticks, "Wait LW-Event (ticks)", 1, TRUE);

  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwevent_set(&lwevent, 1);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_LWEvent, "Set LW-Event", 1, TRUE);
  _lwevent_destroy(&lwevent);

  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _lwevent_create((LWEVENT_STRUCT *)test_array[i], 0);
  } /* Endfor */
  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE - 1; ++i)
  {
    task_array[i]= _task_create(0, LWEVENT_SET_TASK, (uint32_t)i);
  } /* Endfor */
  task_array[TASK_ARRAY_SIZE - 1]= _task_create(0, LWEVENT_SET_LAST_TASK,
                                                (uint32_t)(TASK_ARRAY_SIZE - 1));
  check_created_tasks(RESULT_Set_LWEvent_Context_Switch);
  START_TIME();
  _lwevent_set(&lwevent_array[0], 1);
  _int_enable();
  store_result(RESULT_Set_LWEvent_Context_Switch, "Set LW-Event Context Switch", 1, FALSE);
  _time_delay_ticks(2);


  _mem_zero((void *)task_array, sizeof(task_array));
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _lwevent_clear(&lwevent_array[i], MAX_MQX_UINT);
    task_array[i]= _task_create(0, LWEVENT_SET_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Set_LWEvent_Ready_Task);
  set_priority(EVENT_SET_LAST_PRIO);
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwevent_set((LWEVENT_STRUCT *)test_array[i], 1);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Set_LWEvent_Ready_Task, "Set LW-Event Ready Task", 1, TRUE);
  set_priority(MAIN_PRIO);


  _mem_zero((void *)task_array, sizeof(task_array));
  task_array[0]= _task_create(0, LWEVENT_WAIT_FIRST_TASK, 0);
  _lwevent_clear(&lwevent_array[0], MAX_MQX_UINT);
  for (i = 1; i < TASK_ARRAY_SIZE; ++i)
  {
    _lwevent_clear(&lwevent_array[i], MAX_MQX_UINT);
    task_array[i]= _task_create(0, LWEVENT_WAIT_TASK, (uint32_t)i);
  } /* Endfor */
  check_created_tasks(RESULT_Wait_LWEvent_ticks_Block_Task);
  set_priority(MAIN_PRIO - 2);
  _int_disable();
  for (i = 0; i < TASK_ARRAY_SIZE; ++i)
  {
    _task_ready(_task_get_td(task_array[i]));
  } /* Endfor */
  set_priority(MAIN_PRIO);
  END_TIME();
  store_result(RESULT_Wait_LWEvent_ticks_Block_Task, "Wait LW-Event (ticks) Block Task", 1, FALSE);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _lwevent_set(&lwevent_array[i], MAX_MQX_UINT);
  } /* Endfor */
  _time_delay_ticks(2);
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    _lwevent_destroy(&lwevent_array[i]);
  } /* Endif */
  _time_delay_ticks(2);
#endif

  /* =================== LIGHT WEIGHT MEMORY MANAGEMENT ================ */

#ifdef _LWMEM

  lwmem_pool_id = _lwmem_create_pool(&lwmem_pool, (void *)partitions,
                                     sizeof(partitions));
  _mem_zero((void *)test_array, sizeof(test_array));
  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]= _lwmem_alloc_from(lwmem_pool_id, 64);
  } /* Endfor */
  END_TIME();
  check_operation(RESULT_Allocate_LWMemory);
  store_result(RESULT_Allocate_LWMemory, "Allocate LW-Memory", 1, TRUE);


  START_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    _lwmem_free((void *)test_array[i]);
  } /* Endfor */
  END_TIME();
  store_result(RESULT_Free_LWMemory, "Free LW-Memory", 1, TRUE);

#endif

  /* =================== INTERRUPTS ======================= */

  ints = 0;
  _int_enable();
#ifdef _INTERRUPTS
  _int_install_isr(INTERRUPT_VECTOR, interrupt_ack, NULL);
  setup_interrupt();
  _time_delay_ticks(2);
  START_GET_TIME();
  for (i = 0; i < TEST_ARRAY_SIZE; ++i)
  {
    null_function();
    test_array[i]=(void *)i;
    DEBUGISR(printf("\nAbout to generate interrupt");
             )
    generate_interrupt();
  } /* Endfor */
  END_TIME();
  if ( ints != TEST_ARRAY_SIZE )
  {
    printf("all interrupts did not fire, count is %d\n", ints);
  } /* Endif */
  store_result(RESULT_Service_Interrupt, "Service Interrupt and return to task", 2, TRUE);
#endif

  /* =================== DONE ============================= */

  _int_disable();
#ifndef NO_IO
  printf("Parameters\n");
  printf("Board,            %s\n", tBOARD);
  printf("Version,          %s\n", _mqx_version);
  printf("CPU,              %s %d MHz\n", tCPU, SYSTEM_CLOCK);
  printf("Generic Revision, %s\n", _mqx_generic_revision);
  printf("Memory,           %s\n", tMEMORY);
  printf("PSP Revision,     %s\n", _mqx_psp_revision);
  printf("Target,           %s %s %s\n", tTARGET, tOPTIM, tABI);
  printf("BSP Revision,     %s\n", _mqx_bsp_revision);
  printf("Compiler,         %s\n", tCOMPILER);
  printf("IO Revision,      %s\n", _mqx_io_revision);
  printf("PSP build date,   %s\n", _mqx_date);
  printf("\n");

  config  = kernel_data->CONFIG1;
  config2 = kernel_data->CONFIG2;

  printf("Kernel Options\n");
  printf("Has Code Cache, %u\n", PSP_HAS_CODE_CACHE);
  printf("Has Data Cache, %u\n", PSP_HAS_DATA_CACHE);

  #if MQX_KERNEL_LOGGING
  printf("Kernel Log Support, 1\n");
  printf("Logging Enabled, %u\n", (kernel_data->LOG_CONTROL & KLOG_ENABLED) > 0);
  #else
  printf("Kernel Log Support, 0\n");
  printf("Logging, 0\n");
  #endif

  printf("Check Errors, %u\n", ((config & MQX_CNFG1_CHECK_ERRORS) > 0));
  printf("Check Memory Allocation Errors, %u\n",      ((config & MQX_CNFG1_CHECK_MEMORY_ALLOCATION_ERRORS) > 0));
  printf("Check Validity, %u\n",      ((config & MQX_CNFG1_CHECK_VALIDITY) > 0));
  printf("Component Destruction, %u\n",      ((config & MQX_CNFG1_COMPONENT_DESTRUCTION) > 0));
  printf("Default Time Slice in Ticks, %u\n",      ((config & MQX_CNFG1_DEFAULT_TIME_SLICE_IN_TICKS) > 0));
  printf("Exit Enabled, %u\n",      ((config2 & MQX_CNFG2_EXIT_ENABLED) > 0));
  printf("Has Time Slice, %u\n",      ((config2 & MQX_CNFG2_HAS_TIME_SLICE) > 0));
  printf("Include Floating Point I/O, %u\n",      ((config & MQX_CNFG1_INCLUDE_FLOATING_POINT_IO) > 0));
  printf("Is Multiprocessor, %u\n",      ((config2 & MQX_CNFG2_IS_MULTI_PROCESSOR) > 0));
  printf("LWLog Time Stamp in Ticks, %u\n", ((config & MQX_CNFG1_LWLOG_TIME_STAMP_IN_TICKS) > 0));
  printf("Memory Free List Sorted, %u\n", ((config & MQX_CNFG1_MEMORY_FREE_LIST_SORTED) > 0));
  printf("Monitor Stack, %u\n",      ((config & MQX_CNFG1_MONITOR_STACK) > 0));
  printf("Mutex has Polling, %u\n",      ((config2 & MQX_CNFG2_MUTEX_HAS_POLLING) > 0));
  printf("Profiling Enabled, %u\n",      ((config & MQX_CNFG1_PROFILING_ENABLE) > 0));
  printf("Run Time Error Check Enabled, %u\n",      ((config & MQX_CNFG1_RUN_TIME_ERR_CHECK_ENABLE) > 0));
  printf("Task Creation Blocks, %u\n",      ((config & MQX_CNFG1_TASK_CREATION_BLOCKS) > 0));
  printf("Task Destruction, %u\n",      ((config & MQX_CNFG1_TASK_DESTRUCTION) > 0));
  printf("Time Uses Ticks Only, %u\n",      ((config2 & MQX_CNFG2_TIMER_USES_TICKS_ONLY) > 0));
  printf("Use 32-bit Message Qids, %u\n",      ((config & MQX_CNFG1_USE_32BIT_MESSAGE_QIDS) > 0));
  printf("Use 32-bit Types, %u\n",      ((config2 & MQX_CNFG2_USE_32BIT_TYPES) > 0));
  printf("Use Idle Task, %u\n",      ((config2 & MQX_CNFG2_USE_IDLE_TASK) > 0));
  printf("Use Inline Macros, %u\n",      ((config & MQX_CNFG1_USE_INLINE_MACROS) > 0));
  printf("Use Lwmem Allocator, %u\n",      ((config2 & MQX_CNFG2_USE_LWMEM_ALLOCATOR) > 0));
  printf("\n");

  printf("Resource Usage\n");
  printf("Kernel RAM,   %u\nMemory Block, %u + data\nMessage,      %u + data\nSemaphore,    %u\n",
         tKERNEL_RAM, tMEMORY_BLOCK, tMESSAGE, tSEM);

  printf("ISR,          %u * #ints\nPartition,    %u + data\nMsg Queue,    %u\n",
         tISR, tPARTITION, tMSGQ);

  printf("Task,         %u + stack\nTask Queue,   %u\nMutex,        %u\nEvent,        %u\n",
         tTASK, tTASKQ, tMUTEX, tEVENT);

  printf("LWEvent,      %u\nLWSem,        %u\nLWMem,        %u\n",
         tLWEVENT, tLWSEM, tLWMEM);

  printf("\n");
#endif

  print_results();

  _time_delay_ticks(10);
  printf("\n# Test complete.");
  while (1)
  {};

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : store_heading
* Return value  : none
* Comments      :
*    This function stores the heading for a group of tests.
*
*END*----------------------------------------------------------------------*/

void store_heading
(
 _mqx_uint  index,
 char   *str
 )
{ /* Body */

  div_array[index - 1]= 0;
  test_id[index - 1]= str;

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : store_result
* Return value  : none
* Comments      :
*    This function stores the results for this test.
*
*END*----------------------------------------------------------------------*/

void store_result
(
 _mqx_uint  index,
 char   *str,
 _mqx_uint  div,   /* Division factor */
 bool    looping
 )
{ /* Body */

  start_ticks_array[index - 1]= start_ticks;
  end_ticks_array[index - 1]= end_ticks;
  div_array[index - 1]= div;
  is_looping[index - 1]= looping;
  test_id[index - 1]= str;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : print_results
* Return value  : none
* Comments      :
*    This function prints out the results.
*
*END*----------------------------------------------------------------------*/

void print_results
(
 void
 )
{
  MQX_TICK_STRUCT  start_ticks, end_ticks;
  int32_t           ns;
  _mqx_uint        i;
  bool          overflow;

  for (i = 0; i < TEST_RESULTS_SIZE; i++)
  {

    if ( div_array[i] )
    {
      end_ticks = end_ticks_array[i];
      PSP_NORMALIZE_TICKS(&end_ticks);

      start_ticks = start_ticks_array[i];
      PSP_NORMALIZE_TICKS(&start_ticks);

      if ( is_looping[i] )
      {
        start_ticks.HW_TICKS += loop_overhead;
        PSP_NORMALIZE_TICKS(&start_ticks);
      } /* Endif */

      ns = _time_diff_nanoseconds(&end_ticks, &start_ticks, &overflow);

      if ( overflow )
      {
        ns = 0;
      } /* Endif */

      if ( ns && div_array[i] > 1 )
      {
        ns = ns / div_array[i];
      } /* Endif */

#ifdef NO_IO
      result_ns[i]= ns;
#else
      printf("%s , %ld , nsec in , %d , loops, %d , ns norm 1MHz %s\n",
             test_id[i], ns, TEST_ARRAY_SIZE / TEST_ARRAY_DIV, ns * SYSTEM_CLOCK, overflow ? "OVERFLOW DETECTED" : "");
#endif
    }
    else
    {
      printf("%s\n", test_id[i]);
    }
  } /* Endfor */
}


#if (DEBUG_LEVEL > 4)

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : print_array
* Return value  : none
* Comments      :
*    This functions prints out an array
*
*END*----------------------------------------------------------------------*/

void print_array
(
 void     *array,
 uint32_t   num_of_elements,
 uint32_t   element_size
 )
{ /* Body */
  uint32_t     i;
  uint32_t *lw_ptr;
  uint16_t *w_ptr;
  unsigned char   *c_ptr;

  switch (element_size)
  {
  case 1:
    c_ptr =(unsigned char *)array;
    break;
  case 2:
    w_ptr =(uint16_t *)array;
    break;
  case 4:
    lw_ptr =(uint32_t *)array;
    break;
  default:
    return;
  } /* Endswitch */

  for (i = 0; i < num_of_elements; i++)
  {
    switch (element_size)
    {
    case 1:
      printf("0x%x ", (uint32_t)(c_ptr[i]));
      break;
    case 2:
      printf("0x%x ", (uint32_t)(w_ptr[i]));
      break;
    case 4:
      printf("0x%x ", (uint32_t)(lw_ptr[i]));
      break;
    } /* Endswitch */
    if ( i >= 80 )
    {
      putchar('\n');
    } /* Endif */
  } /* Endfor */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : print_free_list
* Return value  : none
* Comments      :
*    This functions prints out the free list of the memory pool
*
*END*----------------------------------------------------------------------*/

void print_free_list
(
 void
 )
{ /* Body */
  KERNEL_DATA_STRUCT      *kd_ptr;
  STOREBLOCK_STRUCT_PTR  block_ptr;
  uint32_t                i;

  kd_ptr = _mqx_get_kernel_data();
  block_ptr = kd_ptr->KD_POOL.POOL_FREE_LIST_PTR;
  i = 0;
  printf("\ntMEMORY POOL FREE LIST:\n");
  while (block_ptr != NULL)
  {
    printf("%3d. addr=%08x, sz=%-5d, td=%08x, nxt=%08x, prv=%08x\n",
           i,
           block_ptr,
           block_ptr->BLOCKSIZE,
           block_ptr->TASK_NUMBER,
           block_ptr->NEXTBLOCK,
           block_ptr->PREVBLOCK
           );
    block_ptr = block_ptr->NEXTBLOCK;
    i++;
  } /* Endfor */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name : print_memory_pool
* Return value  : none
* Comments      :
*    This functions prints out the entire memory pool
*
*END*----------------------------------------------------------------------*/

void print_memory_pool
(
 void
 )
{ /* Body */
  KERNEL_DATA_STRUCT      *kd_ptr;
  STOREBLOCK_STRUCT_PTR  block_ptr;
  uint32_t                i;
  int32_t                 size;

  kd_ptr = _mqx_get_kernel_data();
  block_ptr = kd_ptr->KD_POOL.POOL_PTR;
  i = 0;
  printf("\ntMEMORY POOL:\n");
  while (block_ptr <= kd_ptr->KD_POOL.POOL_END_PTR)
  {
    size = block_ptr->BLOCKSIZE;
    if ( size < 0 )
    {
      size = -size;
      printf(" ");
    }
    else
    {
      printf("*");   /* mark free blocks with an '*' */
    } /* Endif */
    printf("%3d. addr=%08x, sz=%-5d, td=%08x, nxt=%08x, prv=%08x\n",
           i,
           block_ptr,
           size,
           block_ptr->TASK_NUMBER,
           block_ptr->NEXTBLOCK,
           block_ptr->PREVBLOCK
           );
    block_ptr =(void *)((unsigned char *)block_ptr + size);
    if ( size == 0 )
    {
      return;
    } /* Endif */
    i++;
  } /* Endfor */

} /* Endbody */

#endif


/* EOF */
