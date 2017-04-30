#ifndef _flashxprv_h_
#define _flashxprv_h_
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
*   The file contains functions prototype, defines, structure 
*   definitions private to the flash drivers
*
*
*END************************************************************************/


/*----------------------------------------------------------------------*/
/*
**                    Structure Definitions
*/

/*
** The address of this structure is used to maintain flash specific 
** information.
*/
typedef struct io_flashx_struct
{
    /* The function to call to erase a sector on the device */
    bool (_CODE_PTR_           SECTOR_ERASE)(struct io_flashx_struct *, char *, _mem_size);

    /* The function to call to program a sector on the device */
    bool (_CODE_PTR_           SECTOR_PROGRAM)(struct io_flashx_struct *, char *, char *, _mem_size);
   
    /* The function to call to erase the entire device */
    bool (_CODE_PTR_           CHIP_ERASE)(struct io_flashx_struct *);
   
    /* The function to call to read data from flash */
    _mqx_int (_CODE_PTR_          READ)(struct io_flashx_struct *, char *, char *, _mem_size);

    /* The function to call to initialize the device */
    bool (_CODE_PTR_           INIT)(struct io_flashx_struct *);
   
    /* The function to call to disable the device */
    void (_CODE_PTR_              DEINIT)(struct io_flashx_struct *);

    /* The function to call to write enable or protect the device */
    bool (_CODE_PTR_           WRITE_PROTECT)(struct io_flashx_struct *, _mqx_uint);
   
    /* The ioctl function to call */
    _mqx_int (_CODE_PTR_          IOCTL)(struct io_flashx_struct *, _mqx_uint, void *);

    /* Base address of the flash device. Useful for external flash devices
    ** whose address space is determined by BSP.
    */
    _mem_size                     BASE_ADDR;

    /* This array of strutures provides the description of HW flash configuration */ 
    FLASHX_BLOCK_INFO_STRUCT_PTR  HW_BLOCK;
   
    /* This array of strutures provides the description of HW flash configuration */ 
    FLASHX_FILE_BLOCK_PTR         FILE_BLOCK;
   
    /* The bus width of the device (in bytes, i.e. WIDTH = 4 for 32 bits) */
    _mqx_uint                     WIDTH;
   
    /* The number of parallel devices */
    _mqx_uint                     DEVICES;
   
    /* Light weight semaphore for HW access exclusion */
    LWSEM_STRUCT                  HW_ACCESS;

    /* Number of files opened */
    _mqx_uint                     FILES;
   
    /* Flags */
    _mqx_uint                     FLAGS;
   
    /* When finished programming, should a comparison of data be made
    ** to verify that the write worked correctly.
    */
    _mqx_uint                     WRITE_VERIFY;

    /* The address of device specific structure got in init phase */ 
    void                         *DEVICE_SPECIFIC_INIT;

    /* The address of device specific structure */ 
    void                         *DEVICE_SPECIFIC_DATA;

} IO_FLASHX_STRUCT, * IO_FLASHX_STRUCT_PTR;

typedef struct io_flashx_file_struct
{
    /* HW block reference */
    FLASHX_FILE_BLOCK const      *FILE_BLOCK;

    /* Next four are needed for caching data */
    _mqx_uint                     DIRTY_DATA;
    _mqx_int                      CACHE_BLOCK;
    _mqx_int                      CACHE_SECTOR;
    _mqx_int                      CACHE_FILE_SECTOR;
    /* One sector cache memory of size MAX_SECT_SIZE */
    char                      *CACHE_PTR;

    /* The maximum sector size of this file: useful for allocating cache space */
    _mem_size                     MAX_SECT_SIZE;

    /* The size of the erase array in bytes */
    _mqx_uint                     ERASE_ARRAY_MAX;

    /* The address of erase check array */
    uint8_t                       *ERASE_ARRAY;

    /* Flags */
    _mqx_uint                     FLAGS;
   
} IO_FLASHX_FILE_STRUCT, * IO_FLASHX_FILE_STRUCT_PTR;

/*----------------------------------------------------------------------*/
/*
**                    FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

_mqx_int  _io_flashx_flush_buffer(MQX_FILE_PTR); 
_mqx_int  _io_flashx_flush_sector_buffer(MQX_FILE_PTR); 
 bool  _io_flashx_erase_sector(MQX_FILE_PTR, _mqx_uint, _mqx_uint, _mqx_uint);
FLASHX_BLOCK_INFO_STRUCT_PTR _io_flashx_phys_to_sector(IO_FLASHX_STRUCT_PTR, _mem_size, _mqx_uint *, _mem_size *, _mem_size *);
_mqx_int  _io_flashx_find_correct_sectors(MQX_FILE_PTR, _mqx_uint, _mqx_uint *, _mqx_uint *, _mqx_uint *, _mqx_uint *);
_mqx_int  _io_flashx_open(MQX_FILE_PTR, char *, char *);
_mqx_int  _io_flashx_close(MQX_FILE_PTR);
_mqx_int  _io_flashx_write(MQX_FILE_PTR, char *, _mqx_int);
_mqx_int  _io_flashx_read(MQX_FILE_PTR, char *, _mqx_int);
_mqx_int  _io_flashx_ioctl(MQX_FILE_PTR, _mqx_uint, void *);
_mem_size _io_flashx_write_partial_sector(MQX_FILE_PTR, _mqx_uint, _mqx_uint, _mem_size, _mem_size, _mqx_uint, char *);
_mem_size _io_flashx_check_free_space(char *, _mem_size);
void      _io_flashx_wait_us(_mqx_int);

bool _allocate_sector_cache(MQX_FILE_PTR);
   
#ifdef __cplusplus
}
#endif

#endif
/* EOF */
