#ifndef _pccardflexbuspr_h_
#define _pccardflexbuspr_h_
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
*   private to the PC Card FlexbBus
*
*
*END************************************************************************/

/* default options */
#ifndef BSP_PCCARDFLEXBUS_WIDTH
#define BSP_PCCARDFLEXBUS_WIDTH              1
#endif

#ifndef BSP_PCCARDFLEXBUS_ADDR_SHIFT
#define BSP_PCCARDFLEXBUS_ADDR_SHIFT         0
#endif

#ifndef BSP_PCCARDFLEXBUS_CARD_PRESENT
#define BSP_PCCARDFLEXBUS_CARD_PRESENT       229
#endif

#ifndef BSP_PCCARDFLEXBUS_SLOT_EMPTY
#define BSP_PCCARDFLEXBUS_SLOT_EMPTY         173
#endif

/*----------------------------------------------------------------------*/
/*
**                          CONSTANT DEFINITIONS
*/

#define PCCARDFLEXBUS_REGISTER_MASK          (0x00000000<<BSP_PCCARDFLEXBUS_ADDR_SHIFT)
#define PCCARDFLEXBUS_COMMON_MEM_MASK        (0x00001000<<BSP_PCCARDFLEXBUS_ADDR_SHIFT)
#define PCCARDFLEXBUS_CARD_PRESENT_MASK      (0x00002000<<BSP_PCCARDFLEXBUS_ADDR_SHIFT)



/*
** Timing parameters
*/

/* Minimum time to wait from VCC application to when the card is ready (ms) */
#define PCCARDFLEXBUS_RESET_ALLOWED_TIME  (300 + BSP_ALARM_RESOLUTION)

/* 
** State conditions 
*/
#define PCCARDFLEXBUS_CARD_REMOVED    0
#define PCCARDFLEXBUS_CARD_INSERTED   1
#define PCCARDFLEXBUS_ACCESS_ALLOWED  5


/*----------------------------------------------------------------------*/
/*
**                    DATATYPE DEFINITIONS
*/

/*
** Run time state information for a PC Card slot
*/
typedef struct io_pccardflexbus_struct
{
   PCCARDFLEXBUS_INIT_STRUCT INIT_DATA;
   bool                CARD_IN;
   uint32_t                STATE;
   volatile unsigned char        *PCMCIA_BASE;
   volatile unsigned char        *ATTRIB_PTR;
   TIME_STRUCT            START_TIME;
} IO_PCCARDFLEXBUS_STRUCT, * IO_PCCARDFLEXBUS_STRUCT_PTR;


#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_int _io_pccardflexbus_open(FILE_DEVICE_STRUCT_PTR, char *, char *);
extern _mqx_int _io_pccardflexbus_close(FILE_DEVICE_STRUCT_PTR);
extern _mqx_int _io_pccardflexbus_ioctl(FILE_DEVICE_STRUCT_PTR, _mqx_uint, void *);

extern bool   _io_pccardflexbus_card_detect(IO_PCCARDFLEXBUS_STRUCT_PTR);
extern uint32_t   _io_pccardflexbus_read_tuple(volatile unsigned char *, uint32_t, unsigned char *);
extern void      _io_pccardflexbus_card_wait_till_ready(IO_PCCARDFLEXBUS_STRUCT_PTR);
extern void      _io_pccardflexbus_wait(IO_PCCARDFLEXBUS_STRUCT_PTR, uint32_t);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
