
/*HEADER**********************************************************************
*
* Copyright 2008-2011 Freescale Semiconductor, Inc.
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
*   This file contains generic macros
*
*
*END************************************************************************/

#ifndef __mqx_macros_h__
#define __mqx_macros_h__


/*--------------------------------------------------------------------------*/
/*      MACROS FOR CONVENIENT DATA STRUCTURE DECLARATION/MANIPULATION       */

#ifndef RESERVED_REGISTER
   #define RESERVED_REGISTER(start,next) unsigned char R_ ## start[next-start]
#endif

#ifndef ELEMENTS_OF
   #define ELEMENTS_OF(x) ( sizeof(x)/sizeof(x[0]) )
#endif

#ifndef MEM_ALIGN
   #define MEM_ALIGN(x,pow)  ( ( ((uint32_t)x) + ((pow)-1)) & ~((pow)-1) )
#endif

#endif
