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
*   This file contains an implementation of an
*   FTP server.
*
*
*END************************************************************************/

#include "mqx_cnfg.h"


#define FTPSRV_PRODUCT_STRING "RTCS FTPSRV"
/* Response messages */
const char ftpsrvmsg_opening_datacon[]  = "150 Opening data connection.\r\n";
const char ftpsrvmsg_ok[]               = "200 OK.\r\n";
const char ftpsrvmsg_portok[]           = "200 Port command OK\r\n";
const char ftpsrvmsg_site_info[]        = "200 "FTPSRV_PRODUCT_STRING": No site specific information.\r\n";
const char ftpsrvmsg_type_ascii[]       = "200 Type ASCII.\r\n";
const char ftpsrvmsg_type_image[]       = "200 Type Binary.\r\n";
const char ftpsrvmsg_help_start[]       = "214-The following commands are recognized:\r\n";
const char ftpsrvmsg_help_end[]         = "214 Direct comments to Freescale.\r\n";
const char ftpsrvmsg_syst[]             = "215 MQX\r\n";
const char ftpsrvmsg_banner[]           = "220 "FTPSRV_PRODUCT_STRING" Ready\r\n";
const char ftpsrvmsg_goodbye[]          = "221 Goodbye.\r\n";
const char ftpsrvmsg_closing_succ[]     = "226 Closing data connection. Requested file action successful.\r\n";
const char ftpsrvmsg_size[]             = "226 File size is %d.\r\n";
const char ftpsrvmsg_trans_complete[]   = "226 Transfer complete.\r\n";
const char ftpsrvmsg_pasv_mode[]        = "227 Entering Passive Mode (%ld,%ld,%ld,%ld,%hd,%hd).\r\n";
const char ftpsrvmsg_epsvok[]           = "229 Entering Extended Passive Mode (|||%d|)\r\n";
const char ftpsrvmsg_pass[]             = "230 Password ok, User logged in.\r\n";
const char ftpsrvmsg_need_password[]    = "331 User name okay, need password.\r\n";
const char ftpsrvmsg_ready_for_dest[]   = "350 File exists, ready for destination name.\r\n";
const char ftpsrvmsg_cannot_open[]      = "425 Can't open data connection.\r\n";
const char ftpsrvmsg_no_datacon[]       = "425 Data connection refused.\r\n";
const char ftpsrvmsg_trans_abort[]      = "426 Connection closed; transfer aborted.\r\n";
const char ftpsrvmsg_writeeof[]         = "426 Device full\r\n";
const char ftpsrvmsg_writefail[]        = "426 Write error\r\n";
const char ftpsrvmsg_authfail[]         = "430 Invalid username or password\r\n";
const char ftpsrvmsg_locerr[]           = "451 Requested action aborted. Local error in processing\r\n";
const char ftpsrvmsg_no_space[]         = "452 Requested action not taken. Insufficient storage space in system.\r\n";
const char ftpsrvmsg_unrecognized[]     = "500 Syntax error, command unrecognized.\r\n";
const char ftpsrvmsg_badport[]          = "501 Bad port syntax\r\n";
const char ftpsrvmsg_badsyntax[]        = "501 Incorrect command syntax\r\n";
const char ftpsrvmsg_syntax_error[]     = "501 Syntax error in parameters or arguments.\r\n";
const char ftpsrvmsg_type_unknown[]     = "501 Unknown type.\r\n";
const char ftpsrvmsg_unimp[]            = "502 Command not implemented\r\n";
const char ftpsrvmsg_badseq[]           = "503 Bad sequence of commands\r\n";
const char ftpsrvmsg_pwd_error[]        = "521 Error getting current directory.\r\n";
const char ftpsrvmsg_cd_error[]         = "521 Error changing directory.\r\n";
const char ftpsrvmsg_rmdir_error[]      = "521 Error removing directory.\r\n";
const char ftpsrvmsg_delete_error[]     = "521 Error deleting file.\r\n";
const char ftpsrvmsg_mkdir_error[]      = "521 Directory already exists.\r\n";
const char ftpsrvmsg_badprot[]          = "552 Network protocol not supported, use %s\r\n";
const char ftpsrvmsg_not_logged[]       = "530 Not logged in.\r\n";
const char ftpsrvmsg_no_filename[]      = "550 Rename from file not specified.\r\n";
const char ftpsrvmsg_no_fs[]            = "550 Requested action not taken. File system not mounted.\r\n";
const char ftpsrvmsg_no_file[]          = "550 Requested action not taken. File unavailable.\r\n";
const char ftpsrvmsg_no_memory[]        = "550 Requested action not taken. Memory unavailable.\r\n";
/* EOF*/
