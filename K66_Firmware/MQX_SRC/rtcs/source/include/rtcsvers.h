#ifndef __rtcs_version_h__
#define __rtcs_version_h__
/*HEADER**********************************************************************
*
* Copyright 2008-2014 Freescale Semiconductor, Inc.
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
*   The current RTCS version.
*
*
*END************************************************************************/

#define __RTCS__

#define RTCS_VERSION_MAJOR    0x04
#define RTCS_VERSION_MINOR    0x02
#define RTCS_VERSION_REV      0x00

#define RTCS_VERSION  (0 | (RTCS_VERSION_MAJOR << 16) | \
                           (RTCS_VERSION_MINOR <<  8) | \
                            RTCS_VERSION_REV)

#endif
/* EOF */
