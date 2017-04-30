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
*   This file contains the SD card driver functions.
*
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include <io_prv.h>
#include <sdcard.h>
#include <sdcard_prv.h>
#include <sdcard_esdhc.h>
#include <esdhc.h>


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_sdcard_esdhc_r2_format
* Returned Value   :
* Comments         :
*    Reformats R2 (CID,CSD) as read from ESDHC registers to a byte array.
*
*END*----------------------------------------------------------------------*/

static void _io_sdcard_esdhc_r2_format(uint32_t r[4], uint8_t r2[16])
{
    int i;
    uint32_t tmp = 0; /* initialization required to avoid compilation warning */

    i=15;
    while (i)
    {
        if ((i%4) == 3)
        {
            tmp = r[3-i/4];
        }
        r2[--i] = tmp & 0xff;
        tmp >>= 8;
    }
    r2[15] = 0;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _set_sd_high_speed_mode
* Returned Value   :
* Comments         :
*    Configures SDCARD for high speed mode
*
*END*----------------------------------------------------------------------*/
 static int32_t _set_sd_high_speed_mode(SDCARD_STRUCT_PTR sdcard_ptr, uint32_t baudrate)
{
  ESDHC_COMMAND_STRUCT command;
  uint32_t              param;
  uint8_t cmd6_data[64];

#define SFS_FUNC_GROUP1_BIT 400
#define SFS_FUNC_GROUP2_BIT 416
#define SFS_FUNC_GROUP3_BIT 432
#define SFS_FUNC_GROUP4_BIT 448
#define SFS_FUNC_GROUP5_BIT 464
#define SFS_FUNC_GROUP6_BIT 480


#define SFS_FUNC_GROUP1_FUNC 376

#define SFS_GET_BYTE_CNT(bit) (63 - (bit) / 8)

  //  set BLKCNT field to 1 (block), set BLKSIZE field to 64 (bytes);
  //  send CMD6, with argument 0xFFFFF1 and read 64 bytes of data accompanying the R1 response;
  //  wait data transfer done bit is set;

  /* Check the card capability of bus speed*/
  command.COMMAND = ESDHC_CREATE_CMD(6, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG);
  command.ARGUMENT = 0xFFFFF1;
  command.BLOCKS = 1;
  command.BLOCKSIZE = 64;

  if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
  {
      return IO_ERROR;
  }

  if (64 != fread (cmd6_data, 1, 64, sdcard_ptr->COM_DEVICE))
  {
      return IO_ERROR;

  }

  if (ESDHC_OK != fflush (sdcard_ptr->COM_DEVICE))
  {
      return IO_ERROR;
  }

  // Check the response
  // in the function group 1, check the first function availability
  if((cmd6_data[SFS_GET_BYTE_CNT(SFS_FUNC_GROUP1_BIT)] & 0x01) == 0)
    return MQX_IO_OPERATION_NOT_AVAILABLE; //  if (bit 401 is '0') report the SD card does not support high speed mode and return;

  /* Set the high speed of bus */
  command.COMMAND = ESDHC_CREATE_CMD(6, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG);
  command.ARGUMENT = 0x80FFFFF1;
  command.BLOCKS = 1;
  command.BLOCKSIZE = 64;

  if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
  {
      return IO_ERROR;
  }

  if (64 != fread (cmd6_data, 1, 64, sdcard_ptr->COM_DEVICE))
  {
      return IO_ERROR;

  }

  if (ESDHC_OK != fflush (sdcard_ptr->COM_DEVICE))
  {
      return IO_ERROR;
  }

  // Check the response
  // in the function group 1, check the first function availability
  if((cmd6_data[SFS_GET_BYTE_CNT(SFS_FUNC_GROUP1_FUNC)] & 0x0F) != 0x01)
    return MQX_IO_OPERATION_NOT_AVAILABLE; //  if (bit 401 is '0') report the SD card does not support high speed mode and return;


  param = baudrate;
  if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SET_BAUDRATE, &param))
  {
      return IO_ERROR;
  }

  return 0;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_sdcard_esdhc_init
* Returned Value   : TRUE if successful, FALSE otherwise
* Comments         :
*    Initializes ESDHC communication, SD card itself and reads its parameters.
*
*END*----------------------------------------------------------------------*/

bool _io_sdcard_esdhc_init
(
    /* [IN/OUT] SD card file descriptor */
    MQX_FILE_PTR fd_ptr
)
{
    uint32_t                baudrate, param;
    ESDHC_COMMAND_STRUCT    command;
    IO_DEVICE_STRUCT_PTR    io_dev_ptr = fd_ptr->DEV_PTR;
    SDCARD_STRUCT_PTR       sdcard_ptr = (SDCARD_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    uint8_t                 csd[16];

    /* Check parameters */
    if ((NULL == sdcard_ptr) || (NULL == sdcard_ptr->COM_DEVICE) || (NULL == sdcard_ptr->INIT))
    {
        return FALSE;
    }

    sdcard_ptr->TIMEOUT = 0;
    sdcard_ptr->NUM_BLOCKS = 0;
    sdcard_ptr->ADDRESS = 0;
    sdcard_ptr->SDHC = FALSE;
    sdcard_ptr->VERSION2 = FALSE;
    sdcard_ptr->ALIGNMENT = 1;
    _mem_zero(sdcard_ptr->CID, sizeof(sdcard_ptr->CID));

    /* Get buffer alignment requirements from the communication driver and check it */
    ioctl(sdcard_ptr->COM_DEVICE, IO_IOCTL_GET_REQ_ALIGNMENT, &sdcard_ptr->ALIGNMENT);
    if ((sdcard_ptr->ALIGNMENT & (sdcard_ptr->ALIGNMENT-1)) || (sdcard_ptr->ALIGNMENT > 4))
    {
        /* Alignment is either not power of 2 or greater than 4 */
        return FALSE;
    }

    /* Initialize and detect card */
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_INIT, NULL))
    {
        return FALSE;
    }

    param = ESDHC_INIT_BAUDRATE;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SET_BAUDRATE, &param))
    {
        return FALSE;
    }

    /* GET CARD TYPE */
    param = 0;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_GET_CARD, &param))
    {
        return FALSE;
    }

    /* SDHC check */
    if ((ESDHC_CARD_SD == param) || (ESDHC_CARD_SDHC == param) || (ESDHC_CARD_SDCOMBO == param) || (ESDHC_CARD_SDHCCOMBO == param))
    {
        if ((ESDHC_CARD_SDHC == param) || (ESDHC_CARD_SDHCCOMBO == param))
        {
            sdcard_ptr->SDHC = TRUE;
        }
    }
    else
    {
        return FALSE;
    }

    /* Card identify */
    command.COMMAND = ESDHC_CREATE_CMD(2, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R2, ESDHC_COMMAND_NONE_FLAG);
    command.ARGUMENT = 0;
    command.BLOCKS = 0;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return FALSE;
    }

    _io_sdcard_esdhc_r2_format(command.RESPONSE, sdcard_ptr->CID);

    /* Get card address */
    command.COMMAND = ESDHC_CREATE_CMD(3, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_NONE_FLAG);
    command.ARGUMENT = 0;
    command.BLOCKS = 0;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return FALSE;
    }
    sdcard_ptr->ADDRESS = command.RESPONSE[0] & 0xFFFF0000;

    /* Get card parameters */
    command.COMMAND = ESDHC_CREATE_CMD(9, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R2, ESDHC_COMMAND_NONE_FLAG);
    command.ARGUMENT = sdcard_ptr->ADDRESS;
    command.BLOCKS = 0;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return FALSE;
    }

    _io_sdcard_esdhc_r2_format(command.RESPONSE, csd);
    sdcard_ptr->NUM_BLOCKS = _io_sdcard_csd_capacity(csd);
    param = _io_sdcard_csd_baudrate(csd);

    /* Get maximal baudrate for card setup */
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_GET_BAUDRATE_MAX, &baudrate))
    {
        return FALSE;
    }

    //baudrate = 10000000;
    if (param > baudrate)
    {
        param = baudrate;
    }
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SET_BAUDRATE, &param))
    {
        return FALSE;
    }

    /* Select card */
    command.COMMAND = ESDHC_CREATE_CMD(7, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1b, ESDHC_COMMAND_NONE_FLAG);
    command.ARGUMENT = sdcard_ptr->ADDRESS;
    command.BLOCKS = 0;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return FALSE;
    }

    /* Set block size to 512 */
    command.COMMAND = ESDHC_CREATE_CMD(16, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_NONE_FLAG);
    command.ARGUMENT = IO_SDCARD_BLOCK_SIZE;
    command.BLOCKS = 0;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return FALSE;
    }

    if (ESDHC_BUS_WIDTH_4BIT == sdcard_ptr->INIT->SIGNALS)
    {
        /* Application specific command */
        command.COMMAND = ESDHC_CREATE_CMD(55, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_NONE_FLAG);
        command.ARGUMENT = sdcard_ptr->ADDRESS;
        command.BLOCKS = 0;
        if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            return FALSE;
        }

        /* Set bus width == 4 */
        command.COMMAND = ESDHC_CREATE_CMD(6, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, 0/*ESDHC_COMMAND_ACMD_FLAG*/);
        command.ARGUMENT = 2;
        command.BLOCKS = 0;
        if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            return FALSE;
        }

        param = ESDHC_BUS_WIDTH_4BIT;
        if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SET_BUS_WIDTH, &param))
        {
            return FALSE;
        }
    }

    if (baudrate > ESDHC_DEFAULT_BAUDRATE)
    {
      //Try to set high speed mode
      _set_sd_high_speed_mode(sdcard_ptr, baudrate);
    }

    return TRUE;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_sdcard_esdhc_read_block
* Returned Value   : number of ssuccessfully processed blocks or IO_ERROR
* Comments         :
*    Reads sectors from SD card starting with given index into given buffer.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_sdcard_esdhc_read_block
(
    /* [IN] SD card info */
    MQX_FILE_PTR fd_ptr,

    /* [OUT] Buffer to fill with read 512 bytes */
    unsigned char *buffer,

    /* [IN] Index of first sector to read */
    uint32_t   index,

    /* [IN] Number of sectors to read read */
    uint32_t   num
)
{
    ESDHC_COMMAND_STRUCT command;
    IO_DEVICE_STRUCT_PTR io_dev_ptr = fd_ptr->DEV_PTR;
    SDCARD_STRUCT_PTR    sdcard_ptr = (SDCARD_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    _mqx_int             result;

    /* Check parameters */
    if ((NULL == sdcard_ptr) || (NULL == sdcard_ptr->COM_DEVICE) || (NULL == sdcard_ptr->INIT) || (NULL == buffer))
    {
        return IO_ERROR;
    }

    /* SD card data address adjustment */
    if (! sdcard_ptr->SDHC)
    {
        index <<= IO_SDCARD_BLOCK_SIZE_POWER;
    }



        /* Read block(s) command */
      if (num > 1)
      {
          command.COMMAND = ESDHC_CREATE_CMD(18, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG));
      }
      else
      {
          command.COMMAND = ESDHC_CREATE_CMD(17, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG));
      }

      command.ARGUMENT = index;
      command.BLOCKS = num;
      command.BLOCKSIZE = IO_SDCARD_BLOCK_SIZE;

      ;

      if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
      {
          return IO_ERROR;
      }




    /* Read data */
    result = fread (buffer, IO_SDCARD_BLOCK_SIZE, num, sdcard_ptr->COM_DEVICE);

    if(result == IO_ERROR_TIMEOUT)
    {
      // restore the card communication
      _io_sdcard_esdhc_init(fd_ptr);
    }


    /* Wait for transfer complete */
//    if (ESDHC_OK != fflush (sdcard_ptr->COM_DEVICE))
//    {
//        return IO_ERROR;
//    }

    return result;
}


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    : _io_sdcard_esdhc_write_block
* Returned Value   : number of ssuccessfully processed blocks or IO_ERROR
* Comments         :
*    Writes sectors starting with given index to SD card from given buffer.
*
*END*----------------------------------------------------------------------*/

_mqx_int _io_sdcard_esdhc_write_block
(
    /* [IN] SD card file descriptor */
    MQX_FILE_PTR fd_ptr,

    /* [IN] Buffer with data to write */
    unsigned char *buffer,

    /* [IN] Index of first sector to write */
    uint32_t   index,

    /* [IN] Number of sectors to be written */
    uint32_t   num
)
{
    ESDHC_COMMAND_STRUCT command;
    IO_DEVICE_STRUCT_PTR io_dev_ptr = fd_ptr->DEV_PTR;
    SDCARD_STRUCT_PTR    sdcard_ptr = (SDCARD_STRUCT_PTR)io_dev_ptr->DRIVER_INIT_PTR;
    uint8_t               tmp[4];
    _mqx_int             count;

    /* Check parameters */
if ((NULL == sdcard_ptr) || (NULL == sdcard_ptr->COM_DEVICE) || (NULL == sdcard_ptr->INIT) || (NULL == buffer))
    {
        return IO_ERROR;
    }

    /* SD card data address adjustment */
    if (! sdcard_ptr->SDHC)
    {
        index <<= IO_SDCARD_BLOCK_SIZE_POWER;
    }

    /* Write block(s) command */
    if (num > 1)
    {
        command.COMMAND = ESDHC_CREATE_CMD(25, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_DATACMD_FLAG));
    }
    else
    {
        command.COMMAND = ESDHC_CREATE_CMD(24, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_DATACMD_FLAG));
    }

    command.ARGUMENT = index;
    command.BLOCKS = num;
    command.BLOCKSIZE = IO_SDCARD_BLOCK_SIZE;
    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return IO_ERROR;
    }

    count = fwrite (buffer, IO_SDCARD_BLOCK_SIZE, num, sdcard_ptr->COM_DEVICE);

    if(count == IO_ERROR_TIMEOUT)
    {
      // restore the card communication
      _io_sdcard_esdhc_init(fd_ptr);

      return IO_ERROR;
    }

    /* Wait for transfer complete */
    if (ESDHC_OK != fflush (sdcard_ptr->COM_DEVICE))
    {
        return IO_ERROR;
    }


    command.COMMAND = ESDHC_CREATE_CMD(13, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_NONE_FLAG));
    command.ARGUMENT = sdcard_ptr->ADDRESS;
    command.BLOCKS = 0;

    if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
    {
        return IO_ERROR;
    }


    /* Card status error check */
    if (command.RESPONSE[0] & 0xFFD98008)
    {
      count = 0; /* necessary to get real number of written blocks */
    }
    else if(0x000000900 != (command.RESPONSE[0] & 0x00001F00))
    {
      // the Card is not in the Transfer state and data ready
      return IO_ERROR;
    }

    if (count != num)
    {
        /* Application specific command */
        command.COMMAND = ESDHC_CREATE_CMD(55, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, ESDHC_COMMAND_NONE_FLAG);
        command.ARGUMENT = sdcard_ptr->ADDRESS;
        command.BLOCKS = 0;
        if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            return IO_ERROR;
        }

        /* use ACMD22 to get number of written sectors */
        command.COMMAND = ESDHC_CREATE_CMD(22, ESDHC_COMMAND_TYPE_NORMAL, ESDHC_COMMAND_RESPONSE_R1, (ESDHC_COMMAND_DATACMD_FLAG | ESDHC_COMMAND_DATA_READ_FLAG));
        command.ARGUMENT = 0;
        command.BLOCKS = 1;
        command.BLOCKSIZE = 4;
        if (ESDHC_OK != ioctl (sdcard_ptr->COM_DEVICE, IO_IOCTL_ESDHC_SEND_COMMAND, &command))
        {
            return IO_ERROR;
        }

        if (4 != fread (tmp, 1, 4, sdcard_ptr->COM_DEVICE))
        {
            return IO_ERROR;

        }

        if (ESDHC_OK != fflush (sdcard_ptr->COM_DEVICE))
        {
            return IO_ERROR;
        }

        //    SDHC_SYSCTL |= SDHC_SYSCTL_RSTD_MASK | SDHC_SYSCTL_RSTC_MASK;

        count = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];
        if ((count < 0) || (count > num))
            return IO_ERROR;
    }

    return count;
}

/* EOF */
