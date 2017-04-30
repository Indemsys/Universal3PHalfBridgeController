
/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   Implementation of a _mqx_assert functions.
*
*
*END************************************************************************/
#include <mqx.h>
#include <bsp.h>

#if MQX_ASSERT == MQX_ASSERT_LOOP
void _mqx_assert(const char *string, const char *func, const char *file, int line)
{
   __asm("cpsid i");
   while(1) {}
} 
#endif

#if MQX_ASSERT == MQX_ASSERT_MSG
void _mqx_assert(const char *string, const char *func, const char *file, int line)
{
   printf("%s, %s, %s, %d \n\r",string, func, file, line);
   __asm("cpsid i");
   while(1) {}
}
#endif

#if MQX_ASSERT == MQX_ASSERT_BKPT
void _mqx_assert(const char *string, const char *func, const char *file, int line)
{
   __asm("BKPT 0");
}
#endif
