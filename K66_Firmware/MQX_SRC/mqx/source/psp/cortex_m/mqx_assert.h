
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
*   
*
*
*END************************************************************************/
 
#ifndef __mqx_assert_h__
#define __mqx_assert_h__

#ifdef __cplusplus
    extern "C" {
#endif

#define MQX_ASSERT_NONE         (0)
#define MQX_ASSERT_TOOLCHAIN    (1)
#define MQX_ASSERT_BKPT         (2)
#define MQX_ASSERT_BKPT_INL     (3)
#define MQX_ASSERT_MSG          (4)
#define MQX_ASSERT_LOOP         (5)
#define MQX_ASSERT_LOOP_INL     (6)

#ifndef MQX_ASSERT
#   define MQX_ASSERT MQX_ASSERT_NONE
#endif

#if MQX_ASSERT == MQX_ASSERT_TOOLCHAIN
#   ifndef assert
#       include <assert.h>
#   endif
#else
#   if defined(NDEBUG) || (MQX_ASSERT == MQX_ASSERT_NONE)
#       undef assert
#       define assert(exp)  
#   else
#       undef assert
#       define assert(exp) ((exp) || (_mqx_assert(#exp, __func__, __FILE__, __LINE__),0))
#   endif

#   if MQX_ASSERT == MQX_ASSERT_LOOP_INL
        inline void _mqx_assert(const char *string, const char *func, const char *file, int line)
        {
            __asm("cpsid i");
            while(1) {}
        }
#   elif MQX_ASSERT == MQX_ASSERT_BKPT_INL
        inline void _mqx_assert(const char *string, const char *func, const char *file, int line)
        {
            __asm("BKPT 0");
        }
#   else
        extern void _mqx_assert(const char *string, const char *func, const char *file, int line);
#   endif
  
#endif
#ifdef __cplusplus
}
#endif

#endif   /* __mqx_assert_h__ */
