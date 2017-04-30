#ifndef MAIN_H
  #define MAIN_H
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
*   This include file is used to define the demonstation constants
*
*
*END************************************************************************/

/* Define the level of debugging. Zero means no debugging. */
#define DEBUG_LEVEL 0

#if (DEBUG_LEVEL > 4)
  #define DEBUGM(x)          x
  #define DEBUGISR(x)        x
  #define DEBUGSTAKERR(x)    x
  #define DEBUGMEMERR(x)     x
  #define DEBUGTASKERR(x)    x
  #define DEBUGCHKPOINT(x)   x
#elif (DEBUG_LEVEL > 3)
  #define DEBUGM(x)
  #define DEBUGISR(x)        x
  #define DEBUGMEMERR(x)     x
  #define DEBUGSTAKERR(x)    x
  #define DEBUGTASKERR(x)    x
  #define DEBUGCHKPOINT(x)   x
#elif (DEBUG_LEVEL > 2)
  #define DEBUGM(x)
  #define DEBUGISR(x)
  #define DEBUGMEMERR(x)     x
  #define DEBUGSTAKERR(x)    x
  #define DEBUGTASKERR(x)    x
  #define DEBUGCHKPOINT(x)   x
#elif (DEBUG_LEVEL > 1)
  #define DEBUGM(x)
  #define DEBUGISR(x)
  #define DEBUGMEMERR(x)
  #define DEBUGSTAKERR(x)
  #define DEBUGTASKERR(x)    x
  #define DEBUGCHKPOINT(x)   x
#elif (DEBUG_LEVEL > 0)
  #define DEBUGM(x)
  #define DEBUGISR(x)
  #define DEBUGMEMERR(x)
  #define DEBUGSTAKERR(x)
  #define DEBUGTASKERR(x)
  #define DEBUGCHKPOINT(x)   x
#else
  #define DEBUGM(x)
  #define DEBUGISR(x)
  #define DEBUGMEMERR(x)
  #define DEBUGTASKERR(x)
  #define DEBUGSTAKERR(x)
  #define DEBUGCHKPOINT(x)
#endif



/*#define NEED_SPLITTING*/

#ifdef NEED_SPLITTING
  #define TESTNUM0
  #define TESTNUM1
  #define TESTNUM2
  #define TESTNUM3
  #ifdef TESTNUM0
    #define _TASKING
    #define _LWSEM
    #define _INTERRUPTS
  #endif

  #ifdef TESTNUM1
    #define _EVENT_MUTEX
  #endif

  #ifdef TESTNUM2
    #define _SEM
    #define _PARTITION
  #endif

  #ifdef TESTNUM3
    #define _MESSAGE
  #endif
#else
  #define _TASKING
  #define _LWSEM
  #define _INTERRUPTS
  #define _EVENT_MUTEX
  #define _MESSAGE
  #define _SEM
  #define _PARTITION
  #define _LWMEM
#endif


/*--------------------------------------------------------------------------*/
/* How many tests are there? */

typedef enum {
  HEADING_System_Parameters = 1,
  RESULT_Context_Switch,
  RESULT_Service_Interrupt,
  RESULT_System_Timer_Tick_Overhead,
#ifdef _TASKING
  HEADING_Task_Management,
  RESULT_Create_Task,
  RESULT_Delete_Task,
  RESULT_Block_Task,
  RESULT_Ready_Task,
  RESULT_Yield_Task,
  HEADING_Task_Queues,
  RESULT_Suspend_Task,
  RESULT_Suspend_Task_Context_Switch,
  RESULT_Resume_Task,
  RESULT_Resume_Task_Context_Switch,
#endif
  HEADING_Memory_Management,
#ifdef _TASKING
  RESULT_Allocate_Memory,
  RESULT_Free_Memory,
#endif
#ifdef _LWMEM
  RESULT_Allocate_LWMemory,
  RESULT_Free_LWMemory,
#endif
#ifdef _PARTITION
  HEADING_Partitions,
  RESULT_Partition_Create,
  RESULT_Partition_Allocate_Block,
  RESULT_Partition_Free_Block,
#endif
#ifdef _LWSEM
  HEADING_Light_Weight_Semaphores,
  RESULT_Open_Light_Weight_Semaphore,
  RESULT_Set_Light_Weight_Semaphore,
  RESULT_Set_Light_Weight_Semaphore_Ready_Task,
  RESULT_Set_Light_Weight_Semaphore_Context_Switch,
  RESULT_Wait_Light_Weight_Semaphore,
  RESULT_Wait_Light_Weight_Semaphore_Block_Task,
  RESULT_Close_Light_Weight_Semaphore,
#endif
#ifdef _EVENT_MUTEX
  HEADING_LWEvents,
  RESULT_Open_LWEvent,
  RESULT_Close_LWEvent,
  RESULT_Wait_LWEvent_ticks,
  RESULT_Set_LWEvent,
  RESULT_Set_LWEvent_Context_Switch,
  RESULT_Set_LWEvent_Ready_Task,
  RESULT_Wait_LWEvent_ticks_Block_Task,
#endif
#ifdef _MESSAGE
  HEADING_Message_Passing,
  RESULT_Open_Message_Queue,
  RESULT_Send_Message,
  RESULT_Send_Message_Ready_Task,
  RESULT_Send_Message_Context_Switch,
  RESULT_Receive_Message_ticks,
  RESULT_Receive_Message_ticks_Block_Task,
  RESULT_Close_Message_Queue,
  HEADING_Message_Pools,
  RESULT_Create_Message_Pool,
  RESULT_Delete_Message_Pool,
  RESULT_Allocate_Message,
  RESULT_Free_Message,
#endif
#ifdef _SEM
  HEADING_Semaphores,
  RESULT_Open_Semaphore,
  RESULT_Open_Semaphore_Fast,
  RESULT_Set_Semaphore,
  RESULT_Set_Semaphore_Ready_Task,
  RESULT_Set_Semaphore_Context_Switch,
  RESULT_Wait_Semaphore_ticks,
  RESULT_Wait_Semaphore_ticks_Block_Task,
  RESULT_Close_Semaphore,
#endif
#ifdef _EVENT_MUTEX
  HEADING_Events,
  RESULT_Open_Event,
  RESULT_Open_Event_Fast,
  RESULT_Set_Event,
  RESULT_Set_Event_Ready_Task,
  RESULT_Set_Event_Context_Switch,
  RESULT_Wait_Event_ticks,
  RESULT_Wait_Event_ticks_Block_Task,
  RESULT_Close_Event,
  HEADING_Mutexes,
  RESULT_Open_Mutex,
  RESULT_Unlock_Mutex,
  RESULT_Unlock_Mutex_Ready_Task,
  RESULT_Unlock_Mutex_Context_Switch,
  RESULT_Lock_Mutex,
  RESULT_Lock_Mutex_Block_Task,
  RESULT_Close_Mutex,
#endif
  TEST_RESULTS_SIZE
} test_t;



/* When we create N number of things, this is 'N'
**
** Faster CPUs have more to do
*/
#define WAIT_FOR_TICK(x) \
   { \
      _mqx_uint new_ticks; \
      x = PSP_GET_ELEMENT_FROM_TICK_STRUCT(&kernel_data->TIME, 0); \
      new_ticks = x; \
      while ( x == new_ticks) { \
         new_ticks = PSP_GET_ELEMENT_FROM_TICK_STRUCT(&kernel_data->TIME, 0); \
         null_function(); \
      } \
   }

#define _time_get_hwticks() \
   (*get_hwticks)(get_hwticks_param)


#if MQX_NUM_TICK_FIELDS == 1
  #define START_GET_TIME() \
   end_ticks.TICKS[0]   = 0; \
   start_ticks.TICKS[0] = kernel_data->TIME.TICKS[0]; \
   start_ticks.HW_TICKS = _time_get_hwticks()
#elif MQX_NUM_TICK_FIELDS == 2
  #define START_GET_TIME() \
   end_ticks.TICKS[0]   = 0; \
   end_ticks.TICKS[1]   = 0; \
   start_ticks.TICKS[0] = kernel_data->TIME.TICKS[0]; \
   start_ticks.TICKS[1] = kernel_data->TIME.TICKS[1]; \
   start_ticks.HW_TICKS = _time_get_hwticks()
#elif MQX_NUM_TICK_FIELDS == 3
  #define START_GET_TIME() \
   end_ticks.TICKS[0]   = 0; \
   end_ticks.TICKS[1]   = 0; \
   end_ticks.TICKS[2]   = 0; \
   start_ticks.TICKS[0] = kernel_data->TIME.TICKS[0]; \
   start_ticks.TICKS[1] = kernel_data->TIME.TICKS[1]; \
   start_ticks.TICKS[2] = kernel_data->TIME.TICKS[2]; \
   start_ticks.HW_TICKS = _time_get_hwticks()
#elif MQX_NUM_TICK_FIELDS == 4
  #define START_GET_TIME() \
   end_ticks.TICKS[0]   = 0; \
   end_ticks.TICKS[1]   = 0; \
   end_ticks.TICKS[2]   = 0; \
   end_ticks.TICKS[3]   = 0; \
   start_ticks.TICKS[0] = kernel_data->TIME.TICKS[0]; \
   start_ticks.TICKS[1] = kernel_data->TIME.TICKS[1]; \
   start_ticks.TICKS[2] = kernel_data->TIME.TICKS[2]; \
   start_ticks.TICKS[3] = kernel_data->TIME.TICKS[3]; \
   start_ticks.HW_TICKS = _time_get_hwticks()
#elif MQX_NUM_TICK_FIELDS == 5
  #define START_GET_TIME() \
   end_ticks.TICKS[0]   = 0; \
   end_ticks.TICKS[1]   = 0; \
   end_ticks.TICKS[2]   = 0; \
   end_ticks.TICKS[3]   = 0; \
   end_ticks.TICKS[4]   = 0; \
   start_ticks.TICKS[0] = kernel_data->TIME.TICKS[0]; \
   start_ticks.TICKS[1] = kernel_data->TIME.TICKS[1]; \
   start_ticks.TICKS[2] = kernel_data->TIME.TICKS[2]; \
   start_ticks.TICKS[3] = kernel_data->TIME.TICKS[3]; \
   start_ticks.TICKS[4] = kernel_data->TIME.TICKS[4]; \
   start_ticks.HW_TICKS = _time_get_hwticks()
#elif MQX_NUM_TICK_FIELDS == 6
  #define START_GET_TIME() \
   end_ticks.TICKS[0]   = 0; \
   end_ticks.TICKS[1]   = 0; \
   end_ticks.TICKS[2]   = 0; \
   end_ticks.TICKS[3]   = 0; \
   end_ticks.TICKS[4]   = 0; \
   end_ticks.TICKS[5]   = 0; \
   start_ticks.TICKS[0] = kernel_data->TIME.TICKS[0]; \
   start_ticks.TICKS[1] = kernel_data->TIME.TICKS[1]; \
   start_ticks.TICKS[2] = kernel_data->TIME.TICKS[2]; \
   start_ticks.TICKS[3] = kernel_data->TIME.TICKS[3]; \
   start_ticks.TICKS[4] = kernel_data->TIME.TICKS[4]; \
   start_ticks.TICKS[5] = kernel_data->TIME.TICKS[5]; \
   start_ticks.HW_TICKS = _time_get_hwticks()
#else
  # error not enought tick fields
#endif

#define START_TIME() \
   ALIGN(); \
   { \
      volatile _mqx_uint new_ticks; \
      _mqx_uint ticks; \
      ticks = PSP_GET_ELEMENT_FROM_TICK_STRUCT(&kernel_data->TIME, 0); \
      new_ticks = ticks; \
      while (ticks == new_ticks) { \
         new_ticks = PSP_GET_ELEMENT_FROM_TICK_STRUCT(&kernel_data->TIME, 0); \
         null_function(); \
      } /* Endwhile */ \
   } \
   _int_disable(); \
   START_GET_TIME()

#if MQX_NUM_TICK_FIELDS == 1
  #define END_TIME() \
   end_hwticks        = _time_get_hwticks();        \
   end_ticks.TICKS[0] = kernel_data->TIME.TICKS[0];    \
   end_ticks.HW_TICKS = end_hwticks;           \
   _int_enable()
#elif MQX_NUM_TICK_FIELDS == 2
  #define END_TIME() \
   end_hwticks        = _time_get_hwticks();        \
   end_ticks.TICKS[0] = kernel_data->TIME.TICKS[0];    \
   end_ticks.TICKS[1] = kernel_data->TIME.TICKS[1];    \
   end_ticks.HW_TICKS = end_hwticks;           \
   _int_enable()
#elif MQX_NUM_TICK_FIELDS == 3
  #define END_TIME() \
   end_hwticks        = _time_get_hwticks();        \
   end_ticks.TICKS[0] = kernel_data->TIME.TICKS[0];    \
   end_ticks.TICKS[1] = kernel_data->TIME.TICKS[1];    \
   end_ticks.TICKS[2] = kernel_data->TIME.TICKS[2];    \
   end_ticks.HW_TICKS = end_hwticks;           \
   _int_enable()
#elif MQX_NUM_TICK_FIELDS == 4
  #define END_TIME() \
   end_hwticks        = _time_get_hwticks();        \
   end_ticks.TICKS[0] = kernel_data->TIME.TICKS[0];    \
   end_ticks.TICKS[1] = kernel_data->TIME.TICKS[1];    \
   end_ticks.TICKS[2] = kernel_data->TIME.TICKS[2];    \
   end_ticks.TICKS[3] = kernel_data->TIME.TICKS[3];    \
   end_ticks.HW_TICKS = end_hwticks;           \
   _int_enable()
#elif MQX_NUM_TICK_FIELDS == 5
  #define END_TIME() \
   end_hwticks        = _time_get_hwticks();        \
   end_ticks.TICKS[0] = kernel_data->TIME.TICKS[0];    \
   end_ticks.TICKS[1] = kernel_data->TIME.TICKS[1];    \
   end_ticks.TICKS[2] = kernel_data->TIME.TICKS[2];    \
   end_ticks.TICKS[3] = kernel_data->TIME.TICKS[3];    \
   end_ticks.TICKS[4] = kernel_data->TIME.TICKS[4];    \
   end_ticks.HW_TICKS = end_hwticks;           \
   _int_enable()
#elif MQX_NUM_TICK_FIELDS == 6
  #define END_TIME() \
   end_hwticks        = _time_get_hwticks();        \
   end_ticks.TICKS[0] = kernel_data->TIME.TICKS[0];    \
   end_ticks.TICKS[1] = kernel_data->TIME.TICKS[1];    \
   end_ticks.TICKS[2] = kernel_data->TIME.TICKS[2];    \
   end_ticks.TICKS[3] = kernel_data->TIME.TICKS[3];    \
   end_ticks.TICKS[4] = kernel_data->TIME.TICKS[4];    \
   end_ticks.TICKS[5] = kernel_data->TIME.TICKS[5];    \
   end_ticks.HW_TICKS = end_hwticks;           \
   _int_enable()
#else
  # error not enought tick fields
#endif

#define set_priority(priority) _task_set_priority(0, priority, &old_prio)

/*--------------------------------------------------------------------------*/
/* Task priorities */
#define MAIN_PRIO               TEST_ARRAY_SIZE+10
#define CREATE_TASK_PRIO        MAIN_PRIO+1

#define SUSPEND_PRIO            MAIN_PRIO
#define SUSPEND_FIRST_PRIO      MAIN_PRIO
#define RESUME_PRIO             MAIN_PRIO-1
#define RESUME_LAST_PRIO        9

#define SEND_PRIO               TEST_ARRAY_SIZE+9
#define SEND_LAST_PRIO          9
#define RECEIVE_PRIO            TEST_ARRAY_SIZE+9
#define RECEIVE_FIRST_PRIO      9

#define SEM_POST_PRIO           TEST_ARRAY_SIZE+9
#define SEM_POST_LAST_PRIO      9
#define SEM_WAIT_PRIO           MAIN_PRIO-1
#define SEM_WAIT_FIRST_PRIO     MAIN_PRIO-1

#define LWSEM_POST_PRIO         TEST_ARRAY_SIZE+9
#define LWSEM_POST_LAST_PRIO    9
#define LWSEM_WAIT_PRIO         MAIN_PRIO-1
#define LWSEM_WAIT_FIRST_PRIO   MAIN_PRIO-1

#define EVENT_SET_PRIO          TEST_ARRAY_SIZE+9
#define EVENT_SET_LAST_PRIO     9
#define EVENT_WAIT_PRIO         MAIN_PRIO-1
#define EVENT_WAIT_FIRST_PRIO   MAIN_PRIO-1

#define LWEVENT_SET_PRIO          TEST_ARRAY_SIZE+9
#define LWEVENT_SET_LAST_PRIO     9
#define LWEVENT_WAIT_PRIO         MAIN_PRIO-1
#define LWEVENT_WAIT_FIRST_PRIO   MAIN_PRIO-1

#define MUTEX_UNLOCK_PRIO       TEST_ARRAY_SIZE+9
#define MUTEX_UNLOCK_LAST_PRIO  9
#define MUTEX_LOCK_PRIO         MAIN_PRIO-1
#define MUTEX_LOCK_FIRST_PRIO   MAIN_PRIO-1

/*--------------------------------------------------------------------------*/
/* Task template indexes */
#define MAIN_TASK               10

#define SEND_TASK               11
#define SEND_LAST_TASK          12
#define RECEIVE_TASK            13
#define RECEIVE_FIRST_TASK      14

#define CREATE_TEST_TASK        18
#define ADD_READY_TASK          19
#define ADD_READY_FIRST_TASK    20

#define SEM_POST_TASK           21
#define SEM_POST_LAST_TASK      22
#define SEM_WAIT_TASK           23
#define SEM_WAIT_FIRST_TASK     24

#define EVENT_SET_TASK          25
#define EVENT_SET_LAST_TASK     26
#define EVENT_WAIT_TASK         27
#define EVENT_WAIT_FIRST_TASK   28

#define MUTEX_UNLOCK_TASK       29
#define MUTEX_UNLOCK_LAST_TASK  30
#define MUTEX_LOCK_TASK         31
#define MUTEX_LOCK_FIRST_TASK   32

#define LWSEM_POST_TASK         33
#define LWSEM_POST_LAST_TASK    34
#define LWSEM_WAIT_TASK         35
#define LWSEM_WAIT_FIRST_TASK   36

#define SUSPEND_TASK            37
#define SUSPEND_FIRST_TASK      38
#define RESUME_TASK             39
#define RESUME_LAST_TASK        40

#define LWEVENT_SET_TASK          41
#define LWEVENT_SET_LAST_TASK     42
#define LWEVENT_WAIT_TASK         43
#define LWEVENT_WAIT_FIRST_TASK   44

/*--------------------------------------------------------------------------*/

/* Message queue numbers */
#define MAIN_QUEUE           9
#define SEND_QUEUE          10

/* Type declarations */
typedef struct the_message
/* This message is the test message sent between tasks */
{
  MESSAGE_HEADER_STRUCT  HEADER;
} THE_MESSAGE, *THE_MESSAGE_PTR;

/*--------------------------------------------------------------------------*/

extern void main_task(uint32_t dummy);
extern void send_task(uint32_t index);
extern void send_last_task(uint32_t index);
extern void receive_task(uint32_t index);
extern void receive_first_task(uint32_t index);

extern void create_test_task(uint32_t dummy);
extern void add_ready_task(uint32_t dummy);
extern void add_ready_first_task(uint32_t dummy);

extern void suspend_task(uint32_t dummy);
extern void suspend_first_task(uint32_t dummy);
extern void resume_task(uint32_t index);
extern void resume_last_task(uint32_t index);

extern void null_function(void);
extern void test_notifier(void);
extern void analogous_clock_notifier(void);

extern void sem_task(uint32_t index);
extern void sem_last_task(uint32_t index);
extern void sem_wait_task(uint32_t index);
extern void sem_wait_first_task(uint32_t index);

extern void lwsem_task(uint32_t index);
extern void lwsem_last_task(uint32_t index);
extern void lwsem_wait_task(uint32_t index);
extern void lwsem_wait_first_task(uint32_t index);

extern void event_task(uint32_t index);
extern void event_last_task(uint32_t index);
extern void event_wait_task(uint32_t index);
extern void event_wait_first_task(uint32_t index);

extern void lwevent_task(uint32_t index);
extern void lwevent_last_task(uint32_t index);
extern void lwevent_wait_task(uint32_t index);
extern void lwevent_wait_first_task(uint32_t index);

extern void mutex_task(uint32_t index);
extern void mutex_last_task(uint32_t index);
extern void mutex_lock_task(uint32_t index);
extern void mutex_lock_first_task(uint32_t index);

extern void store_heading(_mqx_uint, char *);

extern void store_result(_mqx_uint, char *, _mqx_uint, bool);
extern void print_results(void);

extern void print_array(void *, uint32_t, uint32_t);
extern void print_free_list(void);
extern void print_memory_pool(void);


#endif
/* EOF */

