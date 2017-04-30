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
*   This file contains the interface to the RTOS
*   task management functions.
*
*
*END************************************************************************/

#include <rtcsrtos.h>
#include <rtcs.h>


struct rtcs_task_state {
   _rtcs_sem        sem;
   void (_CODE_PTR_ start)(void *, void *);
   void            *arg;
   uint32_t          error;
};

/*TASK*-----------------------------------------------------------------
*
* Function Name   : RTCS_task
* Returned Values :
* Comments        :
*     The parent of all RTCS tasks.
*
*END*-----------------------------------------------------------------*/

static void RTCS_task
   (
      uint32_t task_parm
   )
{ /* Body */
   struct rtcs_task_state      *task = (struct rtcs_task_state *)task_parm;
   /* This is completely unnecessary -- it's done only for Task Aware Debugging */
   _task_get_template_ptr (MQX_NULL_TASK_ID)->TASK_ADDRESS = (void(_CODE_PTR_)(uint32_t))task->start;
 
   task->start(task->arg, task);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_task_create
* Returned Values : uint32_t (error code)
* Comments        :
*     Create a task and wait for it to complete initialization.
*
*END*-----------------------------------------------------------------*/

uint32_t RTCS_task_create
   (
      char          *name,
      uint32_t           priority,
      uint32_t           stacksize,
      void (_CODE_PTR_  start)(void *, void *),
      void             *arg
   )
{ /* Body */
   TASK_TEMPLATE_STRUCT    task_template;
   struct rtcs_task_state  task;
   
#if (RTCSCFG_ENABLE_ASSERT_PRINT==1) ||  (RTCSCFG_ENABLE_ASSERT==1)
  /* for TCP/IP task we bypass this check as for this one */
  /* priority = _RTCSTASK_priority */
  if(TRUE == _RTCS_initialized)
  {
    RTCS_ASSERT(priority>_RTCSTASK_priority);
  }
#endif
   
   
   RTCS_sem_init(&task.sem);
   task.start = start;
   task.arg   = arg;
   task.error = RTCS_OK;

   _mem_zero((unsigned char *)&task_template, sizeof(task_template));
   task_template.TASK_NAME          = name;
   task_template.TASK_PRIORITY      = priority;
   task_template.TASK_STACKSIZE     = stacksize;
   task_template.TASK_ADDRESS       = RTCS_task;
   task_template.CREATION_PARAMETER = (uint32_t)&task;
   if (_task_create(0, 0, (uint32_t)&task_template) == MQX_NULL_TASK_ID) {
      RTCS_sem_destroy(&task.sem);
      return RTCSERR_CREATE_FAILED;
   } /* Endif */

   RTCS_sem_wait(&task.sem);
   RTCS_sem_destroy(&task.sem);
   return task.error;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_task_resume_creator
* Returned Values : void
* Comments        :
*     Return an error code to the caller's creator.
*
*END*-----------------------------------------------------------------*/

void RTCS_task_resume_creator
   (
      void   *creator,
      uint32_t error
   )
{ /* Body */
   struct rtcs_task_state      *task = creator;

   if (task) {
      task->error = error;
      RTCS_sem_post(&task->sem);
   } /* Endif */

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_task_exit
* Returned Values : void
* Comments        :
*     Return an error code to the caller's creator, and block.
*
*END*-----------------------------------------------------------------*/

void RTCS_task_exit
   (
      void   *creator,
      uint32_t error
   )
{ /* Body */

   RTCS_task_resume_creator(creator, error);
   _task_set_error(error);
   _task_block();

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_task_id_from_td
* Returned Values : MQX _task_id
* Comments        :
*     Returns MQX task id from RTCS task id.
*
*END*-----------------------------------------------------------------*/

_task_id RTCS_task_id_from_td
   (
      _rtcs_taskid   taskid
   )
{ /* Body */

   return _task_get_id_from_td (taskid);

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_task_destroy
* Returned Values : void
* Comments        :
*     Destroy a task.
*
*END*-----------------------------------------------------------------*/

void RTCS_task_destroy
   (
      _rtcs_taskid   taskid
   )
{ /* Body */

   _task_destroy (RTCS_task_id_from_td (taskid));

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : RTCS_task_abort
* Returned Values : void
* Comments        :
*     Destroy a task.
*
*END*-----------------------------------------------------------------*/

void RTCS_task_abort
   (
      _rtcs_taskid   taskid
   )
{ /* Body */

   _task_abort (RTCS_task_id_from_td (taskid));

} /* Endbody */

/* EOF */
