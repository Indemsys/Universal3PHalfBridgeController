#ifndef __rtcs_util_h__
#define __rtcs_util_h__

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
*   This file is header for UTF-8 functions.
*
*END************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void rtcs_url_decode(char *url);
void rtcs_url_cleanup(char *url);
void rtcs_path_normalize(char *path);
char * rtcs_path_create(const char *root, char *filename);
char * rtcs_path_strip_delimiters(char *path);

#ifdef __cplusplus
}
#endif

#endif
