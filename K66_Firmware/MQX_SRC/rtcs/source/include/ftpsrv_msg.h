#ifndef __ftpsrv_msg_h__
#define __ftpsrv_msg_h__
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

#ifdef __cplusplus
extern "C" {
#endif

extern const char ftpsrvmsg_opening_datacon[];
extern const char ftpsrvmsg_ok[];
extern const char ftpsrvmsg_portok[];
extern const char ftpsrvmsg_site_info[];
extern const char ftpsrvmsg_type_ascii[];
extern const char ftpsrvmsg_type_image[];
extern const char ftpsrvmsg_help_start[];
extern const char ftpsrvmsg_help_end[];
extern const char ftpsrvmsg_syst[];
extern const char ftpsrvmsg_banner[];
extern const char ftpsrvmsg_goodbye[];
extern const char ftpsrvmsg_closing_succ[];
extern const char ftpsrvmsg_size[];
extern const char ftpsrvmsg_trans_complete[];
extern const char ftpsrvmsg_pasv_mode[];
extern const char ftpsrvmsg_epsvok[];
extern const char ftpsrvmsg_pass[];
extern const char ftpsrvmsg_need_password[];
extern const char ftpsrvmsg_ready_for_dest[];
extern const char ftpsrvmsg_cannot_open[];
extern const char ftpsrvmsg_no_datacon[];
extern const char ftpsrvmsg_trans_abort[];
extern const char ftpsrvmsg_writeeof[];
extern const char ftpsrvmsg_writefail[];
extern const char ftpsrvmsg_authfail[];
extern const char ftpsrvmsg_locerr[];
extern const char ftpsrvmsg_no_space[];
extern const char ftpsrvmsg_unrecognized[];
extern const char ftpsrvmsg_badport[];
extern const char ftpsrvmsg_badsyntax[];
extern const char ftpsrvmsg_syntax_error[];
extern const char ftpsrvmsg_type_unknown[];
extern const char ftpsrvmsg_unimp[];
extern const char ftpsrvmsg_badseq[];
extern const char ftpsrvmsg_pwd_error[];
extern const char ftpsrvmsg_cd_error[];
extern const char ftpsrvmsg_rmdir_error[];
extern const char ftpsrvmsg_delete_error[];
extern const char ftpsrvmsg_mkdir_error[];
extern const char ftpsrvmsg_badprot[];
extern const char ftpsrvmsg_not_logged[];
extern const char ftpsrvmsg_no_filename[];
extern const char ftpsrvmsg_no_fs[];
extern const char ftpsrvmsg_no_file[];
extern const char ftpsrvmsg_no_memory[];

#ifdef __cplusplus
}
#endif

#endif
/*EOF*/
