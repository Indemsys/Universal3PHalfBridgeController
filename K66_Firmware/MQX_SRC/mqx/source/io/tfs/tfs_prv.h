#ifndef __tfs_prv_h__
#define __tfs_prv_h__
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
*   This file contains the structure definitions and constants
*   that are internal to the Embedded MS-DOS File System (TFS)
*
*
*END************************************************************************/


#include <tfs.h>


/* configuration data for the TFS */
typedef struct tfs_drive_struct
{
   char                 *IDENTIFIER;
   const TFS_DIR_ENTRY      *ROOT;
  
} TFS_DRIVE_STRUCT, * TFS_DRIVE_STRUCT_PTR;


/*
** extern statements for TFS
*/
#ifdef __cplusplus
extern "C" {
#endif

extern char *TFS_Parse_Out_Device_Name (char *);
extern int32_t TFS_Cmp (char *, char *);
extern void   *TFS_Open_File (TFS_DRIVE_STRUCT_PTR, char *, uint32_t *);
extern uint32_t TFS_Read (MQX_FILE_PTR, uint32_t, char *, uint32_t *);
extern uint32_t TFS_Write (MQX_FILE_PTR, uint32_t, char *, uint32_t *);
extern uint32_t TFS_Move_File_Pointer (MQX_FILE_PTR, uint32_t *);
extern int32_t TFS_Close_File (MQX_FILE_PTR);
    
#ifdef __cplusplus
}
#endif

    
#endif
/* EOF */
