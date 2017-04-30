#ifndef __echocln_h__
#define __echocln_h__
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
*   Definitions for use by the ECHO Client.
*
*
*END************************************************************************/

#define ECHOCLN_ERR_DATA_COMPARE_FAIL  1
#define ECHOCLN_ERR_SOCKET             2
#define ECHOCLN_ERR_OUT_OF_MEMORY      3
#define ECHOCLN_ERR_INVALID_PARAM      4

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ECHOCLN_connect(const struct addrinfo * addrinfo_ptr);
int32_t  ECHOCLN_process(uint32_t sock, char * buffer, uint32_t buflen, int32_t count, TIME_STRUCT_PTR time_ptr);

#ifdef __cplusplus
}
#endif

#endif /* __echocln_h__ */

