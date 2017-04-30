/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2011 Embedded Access Inc.
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
*   LWADC driver header file
*
*
*END************************************************************************/
#ifndef __lwadc_h__
#define __lwadc_h__

typedef enum {
    LWADC_RESOLUTION=1,
    LWADC_REFERENCE,
    LWADC_FREQUENCY,
    LWADC_DIVIDER,
    LWADC_DIFFERENTIAL,
    LWADC_POWER_DOWN,
    LWADC_NUMERATOR ,
    LWADC_DENOMINATOR,
    LWADC_FORMAT,
    LWADC_INPUT_CONVERSION_ENABLE
} LWADC_ATTRIBUTE;

#define LWADC_FORMAT_LEFT_JUSTIFIED     1
#define LWADC_FORMAT_RIGHT_JUSTIFIED    2

typedef uint32_t LWADC_VALUE, * LWADC_VALUE_PTR;

/*----------------------------------------------------------------------*/
/*
**                    FUNCTION PROTOTYPES
*/

#ifdef __cplusplus
extern "C" {
#endif

bool _lwadc_init( const LWADC_INIT_STRUCT * init_ptr);
bool _lwadc_init_input( LWADC_STRUCT_PTR lwadc_ptr, uint32_t input);
bool _lwadc_read_raw( LWADC_STRUCT_PTR lwadc_ptr,  LWADC_VALUE_PTR outSample);
bool _lwadc_read( LWADC_STRUCT_PTR lwadc_ptr, LWADC_VALUE_PTR outSample);
bool _lwadc_wait_next( LWADC_STRUCT_PTR lwadc_ptr);
bool _lwadc_read_average( LWADC_STRUCT_PTR lwadc_ptr, uint32_t num_samples, LWADC_VALUE_PTR outSample);
bool _lwadc_set_attribute( LWADC_STRUCT_PTR lwadc_ptr, LWADC_ATTRIBUTE attribute, uint32_t value);
bool _lwadc_get_attribute( LWADC_STRUCT_PTR lwadc_ptr, LWADC_ATTRIBUTE attribute, uint32_t *value_ptr);

#ifdef __cplusplus
}
#endif

#endif //__lwadc_h__
