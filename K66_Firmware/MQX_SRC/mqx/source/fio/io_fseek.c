
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the function for setting the current location
*   in a file.
*
*
*END************************************************************************/
#include "mqx_cnfg.h"
#if MQX_USE_IO_OLD

#include "mqx.h"
#include "fio.h"
#include "fio_prv.h"
#include "io.h"
#include "io_prv.h"


/*!
 * \brief This function sets the current file position.
 *
 * \param[in] file_ptr The stream to use.
 * \param[in] offset   The offset for the seek.
 * \param[in] mode     Mode to determine where to start the seek from.
 *
 * \return MQX_OK
 * \return IO_ERROR (Failure.)
 */
_mqx_int _io_fseek
    (
        MQX_FILE_PTR file_ptr,
        _file_offset offset,
        _mqx_uint    mode
    )
{
    IO_DEVICE_STRUCT_PTR dev_ptr;
    _mqx_int             result = MQX_OK;
    _file_offset         location;

#if MQX_CHECK_ERRORS
    if (file_ptr == NULL) {
        return (IO_ERROR);
    }
#endif

    /* Obtain pointer to device structure */
    dev_ptr = file_ptr->DEV_PTR;

    /* Adjust possibly fake location before relative seek */
    if ((mode == IO_SEEK_CUR) && file_ptr->HAVE_UNGOT_CHARACTER && (file_ptr->LOCATION > 0)) {
        offset--;
    }

    /* Empty ungetc buffer */
    file_ptr->HAVE_UNGOT_CHARACTER = FALSE;

    /* Clear EOF flag */
    file_ptr->FLAGS &= ~((_mqx_uint) IO_FLAG_AT_EOF);

    /* Possibly call lseek to handle the seek operation */
    if (dev_ptr->IO_LSEEK != NULL) {
        location = (*dev_ptr->IO_LSEEK)(file_ptr, offset, mode);
    }
    else {
        /* Otherwise use common implementation of seek */
        switch (mode) {
            case IO_SEEK_SET:
                if (0 > offset)
                {
                    location = -1;
                }
                else
                {
                    location = offset;
                }
                break;
            case IO_SEEK_CUR:
                if ((offset >= 0 && ((UINT64_MAX - file_ptr->LOCATION) < offset)) ||
                    (offset < 0 && (file_ptr->LOCATION < -offset)))
                {
                    location = -1;
                }
                else
                {
                    location = file_ptr->LOCATION + offset;
                }
                break;
            case IO_SEEK_END:
                if ((offset >= 0 && ((UINT64_MAX - file_ptr->SIZE) < offset)) ||
                    (offset < 0 && (file_ptr->SIZE < -offset)))
                {
                    location = -1;
                }
                else
                {
                    location = file_ptr->SIZE + offset;
                }
                break;
            default:
                location = -1; /* Set location to negative value to induce IO_ERROR */
        }
    }

    /* Check for overflow/underflow */
    if (location < 0) {
        result = IO_ERROR;
    }
    else {
        file_ptr->LOCATION = location;
        /* In case lseek is not implemented, notify the device by IOCTL */
        if ((dev_ptr->IO_LSEEK == NULL) && (dev_ptr->IO_IOCTL != NULL)) {
            result = (*dev_ptr->IO_IOCTL)(file_ptr, IO_IOCTL_SEEK, NULL);
            if (result == IO_ERROR_INVALID_IOCTL_CMD) {
                result = MQX_OK; /* Seek on non-seekable device never reports an error */
            }
        }
    }

    return result;
}

#endif // MQX_USE_IO_OLD
