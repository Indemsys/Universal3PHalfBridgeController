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
*   This file contains the RTCS shell.
*
*
*END************************************************************************/

#ifndef __sh_enet_h__
#define __sh_enet_h__

#include <enet.h>


#define eaddrassign(p,x)   ((p)[0] = (x)[0], \
                           (p)[1] = (x)[1], \
                           (p)[2] = (x)[2], \
                           (p)[3] = (x)[3], \
                           (p)[4] = (x)[4], \
                           (p)[5] = (x)[5]  \
                          )

#define eaddriszero(p)   ( ((p)[0] == 0) && \
                           ((p)[1] == 0) && \
                           ((p)[2] == 0) && \
                           ((p)[3] == 0) && \
                           ((p)[4] == 0) && \
                           ((p)[5] == 0)    \
                          )

/*
** Function prototypes 
*/
#ifdef __cplusplus
extern "C" {
#endif

extern bool Shell_parse_enet_address( char *arg, _enet_address enet_address);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
