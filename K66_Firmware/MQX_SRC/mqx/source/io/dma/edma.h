#ifndef _edma_h_
#define _edma_h_ 1
/*HEADER**********************************************************************
*
* Copyright 2013 Freescale Semiconductor, Inc.
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
*   This file contains exports of the edma driver
*
*
*END************************************************************************/

#include "dma.h"


/* Exported device interface of the low level driver */
extern DMA_DEVIF edma_devif;


/* Prototypes for board/platform specific functions */
int _bsp_get_edma_done_vectors(uint32_t dev_num, const uint32_t **vectors_ptr);
int _bsp_get_edma_error_vectors(uint32_t dev_num, const uint32_t **vectors_ptr);
int _bsp_edma_enable(uint32_t dev_num);


#endif
