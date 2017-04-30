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
*   This file contains implementation of DMA driver for eDMA
*
*
*END************************************************************************/


#include <mqx.h>
#include <bsp.h>
#include "dma.h"
#include "edma.h"
#include "edma_prv.h"


/*
 * Forward declarations of exported functions
 */
static int edma_init(void *devif_data);
static int edma_deinit(void *devif_data);

static int edma_channel_claim(DMA_CHANNEL_HANDLE *, void *devif_data, int channel_no);
static int edma_channel_release(DMA_CHANNEL_HANDLE);
static int edma_channel_reset(DMA_CHANNEL_HANDLE);
static int edma_channel_setup(DMA_CHANNEL_HANDLE, int tcd_slots, uint32_t flags);
static int edma_channel_status(DMA_CHANNEL_HANDLE, uint32_t *tcd_seq, uint32_t *tcd_progress);

static int edma_transfer_submit(DMA_CHANNEL_HANDLE, DMA_TCD *,uint32_t *tcd_seq);

static int edma_request_source(DMA_CHANNEL_HANDLE, uint32_t source);
static int edma_request_enable(DMA_CHANNEL_HANDLE);
static int edma_request_disable(DMA_CHANNEL_HANDLE);


/*
 * Device interface
 */
DMA_DEVIF edma_devif =
{
    edma_init,
    edma_deinit,
    edma_channel_claim,
    edma_channel_release,
    edma_channel_reset,
    edma_channel_setup,
    edma_channel_status,
    edma_transfer_submit,
    edma_request_source,
    edma_request_enable,
    edma_request_disable
};


/*
 * Peripheral base pointers initialized by a macro from generated header,
 * but may be overloaded by BSP.
 */
#ifdef BSP_DMA_BASE_PTRS
static DMA_MemMapPtr edma_base_tab[] = BSP_DMA_BASE_PTRS;
#else
static DMA_MemMapPtr edma_base_tab[] = DMA_BASE_PTRS;
#endif

#ifdef BSP_DMAMUX_BASE_PTRS
static DMAMUX_MemMapPtr dmamux_base_tab[] = BSP_DMAMUX_BASE_PTRS;
#else
static DMAMUX_MemMapPtr dmamux_base_tab[] = DMAMUX_BASE_PTRS;
#endif

#define EDMA_MODULES  (ELEMENTS_OF(edma_base_tab))
#define EDMA_CHANNELS (ELEMENTS_OF(edma_base_tab[0]->TCD))

#define DMAMUX_MODULES  (ELEMENTS_OF(dmamux_base_tab))
#define DMAMUX_CHANNELS (ELEMENTS_OF(dmamux_base_tab[0]->CHCFG))


/*
 * Compatibility macros for features implemented only on certain devices
 */
#ifndef DMA_CR_ERGA_MASK
#define DMA_CR_ERGA_MASK 0
#endif

#ifndef DMA_CR_GRP0PRI_MASK
#define DMA_CR_GRP0PRI_MASK 0
#endif

#ifndef DMA_CR_GRP1PRI_MASK
#define DMA_CR_GRP1PRI_MASK 0
#endif


/*
** Channel context data (driver private)
*/
static EDMA_CHANNEL_CONTEXT *edma_channel_context[EDMA_MODULES][EDMA_CHANNELS];


/*FUNCTION****************************************************************
*
* Function Name    : edma_channel_sgaidx
* Returned Value   :
* Comments         :
*    Calculates index of TCD slot which the given scatter-gather address points to
*
*END**********************************************************************/
static int edma_channel_sgaidx(EDMA_CHANNEL_CONTEXT *channel_context, uint32_t sga)
{
    int idx;

    if (channel_context->TCD_SLOTS <= 0) {
        return -1; /* no TCDs allocated */
    }

    if (sga < (uint32_t)(channel_context->TCD)) {
        return -1; /* address is out of scope */
    }

    /* adjust address to obtain TCD offset */
    sga -= (uint32_t)(channel_context->TCD);

    if (sga % sizeof(EDMA_HW_TCD)) {
        return -1; /* not aligned to TCD size */
    }

    idx = sga / sizeof(EDMA_HW_TCD);

    if (idx > channel_context->TCD_SLOTS - 1) {
        return -1; /* address was out of scope */
    }

    return idx;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_done_isr
* Returned Value   :
* Comments         :
*    EDMA interrupt service routine to handle transfer completion
*
*END**********************************************************************/
static void edma_done_isr(void *parameter)
{
    int edma_module;
    int edma_channel;

    int sga_idx;
    int tcd_new_head;
    int tcd_current_done;
    int tcds_done;
    uint32_t tcd_seq;

    DMA_MemMapPtr edma_base;
    EDMA_CHANNEL_CONTEXT *context;

    uint32_t sga;
    uint32_t flags;
    uint32_t mask;

    edma_module = (int)parameter;
    edma_base = edma_base_tab[edma_module];

    /* Read out channel int flags */
    flags = DMA_INT_REG(edma_base);

    for (edma_channel=0, mask=1; mask && (flags >= mask); edma_channel++, mask<<=1) {

        if (flags & mask) {

            /* Clear channel int flag (w1c) */
            DMA_INT_REG(edma_base) = mask;

            /* Read out scatter-gather address to identify TCD in the registers later on */
            sga = DMA_DLAST_SGA_REG(edma_base, edma_channel);

            /* Check if the TCD in the registers is already finished */
            tcd_current_done = ((DMA_CSR_REG(edma_base, edma_channel) & DMA_CSR_DONE_MASK) != 0);

            /* All necessary data read out from registers, check if it is valid (there is no error recorded so far) */
            if (DMA_ERR_REG(edma_base) & mask)
                continue;

            /* == No access to registers related to this channel is done after this line == */

            /* Pointer to channel context */
            context = edma_channel_context[edma_module][edma_channel];

            /* Suppress execution of pending callbacks for a channel in an error state */
            if (context->STATUS & EDMA_STATUS_ERROR_MASK) {
                continue;
            }

            /* Obtain descriptor index from sgatter-gather address */
            sga_idx = edma_channel_sgaidx(context, sga);
            if (sga_idx < 0) {
                /* Failed to identify the TCD in the registers, unexpected condition (internal error) */
                context->STATUS |= EDMA_STATUS_ERROR_MASK;

                /* Execute channel callback to notify about the error */
                if (NULL != context->CHANNEL.CALLBACK_FUNC)
                    context->CHANNEL.CALLBACK_FUNC(context->CHANNEL.CALLBACK_DATA, 0, context->TCD_SEQ);

                continue;
            }
            /* sga_idx points to next descriptor */

            /* Obtain new head position */
            if (tcd_current_done) {
                /* New head shall point to the next descriptor (current one is already finished) */
                tcd_new_head = sga_idx;
            }
            else {
                /* New head shall point to this descriptor (not finished yet) */
                tcd_new_head = sga_idx>0 ? sga_idx-1 : context->TCD_SLOTS-1;
            }

            /* Calculate number of previously finished TCDs */
            if (tcd_new_head == context->TCD_HEAD) {
                if (context->TCD_USED == context->TCD_SLOTS)
                    /* New head at the same position and all slots used - assuming that the whole round of the queue was executed */
                    tcds_done = context->TCD_SLOTS;
                else
                    tcds_done = 0;
            }
            else {
                tcds_done = tcd_new_head - context->TCD_HEAD;
                if (tcds_done < 0)
                   tcds_done += context->TCD_SLOTS;
            }

            /* Is there is no TCD finished then there is no reason for further processing */
            if (tcds_done <= 0)
                continue;

            /* Advance head index to point beyond the last finished TCD */
            context->TCD_HEAD = tcd_new_head;

            /* Save sequence number to pass to callback before incrementing it */
            tcd_seq = context->TCD_SEQ;
            context->TCD_SEQ += tcds_done;

            /* Free up TCD slot(s) unless they shall be reused in loop mode */
            if ((context->STATUS & EDMA_STATUS_LOOP_MASK) == 0)
                context->TCD_USED -= tcds_done;

            /* Execute channel callback to notify about finished TCDs */
            if (NULL != context->CHANNEL.CALLBACK_FUNC)
                context->CHANNEL.CALLBACK_FUNC(context->CHANNEL.CALLBACK_DATA, tcds_done, tcd_seq);

        }
    }
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_err_isr
* Returned Value   :
* Comments         :
*    EDMA interrupt service routine to handle transfer error interrupt
*
*END**********************************************************************/
static void edma_err_isr(void * parameter)
{
    int edma_module;
    int edma_channel;

    DMA_MemMapPtr edma_base;
    EDMA_CHANNEL_CONTEXT *context;

    uint32_t flags;
    uint32_t mask;

    edma_module = (int)parameter;
    edma_base = edma_base_tab[edma_module];

    /* read out channel int flags */
    flags = DMA_ERR_REG(edma_base);

    edma_channel = 0;
    mask = 1;
    while (0 != mask && flags >= mask) {

        if (flags & mask) {
            /* disable request for the channel */
            DMA_CERQ_REG(edma_base) = edma_channel;

            /* clear channel error flag (w1c) */
            DMA_ERR_REG(edma_base) = mask;

            /* clear channel int flag (w1c) */
            DMA_INT_REG(edma_base) = mask;

            /* Pointer to channel context */
            context = edma_channel_context[edma_module][edma_channel];

            /* set status */
            context->STATUS |= EDMA_STATUS_ERROR_MASK;

            /* execute callback */
            if (NULL != context->CHANNEL.CALLBACK_FUNC) {
                context->CHANNEL.CALLBACK_FUNC(context->CHANNEL.CALLBACK_DATA, 0, context->TCD_SEQ);
            }
        }

        mask <<= 1;
        edma_channel++;
    }
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_reset_module
* Returned Value   :
* Comments         :
*    Resets single EDMA module to initial state
*
*END**********************************************************************/
static int edma_reset_module(int edma_module)
{
    DMA_MemMapPtr edma_base;

    int i;

    if ((edma_module >= EDMA_MODULES) || (NULL == edma_base_tab[edma_module]))
        return MQX_INVALID_PARAMETER;

    _bsp_edma_enable(edma_module);

    edma_base = edma_base_tab[edma_module];

    DMA_ERQ_REG(edma_base) = 0;

    /* Halt, cancel transfer and wait until transfer is canceled */
    DMA_CR_REG(edma_base) = DMA_CR_CX_MASK | DMA_CR_HALT_MASK;
    while (DMA_CR_REG(edma_base) & DMA_CR_CX_MASK)
        ;

    /* The same one more time in case channel preemption was active (to stop also the preempted channel) */
    DMA_CR_REG(edma_base) = DMA_CR_CX_MASK | DMA_CR_HALT_MASK;
    while (DMA_CR_REG(edma_base) & DMA_CR_CX_MASK)
        ;

    DMA_CERR_REG(edma_base) = DMA_CERR_CAEI_MASK; /* clear all error flags */

    for (i = 0; i < EDMA_CHANNELS; i++) {
        int vchannel = edma_module * EDMA_CHANNELS + i;
        int dmamux_module = vchannel / DMAMUX_CHANNELS;
        int dmamux_channel = vchannel % DMAMUX_CHANNELS;

        DMA_CERQ_REG(edma_base) = i;
        DMAMUX_CHCFG_REG(dmamux_base_tab[dmamux_module], dmamux_channel) = 0;
    }

    /* Minor loop mapping enabled, round robin arbitration */
    DMA_CR_REG(edma_base) = DMA_CR_EMLM_MASK | DMA_CR_ERGA_MASK | DMA_CR_ERCA_MASK; /* | DMA_CR_HOE_MASK */

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_install_isrs
* Returned Value   :
* Comments         :
*    Installs ISRs related to given EDMA module
*
*END**********************************************************************/
static int edma_install_isrs(int edma_module)
{
    DMA_MemMapPtr edma_base;

    const uint32_t *vectors;
    int vectors_count;

    int i;

    if ((edma_module >= EDMA_MODULES) || (NULL == edma_base_tab[edma_module]))
        return MQX_INVALID_PARAMETER;

    edma_base = edma_base_tab[edma_module];

    vectors_count = _bsp_get_edma_done_vectors(edma_module, &vectors);
    for (i = 0; i < vectors_count; i++) {
        _int_install_isr(vectors[i], edma_done_isr, (void *)(edma_module));
        _bsp_int_init(vectors[i], BSP_EDMA_INT_LEVEL, 0, TRUE);
    }

    vectors_count = _bsp_get_edma_error_vectors(edma_module, &vectors);
    for (i = 0; i < vectors_count; i++) {
        _int_install_isr(vectors[i], edma_err_isr, (void *)(edma_module));
        _bsp_int_init(vectors[i], BSP_EDMA_INT_LEVEL, 0, TRUE);
    }

    /* enable error interrupts */
    DMA_SEEI_REG(edma_base) = DMA_SEEI_SAEE_MASK;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_uninstall_isrs
* Returned Value   :
* Comments         :
*    Installs ISRs related to given EDMA module
*
*END**********************************************************************/
static int edma_uninstall_isrs(int edma_module)
{
    DMA_MemMapPtr edma_base;

    const uint32_t *vectors;
    int vectors_count;

    int i;

    if ((edma_module >= EDMA_MODULES) || (NULL == edma_base_tab[edma_module]))
        return MQX_INVALID_PARAMETER;

    edma_base = edma_base_tab[edma_module];

    vectors_count = _bsp_get_edma_done_vectors(edma_module, &vectors);
    for (i = 0; i < vectors_count; i++) {
        _bsp_int_disable(vectors[i]);
        _int_install_isr(vectors[i], _int_get_default_isr(), NULL);
    }

    /* Disable all error interrupts */
    DMA_CEEI_REG(edma_base) = DMA_CEEI_CAEE_MASK;

    vectors_count = _bsp_get_edma_error_vectors(edma_module, &vectors);
    for (i = 0; i < vectors_count; i++) {
        _bsp_int_disable(vectors[i]);
        _int_install_isr(vectors[i], _int_get_default_isr(), NULL);
    }

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_init
* Returned Value   :
* Comments         :
*    Initialization of DMA driver
*
*END**********************************************************************/
static int edma_init(void *devif_data)
{
    int edma_module = (int)devif_data;
    int result;

    result = edma_reset_module(edma_module);
    if (MQX_OK != result) {
        return result;
    }

    result = edma_install_isrs(edma_module);
    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_deinit
* Returned Value   :
* Comments         :
*    DMA driver cleanup
*
*END**********************************************************************/
static int edma_deinit(void *devif_data)
{
    int edma_module = (int)devif_data;
    int result_tmp;
    int result = MQX_OK;

    result_tmp = edma_uninstall_isrs(edma_module);
    if (MQX_OK == result) {
        result = result_tmp;
    }

    result_tmp = edma_reset_module(edma_module);
    if (MQX_OK == result) {
        result = result_tmp;
    }

    return result;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_channel_claim
* Returned Value   :
* Comments         :
*    Reserves requested channel and allocates necessary memory resources
*
*END**********************************************************************/
static int edma_channel_claim(DMA_CHANNEL_HANDLE *handle_ptr, void *devif_data, int channel_no)
{
    EDMA_CHANNEL_CONTEXT *context;

    int vchannel;

    int edma_module;
    int edma_channel;

    int dmamux_module;
    int dmamux_channel;

    edma_module = (int)devif_data;
    edma_channel = channel_no;

    if ((edma_channel >= EDMA_CHANNELS) || (edma_module >= EDMA_MODULES) || (NULL == edma_base_tab[edma_module]))
        return MQX_INVALID_PARAMETER;

    vchannel = edma_module * EDMA_CHANNELS + edma_channel;
    dmamux_module = vchannel / DMAMUX_CHANNELS;
    dmamux_channel = vchannel % DMAMUX_CHANNELS;

    if ((dmamux_channel >= DMAMUX_CHANNELS) ||  (dmamux_module >= DMAMUX_MODULES) || (NULL == dmamux_base_tab[dmamux_module]))
        return MQX_INVALID_PARAMETER;

    /* Atomicity required */
    _int_disable();
    if (edma_channel_context[edma_module][edma_channel] != NULL) {
        _int_enable();
        return MQX_NOT_RESOURCE_OWNER;
    }
    edma_channel_context[edma_module][edma_channel] = (void *)-1;
    _int_enable();

    context = (EDMA_CHANNEL_CONTEXT *)_mem_alloc_system_zero(sizeof(EDMA_CHANNEL_CONTEXT));
    edma_channel_context[edma_module][edma_channel] = context;
    *handle_ptr = (DMA_CHANNEL_HANDLE)context;

    if (context == NULL)
        return MQX_OUT_OF_MEMORY;

    /* Fill in necessary context data */
    context->EDMA_MODULE = edma_module;
    context->EDMA_CHANNEL = edma_channel;
    context->DMAMUX_MODULE = dmamux_module;
    context->DMAMUX_CHANNEL = dmamux_channel;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_channel_release
* Returned Value   :
* Comments         :
*    Frees the channel data and releases the channel
*
*END**********************************************************************/
static int edma_channel_release(DMA_CHANNEL_HANDLE handle)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;

    int result;

    result = edma_channel_reset(handle);
    if (result != MQX_OK)
        return result;

    edma_channel_context[context->EDMA_MODULE][context->EDMA_CHANNEL] = NULL;

    if (context->TCD != NULL) {
        _mem_free(context->TCD);
    }
    _mem_free(context);

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_channel_reset
* Returned Value   :
* Comments         :
*    Aborts transfer in progress and discards any waiting transfers
*
*END**********************************************************************/
static int edma_channel_reset(DMA_CHANNEL_HANDLE handle)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;
    DMA_MemMapPtr edma_base = edma_base_tab[context->EDMA_MODULE];
    EDMA_HW_TCD volatile *tcdregs = (EDMA_HW_TCD volatile *)&edma_base->TCD[context->EDMA_CHANNEL];

    /* Disable request for the channel to prevent further transfers */
    DMA_CERQ_REG(edma_base) = context->EDMA_CHANNEL;

    /* There is no reliable way to break minor loop on particular channel, so just wait for it to finish */
    while (tcdregs->CSR & DMA_CSR_ACTIVE_MASK)
        ;

    /* Reset registers */
    tcdregs->SADDR = 0;
    tcdregs->SOFF = 0;
    tcdregs->ATTR = 0;
    tcdregs->NBYTES = 0;
    tcdregs->SLAST = 0;
    tcdregs->DADDR = 0;
    tcdregs->DOFF = 0;
    tcdregs->CITER = 0;
    tcdregs->DLAST_SGA = 0;
    tcdregs->CSR = 0;
    tcdregs->BITER = 0;

    /* Reset context */
    context->STATUS = 0;
    context->TCD_SEQ = 0;
    context->TCD_HEAD = 0;
    context->TCD_TAIL = 0;
    context->TCD_USED = 0;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_channel_setup
* Returned Value   :
* Comments         :
*    Allocates TCD FIFO for and prepares the channel to accept transfers
*
*END**********************************************************************/
static int edma_channel_setup(DMA_CHANNEL_HANDLE handle, int tcd_slots, uint32_t flags)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;

    int result;

    result = edma_channel_reset(handle);
    if (result != MQX_OK)
        return result;

    if (tcd_slots < 0)
        return MQX_INVALID_PARAMETER;

    /* If reallocation is required then free currently allocated slots first */
    if (context->TCD_SLOTS != tcd_slots) {
        context->TCD_SLOTS = 0;
        if (context->TCD != NULL) {
            result = _mem_free(context->TCD);
            if (result != MQX_OK)
                return result;
        }
    }

    /* Try to allocate requested number of slots */
    context->TCD = (EDMA_HW_TCD *)_mem_alloc_system_align(tcd_slots * sizeof(EDMA_HW_TCD), 32);
    if (context->TCD == NULL) {
        return MQX_OUT_OF_MEMORY;
    }
    context->TCD_SLOTS = tcd_slots;

    if (flags & DMA_CHANNEL_FLAG_LOOP_MODE) {
        context->STATUS |= EDMA_STATUS_LOOP_MASK;
    }

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_channel_status
* Returned Value   :
* Comments         :
*    Returns current status of given channel and optionally returns transfer progress
*
*END**********************************************************************/
static int edma_channel_status(DMA_CHANNEL_HANDLE handle, uint32_t *tcd_seq, uint32_t *tcd_remaining)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;
    DMA_MemMapPtr edma_base = edma_base_tab[context->EDMA_MODULE];

    int sga_idx;
    int current_idx;
    int context_head;
    uint32_t context_seq;

    uint32_t sga;
    uint32_t citer;

    if (tcd_seq || tcd_remaining) {

        /* Atomically read out data necessary for identification of current TCD */
        _int_disable();
        context_head = context->TCD_HEAD;
        context_seq = context->TCD_SEQ;
        _int_enable();

        sga = DMA_DLAST_SGA_REG(edma_base, context->EDMA_CHANNEL);

        sga_idx = edma_channel_sgaidx(context, sga);
        if (sga_idx < 0) {
            /* Unexpected condition, internal error */
            return MQX_ERROR;
        }
        current_idx = sga_idx>0 ? sga_idx-1 : context->TCD_SLOTS-1;

        if (tcd_seq) {
            if (context->STATUS & EDMA_STATUS_LOOP_MASK) {
                /* Report the descriptor index in loop mode */
                *tcd_seq = current_idx;
            }
            else {
                /* Calculate seq number for current TCD */
                *tcd_seq = current_idx - context_head + context_seq;
                if (current_idx < context_head) {
                    *tcd_seq += context->TCD_SLOTS;
                }
            }
        }

        if (tcd_remaining) {
            *tcd_remaining = context->TCD[current_idx].NBYTES;
                            
            citer = DMA_CITER_ELINKNO_REG(edma_base, context->EDMA_CHANNEL);

            if ((sga != DMA_DLAST_SGA_REG(edma_base, context->EDMA_CHANNEL)) || 
                (DMA_CSR_REG(edma_base, context->EDMA_CHANNEL) & DMA_CSR_DONE_MASK)) {
                /* Next descriptor was already loaded, the current one already finished */
                *tcd_remaining = 0;
            }
            else {
                /* Calculate remaining bytes to be transferred */
                *tcd_remaining *= citer;
            }
        }
    }

    /* Check channel error status */
    if ((DMA_ERR_REG(edma_base) & (1 << context->EDMA_CHANNEL)) || (context->STATUS & EDMA_STATUS_ERROR_MASK)) {
        return MQX_EIO;
    }

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_hwtcd_prepare
* Returned Value   :
* Comments         :
*    Fills in hw TCD according to sw TCD
*
*END**********************************************************************/
static int edma_tcd_prepare(DMA_TCD *tcd, EDMA_HW_TCD *hwtcd)
{
    int loop_offset;

    uint32_t attr;
    uint32_t nbytes;

    /* if both offsets are non zero then they have to be equal */
    if ((0 != tcd->LOOP_SRC_OFFSET) && (0 != tcd->LOOP_DST_OFFSET)) {
        if (tcd->LOOP_SRC_OFFSET != tcd->LOOP_DST_OFFSET) {
            return MQX_INVALID_PARAMETER;
        }
    }

    /* take any non-zero offset value */
    loop_offset = tcd->LOOP_SRC_OFFSET ? tcd->LOOP_SRC_OFFSET : tcd->LOOP_DST_OFFSET;

    if (loop_offset > ((1<<19)-1) || loop_offset < -((1<<19)))
        return MQX_INVALID_PARAMETER;

    if (tcd->LOOP_BYTES > 1023)
        return MQX_INVALID_PARAMETER;

    if (tcd->SRC_MODULO > 31)
        return MQX_INVALID_PARAMETER;

    if (tcd->DST_MODULO > 31)
        return MQX_INVALID_PARAMETER;


    attr = DMA_ATTR_SMOD(tcd->SRC_MODULO) | DMA_ATTR_DMOD(tcd->DST_MODULO);

    switch (tcd->SRC_WIDTH) {
        case 1:
            attr |= DMA_ATTR_SSIZE(0);
            break;
        case 2:
            attr |= DMA_ATTR_SSIZE(1);
            break;
        case 4:
            attr |= DMA_ATTR_SSIZE(2);
            break;
        case 16:
            attr |= DMA_ATTR_SSIZE(4);
            break;
        default:
            return MQX_INVALID_PARAMETER;
    }

    switch (tcd->DST_WIDTH) {
        case 1:
            attr |= DMA_ATTR_DSIZE(0);
            break;
        case 2:
            attr |= DMA_ATTR_DSIZE(1);
            break;
        case 4:
            attr |= DMA_ATTR_DSIZE(2);
            break;
        case 16:
            attr |= DMA_ATTR_DSIZE(4);
            break;
        default:
            return MQX_INVALID_PARAMETER;
    }


    /* loop has to divisible by transfer width */
    if ((tcd->LOOP_BYTES % tcd->SRC_WIDTH) || (tcd->LOOP_BYTES % tcd->DST_WIDTH))
        return MQX_INVALID_PARAMETER;


    /* fill in hw TCD */
    hwtcd->CSR = DMA_CSR_DREQ_MASK | DMA_CSR_INTMAJOR_MASK;

    hwtcd->ATTR = attr;


    hwtcd->SADDR = tcd->SRC_ADDR;
    hwtcd->SOFF = tcd->SRC_OFFSET;

    hwtcd->DADDR = tcd->DST_ADDR;
    hwtcd->DOFF = tcd->DST_OFFSET;

    nbytes = DMA_NBYTES_MLOFFYES_NBYTES(tcd->LOOP_BYTES) | DMA_NBYTES_MLOFFYES_MLOFF(loop_offset);
    
    if (tcd->LOOP_SRC_OFFSET)
        nbytes |= DMA_NBYTES_MLOFFYES_SMLOE_MASK;

    if (tcd->LOOP_DST_OFFSET)
        nbytes |= DMA_NBYTES_MLOFFYES_DMLOE_MASK;

    hwtcd->NBYTES = nbytes;


    hwtcd->CITER = tcd->LOOP_COUNT;
    hwtcd->BITER = tcd->LOOP_COUNT;


    hwtcd->SLAST = 0;
    hwtcd->DLAST_SGA = 0;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_hwtcd_push
* Returned Value   :
* Comments         :
*    Pushes given hw TCD to registers.
*
*END**********************************************************************/
static void edma_hwtcd_push(EDMA_HW_TCD *hwtcd, EDMA_HW_TCD volatile *tcdregs)
{
    tcdregs->SADDR = hwtcd->SADDR;
    tcdregs->SOFF = hwtcd->SOFF;
    tcdregs->ATTR = hwtcd->ATTR;
    tcdregs->NBYTES = hwtcd->NBYTES;
    tcdregs->SLAST = hwtcd->SLAST;
    tcdregs->DADDR = hwtcd->DADDR;
    tcdregs->DOFF = hwtcd->DOFF;
    tcdregs->CITER = hwtcd->CITER;
    tcdregs->DLAST_SGA = hwtcd->DLAST_SGA;
    tcdregs->CSR = 0; /* Clear DONE bit first, otherwise ESG cannot be set */
    tcdregs->CSR = hwtcd->CSR;
    tcdregs->BITER = hwtcd->BITER;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_transfer_submit
* Returned Value   :
* Comments         :
*    Queues transfer on given channel according to parameters in DMA_TCD structure
*
*END**********************************************************************/
static int edma_transfer_submit(DMA_CHANNEL_HANDLE handle, DMA_TCD *tcd, uint32_t *tcd_seq)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;
    DMA_MemMapPtr edma_base = edma_base_tab[context->EDMA_MODULE];
    EDMA_HW_TCD volatile *tcdregs = (EDMA_HW_TCD volatile *)&edma_base->TCD[context->EDMA_CHANNEL];

    uint32_t csr;

    int tcd_this;
    int tcd_next;
    int tcd_prev;

    int context_head;
    uint32_t context_seq;

    int result;

    /* Check if there is a hw TCD slot available */
    if (context->TCD_USED >= context->TCD_SLOTS)
        return MQX_OUT_OF_MEMORY;

    tcd_this = context->TCD_TAIL;

    /* Calculate index of previous TCD */
    tcd_prev = context->TCD_TAIL ? context->TCD_TAIL-1 : context->TCD_SLOTS-1;

    /* Calculate index of next TCD */
    tcd_next = context->TCD_TAIL + 1;
    if (tcd_next == context->TCD_SLOTS)
        tcd_next = 0;

    /* Fill in hw TCD */
    result = edma_tcd_prepare(tcd, &context->TCD[tcd_this]);

    if (result != MQX_OK)
        return result;

    /* Prepare pointer to next (yet invalid) TCD - necessary for identification of current TCD */
    context->TCD[tcd_this].DLAST_SGA = (uint32_t)&context->TCD[tcd_next];

    /* Advance queue tail index */
    context->TCD_TAIL = tcd_next;

    _int_disable();
    context->TCD_USED++;
    context_head = context->TCD_HEAD;
    context_seq = context->TCD_SEQ;
    _int_enable();

    if (tcd_seq) {
        /* Calculate seq number for current TCD */
        *tcd_seq = tcd_this - context_head + context_seq;
        if (tcd_this < context_head) {
            *tcd_seq += context->TCD_SLOTS;
        }
    }

    /* If loop mode is on and this is the last TCD slot then enable chaining to the next TCD to close the loop */
    if ((context->STATUS & EDMA_STATUS_LOOP_MASK) && (context->TCD_USED == context->TCD_SLOTS)) {
        csr = (context->TCD[tcd_this].CSR | DMA_CSR_ESG_MASK) & ~DMA_CSR_DREQ_MASK;
        context->TCD[tcd_this].CSR = csr;
    }

    _DCACHE_FLUSH_MBYTES(&context->TCD[tcd_this], sizeof(EDMA_HW_TCD));

    /* Chain from previous descriptor unless this descriptor is its own predecessor */
    if (tcd_prev != tcd_this) {

        /* Enable scatter/gather in the previous descriptor */
        csr = (context->TCD[tcd_prev].CSR | DMA_CSR_ESG_MASK) & ~DMA_CSR_DREQ_MASK;
        context->TCD[tcd_prev].CSR = csr;

        _DCACHE_FLUSH_MBYTES(&context->TCD[tcd_prev], sizeof(EDMA_HW_TCD));

        /* Check whether the descriptor in the registers is the previous one (points to this one) */
        if (tcdregs->DLAST_SGA == (uint32_t)&context->TCD[tcd_this]) {

            /* Enable scatter/gather also in the registers */
            csr = (tcdregs->CSR | DMA_CSR_ESG_MASK) & ~DMA_CSR_DREQ_MASK;
            tcdregs->CSR = csr;
            if (tcdregs->CSR & DMA_CSR_ESG_MASK)
                return MQX_OK; /* Success */

            /* Check whether the last (this) descriptor is already loaded in the registers */
            if (tcdregs->DLAST_SGA == (uint32_t)&context->TCD[tcd_next])
                return MQX_OK; /* Yes, we are done */
        }
        else if (tcdregs->DLAST_SGA != 0) {
            /* There is a valid descriptor in the registers, which means that the TCD was placed to a live chain */
            return MQX_OK;
        }
    }

    /* The chain has finished or there is no chain so far, it is necessary to push the descriptor to the registers */
    edma_hwtcd_push(&context->TCD[tcd_this], tcdregs);

    /* Eventually re-enable data request */
    if (context->STATUS & EDMA_STATUS_ENABLED_MASK)
        DMA_SERQ_REG(edma_base) = context->EDMA_CHANNEL;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_request_source
* Returned Value   :
* Comments         :
*    Configures request source for given channel
*
*END**********************************************************************/
static int edma_request_source(DMA_CHANNEL_HANDLE handle, uint32_t source)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;

    if (source > DMAMUX_CHCFG_SOURCE_MASK)
        return MQX_INVALID_PARAMETER;

    DMAMUX_CHCFG_REG(dmamux_base_tab[context->DMAMUX_MODULE], context->DMAMUX_CHANNEL) = 0;
    DMAMUX_CHCFG_REG(dmamux_base_tab[context->DMAMUX_MODULE], context->DMAMUX_CHANNEL) = source | DMAMUX_CHCFG_ENBL_MASK;

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_request_enable
* Returned Value   :
* Comments         :
*    Enables request on given channel to start/resume a transfer
*
*END**********************************************************************/
static int edma_request_enable(DMA_CHANNEL_HANDLE handle)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;
    DMA_MemMapPtr edma_base = edma_base_tab[context->EDMA_MODULE];
    EDMA_HW_TCD volatile *tcdregs = (EDMA_HW_TCD volatile *)&edma_base->TCD[context->EDMA_CHANNEL];

    context->STATUS |= EDMA_STATUS_ENABLED_MASK;

    /* Check if there was at least one descriptor submitted since reset (TCD in registers is valid) */
    if (0 == tcdregs->DLAST_SGA)
        return MQX_OK;

    _int_disable();
    /* Check if the request is currently disabled, otherwise take no action */
    if ((DMA_ERQ_REG(edma_base) & (1<<context->EDMA_CHANNEL)) == 0)
    {
        /* If the descriptor in the registers is either not done, or there is another descriptor waiting */
        if (!(tcdregs->CSR & DMA_CSR_DONE_MASK) || (tcdregs->CSR & DMA_CSR_ESG_MASK))
            DMA_SERQ_REG(edma_base) = context->EDMA_CHANNEL;  // enable request
    }
    _int_enable();

    return MQX_OK;
}


/*FUNCTION****************************************************************
*
* Function Name    : edma_request_disable
* Returned Value   :
* Comments         :
*    Disables request on given channel to pause a transfer
*
*END**********************************************************************/
static int edma_request_disable(DMA_CHANNEL_HANDLE handle)
{
    EDMA_CHANNEL_CONTEXT *context = (EDMA_CHANNEL_CONTEXT *)handle;
    DMA_MemMapPtr edma_base = edma_base_tab[context->EDMA_MODULE];

    context->STATUS &= ~EDMA_STATUS_ENABLED_MASK;

    DMA_CERQ_REG(edma_base) = context->EDMA_CHANNEL;

    return MQX_OK;
}

