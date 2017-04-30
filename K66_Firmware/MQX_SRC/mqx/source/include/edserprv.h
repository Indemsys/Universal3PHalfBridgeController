
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
*   Serial Embedded Debug Server private header file
*
*
*END************************************************************************/
#ifndef __edserprv_h__
#define __edserprv_h__

#include "fio.h"
#include "pcb.h"

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Define the following to send a space character after each byte */
/* #define EDS_USE_DEBUG_SPACES */

/* Define the following for extra debug information */
/* #define DO_DEBUG */
#ifdef DO_DEBUG
#define DEBUGM(x) x
#define EDS_USE_DEBUG_SPACES 
#else
#define DEBUGM(x)
#undef EDS_USE_DEBUG_SPACES 
#endif

/* Private defines */

/* The eds task message queue number for ipc operation */
#define EDS_IPC_MESSAGE_QUEUE_NUMBER  (2)

/* ED Serial Task info */
#define EDS_SERIAL_STACK_SIZE            1500L
#define EDS_SERIAL_TASK_NAME             "EDS_Serial_task"

/* server operations */
#define EDS_IDENTIFY               1
#define EDS_LITTLE_ENDIAN_IDENTIFY 0x80000000 /* PRIVATE */
#define EDS_READ                   2
#define EDS_WRITE                  3
#define EDS_CHANGE_PROCESSOR       4
#define EDS_CONFIG_MULTIPROC       5

#define EDS_BIG_ENDIAN      0
#define EDS_LITTLE_ENDIAN   0xFFFFFFFF

/* Start and end of frame characters */
#define STX                 0x02
#define ETX                 0x03

/* Receive states */
#define STATE_WAIT_STX      0x0001
#define STATE_WAIT_NON_STX  0x0002
#define STATE_GETTING_DATA  0x0003

/* error codes */
#define EDS_OK              0
#define EDS_INVALID_OP      1
#define EDS_INVALID_SIZE    2

#define EDS_OP_STRUCT_SIZE  (sizeof(EDS_OP_STRUCT))
#define EDS_BUFFER_SIZE     (128)

#define EDS_PROC_DEF_STRUCT_SIZE  (sizeof(EDS_PROC_DEF_STRUCT))

#define EDS_SERIAL_VALID    ((_mqx_uint)0x65647373)   /* "edss" */

/* Macros */
#if (PSP_MEMORY_ADDRESSING_CAPABILITY != 8)
   #if (PSP_MEMORY_ADDRESSING_CAPABILITY == 32)
      #define UNPACK(src,dest,size) \
         { \
            uint32_t i; \
            uint32_t      *src_lp;\
            unsigned char        *dest_lp;\
            src_lp = (src);\
            dest_lp = (dest);\
            for(i = 0; i < (size); i+=4) { \
               mqx_htonl(dest_lp,*src_lp);\
               src_lp++;\
               dest_lp+=4;\
            }\
         }
      #define PACK(src,dest,size) \
         { \
            uint32_t i; \
            unsigned char       *src_lp;\
            unsigned char       *dest_lp;\
            src_lp = (src);\
            dest_lp = (dest);\
            for(i = 0; i < (size); i+=4) { \
               *dest_lp = mqx_ntohl(src_lp);\
               dest_lp++;\
               src_lp +=4;\
            }\
         }
   #elif (PSP_MEMORY_ADDRESSING_CAPABILITY == 24)
      #define UNPACK(src,dest,size) \
         { \
            uint_24 i; \
            uint_24      *src_lp;\
            unsigned char        *dest_lp;\
            src_lp = (src);\
            dest_lp = (dest);\
            for(i = 0; i < (size); i+=3) { \
               mqx_htonls(dest_lp,*src_lp);\
               src_lp++;\
               dest_lp+=3;\
            }\
         }
      #define PACK(src,dest,size) \
         { \
            uint_24 i; \
            unsigned char       *src_lp;\
            unsigned char       *dest_lp;\
            src_lp = (src);\
            dest_lp = (dest);\
            for(i = 0; i < (size); i+=3) { \
               *dest_lp = mqx_ntohls(src_lp);\
               dest_lp++;\
               src_lp +=3;\
            }\
         }
   #elif (PSP_MEMORY_ADDRESSING_CAPABILITY == 16)
      #define UNPACK(src,dest,size) \
         { \
            uint16_t i; \
            uint16_t      *src_lp;\
            unsigned char        *dest_lp;\
            src_lp = (src);\
            dest_lp = (dest);\
            for(i = 0; i < (size); i+=2) { \
               mqx_htons(dest_lp,*src_lp);\
               src_lp++;\
               dest_lp+=2;\
            }\
         }
      #define PACK(src,dest,size) \
         { \
            uint16_t i; \
            unsigned char       *src_lp;\
            unsigned char       *dest_lp;\
            src_lp = (src);\
            dest_lp = (dest);\
            for(i = 0; i < (size); i+=2) { \
               *dest_lp = mqx_ntohs(src_lp);\
               dest_lp++;\
               src_lp +=2;\
            }\
         }
   #else
      #error "Memory type not understood"
   #endif

   #define READ_BYTES(src,dest,size)   UNPACK(src,dest,size)
   #define WRITE_BYTES(src,dest,size)  PACK(src,dest,size)

#else 
   #define READ_BYTES(dest,src,size)   _mem_copy(dest,src,size)
   #define WRITE_BYTES(dest,src,size)  _mem_copy(dest,src,size)
#endif

/*--------------------------------------------------------------------------*/
/*                      DATA STRUCTURE DEFINITIONS                          */

/* Structures */
/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief Operation structure
 */ 
typedef struct op_struct {
   /*! \brief Server operation. */
   unsigned char OPERATION[4];   
   /*! \brief Read/write memory address. */
   unsigned char ADDRESS[4];     
   /*! \brief Extra address field. */
   unsigned char ADDRESS2[4];    
   /*! \brief Size of buffer. */
   unsigned char SIZE[4];        
   /*! \brief Processor type. */
   unsigned char PROCESSOR[4];   
   /*! \brief Endian of processor. */
   unsigned char ENDIAN[4];      
   /*! \brief Error code. */
   unsigned char EDS_ERROR[4];   
} EDS_OP_STRUCT, * EDS_OP_STRUCT_PTR;
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief EDS data structure.
 */ 
typedef struct eds_data_struct {
   /*! \brief EDS operation data structure. */
   EDS_OP_STRUCT  EDS_OP_DATA;
   /*! \brief Buffer. Size is specified by EDS_BUFFER_SIZE. */
   unsigned char          BUFFER[EDS_BUFFER_SIZE];
} EDS_DATA_STRUCT, * EDS_DATA_STRUCT_PTR;
/*! \endcond */

/*! 
 * \cond DOXYGEN_PRIVATE
 *  
 * \brief Structure used to define a processor. 
 */
typedef struct eds_proc_def_struct {
   /*! \brief Endian of processor. */
   unsigned char        ENDIAN[4];
   unsigned char        KD_START[4];
   unsigned char        KD_END[4];
   /*! \brief Processor number. */
   unsigned char        PROC_NUM[4];
   /*! \brief CPU type. */
   unsigned char        CPU_TYPE[4];
} EDS_PROC_DEF_STRUCT, * EDS_PROC_DEF_STRUCT_PTR;
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */ 
typedef struct eds_proc_reg_struct {
   EDS_PROC_DEF_STRUCT   PROC_DATA;
   QUEUE_ELEMENT_STRUCT  LIST;
} EDS_PROC_REG_STRUCT, * EDS_PROC_REG_STRUCT_PTR;
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief EDS component structure.
 */ 
typedef struct eds_component_struct
{
   /*! \brief Lightweight semaphore. */
   LWSEM_STRUCT SEM;
   /*! \brief state flag. */
   _mqx_uint     STATE;
   /*! \brief Valid flag. */
   _mqx_uint     VALID;
} EDS_COMPONENT_STRUCT, * EDS_COMPONENT_STRUCT_PTR;
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * 
 * \brief Configuration structure for IO channel.
 */ 
typedef struct io_channel_struct
{
    /*! \brief Pointer to channel to use. */
    char    *CHANNEL;
    /*! \brief Baud rate. */
    uint32_t     BAUD;
} IO_CHAN_STRUCT, * IO_CHAN_STRUCT_PTR;
/*! \endcond */
/* function prototypes */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__
extern void    _eds_serial_master_task(uint32_t);
extern void    _eds_serial_slave_task(uint32_t);

extern bool _eds_serial_process_master(EDS_COMPONENT_STRUCT_PTR);
extern bool _eds_serial_process_slave(EDS_COMPONENT_STRUCT_PTR);

extern void    _eds_serial_task(uint32_t);
extern bool _eds_serial_process(MQX_FILE_PTR, char *);
extern void    _eds_serial_check_errors(EDS_OP_STRUCT_PTR);
extern void    _eds_serial_identify(EDS_OP_STRUCT_PTR);
extern int32_t  _eds_serial_send(MQX_FILE_PTR, char *, int32_t);
extern int32_t  _eds_serial_recv(MQX_FILE_PTR, char *, int32_t);
#endif
 
#ifdef __cplusplus
}
#endif

#endif

/* EOF */
