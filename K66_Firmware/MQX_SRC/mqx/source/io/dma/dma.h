#ifndef _dma_h_
#define _dma_h_ 1
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
*   This header defines API for MQX DMA drivers.
*
*
*END************************************************************************/

#include <mqx.h>


#define DMA_CHANNEL_FLAG_LOOP_MODE 0x01


/*
** DMA_EOT_CALLBACK
**
** This callback function is used to notify about end of transfer on a DMA channel.
** Parameter tcds_done contains number of TCDs finished since last callback - zero or negative value indicates an error.
** Parameter tcd_seq contains sequence number of first finished TCD.
** In case of an error tcd_seq contains number of last TCD which was prepared to be transferred (the state of such TCD is unspecified).
*/
typedef void (_CODE_PTR_ DMA_EOT_CALLBACK)(void *callback_data, int tcds_done, uint32_t tcd_seq);


/*
** This structure provides channel abstraction and it is mandatory to embed it as first member of device specific channel context.
** Pointer to this structure is used as handle identifying channel at the DMA API level
*/
typedef struct dma_channel
{
    /* Pointer to device interface structure of the low level driver */
    const struct dma_devif *DEVIF;

    /* Callback function for notification about events related to the channel */
    DMA_EOT_CALLBACK       CALLBACK_FUNC;

    /* Context data passed to the the callback function */
    void                   *CALLBACK_DATA;

} DMA_CHANNEL_STRUCT, *DMA_CHANNEL_HANDLE;



typedef struct dma_tcd {
  uint32_t SRC_ADDR;
  uint32_t SRC_WIDTH;
  int32_t  SRC_OFFSET;
  uint32_t SRC_MODULO;
  uint32_t DST_ADDR;
  uint32_t DST_WIDTH;
  int32_t  DST_OFFSET;
  uint32_t DST_MODULO;
  uint32_t LOOP_BYTES;
  uint32_t LOOP_COUNT;
  int32_t  LOOP_SRC_OFFSET;
  int32_t  LOOP_DST_OFFSET;
} DMA_TCD;


typedef int (_CODE_PTR_  DMA_DEVIF_INIT_FPTR)(void *devif_data);
typedef int (_CODE_PTR_  DMA_DEVIF_DEINIT_FPTR)(void *devif_data);

typedef int (_CODE_PTR_  DMA_DEVIF_CHANNEL_CLAIM_FPTR)(DMA_CHANNEL_HANDLE *, void *devif_data, int channel_no);
typedef int (_CODE_PTR_  DMA_DEVIF_CHANNEL_RELEASE_FPTR)(DMA_CHANNEL_HANDLE);

typedef int (_CODE_PTR_  DMA_DEVIF_CHANNEL_RESET_FPTR)(DMA_CHANNEL_HANDLE);
typedef int (_CODE_PTR_  DMA_DEVIF_CHANNEL_SETUP_FPTR)(DMA_CHANNEL_HANDLE, int tcd_slots, uint32_t flags);
typedef int (_CODE_PTR_  DMA_DEVIF_CHANNEL_STATUS_FPTR)(DMA_CHANNEL_HANDLE, uint32_t *tcd_seq, uint32_t *tcd_remaining);

typedef int (_CODE_PTR_  DMA_DEVIF_TRANSFER_SUBMIT_FPTR)(DMA_CHANNEL_HANDLE, DMA_TCD *, uint32_t *tcd_seq);

typedef int (_CODE_PTR_  DMA_DEVIF_REQUEST_SOURCE_FPTR)(DMA_CHANNEL_HANDLE, uint32_t source);
typedef int (_CODE_PTR_  DMA_DEVIF_REQUEST_ENABLE_FPTR)(DMA_CHANNEL_HANDLE);
typedef int (_CODE_PTR_  DMA_DEVIF_REQUEST_DISABLE_FPTR)(DMA_CHANNEL_HANDLE);


typedef struct dma_devif
{
    DMA_DEVIF_INIT_FPTR                 INIT;
    DMA_DEVIF_DEINIT_FPTR               DEINIT;
    DMA_DEVIF_CHANNEL_CLAIM_FPTR        CHANNEL_CLAIM;
    DMA_DEVIF_CHANNEL_RELEASE_FPTR      CHANNEL_RELEASE;
    DMA_DEVIF_CHANNEL_RESET_FPTR        CHANNEL_RESET;
    DMA_DEVIF_CHANNEL_SETUP_FPTR        CHANNEL_SETUP;
    DMA_DEVIF_CHANNEL_STATUS_FPTR       CHANNEL_STATUS;
    DMA_DEVIF_TRANSFER_SUBMIT_FPTR      TRANSFER_SUBMIT;
    DMA_DEVIF_REQUEST_SOURCE_FPTR       REQUEST_SOURCE;
    DMA_DEVIF_REQUEST_ENABLE_FPTR       REQUEST_ENABLE;
    DMA_DEVIF_REQUEST_DISABLE_FPTR      REQUEST_DISABLE;
} DMA_DEVIF;


typedef struct dma_devif_list
{
    DMA_DEVIF *DEVIF;
    void *DEVIF_DATA;
    int CHANNELS;
} DMA_DEVIF_LIST;


#ifdef __cplusplus
extern "C" {
#endif


/* Prototypes for generic TCD handling functions */

void dma_tcd_memcpy(DMA_TCD *tcd, void *src, void *dst, uint32_t size);

int dma_tcd_mem2reg(DMA_TCD *tcd, volatile void *reg, int regw, void *src, uint32_t size);
int dma_tcd_reg2mem(DMA_TCD *tcd, volatile void *reg, int regw, void *dst, uint32_t size);


/* Prototypes for functions accessing DMA engine */

int dma_init(const DMA_DEVIF_LIST *init_devif_list);
int dma_deinit(void);

int dma_channel_claim(DMA_CHANNEL_HANDLE *, int vchannel);
int dma_channel_release(DMA_CHANNEL_HANDLE);
int dma_channel_reset(DMA_CHANNEL_HANDLE);
int dma_channel_setup(DMA_CHANNEL_HANDLE, int tcd_slots, uint32_t flags);
int dma_channel_status(DMA_CHANNEL_HANDLE, uint32_t *tcd_seq, uint32_t *tcd_remaining);

int dma_transfer_submit(DMA_CHANNEL_HANDLE, DMA_TCD *tcd, uint32_t *tcd_seq);

int dma_request_source(DMA_CHANNEL_HANDLE, uint32_t source);
int dma_request_enable(DMA_CHANNEL_HANDLE);
int dma_request_disable(DMA_CHANNEL_HANDLE);

int dma_callback_reg(DMA_CHANNEL_HANDLE, DMA_EOT_CALLBACK callback_func, void *callback_data);


#ifdef __cplusplus
}
#endif


#endif
