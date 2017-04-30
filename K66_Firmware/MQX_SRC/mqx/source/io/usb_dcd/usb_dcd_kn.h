#ifndef _usb_dcd_kn_h
#define _usb_dcd_kn_h 1
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*   This include file is used to provide information needed by
*   applications using the DCD I/O functions.
*
*
*END************************************************************************/


/*--------------------------------------------------------------------------*/
/*
**                    DATATYPE DECLARATIONS
*/

/*
** KUSB_DCD_INIT_STRUCT
**
** This structure defines the initialization parameters to be used
** when a USB DCD is initialized.
*/
typedef struct kusb_dcd_init_struct
{
   /* Clock speed in Khz */
   uint32_t                CLOCK_SPEED;
   
   /* Sequence Initiation Time */
   uint16_t                TSEQ_INIT;

   /* Time Period to Debounce D+ Signal */
   uint16_t                TDCD_DBNC;
   
   /* Time Period to Comparator Enabled */
   uint16_t                TVDPSRC_ON;
   
   /* Time Period Before Enabling D+ Pullup */
   uint16_t                TVDPSRC_CON;
   
   /* Time Before Check of D- Line */
   uint16_t                CHECK_DM;

   /* Interrupt level to use if interrupt driven */
   _int_level             LEVEL;

} KUSB_DCD_INIT_STRUCT, * KUSB_DCD_INIT_STRUCT_PTR;

typedef const KUSB_DCD_INIT_STRUCT * KUSB_DCD_INIT_STRUCT_CPTR;
                                           

/*--------------------------------------------------------------------------*/
/* 
**                        FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t _kusb_dcd_polled_install (char *, KUSB_DCD_INIT_STRUCT_CPTR);
extern uint32_t _kusb_dcd_int_install (char *, KUSB_DCD_INIT_STRUCT_CPTR);
extern void   *_bsp_get_usb_dcd_base_address (void);
extern uint32_t _bsp_get_usb_dcd_vector (void);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
