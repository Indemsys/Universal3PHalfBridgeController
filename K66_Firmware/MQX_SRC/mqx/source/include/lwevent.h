
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
*   This include file is used to define constants and data types for the
*   light weight event component.
*
*
*END************************************************************************/

#ifndef __lwevent_h__
#define __lwevent_h__   1

#include <mqx_cnfg.h>

#if (! MQX_USE_LWEVENTS) && (! defined (MQX_DISABLE_CONFIG_CHECK))
#error LWEVENT component is currently disabled in MQX kernel. Please set MQX_USE_LWEVENTS to 1 in user_config.h and recompile kernel.
#endif

/*--------------------------------------------------------------------------*/
/*                        CONSTANT DEFINITIONS                              */

/* Creation flags */
#define LWEVENT_AUTO_CLEAR          (0x00000001)

/* Error code */
#define LWEVENT_WAIT_TIMEOUT        (EVENT_ERROR_BASE|0x10)

/*--------------------------------------------------------------------------*/
/*!
 * \brief This structure defines a lightweight event.
 *
 * Tasks can wait on and set event bits.
 *
 * \see _lwevent_clear
 * \see _lwevent_create
 * \see _lwevent_destroy
 * \see _lwevent_set
 * \see _lwevent_set_auto_clear
 * \see _lwevent_usr_check
 * \see _lwevent_wait_for
 * \see _lwevent_wait_ticks
 * \see _lwevent_wait_until
 * \see _usr_lwevent_clear
 * \see _usr_lwevent_create
 * \see _usr_lwevent_destroy
 * \see _usr_lwevent_set
 * \see _usr_lwevent_set_auto_clear
 * \see _usr_lwevent_wait_for
 * \see _usr_lwevent_wait_ticks
 * \see _usr_lwevent_wait_until
 */
typedef struct lwevent_struct
{
    /*! \brief Queue data structures. */
    QUEUE_ELEMENT_STRUCT LINK;

    /*! \brief Queue of tasks waiting for event bits to be set. */
    QUEUE_STRUCT WAITING_TASKS;

    /*! \brief Validation stamp. */
    _mqx_uint VALID;

    /*! \brief Current bit value of the lightweight event. */
    _mqx_uint VALUE;

    /*! \brief Flags associated with the light weight event. */
    _mqx_uint FLAGS;

    /*! \brief Mask specifying lightweight event bits that are configured as auto-clear. */
    _mqx_uint AUTO;

}LWEVENT_STRUCT, * LWEVENT_STRUCT_PTR;


/*--------------------------------------------------------------------------*/
/*                           EXTERNAL DECLARATIONS                          */

#ifdef __cplusplus
extern "C" {
#endif

extern _mqx_uint _lwevent_create(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _lwevent_destroy(LWEVENT_STRUCT_PTR);
extern _mqx_uint _lwevent_set(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _lwevent_set_auto_clear(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _lwevent_clear(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _lwevent_wait_for  (LWEVENT_STRUCT_PTR, _mqx_uint, bool, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _lwevent_wait_ticks(LWEVENT_STRUCT_PTR, _mqx_uint, bool, _mqx_uint);
extern _mqx_uint _lwevent_wait_until(LWEVENT_STRUCT_PTR, _mqx_uint, bool, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _lwevent_get_signalled(void);
extern _mqx_uint _lwevent_test(void **, void **);


#if MQX_ENABLE_USER_MODE
extern _mqx_uint _usr_lwevent_create(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _usr_lwevent_destroy(LWEVENT_STRUCT_PTR);
extern _mqx_uint _usr_lwevent_set(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _usr_lwevent_set_auto_clear(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _usr_lwevent_clear(LWEVENT_STRUCT_PTR, _mqx_uint);
extern _mqx_uint _usr_lwevent_wait_for  (LWEVENT_STRUCT_PTR, _mqx_uint, bool, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _usr_lwevent_wait_ticks(LWEVENT_STRUCT_PTR, _mqx_uint, bool, _mqx_uint);
extern _mqx_uint _usr_lwevent_wait_until(LWEVENT_STRUCT_PTR, _mqx_uint, bool, MQX_TICK_STRUCT_PTR);
extern _mqx_uint _usr_lwevent_get_signalled(void);
extern _mqx_uint _lwevent_usr_check(LWEVENT_STRUCT_PTR);
#endif /* MQX_ENABLE_USER_MODE */

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
