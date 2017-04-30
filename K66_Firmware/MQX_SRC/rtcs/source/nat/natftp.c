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
*   This file contains the NAT FTP ALG.
*
*
*END************************************************************************/
#include <rtcs.h>

#if RTCSCFG_ENABLE_NAT

#include <rtcs_prv.h>
#if MQX_USE_IO_OLD
  #include <fio.h>
#else
  #include <stdio.h>
#endif
#include <ctype.h>
#include <string.h>

#include "nat.h"
#include "nat_prv.h"

/*****************************************
**
** NAT FTP ALG Private structures
*/

typedef struct nat_ftp_struct{
   NAT_ALG_CFG_STRUCT   HEADER;
   _rtcs_part           FTP_PART_ID;
} NAT_FTP_STRUCT, * NAT_FTP_STRUCT_PTR;

typedef struct nat_session_ftp_struct {
   NAT_SESSION_EXT_STRUCT     HEADER;
   int32_t                     LAST_ADJ;
   uint32_t                    LAST_SEQ;
   int32_t                     NEXT_ADJ;
   uint32_t                    NEXT_SEQ;
} NAT_SESSION_FTP_STRUCT, * NAT_SESSION_FTP_STRUCT_PTR;

struct  {
   IP_HEADER   IP;
   TCP_HEADER  TCP;
} NAT_ALG_FTP_dummy = {
   {
      {0x45},                       /* VERSLEN */
      {0x00},                       /* TOS */
      {0x00, 0x28},                 /* LENGTH */
      {0xA1, 0x10},                 /* ID */
      {0x40, 0x00},                 /* FRAGMENT */
      {0x80},                       /* TTL */
      {0x06},                       /* PROTOCOL */
      {0xF8, 0xE8},                 /* CHECKSUM */
      {0x00, 0x00, 0x00, 0x00},     /* SOURCE */
      {0x00, 0x00, 0x00, 0x00}      /* DESTINATION */
   },
   {
      {0x00, 0x00},                 /* SOURCE PORT */
      {0x00, 0x00},                 /* DEST PORT */
      {0x00, 0x00, 0x00, 0x00},     /* SEQ */
      {0x00, 0x00, 0x00, 0x00},     /* ACK */
      {0x50, 0x10},                 /* FLAGS */
      {0x22, 0x38},                 /* WINDOW */
      {0x00, 0x00},                 /* CHECKSUM */
      {0x00, 0x00}                  /* URGENT */
   }
};


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_ftp
* Returned Value  : error code
* Comments        :
*        Initializes the NAT FTP ALG.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_ALG_ftp
   (
      void   *ptr       /* [IN] Pointer to NAT config struct */
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR   nat_cfg_ptr = ptr;
   NAT_FTP_STRUCT_PTR   ftp_ptr;

   ftp_ptr = _mem_alloc_system(sizeof(NAT_FTP_STRUCT));

   if (ftp_ptr)  {
      ftp_ptr->FTP_PART_ID = RTCS_part_create(
         sizeof(NAT_SESSION_FTP_STRUCT), 4, 4, 0, NULL, NULL);

      if (ftp_ptr->FTP_PART_ID) {
         ftp_ptr->HEADER.NEXT = nat_cfg_ptr->ALG_INFO_PTR;
         nat_cfg_ptr->ALG_INFO_PTR = (NAT_ALG_CFG_STRUCT_PTR)(void *)ftp_ptr;
         ftp_ptr->HEADER.ALG_TYPE = NAT_ALG_FTP_TYPE;
         ftp_ptr->HEADER.ALG_EXEC = NAT_ALG_ftp_apply;
         return RTCS_OK;
      } else {
         _mem_free((void *)ftp_ptr);
      } /* Endif */
   } /* Endif */

   return RTCSERR_OUT_OF_MEMORY;

} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_detect_port
* Returned Value  : bool
* Comments        :
*        Detects a "PORT" command. Assumes data starts with "PORT".
*
*END*-----------------------------------------------------------------*/

static bool NAT_ALG_detect_port
   (
      unsigned char      *data_ptr,            /* [IN] pointer to data          */
      uint32_t        len,                 /* [IN] length of data           */
      uint32_t    *ip_address_ptr,      /* [OUT] address read from data  */
      uint16_t    *port_ptr             /* [OUT] port read from data     */
   )
{ /* Body */
   unsigned char       buf[6] =  { 0, 0, 0, 0, 0, 0 };
   unsigned char   *ptr;
   uint32_t     state;

   *ip_address_ptr = 0;
   *port_ptr = 0;

   /* "PORT a,a,a,a,p,p" 16 bytes minimum */
   if (len < 18)  {
      return FALSE;
   } /* Endif */

   /* Skip "PORT" */
   ptr = data_ptr + 4;

   /*
   ** Regular expression for PORT command: ' '+([0-9]{1,3},){5}[0-9]{1,3}
   */

   /* At least 1 space */
   if (*ptr++ != ' ')  {
      return FALSE;
   } /* Endif */

   /* Skip all spaces */
   while ((ptr < (data_ptr + len)) && (*ptr == ' '))  {
      ptr++;
   } /* Enwhile */

   for (state = 0; ptr < (data_ptr + len); ptr++)  {
      if (isdigit(*ptr) && (state < 12))  {
         if ((state & 1) == 0)  {
            state++;
         } /* Endif */
         buf[state/2] = buf[state/2] * 10 + *ptr - '0';
      } else if ((*ptr == ',') && (state & 1)) {
         state++;
      } else {
         break;
      } /* Endif */
   } /* Endfor */

   if (state != 11)  {
      return FALSE;
   } /* Endif */

   *ip_address_ptr = ((buf[0] & 0xFF) << 24) +
         ((buf[1] & 0xFF) << 16) +
         ((buf[2] & 0xFF) <<  8) +
         (buf[3] & 0xFF);

   *port_ptr = ((buf[4] & 0xFF) << 8) + (buf[5] & 0xFF);

   return TRUE;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_ftp_pcb_free
* Returned Value  : void
* Comments        :
*        Frees a PCB
*
*END*-----------------------------------------------------------------*/

static void NAT_ALG_ftp_pcb_free
   (
      PCB_PTR  pcb_ptr   /* [IN] PCB to free */
   )
{ /* Body */
   _mem_free(pcb_ptr);
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_ftp_bwrite
* Returned Value  : void
* Comments        :
*        Writes the ascii representation of a byte
*
*END*-----------------------------------------------------------------*/

static void NAT_ALG_ftp_bwrite
   (
      unsigned char             byte,          /* [IN] Byte to write         */
      unsigned char *   *store_ptr_ptr  /* [IN/OUT] Pointer to data   */
   )
{
   unsigned char   *store_ptr = *store_ptr_ptr;
   uint32_t     len = 1;

   /* Determine length of byte */
   if (byte > 99)  {
      len = 3;
   } else if (byte > 9)  {
      len = 2;
   } /* Endif */

   /* Write out byte */
   switch (len)  {
      case 3:
         *store_ptr++ = '0' + byte / 100;
         byte %= 100;
      case 2:
         *store_ptr++ = '0' + byte / 10;
         byte %= 10;
      case 1:
         *store_ptr++ = '0' + byte;
         break;
   } /* Endif */

   *store_ptr_ptr = store_ptr;
} /* Endbody */


/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : NAT_ALG_ftp_apply
* Returned Value  : error code
* Comments        :
*        Applies the NAT FTP ALG to a packet.
*
*END*-----------------------------------------------------------------*/

uint32_t NAT_ALG_ftp_apply
   (
      RTCSPCB_PTR  *pcb_ptr_ptr,
      bool           pub_to_prv,
      void             *alg_cfg_ptr,
      void        **session_ptr_ptr
   )
{ /* Body */
   NAT_CFG_STRUCT_PTR         nat_cfg_ptr = RTCS_getcfg(NAT);
   NAT_SESSION_STRUCT_PTR     nat_session_ptr = *((NAT_SESSION_STRUCT_PTR *)session_ptr_ptr);
   NAT_SESSION_STRUCT_PTR     nat_data_session_ptr;
   unsigned char                  *orig_pcb_data_ptr = (void *)RTCSPCB_DATA(*pcb_ptr_ptr);
   unsigned char * new_pcb_data_ptr;
   IP_HEADER_PTR              ip_header_ptr = (IP_HEADER_PTR)(void *)orig_pcb_data_ptr;
   TRANSPORT_UNION            transport;
   NAT_SESSION_FTP_STRUCT_PTR ftp_session_ptr;
   RTCSPCB_PTR                new_pcb_ptr;
   unsigned char                  *tcp_data_ptr;
   bool                    src_port_is_ftp, dest_port_is_ftp;
   uint32_t                    ack, temp, seq, addr, error;
   uint16_t                    orig_tcp_data_length, new_tcp_data_length;
   int32_t                     seq_adj, i;
   uint16_t                    chksum,  port,  ip_header_length, tcp_header_length;
   uint16_t                    orig_ip_length, new_ip_length;
   PCB_PTR                    new_mqx_pcb_ptr;

   if (nat_session_ptr == NULL)  {
      /*
      ** There is no session yet. This packet may be part of the TCP handshake,
      ** and not a data message. Let NAT first create a session.
      */
      return RTCS_OK;
   } /* Endif */

   /* FTP uses TCP only */
   if (mqx_ntohc(ip_header_ptr->PROTOCOL) != IPPROTO_TCP) {
      return RTCS_OK;
   } /* Endif */

   /* Extract port information */
   transport.PTR = TRANSPORT_PTR(ip_header_ptr);
   src_port_is_ftp = (mqx_ntohs(transport.TCP_PTR->source_port) == IPPORT_FTP);
   dest_port_is_ftp = (mqx_ntohs(transport.TCP_PTR->dest_port) == IPPORT_FTP);


   /* ALG is only applicable on FTP command sessions (typically port 21) */
   if (!src_port_is_ftp && !dest_port_is_ftp) {
     return RTCS_OK;
   } /* Endif */

   if (! ((src_port_is_ftp && pub_to_prv) || (dest_port_is_ftp && !pub_to_prv))) {
     return RTCS_OK;
   } /* Endif */

   /* Get TCP data pointer */
   ip_header_length = IPH_LEN(ip_header_ptr);
   orig_ip_length = mqx_ntohs(ip_header_ptr->LENGTH);
   tcp_header_length = (((mqx_ntohs(transport.TCP_PTR->flags)) & DATAOFS_MASK) >> (DATAOFS_SHIFT - 2));
   tcp_data_ptr = &transport.DATA_OFFSET[tcp_header_length];
   orig_tcp_data_length = orig_ip_length - (ip_header_length + tcp_header_length);

   /* Check if an FTP session struct exists */
   ftp_session_ptr = (NAT_SESSION_FTP_STRUCT_PTR)(void *)nat_session_ptr->ALG_INFO_PTR;
   while (ftp_session_ptr != NULL)  {
      if (ftp_session_ptr->HEADER.ALG_TYPE == NAT_ALG_FTP_TYPE)  {
         /* Found the FTP session struct */
         break;
      } /* Endif */
      ftp_session_ptr = (NAT_SESSION_FTP_STRUCT_PTR)(void *)ftp_session_ptr->HEADER.NEXT;
   } /* Endwhile */

   /*
   ** If the packet is entering the priv net from FTP command port, we don't
   ** modify it if no FTP session struct exists. The session may be using a mode
   ** that doesn't require any ALG.If there is a session struct, update ACK.
   */
   if (src_port_is_ftp && pub_to_prv) {
      if (ftp_session_ptr)  {
         /* Update packet with correct ACK */
         ack = mqx_ntohl(transport.TCP_PTR->ack);
         chksum = mqx_ntohs(transport.TCP_PTR->checksum);
         if ((int32_t)(ack - (ftp_session_ptr->LAST_SEQ + ftp_session_ptr->LAST_ADJ)) <= 0)  {
            temp = ftp_session_ptr->LAST_SEQ;
         } else  {
            temp = ftp_session_ptr->NEXT_SEQ;
         } /* Endif */
         chksum = NAT_chksum_mod(chksum,    /* Old checksum */
            (uint16_t)(ack & 0xFFFF),                   /* Old data */
            (uint16_t)(temp &0xFFFF));                  /* New data */
         chksum = NAT_chksum_mod(chksum,    /* Old checksum */
            (uint16_t)((ack >> 16) & 0xFFFF),           /* Old data */
            (uint16_t)((temp >> 16) & 0xFFFF));         /* New data */
         mqx_htonl(transport.TCP_PTR->ack, temp);             /* Replace old ACK with new one */
         mqx_htons(transport.TCP_PTR->checksum, chksum);
      } /* Endif */

      return RTCS_OK;
   } /* Endif */

   seq_adj = 0;
   seq = mqx_ntohl(transport.TCP_PTR->seq);
   chksum = mqx_ntohs(transport.TCP_PTR->checksum);

   /* We can only reach this point is the dest port is FTP command port */
   if (ftp_session_ptr) {

      /* If SEQ > NEXT_SEQ:
      **
      ** We have received an out of order packet. We are expecting a packet with
      ** SEQ == NEXT_SEQ, but we got a higher SEQ number than expected. The adj
      ** to the current packets SEQ number is unknown, and it depends on the
      ** packet with SEQ == NEXT_SEQ (it may or may not be a PORT command). We
      ** can't do anything until we get the previous packet, so we discard and
      ** let TCP retransmit it later.
      */
      if ((int32_t)(seq - ftp_session_ptr->NEXT_SEQ) > 0) {
         return (uint32_t)RTCS_ERROR;
      } else

      /* If SEQ < LAST_SEQ:
      **
      ** This is a retransmitted packet or one that took a slow path to the
      ** NAT gateway. The outside host has already ACKed this packet, and the
      ** internal host has already properly received the ACK because it has
      ** sent the next FTP command.
      */
      if ((int32_t)(seq - ftp_session_ptr->LAST_SEQ) < 0) {
         return (uint32_t)RTCS_ERROR;
      } else

      /* If SEQ == NEXT_SEQ:
      **
      ** This is a new packet. Normal scenario.
      */
      if (seq == ftp_session_ptr->NEXT_SEQ) {
         seq_adj = ftp_session_ptr->NEXT_ADJ;
         if (orig_tcp_data_length != 0)  {
            /* Only update if packet is not a pure ACK with no data */
            ftp_session_ptr->LAST_SEQ = seq;
            ftp_session_ptr->LAST_ADJ = seq_adj;
            ftp_session_ptr->NEXT_SEQ = seq + orig_tcp_data_length;
         } /* Endif */
      } else /* Endif */

      /* IF SEQ == LAST_SEQ:
      **
      ** The last command sent is being retransmitted. Let it through, and do
      ** any neccessary PORT command modifications
      */
      if (seq == ftp_session_ptr->LAST_SEQ) {
         seq_adj = ftp_session_ptr->LAST_ADJ;
      }

      /* This is when the SEQ is > LAST but < NEXT */
      else {
         return (uint32_t)RTCS_ERROR;   /* Rebuild? */
      } /* Endif */

      temp = seq + seq_adj;

      /* Write new SEQ number to packet */
      chksum = NAT_chksum_mod(chksum,
         (uint16_t)(seq & 0xFFFF),
         (uint16_t)(temp & 0xFFFF));
      chksum = NAT_chksum_mod(chksum,
         (uint16_t)((seq >> 16) & 0xFFFF),
         (uint16_t)((temp >> 16) & 0xFFFF));
      mqx_htonl(transport.TCP_PTR->seq, temp);
      mqx_htons(transport.TCP_PTR->checksum, chksum);
   } /* Endif */

   if (orig_tcp_data_length < 4)  {
      return RTCS_OK;
   } /* Endif */

   /* We send the packet through if we don't have a PORT command */
   if (memcmp(tcp_data_ptr, "PORT", 4)) {
      return RTCS_OK;
   } /* Endif */

   if (!NAT_ALG_detect_port(tcp_data_ptr, orig_tcp_data_length, &addr, &port))  {
      /* This is bad. We only have part of a port command. Toss packet */
      return (uint32_t)RTCS_ERROR;
   } /* Endif */

   if (!ftp_session_ptr)  {
      ftp_session_ptr = RTCS_part_alloc(((NAT_FTP_STRUCT_PTR)(alg_cfg_ptr))->FTP_PART_ID);
      if (!ftp_session_ptr)  {
         return RTCSERR_OUT_OF_MEMORY;
      } /* Endif */

      /* Attach to session */
      ftp_session_ptr->HEADER.ALG_TYPE = NAT_ALG_FTP_TYPE;
      ftp_session_ptr->HEADER.NEXT = nat_session_ptr->ALG_INFO_PTR;
      nat_session_ptr->ALG_INFO_PTR = (NAT_SESSION_EXT_STRUCT_PTR)(void *)ftp_session_ptr;
   } /* Endif */

   /* Create new session for data channel (or find existing session) */
   _mem_copy(ip_header_ptr->DEST, NAT_ALG_FTP_dummy.IP.DEST, 4);
   mqx_htonl(NAT_ALG_FTP_dummy.IP.SOURCE, addr);
   mqx_htons(NAT_ALG_FTP_dummy.TCP.source_port, port);
   mqx_htons(NAT_ALG_FTP_dummy.TCP.dest_port, IPPORT_FTPDATA);

   nat_data_session_ptr = NAT_lookup(&NAT_ALG_FTP_dummy.IP, FALSE, FALSE, &error);

   if (nat_data_session_ptr == NULL) {
      nat_data_session_ptr = NAT_insert(&NAT_ALG_FTP_dummy.IP, &error);
      if (!nat_data_session_ptr)  {
         return error;
      } /* Endif */
      NAT_event_add(&nat_cfg_ptr->TCP_TOUT, &nat_data_session_ptr->TIMEOUT);
   } /* Endif */

   /* Determine space needed for new PORT information */
   new_tcp_data_length = 5;                     /* "PORT " */
   new_tcp_data_length += 7;                    /* 5 commas and 2 termination bytes */
   addr = nat_data_session_ptr->NAT_HST;
   port = nat_data_session_ptr->NAT_PORT;

   /* Add IP addr len */
   for (i = 0; i < 32; i+=8)  {
      temp = ((addr >> i) & 0xFF);
      new_tcp_data_length += (temp < 10) ? 1 : ((temp < 100) ? 2 : 3);
   } /* Endfor */

   /* Add port len */
   for (i = 0; i < 16; i+=8)  {
      temp = ((port >> i) & 0xFF);
      new_tcp_data_length += (temp < 10) ? 1 : ((temp < 100) ? 2 : 3);
   } /* Endif */

   new_ip_length = ip_header_length + tcp_header_length + new_tcp_data_length;
   if (new_ip_length <= orig_ip_length)  {
      RTCSPCB_shrink(*pcb_ptr_ptr, orig_ip_length - new_ip_length);
   } else {
      new_mqx_pcb_ptr = _mem_alloc_system(sizeof(PCB) + sizeof(PCB_FRAGMENT) + new_ip_length);
      if (!new_mqx_pcb_ptr)  {
         return RTCSERR_OUT_OF_MEMORY;
      } /* Endif */
      new_pcb_data_ptr = (unsigned char *)new_mqx_pcb_ptr + sizeof(PCB) + sizeof(PCB_FRAGMENT);
      new_mqx_pcb_ptr->FREE = NAT_ALG_ftp_pcb_free;
      new_mqx_pcb_ptr->PRIVATE = NULL;
      new_mqx_pcb_ptr->FRAG[0].LENGTH = new_ip_length;
      new_mqx_pcb_ptr->FRAG[0].FRAGMENT = new_pcb_data_ptr;
      ((PCB_FRAGMENT_PTR)(void *)(new_mqx_pcb_ptr + 1))->LENGTH = 0;
      ((PCB_FRAGMENT_PTR)(void *)(new_mqx_pcb_ptr + 1))->FRAGMENT = NULL;

      new_pcb_ptr = RTCSPCB_alloc_recv(new_mqx_pcb_ptr);
      if (!new_pcb_ptr)  {
         _mem_free(new_mqx_pcb_ptr);
         return RTCSERR_OUT_OF_MEMORY;
      } /* Endif */

      RTCSPCB_DATA_NETWORK(new_pcb_ptr) = RTCSPCB_DATA(new_pcb_ptr);

      new_pcb_ptr->TYPE = (*pcb_ptr_ptr)->TYPE;
      new_pcb_ptr->IFSRC = (*pcb_ptr_ptr)->IFSRC;
      new_pcb_ptr->LINK_OPTIONS = (*pcb_ptr_ptr)->LINK_OPTIONS;
      new_pcb_ptr->IP_COMPLETE = IP_complete_recv;

      _mem_copy(ip_header_ptr, new_pcb_data_ptr, ip_header_length + tcp_header_length);

      /*
      ** Update all pointers to the new data
      */
      new_pcb_data_ptr = (void *)RTCSPCB_DATA(new_pcb_ptr);
      ip_header_ptr = (void *)new_pcb_data_ptr;
      transport.PTR = TRANSPORT_PTR(ip_header_ptr);
      tcp_data_ptr = &transport.DATA_OFFSET[tcp_header_length];

      RTCSPCB_free(*pcb_ptr_ptr);
      *pcb_ptr_ptr = new_pcb_ptr;
   } /* Endif */

   /* Rewrite data */
   _mem_copy("PORT ", tcp_data_ptr, 5);
   tcp_data_ptr += 5;

   for (i = 24; i >= 0; i-=8)  {
      temp = ((addr >> i) & 0xFF);
      NAT_ALG_ftp_bwrite((unsigned char)temp, &tcp_data_ptr);
      *tcp_data_ptr++ = ',';
   } /* Endfor */

   temp = ((port >> 8) & 0xFF);
   NAT_ALG_ftp_bwrite((unsigned char)temp, &tcp_data_ptr);
   *tcp_data_ptr++ = ',';

   temp = port & 0xFF;
   NAT_ALG_ftp_bwrite((unsigned char)temp, &tcp_data_ptr);
   *tcp_data_ptr++ = '\r';
   *tcp_data_ptr++ = '\n';

   ftp_session_ptr->LAST_SEQ = seq;
   ftp_session_ptr->LAST_ADJ = seq_adj;
   ftp_session_ptr->NEXT_SEQ = seq + orig_tcp_data_length;
   ftp_session_ptr->NEXT_ADJ = seq_adj - orig_tcp_data_length + new_tcp_data_length;

   /* Update IP length */
   chksum = mqx_ntohs(ip_header_ptr->CHECKSUM);
   chksum = NAT_chksum_mod(chksum, orig_ip_length, new_ip_length);
   mqx_htons(ip_header_ptr->CHECKSUM, chksum);
   mqx_htons(ip_header_ptr->LENGTH, new_ip_length);

   /* Calculate TCP checksum */
   NAT_ALG_TCP_checksum(ip_header_ptr);

   return RTCS_OK;
} /* Endbody */

#endif
