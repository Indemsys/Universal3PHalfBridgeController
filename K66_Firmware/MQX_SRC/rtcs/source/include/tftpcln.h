#ifndef __tftpcln_h__
#define __tftpcln_h__
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
*   This file contains the definitions needed by TFTP client.
*
*END************************************************************************/

#include <rtcs.h>

#define TFTPCLN_MAX_DATA_SIZE (512)

typedef void(*TELNETCLN_DATA_CALLBACK)(uint32_t data_length);
typedef void(*TELNETCLN_ERROR_CALLBACK)(uint16_t error_code, char* error_string);

typedef struct tftpcln_param_struct
{
    sockaddr                 sa_remote_host; /* Information about remote host client will connect to. */
    TELNETCLN_DATA_CALLBACK  recv_callback;  /* Callback invoked when data block is received. */
    TELNETCLN_DATA_CALLBACK  send_callback;  /* Callback invoked when data block is send. */
    TELNETCLN_ERROR_CALLBACK error_callback; /* Callback invoked when server responds with error. */
}TFTPCLN_PARAM_STRUCT;

uint32_t TFTPCLN_connect(TFTPCLN_PARAM_STRUCT *params);
int32_t TFTPCLN_disconnect(uint32_t handle);
int32_t TFTPCLN_get(uint32_t handle, char *local_file, char *remote_file);
int32_t TFTPCLN_put(uint32_t handle, char *local_file, char *remote_file);

#endif
