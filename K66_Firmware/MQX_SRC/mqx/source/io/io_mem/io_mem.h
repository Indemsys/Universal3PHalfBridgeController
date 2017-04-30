#ifndef __io_mem_h__
#define __io_mem_h__
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
*   The file contains functions prototype, defines, structure 
*   definitions private to the fdv Ramdisk drivers
*
*
*END************************************************************************/

#include "ioctl.h"

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

/*
** IO_MEM IOCTL calls
*/
#define IO_MEM_IOCTL_GET_BASE_ADDRESS     _IO(IO_TYPE_MEM,0x01)
#define IO_MEM_IOCTL_GET_TOTAL_SIZE       _IO(IO_TYPE_MEM,0x02)
#define IO_MEM_IOCTL_GET_DEVICE_ERROR     _IO(IO_TYPE_MEM,0x03)

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_uint _io_mem_install(char *, void *, _file_size);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
