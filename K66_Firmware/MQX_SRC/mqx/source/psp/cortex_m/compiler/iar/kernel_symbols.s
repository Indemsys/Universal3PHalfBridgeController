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
*   This file contains the symbol used to set the beginning of kernel data.
*
*
*END************************************************************************/

;  PROGRAM KERNEL_DATA
; COMMON KERNEL_DATA:DATA (4)
  SECTION .kernel_data : DATA(4)

  PUBLIC __KERNEL_DATA_START
__KERNEL_DATA_START:
  DC32 0

;  ENDMOD 
  END

; EOF
