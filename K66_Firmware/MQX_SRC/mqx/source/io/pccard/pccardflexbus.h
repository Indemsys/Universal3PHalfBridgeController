#ifndef _pccardflexbus_h_
#define _pccardflexbus_h_
/*HEADER**********************************************************************
*
* Copyright 2011 Freescale Semiconductor, Inc.
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
*   The file contains the structure definitions
*   public to the PC Card FlexBus driver
*
*
*END************************************************************************/


/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

/* provide generic bus width macro to upper layer(s) */
#ifdef BSP_PCCARDFLEXBUS_WIDTH
#define APCCARD_BUS_WIDTH BSP_PCCARDFLEXBUS_WIDTH
#endif

/* provide generic bus address shift macro to upper layer(s) */
#ifdef BSP_PCCARDFLEXBUS_ADDR_SHIFT
#define APCCARD_ADDR_SHIFT BSP_PCCARDFLEXBUS_ADDR_SHIFT
#endif


/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/


/* Initialization parameters for the CompactFlash Card driver */
typedef struct pccardflexbus_init_struct
{
   /* Base address for Compact Flash address space */
   void               *PCMCIA_BASE;

} PCCARDFLEXBUS_INIT_STRUCT, * PCCARDFLEXBUS_INIT_STRUCT_PTR;
typedef const PCCARDFLEXBUS_INIT_STRUCT * PCCARDFLEXBUS_INIT_STRUCT_CPTR;


/*-----------------------------------------------------------------------
**                      FUNCTION PROTOTYPES
*/
#ifdef __cplusplus
extern "C" {
#endif

/* Interface functions */
uint32_t _io_pccardflexbus_install(char *, PCCARDFLEXBUS_INIT_STRUCT_CPTR);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
