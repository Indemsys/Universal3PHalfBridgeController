#ifndef _edma_prv_h_
#define _edma_prv_h_ 1
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
*   This file contains private declarations of the edma driver
*
*
*END************************************************************************/


#define EDMA_STATUS_ENABLED_MASK 0x01
#define EDMA_STATUS_ERROR_MASK 0x02
#define EDMA_STATUS_LOOP_MASK 0x04


#ifndef DMA_NBYTES_MLOFFYES_NBYTES_MASK
    #define DMA_NBYTES_MLOFFYES_NBYTES_MASK DMA_NBYTES_MLOFFYES_NBYTES_OD_MASK
#endif

#ifndef DMA_NBYTES_MLOFFYES_NBYTES
    #define DMA_NBYTES_MLOFFYES_NBYTES DMA_NBYTES_MLOFFYES_NBYTES_OD
#endif

/* Hardware TCD structure */
typedef struct edma_hw_tcd {
    uint32_t SADDR;
    uint16_t SOFF;
    uint16_t ATTR;
    uint32_t NBYTES;
    uint32_t SLAST;
    uint32_t DADDR;
    uint16_t DOFF;
    uint16_t CITER;
    uint32_t DLAST_SGA;
    uint16_t CSR;
    uint16_t BITER;
} EDMA_HW_TCD;


/*
** EMA_CHANNEL_CONTEXT
**
** This structure defines the context kept by EDMA driver for each channel
*/
typedef struct edma_channel_context
{
    /* Generic channel context structure is embedded directly at the beginning */
    DMA_CHANNEL_STRUCT CHANNEL;

    /* Number of EDMA module this context is associated with */
    int                EDMA_MODULE;

    /* Number of channel within the EDMA module */
    int                EDMA_CHANNEL;

    /* Number of DMAMUX module this context is associated with */
    int                DMAMUX_MODULE;

    /* Number of channel within the DMAMUX module */
    int                DMAMUX_CHANNEL;

    /* Current channel status/flags */
    uint32_t           STATUS;

    /* Sequence number of TCD at the head of the queue */
    uint32_t           TCD_SEQ;

    /* Index of head of the TCD queue */
    int                TCD_HEAD;

    /* Index of tail of the TCD queue */
    int                TCD_TAIL;

    /* Number used TCD slots */
    int                TCD_USED;

    /* Total number of TCD slots in the queue */
    int                TCD_SLOTS;

    /* TCD queue slots */
    EDMA_HW_TCD        *TCD;

} EDMA_CHANNEL_CONTEXT;


#endif
