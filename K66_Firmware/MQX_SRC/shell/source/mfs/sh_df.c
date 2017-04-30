/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   This file contains the source for an MFS shell function.
*
*
*END************************************************************************/

#include <string.h>
#include <mqx.h>
#include <fio.h>
#include <shell.h>
#include <sh_prv.h>

#if SHELLCFG_USES_MFS
#include <mfs.h>

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_df
* Returned Value   :  int32_t error code
* Comments  :  Prints out disk free information for current filesystem .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_df(int32_t argc, char *argv[])
{ /* Body */
    bool              print_usage;
    bool              shorthelp = FALSE;
    int32_t               return_code = SHELL_EXIT_SUCCESS;
    SHELL_CONTEXT_PTR    shell_ptr = Shell_get_context(argv);
    MQX_FILE_PTR         fs_ptr;
    int64_t               space;
    int32_t               clusters;
    uint32_t              cluster_size;
    int32_t               error = 0;
    char             *fs_name;

    print_usage = Shell_check_help_request(argc, argv, &shorthelp);

    if (!print_usage)  {
        if (argc > 2)  {
            printf("Error, invalid number of parameters\n\r");
            return_code = SHELL_EXIT_ERROR;
            print_usage=TRUE;
        }
    }

    if (print_usage)  {
        if (shorthelp)  {
            printf("%s [<filesystem>]\n\r", argv[0]);
        }
        else {
            printf("Usage: %s [filesystem]\n\r", argv[0]);
            printf("   <filesystem> = filesystem to query for free space\n\r");
        }
        return return_code;
    }


    if (argc == 2) {
        fs_name = argv[1];
        fs_ptr = _io_get_fs_by_name(fs_name);
    }
    else {
        fs_name = Shell_get_current_filesystem_name(shell_ptr);
        fs_ptr = Shell_get_current_filesystem(argv);
    }

    /* check if filesystem is mounted */
    if (fs_ptr == NULL)  {
        printf("Error, file system not mounted\n\r");
        return return_code = SHELL_EXIT_ERROR;
    }


    ioctl(fs_ptr, IO_IOCTL_FREE_SPACE, &space);
    ioctl(fs_ptr, IO_IOCTL_FREE_CLUSTERS, &clusters);
    error = ioctl(fs_ptr, IO_IOCTL_GET_CLUSTER_SIZE, &cluster_size);
    if (error) cluster_size=0;

    printf("Free disk space on %s\n\r", fs_name);
    printf("%ld clusters, %ld bytes each\n\r", (long int)clusters, (long int)cluster_size);
    printf("%lu KB\n\r", (unsigned long int)(space>>10));

    return return_code;
} /* Endbody */

#endif //SHELLCFG_USES_MFS
