#ifndef _pipe_prv_h_
#define _pipe_prv_h_
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
*   The file contains the private internal functions prototype, defines, 
*   structure   definitions private to the Pipe device
*
*
*END************************************************************************/

/*----------------------------------------------------------------------*/
/*
**                          STRUCTURE DEFINITIONS
*/


/*
** IO_PIPE_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a Pipe is initialized.
*/

typedef struct io_pipe_init_struct
{
  
   /* The pipe character queue size */
   uint32_t QUEUE_SIZE;

   /* Initialization parameters for the pipe, currently not used*/
   uint32_t FLAGS;

 
} IO_PIPE_INIT_STRUCT, * IO_PIPE_INIT_STRUCT_PTR;


/*
** IO_PIPE_INFO_STRUCT
**
** This structure defines the current parameters and status for a Pipe  
**
*/

typedef struct io_pipe_info_struct
{

   CHARQ_STRUCT_PTR     QUEUE;
  
   /* The serial pipe queue size */
   uint32_t    QUEUE_SIZE;

   /* Mutext to protect against simulateous reads from the Pipe */ 
   MUTEX_STRUCT READ_MUTEX;

   /* Mutext to protect against simulateous writes to the Pipe */ 
   MUTEX_STRUCT WRITE_MUTEX;

   /* Mutext to protect against access to the Pipe data structures */ 
   MUTEX_STRUCT ACCESS_MUTEX;

   /* Semaphore used to block when pipe is full  */
   LWSEM_STRUCT FULL_SEM; 

   /* Semaphore used to block when pipe is empty */
   LWSEM_STRUCT EMPTY_SEM; 

   /* Flags to define options for the Pipe, currently not used */
   uint32_t FLAGS;

   /* Abort out of pending pipe reads, cleared by next pipe read */
   bool KM_ABORT_READ;
   
   /* Abort out of pending pipe write, cleared by next pipe write */
   bool KM_ABORT_WRITE;
 
} IO_PIPE_INFO_STRUCT, * IO_PIPE_INFO_STRUCT_PTR;


/*----------------------------------------------------------------------*/
/*
**                          EXTERNAL FUCTION DEFINITIONS
*/

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _io_pipe_open(FILE_DEVICE_STRUCT_PTR, char *, char *);
extern _mqx_int _io_pipe_close(FILE_DEVICE_STRUCT_PTR);
extern _mqx_int _io_pipe_read(FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int _io_pipe_write(FILE_DEVICE_STRUCT_PTR, char *, _mqx_int);
extern _mqx_int _io_pipe_ioctl(FILE_DEVICE_STRUCT_PTR, uint32_t, void *);
extern _mqx_int _io_pipe_uninstall(IO_DEVICE_STRUCT_PTR);

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
