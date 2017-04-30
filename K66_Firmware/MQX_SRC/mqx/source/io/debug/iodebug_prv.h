/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   The file contains private functions prototype, defines, structure 
*   definitions private to debug driver.
*
*
*END************************************************************************/

#ifndef __iodebug_prv_h__
#define __iodebug_prv_h__

#include "iodebug.h"

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

/*----------------------------------------------------------------------*/
/*
**                          DATATYPE DEFINITIONS
*/

typedef struct {
    char                        FLUSH_CHAR;
    char                       *BUFFER_PTR;
    uint32_t                     BUFFER_POS;
    int32_t                      COUNT;
    IODEBUG_INIT_STRUCT_PTR     DEV_INIT_DATA_PTR;
    LWSEM_STRUCT                LWSEM;
} IO_DEBUG_DEVICE_STRUCT, * IO_DEBUG_DEVICE_STRUCT_PTR; 


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

/* Internal debug functions */
#ifdef __cplusplus
extern "C" {
#endif

extern void _io_debug_semi_write_char(register char *);
extern char _io_debug_semi_read_char(void);
extern void _io_debug_semi_write_string(register char *);
extern void _io_debug_semi_buffer_flush(IO_DEBUG_DEVICE_STRUCT_PTR);
extern void _io_debug_semi_buffer_write(IO_DEBUG_DEVICE_STRUCT_PTR, char *);

extern bool _io_debug_itm_write_char(register char *);

extern _mqx_int _io_debug_open(MQX_FILE_PTR, char *, char *);
extern _mqx_int _io_debug_close(MQX_FILE_PTR);
extern _mqx_int _io_debug_read(MQX_FILE_PTR, char *, _mqx_int);
extern _mqx_int _io_debug_write(MQX_FILE_PTR, char *, _mqx_int);
extern _mqx_int _io_debug_ioctl(MQX_FILE_PTR, _mqx_uint, void *);
extern void _io_debug_buffer_flush(IO_DEBUG_DEVICE_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
