/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This file contains various UTF-8 functions.
*
*END************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <rtcs_utf8.h>

static inline bool utf8_check_boundary(uint8_t *position, uint32_t n, uint8_t *max);

/*
 * Check if input sequence is valid UTF-8 sequence.
 */
bool utf8_is_valid(uint8_t *input, uint32_t length, uint8_t **bad, uint32_t *missing)
{
    uint8_t  *position;
    uint8_t  *max;
    uint32_t n;

    position = input;
    max = input + length;
    *missing = 0;
    
    while(position < max)
    {
        n = 0;

        if (utf8_check_1(position))
        {
            position++;
            continue;
        }
        n++;
        if (utf8_check_boundary(position, n, max))
        {
            *missing = 4-n;
            goto BOUNDARY;
        }

        if (utf8_check_2(position))
        {
            position += n+1;
            continue;
        }
        n++;
        if (utf8_check_boundary(position, n, max))
        {
            *missing = 4-n;
            goto BOUNDARY;
        }
        if (utf8_check_3(position))
        {
            position += n+1;
            continue;
        }
        n++;
        if (utf8_check_boundary(position, n, max))
        {
            *missing = 4-n;
            goto BOUNDARY;
        }
        if (utf8_check_4(position))
        {
            position += n+1;
            continue;
        }

    BOUNDARY:
        *bad = position;
        return(false);
    }

    *bad = NULL;
    return(true);
}

/* Check if next step would break array boundary. */
static inline bool utf8_check_boundary(uint8_t *position, uint32_t n, uint8_t *max)
{
    if ((position+n) >= max)
    {
        return(true);
    }
    return(false);
}
