#ifndef __ppphdlc_h__
#define __ppphdlc_h__
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
*   This file contains the defines, externs and data
*   structure definitions required by application
*   programs in order to use the PPPHDLC device.
*
*
*END************************************************************************/

#include <rtcsrtos.h>
#include <rtcs.h>
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif


/***************************************
**
** MQX compatibility
*/

#define _ppphdlc_mutex             LWSEM_STRUCT
#define PPPHDLC_mutex_init(p)      _lwsem_create_hidden(p,1)
#define PPPHDLC_mutex_destroy(p)   _lwsem_destroy(p)
#define PPPHDLC_mutex_lock(p)      _lwsem_wait(p)
#define PPPHDLC_mutex_unlock(p)    _lwsem_post(p)

#define PPPHDLC_memalloc      _mem_alloc_zero
#define PPPHDLC_memfree       _mem_free
#define PPPHDLC_memcopy       _mem_copy

#define _ppphdlc_partid       _rtcs_part
#define PPPHDLC_partcreate    RTCS_part_create
#define PPPHDLC_partdestroy   RTCS_part_destroy
#define PPPHDLC_partalloc     RTCS_part_alloc
#define PPPHDLC_partfree      RTCS_part_free

#define PPPHDLC_delay         _time_delay


/***************************************
**
** Constants
*/

#define PPPHDLC_FRAMESIZE_MAXDATA 1502    /* protocol(2) + data(1500) */
#define PPPHDLC_FRAMESIZE_FCS        2

/*
** Special PPPHDLC characters
*/

#define PPPHDLC_ESC     0x7D
#define PPPHDLC_FLAG    0x7E

#define PPPHDLC_ADDR    0xFF
#define PPPHDLC_CTRL    0x03

#define PPPHDLC_OK      0
#define PPPHDLC_END     1
#define PPPHDLC_ABORT   2

#define PPPHDLC_RECV_TIMEOUT    2000

/*
** Size of the Async Control Character Map
*/

#define PPPHDLC_BITS_IN_UINT32  32
#define PPPHDLC_BITS_IN_ACCM    256


///* TBC */
//#ifndef RTCS_PRINT_PPP_PACKETS
//        #define RTCS_PRINT_PPP_PACKETS 0
//#endif


typedef struct ppphdlc_opt {
   uint32_t           ACCM[PPPHDLC_BITS_IN_ACCM / PPPHDLC_BITS_IN_UINT32];
   bool           PFC;
   bool           ACFC;
} PPPHDLC_OPT, * PPPHDLC_OPT_PTR;

extern PPPHDLC_OPT PPPHDLC_DEFAULT_OPTIONS;

typedef struct ppphdlc_stats {
   RTCS_STATS_STRUCT COMMON;

   uint32_t  ST_RX_ABORTED;
   uint32_t  ST_RX_RUNT;
   uint32_t  ST_RX_GIANT;
   uint32_t  ST_RX_BAD_ADDR;
   uint32_t  ST_RX_BAD_CTRL;
   uint32_t  ST_RX_BAD_FCS;

} PPPHDLC_STATS, * PPPHDLC_STATS_PTR;


typedef struct ppphdlc_struct {
   struct _iopcb_table PCB_TABLE;
   PPPHDLC_STATS       STATS;
   _ppphdlc_partid     PART_ID;
   _ppphdlc_mutex      MUTEX;
   MQX_FILE_PTR        DEVICE;
   PPPHDLC_OPT         SEND_OPT;
   PPPHDLC_OPT         RECV_OPT;
   uint16_t             FCS_SEND;
   uint16_t             FCS_RECV;
   void (_CODE_PTR_    UP)(void *);
   void (_CODE_PTR_    DOWN)(void *);
   void               *PARAM;

} PPPHDLC_STRUCT, * PPPHDLC_STRUCT_PTR;

#ifdef __cplusplus
extern "C" {
#endif

extern _iopcb_handle _iopcb_ppphdlc_init(MQX_FILE_PTR);
extern uint32_t _iopcb_ppphdlc_release(_iopcb_handle handle);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
