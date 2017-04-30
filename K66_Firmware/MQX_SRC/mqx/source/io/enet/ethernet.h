#ifndef __ethernet_h__
#define __ethernet_h__
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
*   This file contains the defines and data structure definitions that
*   reperesent ethernet packets.
*
*
*END************************************************************************/



#define ENET_FRAMESIZE_HEAD       sizeof(ENET_HEADER)
#define ENET_FRAMESIZE_HEAD_VLAN  (sizeof(ENET_HEADER)+sizeof(ENET_8021QTAG_HEADER))
#define ENET_FRAMESIZE_MAXDATA    (1500)
#define ENET_FRAMESIZE_MIN        (64)
#define ENET_FRAMESIZE_TAIL       (4)

#define ENET_FRAMESIZE            ENET_FRAMESIZE_HEAD+ENET_FRAMESIZE_MAXDATA+ENET_FRAMESIZE_TAIL
#define ENET_FRAMESIZE_VLAN       ENET_FRAMESIZE_HEAD_VLAN+ENET_FRAMESIZE_MAXDATA+ENET_FRAMESIZE_TAIL

/***************************************
**
** Ethernet protocols
**
*/
#define ENETPROT_LENGTH_TYPE_BOUNDARY ENET_FRAMESIZE_MAXDATA
#define ENETPROT_IP     0x800
#define ENETPROT_ARP    0x806
#define ENETPROT_8021Q  0x8100
#define ENETPROT_IP6    0x86DD


#define ENET_OPT_8023             0x0001
#define ENET_OPT_8021QTAG         0x0002 /* http://en.wikipedia.org/wiki/IEEE_802.1Q */
#define ENET_SETOPT_8021QPRIO(priority) (((uint32_t)(priority) & 0x7) << 2)
#define ENET_GETOPT_8021QPRIO(flags)    (((flags) >> 2) & 0x7)
#define ENET_SETOPT_8021QVID(vlan_id)   (((uint32_t)(vlan_id) & 0xFFF) << 5)
#define ENET_GETOPT_8021QVID(flags)     (((flags) >> 5) & 0xFFF)


#define eaddrcpy(p,x)   ((p)[0] = (x)[0], \
                         (p)[1] = (x)[1], \
                         (p)[2] = (x)[2], \
                         (p)[3] = (x)[3], \
                         (p)[4] = (x)[4], \
                         (p)[5] = (x)[5]  \
                        )
                        
#define htone(p,x) eaddrcpy(p,x)

#define ntohe(p,x)   ((x)[0] = (p)[0] & 0xFF, \
                      (x)[1] = (p)[1] & 0xFF, \
                      (x)[2] = (p)[2] & 0xFF, \
                      (x)[3] = (p)[3] & 0xFF, \
                      (x)[4] = (p)[4] & 0xFF, \
                      (x)[5] = (p)[5] & 0xFF  \
                     )
                     

typedef unsigned char   _enet_address[6];

/*
** Ethernet packet header
*/
typedef struct enet_header {
   _enet_address    DEST;
   _enet_address    SOURCE;
   unsigned char            TYPE[2];
} ENET_HEADER, * ENET_HEADER_PTR;

typedef struct enet_8021qtag_header {
   unsigned char    TAG[2];
   unsigned char    TYPE[2];
} ENET_8021QTAG_HEADER, * ENET_8021QTAG_HEADER_PTR;

typedef struct enet_8022_header {
   unsigned char    DSAP[1];
   unsigned char    SSAP[1];
   unsigned char    COMMAND[1];
   unsigned char    OUI[3];
   unsigned char    TYPE[2];
} ENET_8022_HEADER, * ENET_8022_HEADER_PTR;


#endif
/* EOF */
