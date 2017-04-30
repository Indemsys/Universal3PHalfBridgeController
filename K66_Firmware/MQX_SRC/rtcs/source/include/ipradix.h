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
*   IP Radix Tree definitions
*
*
*END************************************************************************/

#ifndef __ipradix_h__
#define __ipradix_h__

/***************************************
**
** Type definitions
**
*/

typedef struct ipradix_node {
   void                *DATA;    /* must be first field */
   _ip_address          IP;
   _ip_address          MASK;
   struct ipradix_node      *PARENT;
   struct ipradix_node      *CHILD[2];
} IPRADIX_NODE, * IPRADIX_NODE_PTR;


/***************************************
**
** Prototypes
**
*/

void IPRADIX_init(IPRADIX_NODE_PTR);

#ifdef __cplusplus
extern "C" {
#endif

void IPRADIX_findbest
(
   IPRADIX_NODE_PTR     root,    /* [IN] the root of the radix tree */
   _ip_address          ip,      /* [IN] the IP address to search for */
   bool (_CODE_PTR_  test)(void *, void *),
                                 /* [IN] application-specific test */
   void                *data     /* [IN] data for test() */
);

void IPRADIX_insert
(
   IPRADIX_NODE_PTR     root,    /* [IN] the root of the radix tree */
   _ip_address          ip,      /* [IN] the IP address to insert */
   _ip_address          mask,    /* [IN] the IP network mask */
   _rtcs_part           part,    /* [IN] the node partition */
   void (_CODE_PTR_     insert)(void **, void *),
                                 /* [IN] application-specific insert */
   void                *data     /* [IN] data for insert() */
);

void IPRADIX_walk
(
   IPRADIX_NODE_PTR     root,    /* [IN] the root of the radix tree */
   bool (_CODE_PTR_  test)(_ip_address, _ip_address, void *, void *),
                                 /* [IN] application-specific test */
   void                *data     /* [IN] data for test() */
);

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
