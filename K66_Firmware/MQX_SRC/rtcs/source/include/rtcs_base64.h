#ifndef __rtcs_base64_h__
#define __rtcs_base64_h__
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*  Base64 encode and decode header.
*
*
*END************************************************************************/

#include <mqx.h>

#ifdef __cplusplus
extern "C" {
#endif

char* base64_encode(char *source, char *destination);
char* base64_decode(char *dst, char *src, uint32_t dst_size);
char* base64_encode_binary(char *source, char *destination, uint32_t length);
bool isbase64(const char* string);

#ifdef __cplusplus
}
#endif

#endif
