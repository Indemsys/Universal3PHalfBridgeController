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
*   This file contains the RTCS shell.
*
*
*END************************************************************************/

#ifndef __sh_prv_h__
#define __sh_prv_h__

#define SHELL_MAX_CMDLEN     256

#define Shell_get_context(ptr) ((SHELL_CONTEXT_PTR)(void *) ptr)


typedef struct {
   char             *ARGV[SHELL_MAX_ARGS];           // MUST BE FIRST !!
   int32_t               ARGC;
   char                 CMD_LINE[SHELL_MAX_CMDLEN];
   char                 HISTORY[SHELL_MAX_CMDLEN];
   SHELL_COMMAND_PTR    COMMAND_LIST_PTR;
   MQX_FILE_PTR         COMMAND_FP;
   bool              EXIT;
#if SHELLCFG_USES_MFS   
   char                 CURRENT_DEVICE_NAME[SHELL_MAX_DEVLEN]; 
   MQX_FILE_PTR         CURRENT_DEVICE_FP;
   char                 CURRENT_DIR[SHELL_PATHNAME_LEN + 1];
#endif   
} SHELL_CONTEXT, * SHELL_CONTEXT_PTR;

/*
** Function prototypes 
*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
/*EOF*/
