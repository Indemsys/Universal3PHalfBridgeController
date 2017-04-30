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
*   The file contains public functions prototype, defines, structure 
*   definitions private to the debug driver
*
*
*END************************************************************************/
#ifndef __iodebug_h__
#define __iodebug_h__


/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/
/* IOCLT flag - change flush character */
#define IO_IOCTL_IODEBUG_SET_FLUSH_CHAR         _IO(IO_TYPE_IODEBUG,0x01)
/* No flush character */
#define IODEBUG_NOFLUSH_CHAR                    '\0' 
#define IODEBUG_MODE_SEMIHOST                   0
#define IODEBUG_MODE_ITM                        1

/*--------------------------------------------------------------------------*/
/*
**                            DATATYPE DECLARATIONS
*/

typedef struct { 
    uint32_t MODE;
    uint32_t DATA_LENGTH;
    char    FLUSH_CHAR;
} IODEBUG_INIT_STRUCT, * IODEBUG_INIT_STRUCT_PTR;

typedef const IODEBUG_INIT_STRUCT * IODEBUG_INIT_STRUCT_CPTR;


/*--------------------------------------------------------------------------*/
/*
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _io_debug_install(char *, IODEBUG_INIT_STRUCT_CPTR);
extern _mqx_int _io_debug_uninstall(IO_DEVICE_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */

