/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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

#ifndef __sh_mfs_h__
#define __sh_mfs_h__

/*
** Function prototypes 
*/
#ifdef __cplusplus
extern "C" {
#endif

extern int32_t Shell_cache(int32_t argc, char *argv[] );
extern int32_t Shell_compare(int32_t argc, char *argv[] );
extern int32_t Shell_create(int32_t argc, char *argv[] );
extern int32_t Shell_flush_cache(int32_t argc, char *argv[] );
extern int32_t Shell_cd(int32_t argc, char *argv[] );
extern int32_t Shell_copy(int32_t argc, char *argv[] );
extern int32_t Shell_del(int32_t argc, char *argv[] );
extern int32_t Shell_df(int32_t argc, char *argv[] );
extern int32_t Shell_di(int32_t argc, char *argv[] );
extern int32_t Shell_dir(int32_t argc, char *argv[] );
extern int32_t Shell_dirent(int32_t argc, char *argv[] );
extern int32_t Shell_disect(int32_t argc, char *argv[] );
extern int32_t Shell_format(int32_t argc, char *argv[] );
extern int32_t Shell_mkdir(int32_t argc, char *argv[] );
extern int32_t Shell_pwd(int32_t argc, char *argv[] );
extern int32_t Shell_read(int32_t argc, char *argv[] );
extern int32_t Shell_rename(int32_t argc, char *argv[] );
extern int32_t Shell_rmdir(int32_t argc, char *argv[] );
extern int32_t Shell_type(int32_t argc, char *argv[] ); 
extern int32_t Shell_use(int32_t argc, char *argv[] );
extern int32_t Shell_write(int32_t argc, char *argv[] );
extern int32_t Shell_write_test(int32_t argc, char *argv[] );
extern int32_t Shell_read_test(int32_t argc, char *argv[] );
extern int32_t Shell_append_test(int32_t argc, char *argv[] );

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
