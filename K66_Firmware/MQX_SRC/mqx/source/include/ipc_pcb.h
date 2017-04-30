
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
*   This file contains the definitions for the 
*   interprocessor drivers that work over IO PCB devices.
*
*
*END************************************************************************/
#ifndef __ipc_pcb_h__
#define __ipc_pcb_h__ 1

/*--------------------------------------------------------------------------*/
/*
 *                          CONSTANT DECLARATIONS
 */

/*
 * Initialization errors
 */
#define IPC_PCB_PACKET_POOL_CREATE_FAILED     (IPC_PCB_ERROR_BASE|0x31)
#define IPC_PCB_INVALID_QUEUE                 (IPC_PCB_ERROR_BASE|0x32)
#define IPC_PCB_DEVICE_OPEN_FAILED            (IPC_PCB_ERROR_BASE|0x33)
#define IPC_PCB_OUTPUT_PCB_POOL_CREATE_FAILED (IPC_PCB_ERROR_BASE|0x34)
#define IPC_PCB_INPUT_PCB_POOL_CREATE_FAILED  (IPC_PCB_ERROR_BASE|0x35)

/*--------------------------------------------------------------------------*/
/*
 *                    TYPEDEFS FOR _CODE_PTR_ FUNCTIONS
 */
typedef _mqx_uint (_CODE_PTR_ IPC_PCB_DEVINSTALL_FPTR)( char *, void *);
  

/*--------------------------------------------------------------------------*/
/*
 *                          DATATYPE DECLARATIONS
 */

/* IPC_PCB_INIT_STRUCT */
/*! 
 * \brief Initialization structure for IPCs over PCB devices.
 *
 * \see _ipc_pcb_init
 */
typedef struct ipc_pcb_init_struct
{
   /*! \brief The String name of the PCB device driver to be opened by the IPC. */
   char                 *IO_PCB_DEVICE_NAME;

   /*!
    * \brief The function to call to install the IO Device (optional). If null, 
    * the device is not installed.
    */
   IPC_PCB_DEVINSTALL_FPTR DEVICE_INSTALL;

   /*! \brief The parameter to pass to the IO Device installation function. */
   void                 *DEVICE_INSTALL_PARAMETER;
   
   /*! \brief Maximum size of all messages arriving at the IPC. */
   uint16_t               IN_MESSAGES_MAX_SIZE;
   /*! \brief Initial number of input messages to allocate. */
   uint16_t               IN_MESSAGES_TO_ALLOCATE;
   /*! 
    * \brief Number of input messages to add to the pool when messages are all 
    * in use. 
    */
   uint16_t               IN_MESSAGES_TO_GROW;
   /*! \brief Maximum number of messages in the input message pool. */
   uint16_t               IN_MESSAGES_MAX_ALLOCATE;

   /*! \brief Initial number of PCBs in the output PCB pool. */
   uint16_t               OUT_PCBS_INITIAL;
   /*! 
    * \brief Number of PCBs to add to the output PCB pool when all the PCBs are 
    * in use.
    */
   uint16_t               OUT_PCBS_TO_GROW;
   /*! \brief Maximum number of PCBs in the output PCB pool. */
   uint16_t               OUT_PCBS_MAX;

} IPC_PCB_INIT_STRUCT, * IPC_PCB_INIT_STRUCT_PTR;

/*--------------------------------------------------------------------------*/
/*
 *                          C PROTOTYPES
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern _mqx_uint _ipc_pcb_init(const IPC_PROTOCOL_INIT_STRUCT *, void *);
#endif

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
