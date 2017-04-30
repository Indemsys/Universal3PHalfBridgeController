
/*HEADER**********************************************************************
*
* Copyright 2008-2012 Freescale Semiconductor, Inc.
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains runtime support for the IAR Compiler.
*
*
*END************************************************************************/

#include "mqx.h"

/* IAR EWARM init function prototypes */
int __low_level_init(void);
void *malloc(_mem_size);
void *calloc(_mem_size, _mem_size);
void *realloc(void*,_mem_size);
void free(void *);
void __iar_program_start(void);
int exit(int);

extern void init_hardware(void);

#if MQX_ENABLE_CPP || BSPCFG_ENABLE_CPP
extern void __cexit_call_dtors(void);
#endif /* MQX_ENABLE_CPP || BSPCFG_ENABLE_CPP */

/*!
 * \brief Call function init_hardware() in BSP libraty to initialize CPU registers
 */
int __low_level_init(void)
{
    /* Initialize device. */
    init_hardware();
    return (1);
}

/*!
 * \brief Override C/C++ runtime heap allocation function in IAR's DLIB
 *
 * \param bytes
 *
 * \return pointer
 */
void *malloc(_mem_size bytes)
{
    return _mem_alloc_system(bytes);
}

/*!
 * \brief Override C/C++ runtime heap allocation function in IAR's DLIB
 *
 * \param n
 * \param z
 *
 * \return pointer
 */
void *calloc(_mem_size n, _mem_size z)
{
    return _mem_alloc_system_zero(n*z);
}

/*!
 * \brief Override C/C++ runtime heap reallocation function in IAR's DLIB
 *
 * \param ptr
 * \param bytes
 *
 * \return pointer
 */
void *realloc(void* ptr,_mem_size bytes)
{
    return _mem_realloc(ptr,bytes);
}

/*!
 * \brief Override C/C++ runtime heap deallocation function in IAR's DLIB
 *
 * \param p
 */
void free(void *p)
{
    _mem_free(p);
}


/*!
 * \brief Replacement for Codewarrior's exit function
 *
 * \param status
 *
 * \return should not return
 */
#if MQX_EXIT_ENABLED
int exit(int status)
{
#if MQX_ENABLE_CPP || BSPCFG_ENABLE_CPP
    /* Destroy all constructed global objects */
    __cexit_call_dtors();
#endif /* MQX_ENABLE_CPP || BSPCFG_ENABLE_CPP */

    /*
    ** Change for whatever is appropriate for your board
    ** and application.  Perhaps a software reset or
    ** some kind of trap/call to the kernel soft re-boot.
    */
    while (TRUE) {
    }

    //return(0);
}

#else  /* MQX_EXIT_ENABLED */

/* int exit(int) is supplied by IAR's DLIB */

#endif /* MQX_EXIT_ENABLED */

/*!
 * \brief Perform necessary toolchain startup routines before main()
 */
void toolchain_startup(void)
{
    __iar_program_start();
}

/* EOF */
