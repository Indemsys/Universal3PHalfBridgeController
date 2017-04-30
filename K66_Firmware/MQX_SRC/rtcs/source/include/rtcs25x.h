#ifndef __rtcs25x_h__
#define __rtcs25x_h__
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
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
*   This file contains the definitions required to
*   use RTCS with MQX2.5X.
*
*
*END************************************************************************/
#if PLATFORM_SDK_ENABLED
    #include <fsl_enet_rtcs_adapter.h>  /* ENET KSDK adaptter.*/
#endif
#include <mqx.h>
#include <bsp.h>
#define MQX_DISABLE_CONFIG_CHECK 1
#include <message.h>
#include <partition.h>
#include <log.h>
#include <klog.h>
#include <pcb.h>
#include <lwevent.h>
#undef MQX_DISABLE_CONFIG_CHECK

#include <rtcscfg.h>
#if !MQX_USE_IO_OLD
#include <unistd.h>
#endif

#define RTCS_get_data()  _mqx_get_io_component_handle(IO_RTCS_COMPONENT)
#define RTCS_set_data(p) _mqx_set_io_component_handle(IO_RTCS_COMPONENT,p)

#define SNMP_get_data()  _mqx_get_io_component_handle(IO_SNMP_COMPONENT)
#define SNMP_set_data(p) _mqx_set_io_component_handle(IO_SNMP_COMPONENT,p)



/***************************************
**
** Memory management
*/

extern void   *RTCS_mem_alloc(_mem_size size);
extern void   *RTCS_mem_alloc_zero(_mem_size size);
extern void   *RTCS_mem_alloc_system(_mem_size size);
extern void   *RTCS_mem_alloc_system_zero(_mem_size size);



/***************************************
**
** Message passing
*/

extern bool _msgq_send_blocked_internal(void *);
#define RTCS_msgq_send_blocked(a,b)    _msgq_send_blocked_internal(a)
#define RTCS_msgq_send(a,b)            _msgq_send(a)
#define RTCS_msgq_get_id               _msgq_get_id
#define RTCS_msg_free                  _msg_free
#define RTCS_msg_alloc                 _msg_alloc
#define RTCS_msgq_receive(a,b,c)       _msgq_receive(a,b)
#define RTCS_msgpool_create            _msgpool_create
#define RTCS_msgpool_destroy           _msgpool_destroy
#define RTCS_msgq_open                 _msgq_open
#define RTCS_msgq_close                _msgq_close


/***************************************
**
** PPP Memory management
*/

#define PPP_memzero(ptr, bsize)        _mem_zero(ptr, bsize)
#define PPP_memcopy(src, dest, bsize)  _mem_copy(src, dest, bsize)
#define PPP_memalloc(bsize)            _mem_alloc_zero(bsize)
#define PPP_memfree(ptr)               _mem_free(ptr)


/***************************************
**
** PPP Mutual exclusion
*/

#define _ppp_mutex            LWSEM_STRUCT

#define PPP_mutex_init(p)     _lwsem_create_hidden(p, 1)
#define PPP_mutex_destroy(p)  _lwsem_destroy(p)
#define PPP_mutex_lock(p)     _lwsem_wait(p)
#define PPP_mutex_unlock(p)   _lwsem_post(p)

/***************************************
**
** Partitions
*/
typedef struct rtcs_partition{
#if RTCSCFG_USE_MQX_PARTITIONS
   _partition_id        PART;
   uint32_t              SIZE;
#endif
#if RTCSCFG_USE_INTERNAL_PARTITIONS

   uint32_t              BLOCK_SIZE;
   uint32_t              NUM_BLOCKS;
   uint32_t              GROW_BLOCKS;
   uint32_t              MAX_BLOCKS;
   void                *GROW;
#endif
   void                *FREE;
   int32_t (_CODE_PTR_   CONSTR)(void *);
   int32_t (_CODE_PTR_   DESTR)(void *);
} RTCS_PARTITION, * _rtcs_part;

#ifdef __cplusplus
extern "C" {
#endif

extern _rtcs_part RTCS_part_create(
                     uint32_t size,
                     uint32_t init, uint32_t grow, uint32_t max,
                     int32_t (_CODE_PTR_ cons)(void *),
                     int32_t (_CODE_PTR_ dest)(void *)
                  );

extern void    RTCS_part_destroy    (_rtcs_part);
extern void   *RTCS_part_alloc      (_rtcs_part);
extern void   *RTCS_part_alloc_zero (_rtcs_part);
extern void    RTCS_part_free       (void *);


/***************************************
**
** Time
*/

#define RTCS_time_delay       _time_delay
#define RTCS_get_milliseconds RTCS_time_get

extern uint32_t RTCS_time_get     (void);
extern uint32_t RTCS_time_get_sec (void);

/* This function calculates an interval between two moments in time
 * This function takes into account also a possible counter overrun @c (start>end).*/
extern uint32_t RTCS_timer_get_interval( uint32_t, uint32_t);


/***************************************
**
** Date
*/

extern void RTCS_date_get (uint32_t *, uint32_t *);
extern void RTCS_date_set (uint32_t, uint32_t);


/***************************************
**
** Synchronization
*/

#define _rtcs_sem             LWSEM_STRUCT

#define RTCS_sem_init(p)      _lwsem_create_hidden(p, 0)
#define RTCS_sem_destroy(p)   _lwsem_destroy(p)
#define RTCS_sem_post(p)      _lwsem_post(p)
#define RTCS_sem_wait(p)      _lwsem_wait(p)
#define RTCS_sem_trywait(p)   (_lwsem_poll(p) ? RTCS_OK : RTCS_ERROR)


/***************************************
**
** Mutual exclusion
*/

#define _rtcs_mutex           LWSEM_STRUCT

#define RTCS_mutex_init(p)    _lwsem_create_hidden(p, 1)
#define RTCS_mutex_destroy(p) _lwsem_destroy(p)
#define RTCS_mutex_lock(p)    _lwsem_wait(p)
#define RTCS_mutex_unlock(p)  _lwsem_post(p)


/***************************************
**
** Task management
*/

typedef void     *_rtcs_taskid;

#define RTCS_task_getid()  _task_get_td(0)

extern uint32_t RTCS_task_create
(
   char          *name,
   uint32_t           priority,
   uint32_t           stacksize,
   void (_CODE_PTR_  start)(void *, void *),
   void             *arg
);

extern void RTCS_task_resume_creator (void *, uint32_t);
extern void RTCS_task_exit           (void *, uint32_t);

extern void RTCS_task_destroy (_rtcs_taskid);
extern void RTCS_task_abort (_rtcs_taskid);
extern _task_id RTCS_task_id_from_td (_rtcs_taskid);


/***************************************
**
** I/O
*/

#define RTCS_EACCES MQX_EACCES
#define RTCS_ENOENT MQX_ENOENT
#define RTCS_EEXIST MQX_EEXIST

extern void   *RTCS_io_open(char *, char *, uint32_t *);

#if MQX_USE_IO_OLD
#define RTCS_io_read(f,p,l)   ((int32_t)read(f,(char *)(p),l))
#define RTCS_io_write(f,p,l)  ((int32_t)write(f,(char *)(p),l))
#define RTCS_io_close(f)      fclose(f)
#else
#define RTCS_io_read(f,p,l)   ((int32_t)read(fileno((FILE*)f),(char *)(p),l))
#define RTCS_io_write(f,p,l)  ((int32_t)write(fileno((FILE*)f),(char *)(p),l))
#define RTCS_io_close(f)      fclose(f)
#endif

extern int32_t _io_socket_install(char *);
extern int32_t _io_telnet_install(char *);

#define RTCS_errno _task_errno
static inline void RTCS_set_errno(uint32_t err)
{
  _task_set_error(MQX_OK); /* clear previous errno in case there is some. */
  _task_set_error(err);  /* set the desired errno. */
}
static inline uint32_t RTCS_get_errno(void)
{
  uint32_t err = RTCS_errno;
  _task_set_error(MQX_OK);
  return err;
}

/***************************************
**
** Logging
*/

#define RTCSLOG_TYPE_FNENTRY  KLOG_FUNCTION_ENTRY
#define RTCSLOG_TYPE_FNEXIT   KLOG_FUNCTION_EXIT
#define RTCSLOG_TYPE_PCB      KLOG_RTCS_PCB
#define RTCSLOG_TYPE_TIMER    KLOG_RTCS_TIMER
#define RTCSLOG_TYPE_ERROR    KLOG_RTCS_ERROR


#if MQX_KERNEL_LOGGING
   extern void _klog_log(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
   #define RTCS_log_internal(type,p1,p2,p3,p4,p5)  _klog_log(type,p1,p2,p3,p4,p5)
#else
   #define RTCS_log_internal(type,p1,p2,p3,p4,p5) 
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
