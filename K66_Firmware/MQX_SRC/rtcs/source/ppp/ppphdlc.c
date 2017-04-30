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
*   This file contains the PPPHDLC device functions needed
*   for communication.
*
*
*END************************************************************************/

#include <rtcs.h>

#if RTCSCFG_ENABLE_IP4 && RTCSCFG_ENABLE_PPP && !PLATFORM_SDK_ENABLED

#include <ppphdlc.h>
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#include <rtcs_in.h>
#include "fcs16.h"

PPPHDLC_OPT PPPHDLC_DEFAULT_OPTIONS = {
/* ACCM[]   */   {0xFFFFFFFFl, 0x00000000l, 0x00000000l, 0x60000000l,
                  0x00000000l, 0x00000000l, 0x00000000l, 0x00000000l},
/* PFC      */   FALSE,
/* ACFC     */   FALSE
};

/*
** A macro to test whether a character is flagged in the ACCM
*/

#define ACCM_ISSET(c,accm) ((accm)[(c)/PPPHDLC_BITS_IN_UINT32] & \
                           (1UL << ((c)%PPPHDLC_BITS_IN_UINT32)))

static uint32_t _iopcb_ppphdlc_open  (_iopcb_handle, void (_CODE_PTR_)(void *), void (_CODE_PTR_)(void *), void *);
static uint32_t _iopcb_ppphdlc_close (_iopcb_handle);
static void    _iopcb_ppphdlc_write (_iopcb_handle, PCB_PTR, uint32_t);
static PCB_PTR _iopcb_ppphdlc_read  (_iopcb_handle, uint32_t);
static uint32_t _iopcb_ppphdlc_ioctl (_iopcb_handle, uint32_t, void *);
static void  PPPHDLC_putc (PPPHDLC_STRUCT_PTR, unsigned char, PPPHDLC_OPT_PTR);
static unsigned char PPPHDLC_getc (PPPHDLC_STRUCT_PTR, uint32_t *, uint32_t *, PPPHDLC_OPT_PTR,uint32_t *);
static void    PPPHDLC_CCITT16_start (uint16_t *);
static void    PPPHDLC_CCITT16_calc  (uint16_t *, unsigned char);
static void    PPPHDLC_CCITT16_send  (PPPHDLC_STRUCT_PTR, PPPHDLC_OPT_PTR);
static bool PPPHDLC_CCITT16_check (uint16_t *);


/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_init
*  Returned Value : a HANDLE to be used by other functions
*  Comments       :
*      Sets some of the values of the ppphdlc structure and
*  returns the handle of the structure.
*
*END*-----------------------------------------------------------------*/

_iopcb_handle _iopcb_ppphdlc_init
   (
      MQX_FILE_PTR device
         /* [IN] - the IO handle*/
   )
{  /* Body */

#if RTCSCFG_ENABLE_IP4

   PPPHDLC_STRUCT_PTR ppphdlc_ptr;
   _ppphdlc_partid    part_id;

   ppphdlc_ptr = PPPHDLC_memalloc(sizeof(PPPHDLC_STRUCT));
   if (!ppphdlc_ptr) {
      return NULL;
   } /* Endif */

   part_id = PPPHDLC_partcreate(sizeof(PCB)
                              + sizeof(PCB_FRAGMENT)
                              + PPPHDLC_FRAMESIZE_MAXDATA
                              + PPPHDLC_FRAMESIZE_FCS,
                                PPPHDLC_INIT, PPPHDLC_GROW, PPPHDLC_MAX, NULL, NULL);
   if (!part_id) {
      return NULL;
   } /* Endif */

   if (PPPHDLC_mutex_init(&ppphdlc_ptr->MUTEX) != PPPHDLC_OK) {
      return NULL;
   } /* Endif */

   PPPHDLC_memcopy(&PPPHDLC_DEFAULT_OPTIONS, &ppphdlc_ptr->SEND_OPT, sizeof(PPPHDLC_OPT));
   PPPHDLC_memcopy(&PPPHDLC_DEFAULT_OPTIONS, &ppphdlc_ptr->RECV_OPT, sizeof(PPPHDLC_OPT));

   ppphdlc_ptr->DEVICE  = device;
   ppphdlc_ptr->PART_ID = part_id;

   ppphdlc_ptr->PCB_TABLE.OPEN  = _iopcb_ppphdlc_open;
   ppphdlc_ptr->PCB_TABLE.CLOSE = _iopcb_ppphdlc_close;
   ppphdlc_ptr->PCB_TABLE.READ  = _iopcb_ppphdlc_read;
   ppphdlc_ptr->PCB_TABLE.WRITE = _iopcb_ppphdlc_write;
   ppphdlc_ptr->PCB_TABLE.IOCTL = _iopcb_ppphdlc_ioctl;

   return &ppphdlc_ptr->PCB_TABLE;

#else

    return NULL;

#endif /* RTCSCFG_ENABLE_IP4  */

} /* Endbody */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_release
*  Returned Value : uint32_t, an ERROR code
*  Comments       :
*     Free memory
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

uint32_t _iopcb_ppphdlc_release
   (
      _iopcb_handle  handle
         /* [IN] - the structure handle */
   )
{  /* Body */
   PPPHDLC_STRUCT_PTR ppphdlc_ptr = (PPPHDLC_STRUCT_PTR)((void *)handle);

   /* Free memory */
   /* Free memory block by block. */
   PPPHDLC_partdestroy(ppphdlc_ptr->PART_ID);
   PPPHDLC_mutex_destroy(&ppphdlc_ptr->MUTEX);
   PPPHDLC_memfree(ppphdlc_ptr);

   return PPPHDLC_OK;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_open
*  Returned Value : uint32_t, an ERROR code
*  Comments       :
*      Sets some of the values of the ppphdlc structure and opens the link.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static uint32_t _iopcb_ppphdlc_open
   (
      _iopcb_handle       handle,
         /* [IN] - the IO handle*/
      void (_CODE_PTR_    up)(void *),
         /* [IN] - function to put the link up */
      void (_CODE_PTR_    down)(void *),
         /* [IN] - function to put the link down */
      void               *param
         /* [IN] - for the up/down functions */
   )
{  /* Body */
   PPPHDLC_STRUCT_PTR ppphdlc_ptr = (PPPHDLC_STRUCT_PTR)((void *)handle);

   ppphdlc_ptr->UP      = up;
   ppphdlc_ptr->DOWN    = down;
   ppphdlc_ptr->PARAM   = param;

   /* open connection */
   if (ppphdlc_ptr->UP) {
      up(param);
   } /* Endif */

   return PPPHDLC_OK;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_close
*  Returned Value : uint32_t, an ERROR code
*  Comments       :
*     Close link and free memory
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static uint32_t _iopcb_ppphdlc_close
   (
      _iopcb_handle  handle
         /* [IN] - the structure handle */
   )
{
   PPPHDLC_STRUCT_PTR ppphdlc_ptr = (PPPHDLC_STRUCT_PTR)((void *)handle);

   /* close connection */
   if (ppphdlc_ptr->DOWN) {
      ppphdlc_ptr->DOWN(ppphdlc_ptr->PARAM);
   }

   return PPPHDLC_OK;
}

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_write
*  Returned Value :
*  Comments       :
*      Transmits the data
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void _iopcb_ppphdlc_write
   (
      _iopcb_handle      handle,
         /* [IN] - the handle */
      PCB_PTR            pcb_ptr,
         /* [IN] - the packet */
      uint32_t            flags
         /* [IN] - the flags */
   )
{ /* Body */
    PPPHDLC_STRUCT_PTR  ppphdlc_ptr = (PPPHDLC_STRUCT_PTR)((void *)handle);
    PPPHDLC_OPT         ppphdlc_opt, * ppphdlc_opt_ptr;
    uint32_t            length;
    unsigned char       *fragment;
    uint32_t            i = 0;

    PPPHDLC_mutex_lock(&ppphdlc_ptr->MUTEX);
    ppphdlc_opt = ppphdlc_ptr->SEND_OPT;
    PPPHDLC_mutex_unlock(&ppphdlc_ptr->MUTEX);

    if (flags & 1) {
      ppphdlc_opt_ptr = &PPPHDLC_DEFAULT_OPTIONS;
    } else {
      ppphdlc_opt_ptr = &ppphdlc_opt;
    } /* Endif */

    /* Send the HDLC flag sequence */
    fputc(PPPHDLC_FLAG, ppphdlc_ptr->DEVICE);
    PPPHDLC_CCITT16_start(&ppphdlc_ptr->FCS_SEND);

    /* Send the header */
    if (!ppphdlc_opt_ptr->ACFC) {
        PPPHDLC_putc(ppphdlc_ptr, PPPHDLC_ADDR, ppphdlc_opt_ptr);
        PPPHDLC_putc(ppphdlc_ptr, PPPHDLC_CTRL, ppphdlc_opt_ptr);
    } /* Endif */

    /* Send the packet */
    length = pcb_ptr->FRAG[i].LENGTH;
    fragment = pcb_ptr->FRAG[i].FRAGMENT;

    /* Compress the protocol field if possible */
    if (!ppphdlc_opt_ptr->PFC || *fragment) {
        PPPHDLC_putc(ppphdlc_ptr, *fragment, ppphdlc_opt_ptr);
    } /* Endif */
    fragment++;
    length--;

    while (length != 0)  {
        while (length != 0) {
            PPPHDLC_putc(ppphdlc_ptr, *fragment++, ppphdlc_opt_ptr);
            length--;
        } /* Endwhile */
        i++;
        length = pcb_ptr->FRAG[i].LENGTH;
        fragment = pcb_ptr->FRAG[i].FRAGMENT;
    } /* Endwhile */

    /* Send the checksum */
    PPPHDLC_CCITT16_send(ppphdlc_ptr, ppphdlc_opt_ptr);
    ppphdlc_ptr->STATS.COMMON.ST_TX_TOTAL++;

    /* Verify the checksum */
    if (!PPPHDLC_CCITT16_check(&ppphdlc_ptr->FCS_SEND))  {
        ppphdlc_ptr->STATS.COMMON.ST_TX_MISSED++;
    } /* Endif */

    fputc(PPPHDLC_FLAG, ppphdlc_ptr->DEVICE);
    PCB_free(pcb_ptr);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_read
*  Returned Value : PCB_PTR
*  Comments       :
*      Receives the data
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static PCB_PTR _iopcb_ppphdlc_read
   (
      _iopcb_handle    handle,
         /* [IN] - the structure handle */
      uint32_t          flags
         /* [IN] - flags */
   )
{   /* Body */
    /* Charlie Foxtrot */
    PPPHDLC_STRUCT_PTR ppphdlc_ptr = (PPPHDLC_STRUCT_PTR)((void *)handle);
    PCB_FRAGMENT      *pcb_frag_ptr;
    PPPHDLC_OPT        ppphdlc_opt;
    PCB_PTR            pcb;
    uint32_t            err;
    uint32_t            retry,wait;
    uint32_t           *rx_stop_ptr;
    unsigned char          *p;
    uint16_t            data_len;
    register unsigned char     c;

   /*
    * If device does not have any data it has not sense to create PCB and wait for data.
    * It take less resources.
    */
   if(!fstatus(ppphdlc_ptr->DEVICE))
   {
        return NULL;
   }
    rx_stop_ptr = (uint32_t *)flags;
    PPPHDLC_mutex_lock(&ppphdlc_ptr->MUTEX);
    ppphdlc_opt = ppphdlc_ptr->RECV_OPT;
    PPPHDLC_mutex_unlock(&ppphdlc_ptr->MUTEX);

    retry = 20;

    pcb = PPPHDLC_partalloc(ppphdlc_ptr->PART_ID);

    if (!pcb)
    {
        ppphdlc_ptr->STATS.COMMON.ST_RX_DISCARDED++;
        return NULL;
    }

    pcb->FREE = (void (_CODE_PTR_)(PCB_PTR))PPPHDLC_partfree;
    pcb->PRIVATE = pcb;
    pcb_frag_ptr = pcb->FRAG;
    pcb_frag_ptr->FRAGMENT = (unsigned char *)pcb + sizeof(PCB) + sizeof(PCB_FRAGMENT);
    pcb_frag_ptr++;
    pcb_frag_ptr->LENGTH = 0;
    pcb_frag_ptr->FRAGMENT = NULL;

restart:
    retry--;
    if(!retry)
    {
        /* if we cannot take right packet we are returning 0 pcb */
        pcb->FRAG[0].LENGTH = 0;
        return pcb;
    }

    p = pcb->FRAG[0].FRAGMENT;
    /* zerou's check summ  */
    PPPHDLC_CCITT16_start(&ppphdlc_ptr->FCS_RECV);

    c = PPPHDLC_getc(ppphdlc_ptr, &err, NULL, &ppphdlc_opt,rx_stop_ptr);
    if (err)
    {
        goto restart;
    }
    /* Receive the frame header */
    /*
      ACFC -Address-and-Control-Field-Compression.
      So, if it is not compressed and it is not 0xFF
      start wait 200 times for start of PPP packet 0x7E
    */
    if (!ppphdlc_opt.ACFC && (c != PPPHDLC_ADDR))
    {
        ppphdlc_ptr->STATS.ST_RX_BAD_ADDR++;
        wait = 200;
        while(wait)
        {
            while(!fstatus(ppphdlc_ptr->DEVICE))
            {
                /* checking if RX_STOP is 1, it means that task aborted */
                if(*rx_stop_ptr)
                {
                    if(pcb != NULL)
                    {
                        PCB_free(pcb);
                    }
                    return(0);
                }
                _time_delay(1);
            }
            if(fgetc(ppphdlc_ptr->DEVICE) == PPPHDLC_FLAG)
            {
                break;
            }
            else
            {
                wait--;
                if (wait == 0)
                    goto restart;
            }
        }
    } /* Endif */

    /* If we got one, wait for control byte */
    /*
      If we reach here  it means that we received 0xFF (PPPHDLC_ADDR , and before it 0x7E ,start PPP)
      So , now we are reading for 0x03 (PPPHDLC_CTRL)
    */
    if (c == PPPHDLC_ADDR)
    {
        c = PPPHDLC_getc(ppphdlc_ptr, &err, &ppphdlc_ptr->STATS.ST_RX_RUNT, &ppphdlc_opt,rx_stop_ptr);
        if (err) goto restart;

        if (c != PPPHDLC_CTRL)
        {
            ppphdlc_ptr->STATS.ST_RX_BAD_CTRL++;

            /* new fix */
            wait = 200;
            while(wait)
            {
                while(!fstatus(ppphdlc_ptr->DEVICE))
                {
                    /* checking if RX_STOP is 1, it means that task aborted */
                    if(*rx_stop_ptr)
                    {
                        if(pcb != NULL)
                        {
                            PCB_free(pcb);
                        }
                        return(0);
                    }
                    _time_delay(1);
                }
                if(fgetc(ppphdlc_ptr->DEVICE) == PPPHDLC_FLAG)
                {
                    break;
                }
                else
                {
                    wait--;
                    if (wait == 0)
                        goto restart;
                }
            }
        } /* Endif */
        c = PPPHDLC_getc(ppphdlc_ptr, &err, &ppphdlc_ptr->STATS.ST_RX_RUNT, &ppphdlc_opt,rx_stop_ptr);
        if (err) goto restart;
    } /* Endif */

    data_len = 0;

    /* Decompress the protocol field if necessary */
    if (c & 1)
    {
        *p++ = 0;
        ++data_len;
    } /* Endif */

    /* We must recieve at least a frame check sequence */
    while (data_len < PPPHDLC_FRAMESIZE_FCS)
    {
        *p++ = c;
        ++data_len;
        c = PPPHDLC_getc(ppphdlc_ptr, &err, &ppphdlc_ptr->STATS.ST_RX_RUNT, &ppphdlc_opt,rx_stop_ptr);
        if (err) goto restart;
    } /* Endwhile */

    TIME_STRUCT start_time, current_time;
    _time_get(&start_time);

    /* Receive data */
    for (;;)
    {

        *p++ = c;
        ++data_len;

        while(!fstatus(ppphdlc_ptr->DEVICE))
        {
            /* Handle timeout while waiting a new package */
            _time_delay(1);
            _time_get(&current_time);
            if (current_time.MILLISECONDS - start_time.MILLISECONDS > PPPHDLC_RECV_TIMEOUT)
            {
                PCB_free(pcb);
                return NULL;
            }
        }

        c = PPPHDLC_getc(ppphdlc_ptr, &err, NULL, &ppphdlc_opt,rx_stop_ptr);

        if (err == PPPHDLC_END)
        {
            break;
        }
        if (err == PPPHDLC_ABORT)
        {
            goto restart;
        }
        /*
        If data_len >= maximum packet size - it means some error happend or some unsupported packet come.
        So, we start to wait for another valid packet waiting 200 times for  0x7E (PPPHDLC_FLAG - start of PPP packet)
        */
        if (data_len >= (PPPHDLC_FRAMESIZE_MAXDATA + PPPHDLC_FRAMESIZE_FCS))
        {
            ppphdlc_ptr->STATS.ST_RX_GIANT++;

            wait = 200;
            while(wait)
            {
                while(!fstatus(ppphdlc_ptr->DEVICE))
                {
                    /* checking if RX_STOP is 1, it means that task aborted */
                    if(*rx_stop_ptr)
                    {
                        if(pcb != NULL)
                        {
                            PCB_free(pcb);
                        }
                        return(0);
                    }
                    _time_delay(1);
                }
                if(fgetc(ppphdlc_ptr->DEVICE) == PPPHDLC_FLAG)
                {
                    break;
                }
                else
                {
                    wait--;
                    if (wait == 0)
                        goto restart;
                }
            }
        } /* Endif */
    } /* Endfor */

   /* Validate the FCS */
   /*
      checking controll summ here (last 2 bytes)
   */
   if (!PPPHDLC_CCITT16_check(&ppphdlc_ptr->FCS_RECV))
   {
      ppphdlc_ptr->STATS.ST_RX_BAD_FCS++;
      goto restart;
   } /* Endif */
    pcb->FRAG[0].LENGTH = data_len - PPPHDLC_FRAMESIZE_FCS;
    return pcb;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name  : _iopcb_ppphdlc_ioctl
*  Returned Value : an ERROR code
*  Comments       :
*      Store and set the options
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static uint32_t _iopcb_ppphdlc_ioctl
   (
      _iopcb_handle    handle,
         /* [IN] - the structure handle */
      uint32_t          option,
         /* [IN] - options */
      void            *value
         /* [IN] - option value */
   )
{  /* Body */
   PPPHDLC_STRUCT_PTR ppphdlc_ptr = (PPPHDLC_STRUCT_PTR)((void *)handle);

   PPPHDLC_mutex_lock(&ppphdlc_ptr->MUTEX);

   switch(option)  {
      case IOPCB_IOCTL_S_ACCM:
         ppphdlc_ptr->SEND_OPT.ACCM[0] = *(uint32_t *)value;
         break;
      case IOPCB_IOCTL_R_ACCM:
         ppphdlc_ptr->RECV_OPT.ACCM[0] = *(uint32_t *)value;
         break;
      case IOPCB_IOCTL_S_PFC:
         ppphdlc_ptr->SEND_OPT.PFC = *(bool *)value;
         break;
      case IOPCB_IOCTL_R_PFC:
         ppphdlc_ptr->RECV_OPT.PFC = *(bool *)value;
         break;
      case IOPCB_IOCTL_S_ACFC:
         ppphdlc_ptr->SEND_OPT.ACFC = *(bool *)value;
         break;
      case IOPCB_IOCTL_R_ACFC:
         ppphdlc_ptr->RECV_OPT.ACFC = *(bool *)value;
         break;
      case IOPCB_IOCTL_GET_IFTYPE:
         *(uint32_t *)value = IPIFTYPE_RS232;
         break;
   } /* Endswitch */

   PPPHDLC_mutex_unlock(&ppphdlc_ptr->MUTEX);

   return PPPHDLC_OK;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPHDLC_putc
* Returned Value  :  None
* Comments        :
*     Outputs one character, escaped if necessary, and updates
*     the FCS.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPHDLC_putc
   (
      PPPHDLC_STRUCT_PTR  handle,
            /* [IN] - the ppphdlc structure */
      unsigned char               c,
            /* [IN] - character to send */
      PPPHDLC_OPT_PTR     opt
            /* [IN] - the options */
   )
{ /* Body */

   if (ACCM_ISSET(c, opt->ACCM)) {
       fputc(PPPHDLC_ESC, handle->DEVICE);
       fputc((char)c ^ 0x20, handle->DEVICE);
   } else {
       fputc((char)c, handle->DEVICE);
   } /* Endif */

    PPPHDLC_CCITT16_calc(&handle->FCS_SEND, c);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPHDLC_getc
* Returned Value  :  character
* Comments        :
*     Retrieves one character and updates the FCS.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static unsigned char PPPHDLC_getc
   (
      PPPHDLC_STRUCT_PTR  handle,
            /* [IN] - the handle of the ppphdlc structure */
      uint32_t         *error,
            /* [OUT] - end of frame condition */
      uint32_t         *stat,
            /* [IN] - statistics counter to increment on flag character */
      PPPHDLC_OPT_PTR     opt,
            /* [IN] - the options */
      uint32_t         *rx_stop_ptr
   )
{ /* Body */
    unsigned char c;
    bool esc = FALSE;

    for (;;)
    {
        c = (unsigned char)fgetc(handle->DEVICE);

        /* The flag sequence ends a frame */
        if (c == PPPHDLC_FLAG)
        {
            if (esc)
            {
                handle->STATS.ST_RX_ABORTED++;
                *error = PPPHDLC_ABORT;
            }
            else
            {
                if (stat) (*stat)++;
                *error = PPPHDLC_END;
            } /* Endif */
            return 0;
        } /* Endif */

        /* Toss all flagged control characters */
        if (c < 0x20 && ACCM_ISSET(c, opt->ACCM))
        {
            continue;
        } /* Endif */

        /* Escaped characters get bit 5 toggled */
        if (esc)
        {
            c ^= 0x20;
        }
        else if (c == PPPHDLC_ESC)
        {
            esc = TRUE;
            continue;
        } /* Endif */

        PPPHDLC_CCITT16_calc(&handle->FCS_RECV, c);
        *error = PPPHDLC_OK;
        return c;
    } /* Endfor */

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPHDLC_CCITT16_start
* Returned Value  :  None
* Comments        :
*     Initializes the FCS_DATA for a CRC calculation.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPHDLC_CCITT16_start
   (
      uint16_t        *fcs
            /* [IN] -  fcs information */
   )
{ /* Body */
   *fcs = 0xFFFF;
} /* Endbody */

#endif

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPHDLC_CCITT16_calc
* Returned Value  :  None
* Comments        :
*     Updates the CRC calculation given a new character.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPHDLC_CCITT16_calc
   (
      uint16_t    *fcs,
            /* [IN] - FCS information */
      unsigned char          c
            /* [IN] - character */
   )
{ /* Body */
   uint16_t crc = *fcs;

   crc = (crc >> 8) ^ fcstab[(crc ^ c) & 0xFF];
   *fcs = crc;

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPHDLC_CCITT16_send
* Returned Value  :  None
* Comments        :
*     Sends a CRC.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static void PPPHDLC_CCITT16_send
   (
      PPPHDLC_STRUCT_PTR   ppphdlc_ptr,
            /* [IN] - the handle of the ppphdlc structure */
      PPPHDLC_OPT_PTR      opt
            /* [IN] - the options */
   )
{ /* Body */
   uint16_t crc = ~ppphdlc_ptr->FCS_SEND & 0xFFFF;
   PPPHDLC_putc(ppphdlc_ptr,  crc      & 0xFF, opt);
   PPPHDLC_putc(ppphdlc_ptr, (crc>>=8) & 0xFF, opt);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

/*FUNCTION*-------------------------------------------------------------
*
* Function Name   :  PPPHDLC_CCITT16_check
* Returned Value  :  TRUE if CRC is correct
* Comments        :
*     Validates a CRC calculation.
*
*END*-----------------------------------------------------------------*/

#if RTCSCFG_ENABLE_IP4

static bool PPPHDLC_CCITT16_check
   (
      uint16_t   *fcs
            /* [IN] - FCS information */
   )
{ /* Body */

   return (*fcs == 0xF0B8);

} /* Endbody */

#endif /* RTCSCFG_ENABLE_IP4 */

#endif // RTCSCFG_ENABLE_PPP

