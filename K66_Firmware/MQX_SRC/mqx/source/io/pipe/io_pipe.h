#ifndef _io_pipe_h_
#define _io_pipe_h_
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
*   The file contains the public functions prototype, defines, 
*   structure definitions to the Pipe IO device
*
*
*END************************************************************************/

#include "ioctl.h"

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/


/*
** PIPE IOCTL calls
*/
#define PIPE_IOCTL_GET_SIZE                  _IO(IO_TYPE_PIPE,0x01)
#define PIPE_IOCTL_FULL                      _IO(IO_TYPE_PIPE,0x02)
#define PIPE_IOCTL_EMPTY                     _IO(IO_TYPE_PIPE,0x03)
#define PIPE_IOCTL_RE_INIT                   _IO(IO_TYPE_PIPE,0x04)
#define PIPE_IOCTL_CHAR_AVAIL                _IO(IO_TYPE_PIPE,0x05)
#define PIPE_IOCTL_NUM_CHARS_FULL            _IO(IO_TYPE_PIPE,0x06)
#define PIPE_IOCTL_NUM_CHARS_FREE            _IO(IO_TYPE_PIPE,0x07)



/*----------------------------------------------------------------------*/
/*
**                          EXTERN FUNCTION DEFINITIONS
*/




#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t  _io_pipe_install(char *, uint32_t, uint32_t);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
