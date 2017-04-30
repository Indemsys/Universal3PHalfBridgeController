#ifndef __sdcard_h__
#define __sdcard_h__
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
*   The file contains functions prototype, defines, structure 
*   definitions private to the SD card.
*
*
*END************************************************************************/

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

#define IO_SDCARD_BLOCK_SIZE_POWER   (9)
#define IO_SDCARD_BLOCK_SIZE         (1 << IO_SDCARD_BLOCK_SIZE_POWER)

#define IO_IOCTL_SDCARD_GET_CID      _IO(IO_TYPE_SDCARD,0x01)


/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/

/*
** SDCARD_INIT STRUCT
**
** The address of this structure is used to maintain sd card init
** information.
*/
typedef struct sdcard_init_struct
{
   /* The function to call to initialize sd card */
   bool  (_CODE_PTR_ INIT_FUNC)(MQX_FILE_PTR);

   /* The function to call to read blocks from sd card */
   _mqx_int (_CODE_PTR_ READ_FUNC)(MQX_FILE_PTR, unsigned char *, uint32_t, uint32_t);

   /* The function to call to write blocks to sd card */
   _mqx_int (_CODE_PTR_ WRITE_FUNC)(MQX_FILE_PTR, unsigned char *, uint32_t, uint32_t);
   
   /* Signal specification */
   uint32_t             SIGNALS;

} SDCARD_INIT_STRUCT, * SDCARD_INIT_STRUCT_PTR;


#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _io_sdcard_install(char *,SDCARD_INIT_STRUCT_PTR,MQX_FILE_PTR);

#ifdef __cplusplus
}
#endif


#endif

/* EOF */
