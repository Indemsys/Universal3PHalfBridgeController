#ifndef __addr_info_h__
#define __addr_info_h__
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
*
*END************************************************************************/


/*% INT16 Size */
#define NS_INT16SZ	 				2

/*% IPv4 Address Size */
#define NS_INADDRSZ	 				4

/*% IPv6 Address Size */
#define NS_IN6ADDRSZ				16


#define NET_ORDER			"inet" /* set order inet or inet6 or inet : inet6   */

#define	EAI_OK              0	/* OK 					*/
#define	EAI_BADFLAGS        3	/* invalid value for ai_flags 					*/
#define	EAI_FAIL            4   /* non-recoverable failure in name resolution 	*/
#define	EAI_FAMILY          5	/* ai_family not supported 						*/
#define	EAI_MEMORY          6	/* memory allocation failure 					*/
#define	EAI_NODATA          7	/* no address associated with hostname 			*/
#define	EAI_NONAME          8	/* hostname nor servname provided, or not known */
#define	EAI_SERVICE         9	/* servname not supported for ai_socktype 		*/
#define	EAI_SOCKTYPE        10	/* ai_socktype not supported 					*/
#define	EAI_SYSTEM          11	/* system error returned in errno 				*/

/*
 * Constants for getnameinfo()
 */

#define	NI_MAXHOST	    100
#define	NI_MAXSERV	    32

#define	NI_NOFQDN		0x00000001
#define	NI_NUMERICHOST	0x00000002
#define	NI_NAMEREQD		0x00000004
#define	NI_NUMERICSERV	0x00000008
#define	NI_DGRAM		0x00000010

#endif /* __addr_info_h__ */
