/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   The file contains functions for internal flash read, write, erase 
*
*
*END************************************************************************/

#include "mqx.h"
#include "bsp.h"
#include "flashx.h"
#include "flashxprv.h"
#include "flash_ftfe_prv.h"
#include "flash_ftfe.h"

static void ftfe_ram_function(volatile uint8_t *, void (_CODE_PTR_)(volatile uint32_t)); 
static void ftfe_ram_function_end(void);
static uint32_t ftfe_flash_command_sequence(volatile FTFE_FLASH_INTERNAL_STRUCT_PTR, uint8_t *, uint8_t, void *, uint32_t);
static char *ftfe_init_ram_function(char *, char *);
static _mqx_int ftfe_deinit_ram_function(char *);

uint32_t ftfe_flash_swap_command(FTFE_FLASH_INTERNAL_STRUCT_PTR, uint32_t, uint8_t);
uint32_t ftfe_flash_swap_init_update(IO_FLASHX_STRUCT_PTR, uint32_t);
uint32_t ftfe_flash_swap_complete(IO_FLASHX_STRUCT_PTR, uint32_t);
uint32_t ftfe_flash_swap_status(IO_FLASHX_STRUCT_PTR, uint32_t , uint8_t *);
uint32_t ftfe_flash_swap_and_reset(IO_FLASHX_STRUCT_PTR);

/* Note: bank1-3 reading are made from another addresses than bank1 writing. Therefore we add next parameter as base for writing */
const FLASHX_BLOCK_INFO_STRUCT _flashx_kinetisN_block_map[] = {
    { BSP_INTERNAL_FLASH_SIZE / BSP_INTERNAL_FLASH_SECTOR_SIZE / 4, (uint32_t) BSP_INTERNAL_FLASH_BASE,              BSP_INTERNAL_FLASH_SECTOR_SIZE },
    { BSP_INTERNAL_FLASH_SIZE / BSP_INTERNAL_FLASH_SECTOR_SIZE / 4, (uint32_t) BSP_INTERNAL_FLASH_BASE + 1 * BSP_INTERNAL_FLASH_SIZE / 4, BSP_INTERNAL_FLASH_SECTOR_SIZE },
    { BSP_INTERNAL_FLASH_SIZE / BSP_INTERNAL_FLASH_SECTOR_SIZE / 4, (uint32_t) BSP_INTERNAL_FLASH_BASE + 2 * BSP_INTERNAL_FLASH_SIZE / 4, BSP_INTERNAL_FLASH_SECTOR_SIZE },
    { BSP_INTERNAL_FLASH_SIZE / BSP_INTERNAL_FLASH_SECTOR_SIZE / 4, (uint32_t) BSP_INTERNAL_FLASH_BASE + 3 * BSP_INTERNAL_FLASH_SIZE / 4, BSP_INTERNAL_FLASH_SECTOR_SIZE },
    { 0, 0, 0 }
};

const FLASHX_BLOCK_INFO_STRUCT _flashx_kinetisX_block_map[] = {
    { BSP_INTERNAL_FLASH_SIZE / BSP_INTERNAL_FLASH_SECTOR_SIZE / 2, (uint32_t) BSP_INTERNAL_FLASH_BASE,              BSP_INTERNAL_FLASH_SECTOR_SIZE },
    { BSP_INTERNAL_FLASH_SIZE / BSP_INTERNAL_FLASH_SECTOR_SIZE / 2, (uint32_t) BSP_INTERNAL_FLASH_BASE + 1 * BSP_INTERNAL_FLASH_SIZE / 2, BSP_INTERNAL_FLASH_SECTOR_SIZE },
    { 0, 0, 0 }
};

const FLASHX_DEVICE_IF_STRUCT _flashx_ftfe_if = {
    ftfe_flash_erase_sector,
#if BSPCFG_FLASHX_USE_PA_RAM
    ftfe_flash_write_sector_pa_ram,
#else
    ftfe_flash_write_sector,
#endif
    NULL,
    NULL,
    ftfe_flash_init,
    ftfe_flash_deinit,
    NULL,
    ffte_flash_ioctl
};

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftfe_flash_command_sequence
* Returned Value   : uint32_t an error code
* Comments         :
*    Run command in FTFE device.
*
*END*----------------------------------------------------------------------*/
static uint32_t ftfe_flash_command_sequence
(                                                
    /* [IN] Flash specific structure */
    volatile FTFE_FLASH_INTERNAL_STRUCT_PTR     dev_spec_ptr,

    /* [IN] Command byte array */
    uint8_t                             *command_array,

    /* [IN] Number of values in the array */
    uint8_t                             count,

    /* [IN] The address which will be affected by command */
    void                                *affected_addr,

    /* [IN] The address which will be affected by command */
    uint32_t                            affected_size
)
{
    volatile uint8_t  fstat;
    uint32_t result;
    void (* RunInRAM)(volatile uint8_t *, void (_CODE_PTR_) (volatile uint32_t));
    void (* RunInvalidateInRAM)(volatile uint32_t);
    RunInvalidateInRAM = NULL;

#if PSP_MQX_CPU_IS_COLDFIRE
    uint32_t temp;
#endif
    FTFE_MemMapPtr ftfe_ptr;

    ftfe_ptr = (FTFE_MemMapPtr)dev_spec_ptr->ftfe_ptr;

    /* get pointer to RunInRAM function */
    RunInRAM = (void(*)(volatile uint8_t *, void (_CODE_PTR_) (volatile uint32_t)))(dev_spec_ptr->flash_execute_code_ptr);
#if PSP_MQX_CPU_IS_KINETIS
    RunInvalidateInRAM = (void(*)(volatile uint32_t))(dev_spec_ptr->flash_invalidate_code_ptr);
#endif
    
    /* set the default return as FTFE_OK */
    result = FTFE_OK;
    
    /* check CCIF bit of the flash status register */
    while (0 == (ftfe_ptr->FSTAT & FTFE_FSTAT_CCIF_MASK))
        { };
    
    /* clear RDCOLERR & ACCERR & FPVIOL error flags in flash status register */
    if (ftfe_ptr->FSTAT & FTFE_FSTAT_RDCOLERR_MASK) 
    {
        ftfe_ptr->FSTAT |= FTFE_FSTAT_RDCOLERR_MASK;
    }
    if (ftfe_ptr->FSTAT & FTFE_FSTAT_ACCERR_MASK) 
    {
        ftfe_ptr->FSTAT |= FTFE_FSTAT_ACCERR_MASK;
    }
    if (ftfe_ptr->FSTAT & FTFE_FSTAT_FPVIOL_MASK) 
    {
        ftfe_ptr->FSTAT |= FTFE_FSTAT_FPVIOL_MASK;
    }
    
    switch (count)
    {
        case 12: ftfe_ptr->FCCOBB = command_array[--count];
        case 11: ftfe_ptr->FCCOBA = command_array[--count];
        case 10: ftfe_ptr->FCCOB9 = command_array[--count];
        case 9: ftfe_ptr->FCCOB8 = command_array[--count];
        case 8: ftfe_ptr->FCCOB7 = command_array[--count];
        case 7: ftfe_ptr->FCCOB6 = command_array[--count];
        case 6: ftfe_ptr->FCCOB5 = command_array[--count];
        case 5: ftfe_ptr->FCCOB4 = command_array[--count];
        case 4: ftfe_ptr->FCCOB3 = command_array[--count];
        case 3: ftfe_ptr->FCCOB2 = command_array[--count];
        case 2: ftfe_ptr->FCCOB1 = command_array[--count];
        case 1: ftfe_ptr->FCCOB0 = command_array[--count];
        default: break;
    }

#if PSP_MQX_CPU_IS_COLDFIRE
    temp = _psp_get_sr();
    _psp_set_sr(temp | 0x0700);
#elif PSP_MQX_CPU_IS_KINETIS
    __disable_interrupt ();
#endif //PSP_MQX_CPU_IS_KINETIS

    /* run command and wait for it to finish (must execute from RAM) */ 
    RunInRAM(&ftfe_ptr->FSTAT, (void (_CODE_PTR_)(volatile uint32_t))RunInvalidateInRAM);

    /* get flash status register value */
    fstat = ftfe_ptr->FSTAT;

/* 
invalidate data cache of 'affected_addr' address and 'affected_size' size
because reading flash through code-bus may show incorrect data
*/
#if defined(_DCACHE_INVALIDATE_MLINES) || defined(_ICACHE_INVALIDATE_MLINES)
    if (affected_size)
    {
#if defined(_DCACHE_INVALIDATE_MLINES)
        _DCACHE_INVALIDATE_MLINES(affected_addr, affected_size);
#endif
#if defined(_ICACHE_INVALIDATE_MLINES)
        _ICACHE_INVALIDATE_MLINES(affected_addr, affected_size);
#endif
    }
#endif

#if PSP_MQX_CPU_IS_COLDFIRE
    _psp_set_sr(temp);
#elif PSP_MQX_CPU_IS_KINETIS
    __enable_interrupt();    
#endif //PSP_MQX_CPU_IS_KINETIS

    /* checking access error */
    if (0 != (fstat & FTFE_FSTAT_ACCERR_MASK))
    {
        /* return an error code FTFE_ERR_ACCERR */
        result = FTFE_ERR_ACCERR;
    }
    /* checking protection error */
    else if (0 != (fstat & FTFE_FSTAT_FPVIOL_MASK))
    {
        /* return an error code FTFE_ERR_PVIOL */
        result = FTFE_ERR_PVIOL;
    }
    /* checking MGSTAT0 non-correctable error */
    else if (0 != (fstat & FTFE_FSTAT_MGSTAT0_MASK))
    {
        /* return an error code FTFE_ERR_MGSTAT0 */
        result = FTFE_ERR_MGSTAT0;
    }

    return result;
}

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : ftfe_flash_init
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*   Initialize flash specific information.
*
*END*----------------------------------------------------------------------*/
bool ftfe_flash_init 
(
    /* [IN] File pointer */
    IO_FLASHX_STRUCT_PTR dev_ptr
)
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR    dev_spec_ptr;
    FTFE_MemMapPtr                    ftfe_ptr;

    if (_bsp_ftfx_io_init(0)) {
        /* Cannot initialize FTF module */
        return FALSE;
    }

    /* allocate internal structure */
    dev_spec_ptr = _mem_alloc(sizeof(FTFE_FLASH_INTERNAL_STRUCT));
#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (dev_spec_ptr == NULL)
    {
        return FALSE;
    }
#endif
    dev_ptr->DEVICE_SPECIFIC_DATA = dev_spec_ptr;
        
    /* get the pointer to ftfe registers structure */
    ftfe_ptr = _bsp_get_ftfe_address();
    dev_spec_ptr->ftfe_ptr = (volatile char *)ftfe_ptr;
    
    /* save pointer to function in ram */
    dev_spec_ptr->flash_execute_code_ptr = ftfe_init_ram_function((char *)ftfe_ram_function, (char *)ftfe_ram_function_end);
#if PSP_MQX_CPU_IS_KINETIS
    dev_spec_ptr->flash_invalidate_code_ptr = ftfe_init_ram_function((char *)kinetis_flash_invalidate_cache, (char *)kinetis_flash_invalidate_cache_end);
#endif

#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (dev_spec_ptr->flash_execute_code_ptr == NULL)
    {
        return FALSE;
    }
#if PSP_MQX_CPU_IS_KINETIS
    if (dev_spec_ptr->flash_invalidate_code_ptr == NULL)
    {
        return FALSE;
    }
#endif
#endif

    /* 
    ** TODO: Currently, MQX FlashX driver for FTFE 
    ** doesn't support chips using FlexNVM so no need to configure FlexRAM 
    ** to traditional RAM when using Programing Acceleration RAM 
    */
        
    return TRUE;    
}

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : ftfe_flash_deinit
* Returned Value   : none
* Comments         :
*   Release flash specific information.  
*
*END*----------------------------------------------------------------------*/
void ftfe_flash_deinit 
( 
    /* [IN] File pointer */
    IO_FLASHX_STRUCT_PTR dev_ptr
)
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR dev_spec_ptr = (FTFE_FLASH_INTERNAL_STRUCT_PTR) dev_ptr->DEVICE_SPECIFIC_DATA; 
    
    /* de-allocate the ram function buffer */
    ftfe_deinit_ram_function(dev_spec_ptr->flash_execute_code_ptr);
#if PSP_MQX_CPU_IS_KINETIS
    ftfe_deinit_ram_function(dev_spec_ptr->flash_invalidate_code_ptr);
#endif

    /* de-allocate the device specific structure */
    _mem_free(dev_spec_ptr);

    dev_ptr->DEVICE_SPECIFIC_DATA = NULL;
}   

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : ftfe_init_ram_function
* Returned Value   : pointer to allocated RAM function
* Comments         :
*   Allocate and copy flash RAM function.
*
*END*----------------------------------------------------------------------*/
char *ftfe_init_ram_function
(
    char *function_start,
    char *function_end
)
{
    char *ram_code_ptr;
    _mem_size ftfe_ram_function_start;

    /* remove thumb2 flag from the address and align to word size */
    ftfe_ram_function_start = (_mem_size)function_start & ~3;

    /* allocate space to run flash command out of RAM */
    ram_code_ptr = _mem_alloc_align((char *)function_end - (char *)ftfe_ram_function_start, 4);

#if MQX_CHECK_MEMORY_ALLOCATION_ERRORS
    if (ram_code_ptr == NULL)
    {
        return NULL;
    }
#endif

    /* copy code to RAM buffer */
    _mem_copy ((char *)ftfe_ram_function_start, ram_code_ptr, (char *)function_end - (char *)ftfe_ram_function_start);
    
    /* adjust address with respect to the original alignment */
    ram_code_ptr = (char *)((_mem_size)ram_code_ptr | ((_mem_size)function_start & 3));

    return ram_code_ptr;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftfe_deinit_ram_function
* Returned Value   : MQX_OK or an error code 
* Comments         :
*   Free up flash RAM function, if any.
*
*END*-----------------------------------------------------------------------*/
_mqx_int ftfe_deinit_ram_function
(
    char *ram_code_ptr
)
{
    if (NULL != ram_code_ptr)
    {
        return _mem_free((void *)((_mem_size)ram_code_ptr & ~3));
    }

    return MQX_OK;
}

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : ftfe_flash_erase_sector
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*   Erase a flash memory block.
*
*END*----------------------------------------------------------------------*/
bool ftfe_flash_erase_sector
(
    /* [IN] File pointer */
    IO_FLASHX_STRUCT_PTR dev_ptr,

    /* [IN] Erased sector address */
    char             *from_ptr,

    /* [IN] Erased sector size */
    _mem_size            size
)
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR dev_spec_ptr = (FTFE_FLASH_INTERNAL_STRUCT_PTR) dev_ptr->DEVICE_SPECIFIC_DATA; 
    _mqx_uint                         i;
    _mem_size                         write_addr;
    uint8_t                            command_array[4];

    for (i = 0; dev_ptr->HW_BLOCK[i].NUM_SECTORS != 0; i++)
    {
        if (((uint32_t)from_ptr >= dev_ptr->HW_BLOCK[i].START_ADDR) &&
            ((uint32_t)from_ptr <= dev_ptr->HW_BLOCK[i].START_ADDR + dev_ptr->HW_BLOCK[i].SECTOR_SIZE * dev_ptr->HW_BLOCK[i].NUM_SECTORS)) {
             /* check if the from_ptr is sector aligned or not */
             if ((uint32_t)from_ptr % dev_ptr->HW_BLOCK[i].SECTOR_SIZE)
                return FALSE;
             /* check if the size is sector aligned or not */
             if ((uint32_t)size % dev_ptr->HW_BLOCK[i].SECTOR_SIZE)
                return FALSE;
             break;
        }
    }
 
    write_addr = (_mem_size) from_ptr;

    /* preparing passing parameter to erase a flash block */
    command_array[0] = FTFE_ERASE_SECTOR;
    command_array[1] = (uint8_t)(write_addr >> 16);
    command_array[2] = (uint8_t)((write_addr >> 8) & 0xFF);
    command_array[3] = (uint8_t)(write_addr & 0xFF);

    /* call flash command sequence function to execute the command */
    if (FTFE_OK != ftfe_flash_command_sequence (dev_spec_ptr, command_array, 4, (void*)write_addr, size))
    {
        return FALSE;
    }

    return TRUE;
}

/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : ftfe_flash_write_sector
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*   Performs a write into flash memory.
*
*END*----------------------------------------------------------------------*/
bool ftfe_flash_write_sector
(
    /* [IN] File pointer */
    IO_FLASHX_STRUCT_PTR dev_ptr,

    /* [IN] Source address */
    char             *from_ptr,

    /* [IN] Destination address */
    char             *to_ptr,

    /* [IN] Number of bytes to write */
    _mem_size            size
) 
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR dev_spec_ptr = (FTFE_FLASH_INTERNAL_STRUCT_PTR) dev_ptr->DEVICE_SPECIFIC_DATA; 
    _mqx_uint           i;
    _mem_size           write_addr = 0;
    uint8_t             command_array[4 + FTFE_PHRASE_SIZE];
    char                temp_data[FTFE_PHRASE_SIZE];
    unsigned char       byte_data_counter = 0;
    uint32_t            offset = ((uint32_t)to_ptr) & 0x00000007;

    dev_spec_ptr = dev_ptr->DEVICE_SPECIFIC_DATA;

    /* write to address mod 8 correction */
    if (offset)
    {
        /* align pointer to writable address */
        to_ptr -= offset;
        
        /* jump over old data */
        byte_data_counter = offset; 
    }

    write_addr = (_mem_size) to_ptr;

    /* heading space should be 0xFF */
    for (i = 0; i < offset; i++)
        temp_data[i] = 0xFF;

    while (size)
    {
        /* move data to temporary char array */       
        while ((byte_data_counter < FTFE_PHRASE_SIZE) && size)
        {
            temp_data[byte_data_counter++] = *from_ptr++;
            size--;
        }
        
        /* remaining space should be 0xFF */
        while (byte_data_counter < FTFE_PHRASE_SIZE) {
            temp_data[byte_data_counter++] = 0xFF;
        }

        /* prepare parameters to program the flash block */
        command_array[0] = FTFE_PROGRAM_PHRASE;
        command_array[1] = (uint8_t)(write_addr >> 16);
        command_array[2] = (uint8_t)((write_addr >> 8) & 0xFF);
        command_array[3] = (uint8_t)(write_addr & 0xFF);

        command_array[4] = temp_data[3];
        command_array[5] = temp_data[2];
        command_array[6] = temp_data[1];
        command_array[7] = temp_data[0];

        command_array[8] = temp_data[7];
        command_array[9] = temp_data[6];
        command_array[10] = temp_data[5];
        command_array[11] = temp_data[4];

        /* call flash command sequence function to execute the command */
        if (FTFE_OK != ftfe_flash_command_sequence(dev_spec_ptr, command_array, 4 + FTFE_PHRASE_SIZE, (void*)write_addr, FTFE_PHRASE_SIZE))
        {
            return FALSE;
        }
               
        /* update destination address for next iteration */
        write_addr += FTFE_PHRASE_SIZE;
        /* init variables for next loop */
        byte_data_counter = 0;
    }
    
    return TRUE;
}

#if BSPCFG_FLASHX_USE_PA_RAM
/*FUNCTION*-------------------------------------------------------------------
* 
* Function Name    : ftfe_flash_write_sector_pa_ram
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*   Performs a write into flash memory.
*
*END*----------------------------------------------------------------------*/
bool ftfe_flash_write_sector_pa_ram
(
    /* [IN] File pointer */
    IO_FLASHX_STRUCT_PTR dev_ptr,

    /* [IN] Source address */
    char             *from_ptr,

    /* [IN] Destination address */
    char             *to_ptr,

    /* [IN] Number of bytes to write */
    _mem_size            size
)
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR  dev_spec_ptr = (FTFE_FLASH_INTERNAL_STRUCT_PTR) dev_ptr->DEVICE_SPECIFIC_DATA; 
    FTFE_MemMapPtr                  ftfe_ptr;
    _mem_size                       write_addr = 0;
    uint8_t                         command_array[6];
    _mem_size                       number_of_phrases = 0;
    uint32_t                        i = 0;

    dev_spec_ptr = dev_ptr->DEVICE_SPECIFIC_DATA;

    /* get the pointer to ftfe registers structure */
    ftfe_ptr = (FTFE_MemMapPtr)dev_spec_ptr->ftfe_ptr;   
    
    write_addr = (_mem_size) to_ptr;

    /* check acceleration RAM */
    if (!(ftfe_ptr->FCNFG & FTFE_FCNFG_RAMRDY_MASK))
    {
        return FALSE;
    }
    
    /* check alignment of destination */
    if (write_addr & (FTFE_PROGRAM_SECTION_ALIGNMENT - 1))
    {
        return FALSE;
    }
    
    /* check input size */
    if (size % FTFE_PROGRAM_SECTION_ALIGNMENT == 0)
    {
        number_of_phrases = size / FTFE_PROGRAM_SECTION_ALIGNMENT;
    }
    else
    {
        number_of_phrases = (size / FTFE_PROGRAM_SECTION_ALIGNMENT) + 1;
    }
    
    /* copy data to acceleration RAM */
    _mem_copy(from_ptr, (char *)BSP_INTERNAL_PA_RAM_BASE, size);
        
    /* remaining space should be 0xFF */
    for (i = size; i < (number_of_phrases * FTFE_PROGRAM_SECTION_ALIGNMENT); i++)
    {
        *((char *)(BSP_INTERNAL_PA_RAM_BASE + i)) = 0xFF;
    }
    
    /* preparing passing parameter to program section */
    command_array[0] = FTFE_PROGRAM_SECTION;
    command_array[1] = (uint8_t)((write_addr >> 16) & 0xFF);
    command_array[2] = (uint8_t)((write_addr >> 8) & 0xFF);
    command_array[3] = (uint8_t)(write_addr & 0xFF);
    command_array[4] = (uint8_t)((number_of_phrases >> 8) & 0xFF);
    command_array[5] = (uint8_t)(number_of_phrases & 0xFF);
    
    /* call flash command sequence function to execute the command */
    if (FTFE_OK != ftfe_flash_command_sequence (dev_spec_ptr, command_array, 6, (void*)write_addr, FTFE_PROGRAM_SECTION_ALIGNMENT))
    {
        return FALSE;
    }
    
    return TRUE;
}
#endif

/********************************************************************
*
* Code required to run in SRAM to perform flash commands. 
* All else can be run in flash.
* Parameter is an address of flash status register and function to invalidate cache.
*
********************************************************************/ 
static void ftfe_ram_function
( 
    /* [IN] Flash info structure */
    volatile uint8_t *ftfe_fstat_ptr,
    /* [IN] Pointer to function of invalidate cache*/
    void (* invalidate_cache)(volatile uint32_t)
) 
{ 
    /* start flash write */
    *ftfe_fstat_ptr |= FTFE_FSTAT_CCIF_MASK;
    
    /* wait until execution complete */
    while (0 == ((*ftfe_fstat_ptr) & FTFE_FSTAT_CCIF_MASK))
        { };
    
    if(invalidate_cache != NULL)
    {
        invalidate_cache((uint32_t)FLASHX_INVALIDATE_CACHE_ALL);
    }
    /* Flush the pipeline and ensures that all previous instructions are completed
     * before executing new instructions in flash */
#ifdef ISB
    ISB();
#endif
#ifdef DSB
    DSB();
#endif
}

static void ftfe_ram_function_end(void)
{}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : flexnvm_flash_ioctl
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*   Program Partition Command
*
*END*----------------------------------------------------------------------*/
_mqx_int ffte_flash_ioctl
(
    /* [IN] Device instance */
    IO_FLASHX_STRUCT_PTR dev_ptr,

    /* [IN] the ioctl command */
    _mqx_uint cmd,

    /* [IN] the ioctl command parameter */
    void   *param_ptr
)
{
    _mqx_int result = IO_OK;

    switch (cmd) {
#if FTFE_SWAP_SUPPORT
        case FLASH_IOCTL_SWAP_FLASH_AND_RESET:
            result = ftfe_flash_swap_and_reset(dev_ptr);
            break;
#endif
        default:
            result = IO_ERROR_INVALID_IOCTL_CMD;
            break;
    }

    return result;
}

#if FTFE_SWAP_SUPPORT

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftfe_flash_swap_command
* Returned Value   : 0 if successful, error_code otherwise
* Comments         :
*   Implement swap command.
*
*END*----------------------------------------------------------------------*/
uint32_t ftfe_flash_swap_command
(
    /* [IN] Flash info structure */
    FTFE_FLASH_INTERNAL_STRUCT_PTR dev_spec_ptr,

    /* [IN] Flash swap indicator address */
    uint32_t addr,

    /* [IN] Flash swap command */
    uint8_t swapcmd
)
{
    uint8_t command_array[8]; /* Command sequence array */
    uint32_t result;

    /* Preparing passing parameter to execute the swap control code */
    command_array[0] = FTFE_PFLASH_SWAP;
    command_array[1] = (uint8_t)((addr & 0x00FF0000) >> 16);
    command_array[2] = (uint8_t)((addr & 0x0000FF00) >> 8);
    command_array[3] = (uint8_t)((addr & 0x000000FF));
    command_array[4] = swapcmd;
    command_array[5] = 0xFF;
    command_array[6] = 0xFF;
    command_array[7] = 0xFF;

    /* call flash command sequence function to execute the command */
    result = ftfe_flash_command_sequence(dev_spec_ptr, command_array, 8, NULL, 0);

    return result;
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftfe_flash_swap_status
* Returned Value   : 0 if successful, error_code otherwise
* Comments         :
*   Check Flash Swap Status and pass back.
*
*END*----------------------------------------------------------------------*/
uint32_t ftfe_flash_swap_status
(
   /* [IN] Device instance */
   IO_FLASHX_STRUCT_PTR   dev_ptr,

   /* [IN] Flash swap indicator address */
   uint32_t               flash_address,

   /* [OUT] swap status */
   uint8_t                * swapmode
)
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR     dev_spec_ptr;
    FTFE_MemMapPtr                     ftfe_ptr;
    uint32_t                           result;
    uint8_t                            current_swap_mode = 0xFF;

    dev_spec_ptr = (FTFE_FLASH_INTERNAL_STRUCT_PTR)(dev_ptr->DEVICE_SPECIFIC_DATA);
    ftfe_ptr = (FTFE_MemMapPtr)(dev_spec_ptr->ftfe_ptr);

    /* Set the default return code as FTFE_OK */
    result = FTFE_OK;

     /* Get current swap report */
    result = ftfe_flash_swap_command(dev_spec_ptr, flash_address, FTFE_SWAP_REPORT_STATUS);
    /* Check for the success of command execution */
    if (FTFE_OK != result)
    {
        return (result);
    }

    /* Get current swap mode from FCCOB5 register */
    current_swap_mode = ftfe_ptr->FCCOB5;

    *swapmode = current_swap_mode;
    return (result);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftfe_flash_swap
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*   Perform flash swap.
*
*END*----------------------------------------------------------------------*/
uint32_t ftfe_flash_swap
(
   /* [IN] Device instance */
   IO_FLASHX_STRUCT_PTR   dev_ptr
)
{
    FTFE_FLASH_INTERNAL_STRUCT_PTR     dev_spec_ptr;
    FTFE_MemMapPtr                     ftfe_ptr;
    uint32_t                           result;
    uint8_t                            current_swap_mode = 0xFF;

    dev_spec_ptr = (FTFE_FLASH_INTERNAL_STRUCT_PTR)(dev_ptr->DEVICE_SPECIFIC_DATA);
    ftfe_ptr = (FTFE_MemMapPtr)(dev_spec_ptr->ftfe_ptr);

    /* Set the default return code as FTFE_OK */
    result = FTFE_OK;

    /* Check if the destination is not double-phrase aligned */
    if (0 != ((uint32_t)BSPCFG_SWAP_INDICATOR_ADDR % FTFE_DPHRASE_SIZE))
    {
        /* Return error code FTFE_ERR_ADDR */
        return FTFE_ERR_ADDR;
    }

    /* Check whether BSPCFG_SWAP_INDICATOR_ADDR is in lower flash banks */
    if ((BSPCFG_SWAP_INDICATOR_ADDR < FTFE_SWAP_VALID_RANGE_START) || 
        (BSPCFG_SWAP_INDICATOR_ADDR >= FTFE_SWAP_VALID_RANGE_END)
    )
    {
        return FTFE_ERR_ADDR;
    }

    /* Get current swap report */
    result = ftfe_flash_swap_status(dev_ptr, BSPCFG_SWAP_INDICATOR_ADDR, &current_swap_mode);
    if (FTFE_OK != result)
    {
        return (result);
    }

    /* If current swap mode is Uninitialized */
    if (FTFE_SWAP_UNINIT == current_swap_mode)
    {
        /* Erase the last sector in the right and the second half of memory */
        if( !ftfe_flash_erase_sector (dev_ptr, (void *)(BSPCFG_SWAP_INDICATOR_ADDR) , 0))
        {
            return (FTFE_ERR_ACCERR);
        }
         
        if( !ftfe_flash_erase_sector (dev_ptr, (void *)(FTFE_SWAP_UPPER_INDICATOR_ADDR) , 0))
        {
            return (FTFE_ERR_ACCERR);
        }
           
        /* Initialize Swap to Initialized/READY state */
        result = ftfe_flash_swap_command(dev_spec_ptr, BSPCFG_SWAP_INDICATOR_ADDR, FTFE_SWAP_SET_INDICATOR_ADDR);

        /* Check for the success of command execution */
        if (FTFE_OK != result)
        {
            return (result);
        }

        /* Wait for state transition from Uninitialized to Ready or Update */
        while ( ((current_swap_mode = ftfe_ptr->FCCOB5) != FTFE_SWAP_READY) &&
                ((current_swap_mode = ftfe_ptr->FCCOB5) != FTFE_SWAP_UPDATE) &&
                ((current_swap_mode = ftfe_ptr->FCCOB5) != FTFE_SWAP_UPDATE_ERASED) )
        {
            /* Do nothing*/
        }
    }

    /* Get current swap report */
    result = ftfe_flash_swap_status(dev_ptr, BSPCFG_SWAP_INDICATOR_ADDR, &current_swap_mode);    
    if (FTFE_OK != result)
    {
        return (result);
    }

    /* If current swap mode is Initialized/Ready */
    if (FTFE_SWAP_READY == current_swap_mode)
    {
        /* Progress Swap to UPDATE state */
        result = ftfe_flash_swap_command(dev_spec_ptr, BSPCFG_SWAP_INDICATOR_ADDR, FTFE_SWAP_SET_IN_PREPARE);

        /* Check for the success of command execution */
        if (FTFE_OK != result)
        {
            return (result);
        }

        /* Wait for state transition from Initialized/Ready to Update */
        while ( ((current_swap_mode = ftfe_ptr->FCCOB5) != FTFE_SWAP_UPDATE) &&
                ((current_swap_mode = ftfe_ptr->FCCOB5) != FTFE_SWAP_UPDATE_ERASED) )
        {
            /* Do nothing*/
        }
    }

    /* If current swap mode is Update/Update-Erased */
    if (FTFE_SWAP_UPDATE == current_swap_mode)
    {
        result = ftfe_flash_erase_sector (dev_ptr, (void *)(FTFE_SWAP_UPPER_INDICATOR_ADDR) , 0);
        if (FALSE == (bool)result)
        {
           return (FTFE_ERR_ADDR);
        }
    }

     /* Get current swap report */
    result = ftfe_flash_swap_command(dev_spec_ptr, BSPCFG_SWAP_INDICATOR_ADDR, FTFE_SWAP_REPORT_STATUS);
    /* Check for the success of command execution */
    if (FTFE_OK != result)
    {
         return (result);
    }
    /* Progress Swap to COMPLETE State */
    result = ftfe_flash_swap_command(dev_spec_ptr, BSPCFG_SWAP_INDICATOR_ADDR, FTFE_SWAP_SET_IN_COMPLETE);

    /* Check for the success of command execution */
    if (FTFE_OK != result)
    {
        return (result);
    }

    /* Wait for state transition from Update-Erased to Complete */
    while ((current_swap_mode = ftfe_ptr->FCCOB5) != FTFE_SWAP_COMPLETE)
    {
        /* Do nothing*/
    }

    return (result);
}

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : ftfe_flash_swap_and_reset
* Returned Value   : FALSE if failed
* Comments         :
*   Call flash_swap and perform reset.
*   Never return if succeed.
*
*END*----------------------------------------------------------------------*/
uint32_t ftfe_flash_swap_and_reset
(
   /* [IN] Device instance */
   IO_FLASHX_STRUCT_PTR   dev_ptr
)
{
    uint32_t result = 0;
    result = ftfe_flash_swap(dev_ptr);

    /* Perform reset if swap success */
    if (result == FTFE_OK)
    {
        #if PSP_MQX_CPU_IS_ARM_CORTEX_M4
            // reset with Application Interrupt and Reset Control Register
            SCB_AIRCR = 0x05FA0004;
        #elif PSP_MQX_CPU_IS_COLDFIRE
            // reset by invalid access
            result = *(volatile uint32_t*)(0xFFFF0000);
        #else
            #error "Unsupported architecture"
        #endif
        result = ~FTFE_OK;
    }
    return result;
}

#endif


