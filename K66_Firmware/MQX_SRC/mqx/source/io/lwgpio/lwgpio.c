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
*   The file contains functions used in user program and/or in other
*   kernel modules to access GPIO pins
*
*
*END************************************************************************/
#include "mqx.h"
#include "bsp.h"
#include "lwgpio.h"

/*FUNCTION*****************************************************************
* 
* Function Name    : LWGPIO_SET_PIN_OUTPUT
* Returned Value   : TRUE if successfull
* Comments         :
*    Set pin to output and set its state
*
*END*********************************************************************/
bool lwgpio_set_pin_output(LWGPIO_PIN_ID id, LWGPIO_VALUE pin_state)
{
    LWGPIO_STRUCT tmp;
    return lwgpio_init(&tmp, id, LWGPIO_DIR_OUTPUT, pin_state);
}
        
/*FUNCTION*****************************************************************
* 
* Function Name    : LWGPIO_TOGGLE_PIN_OUTPUT
* Returned Value   : TRUE if succesfull 
* Comments         :
*    Toggles output pin state
*
*END*********************************************************************/
bool lwgpio_toggle_pin_output(LWGPIO_PIN_ID id)
{
    LWGPIO_STRUCT tmp;
    if (lwgpio_init(&tmp, id, LWGPIO_DIR_OUTPUT, LWGPIO_VALUE_NOCHANGE)) {
        lwgpio_toggle_value(&tmp);
        return TRUE;
    }
    return FALSE;
}

/*FUNCTION*****************************************************************
* 
* Function Name    : LWGPIO_GET_PIN_INPUT
* Returned Value   : LWGPIO_VALUE, LWGPIO_VALUE_NOCHANGE if error
* Comments         :
*    Gets input pin state
*
*END*********************************************************************/
LWGPIO_VALUE lwgpio_get_pin_input(LWGPIO_PIN_ID id)
{
    LWGPIO_STRUCT tmp;
    if (lwgpio_init(&tmp, id, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE)) {
        return lwgpio_get_value(&tmp);
    }
    return LWGPIO_VALUE_NOCHANGE;
}
