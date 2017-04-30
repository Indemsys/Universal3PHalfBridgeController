#ifndef _pcflash_h_
#define _pcflash_h_
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
*   The file contains functions prototype, defines, structure 
*   definitions specific for the PC Card flash drivers
*   Revision History:
*   Date          Version        Changes
*   Nov 16, 1998  2.40           Convert to MQX2.40 from MQX2.33
*
*
*END************************************************************************/

#include "ioctl.h"

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

/*
** PCFlash IOCTL calls
*/
#define PCFLASH_IOCTL_IS_CARD_INSERTED     _IO(IO_TYPE_PCFLASH,0x01)
#define PCFLASH_IOCTL_GET_NUM_SECTORS      _IO(IO_TYPE_PCFLASH,0x02)
#define PCFLASH_IOCTL_GET_SECTOR_SIZE      _IO(IO_TYPE_PCFLASH,0x03)
#define PCFLASH_IOCTL_GET_DRIVE_PARAMS     _IO(IO_TYPE_PCFLASH,0x04)
#define PCFLASH_IOCTL_SET_BLOCK_MODE       _IO(IO_TYPE_PCFLASH,0x05)

/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

typedef struct pcflash_drive_info_struct
{
   uint32_t  NUMBER_OF_HEADS; 
   uint32_t  NUMBER_OF_TRACKS;
   uint32_t  SECTORS_PER_TRACK;
} PCFLASH_DRIVE_INFO_STRUCT, * PCFLASH_DRIVE_INFO_STRUCT_PTR;
   
    
/*----------------------------------------------------------------------*/
/*
**                    FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif
 
uint32_t _io_pcflash_install(char *);
uint32_t _io_apcflash_install(char *);

#ifdef __cplusplus
}
#endif


#endif
/* EOF */
