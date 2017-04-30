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
*   MQX configuration set: verification options enabled
*
*
*END************************************************************************/

#ifndef __verif_enabled_config_h__
#define __verif_enabled_config_h__

#ifndef MQX_CHECK_VALIDITY                
#define MQX_CHECK_VALIDITY                  1
#endif

#ifndef MQX_MONITOR_STACK                 
#define MQX_MONITOR_STACK                   1
#endif

#ifndef MQX_CHECK_ERRORS                  
#define MQX_CHECK_ERRORS                    1
#endif

#ifndef MQX_TASK_CREATION_BLOCKS          
#define MQX_TASK_CREATION_BLOCKS            1
#endif

#ifndef MQX_CHECK_MEMORY_ALLOCATION_ERRORS
#define MQX_CHECK_MEMORY_ALLOCATION_ERRORS  1
#endif

#ifndef MQX_VERIFY_KERNEL_DATA   
#define MQX_VERIFY_KERNEL_DATA              1
#endif

#endif
/* EOF */
