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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   RTCS assert macros.
*
*
*END************************************************************************/
#ifndef __rtcs_assert_h__
#define __rtcs_assert_h__

#if RTCSCFG_ENABLE_ASSERT_PRINT

    #define RTCS_ASSERT_ERROR_MESSAGE   "RTCS_ASSERT: %s: %s: %d\n\r"

    /* Checks that the given condition is true, 
     * otherwise it prints error message 
     * and stops the program execution.*/
    #define RTCS_ASSERT(condition)                                                  \
            if (!(condition))                                                       \
            {                                                                       \
                printf(RTCS_ASSERT_ERROR_MESSAGE, __func__, __FILE__, __LINE__);    \
                while (1);                                                          \
            } 

#elif RTCSCFG_ENABLE_ASSERT

    /* Checks that the given condition is true, 
     * otherwise it stops the program execution.*/
    #define RTCS_ASSERT(condition)                                                  \
            if (!(condition))                                                       \
            {                                                                       \
                while (1);                                                          \
            } 
        
#else

    #define RTCS_ASSERT(condition)  ((void) 0)

#endif

#endif  /* __rtcs_assert_h__ */

