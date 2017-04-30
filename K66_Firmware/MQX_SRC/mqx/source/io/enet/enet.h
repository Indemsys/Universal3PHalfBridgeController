#ifndef __enet_h__
#define __enet_h__
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
*   This file contains the defines, externs and data
*   structure definitions required by application
*   programs in order to use the Ethernet packet driver.
*
*
*END************************************************************************/

#include <pcb.h>
#include <ethernet.h>

/*--------------------------------------------------------------------------*/
/*                        
**                            CONSTANT DEFINITIONS
*/

/* Error codes */

#define ENET_OK                     (0)
#define ENET_ERROR                  (ENET_ERROR_BASE | 0xff)  /* general ENET error */

#define ENETERR_INVALID_DEVICE      (ENET_ERROR_BASE | 0x00)   /* Device number out of range  */
#define ENETERR_INIT_DEVICE         (ENET_ERROR_BASE | 0x01)   /* Device already initialized  */
#define ENETERR_ALLOC_CFG           (ENET_ERROR_BASE | 0x02)   /* Alloc state failed          */
#define ENETERR_ALLOC_PCB           (ENET_ERROR_BASE | 0x03)   /* Alloc PCBs failed           */
#define ENETERR_ALLOC_BD            (ENET_ERROR_BASE | 0x04)   /* Alloc BDs failed            */
#define ENETERR_INSTALL_ISR         (ENET_ERROR_BASE | 0x05)   /* Install ISR failed          */
#define ENETERR_FREE_PCB            (ENET_ERROR_BASE | 0x06)   /* PCBs in use                 */
#define ENETERR_ALLOC_ECB           (ENET_ERROR_BASE | 0x07)   /* Alloc ECB failed            */
#define ENETERR_OPEN_PROT           (ENET_ERROR_BASE | 0x08)   /* Protocol not open           */
#define ENETERR_CLOSE_PROT          (ENET_ERROR_BASE | 0x09)   /* Protocol already open       */
#define ENETERR_SEND_SHORT          (ENET_ERROR_BASE | 0x0A)   /* Packet too short            */
#define ENETERR_SEND_LONG           (ENET_ERROR_BASE | 0x0B)   /* Packet too long             */
#define ENETERR_JOIN_MULTICAST      (ENET_ERROR_BASE | 0x0C)   /* Not a multicast address     */
#define ENETERR_ALLOC_MCB           (ENET_ERROR_BASE | 0x0D)   /* Alloc MCB failed            */
#define ENETERR_LEAVE_GROUP         (ENET_ERROR_BASE | 0x0E)   /* Not a joined group          */
#define ENETERR_SEND_FULL           (ENET_ERROR_BASE | 0x0F)   /* Transmit ring full          */
#define ENETERR_IP_TABLE_FULL       (ENET_ERROR_BASE | 0x10)   /* IP Table full of IP pairs   */
#define ENETERR_ALLOC               (ENET_ERROR_BASE | 0x11)   /* Generic alloc failed        */
#define ENETERR_INIT_FAILED         (ENET_ERROR_BASE | 0x12)   /* Device failed to initialize */
#define ENETERR_DEVICE_TIMEOUT      (ENET_ERROR_BASE | 0x13)   /* Device read/write timeout   */
#define ENETERR_ALLOC_BUFFERS       (ENET_ERROR_BASE | 0x14)   /* Buffer alloc failed         */
#define ENETERR_ALLOC_MAC_CONTEXT   (ENET_ERROR_BASE | 0x15)   /* Buffer alloc failed         */
#define ENETERR_NO_TX_BUFFER        (ENET_ERROR_BASE | 0x16)   /* TX Buffer alloc failed      */
#define ENETERR_INVALID_INIT_PARAM  (ENET_ERROR_BASE | 0x17)   /* Invalid init. parameter     */
#define ENETERR_DEVICE_IN_USE       (ENET_ERROR_BASE | 0x18)   /* Shutdown failed, dev. in use*/
#define ENETERR_INITIALIZED_DEVICE  (ENET_ERROR_BASE | 0x19)   /* Device already initialized  */
#define ENETERR_INPROGRESS          (ENET_ERROR_BASE | 0x1A)   /* In Wifi Device Setting of ESSID in progress*/
#define ENETERR_1588_LWEVENT        (ENET_ERROR_BASE | 0x1B)   /* 1588driver lwevent creation failed */
#define ENETERR_INVALID_MODE        (ENET_ERROR_BASE | 0x1C)   /* Invalid mode for this ethernet driver */
#define ENETERR_INVALID_OPTION      (ENET_ERROR_BASE | 0x1D)   /* Invalid option for this ethernet driver */
#define ENETERR_1588L2_ALLOC          (ENET_ERROR_BASE | 0x1E)   /* 1588 init alloc failed*/
#define ENETERR_1588TSBUFF_ALLOC      (ENET_ERROR_BASE | 0x1F)   /* 1588 ts buffer alloca failed*/
#define ENETERR_NO_VALID_TXTS       (ENET_ERROR_BASE | 0x20)   /* No valid TX timestamp when do macnet_send*/
#define ENETERR_MIN                 (ENETERR_INVALID_DEVICE)
#define ENETERR_MAX                 (ENETERR_INVALID_OPTION)

/* Other constants */


/* MODES */
#define ENET_HALF_DUPLEX    0x1
#define ENET_FULL_DUPLEX    0x0
#define ENET_DUPLEX_MASK    0x1

#define ENET_10M            0x02
#define ENET_100M           0x04
#define ENET_1G             0x06
#define ENET_SPEED_MASK     0x0e

#define ENET_AUTONEGOTIATE  0x10

/* OPTIONS */
#define ENET_OPTION_MII             0x00000000
#define ENET_OPTION_VLAN            0x00000001
#define ENET_OPTION_PHY_DISCOVER    0x00000002
#define ENET_OPTION_MAC_LOOPBACK    0x00000004
#define ENET_OPTION_PHY_LOOPBACK    0x00000008
#define ENET_OPTION_NO_PREAMBLE     0x00000010
#define ENET_OPTION_RMII            0x00000020
#define ENET_OPTION_7WIRE           0x00000040
#define ENET_OPTION_STORE_AND_FORW  0x00000080
#define ENET_OPTION_PTP_MASTER_CLK  0x00000100
#define ENET_OPTION_PTP_INBAND      0x00000200

#define ENET_OPTION_HW_TX_IP_CHECKSUM       0x00001000
#define ENET_OPTION_HW_TX_PROTOCOL_CHECKSUM 0x00002000
#define ENET_OPTION_HW_RX_IP_CHECKSUM       0x00004000
#define ENET_OPTION_HW_RX_PROTOCOL_CHECKSUM 0x00008000
#define ENET_OPTION_HW_RX_MAC_ERR           0x00010000

/* NOTE: Not all MAC drivers will support all modes and options. */

/* Media control definition types */
#define IO_TYPE_MEDIACTL_WIFI   0x00
#define IO_TYPE_MEDIACTL_PTP    0x10


#if BSPCFG_ENABLE_ENET_STATS

   #ifndef BSPCFG_ENABLE_ENET_HISTOGRAM
      #define BSPCFG_ENABLE_ENET_HISTOGRAM 1
   #endif
   
   #define ENET_INC_STATS(x)   { enet_ptr->STATS.x++; }
#define ENET_INC_STATS_IF(c,x)   { if (c) ENET_INC_STATS(x) }
#else
   #define ENET_INC_STATS(x)
#define ENET_INC_STATS_IF(c,x)   
#endif

#define ENET_HISTOGRAM_SHIFT       (6)
#define ENET_HISTOGRAM_ENTRIES     (((ENET_FRAMESIZE)>>ENET_HISTOGRAM_SHIFT)+1)


/*--------------------------------------------------------------------------*/
/*                        
**                            TYPE DEFINITIONS
*/

typedef void   *_enet_handle;
  
typedef enum {
  Half_Duplex_10M   = (ENET_HALF_DUPLEX | ENET_10M),
  Full_Duplex_10M   = (ENET_FULL_DUPLEX | ENET_10M),
  Half_Duplex_100M  = (ENET_HALF_DUPLEX | ENET_100M),
  Full_Duplex_100M  = (ENET_FULL_DUPLEX | ENET_100M),
  Half_Duplex_1G    = (ENET_HALF_DUPLEX | ENET_1G),
  Full_Duplex_1G    = (ENET_FULL_DUPLEX | ENET_1G),
  Auto_Negotiate    = ENET_AUTONEGOTIATE
} ENET_mode;
  
#if BSPCFG_ENABLE_ENET_STATS
typedef struct enet_commom_stats_struct {
   uint32_t     ST_RX_TOTAL;         /* Total number of received packets    */
   uint32_t     ST_RX_MISSED;        /* Number of missed packets            */
   uint32_t     ST_RX_DISCARDED;     /* Discarded -- unrecognized protocol  */
   uint32_t     ST_RX_ERRORS;        /* Discarded -- error during reception */

   uint32_t     ST_TX_TOTAL;         /* Total number of transmitted packets */
   uint32_t     ST_TX_MISSED;        /* Discarded -- transmit ring full     */
   uint32_t     ST_TX_DISCARDED;     /* Discarded -- bad packet             */
   uint32_t     ST_TX_ERRORS;        /* Error during transmission           */
} ENET_COMMON_STATS_STRUCT, * ENET_COMMON_STATS_STRUCT_PTR;

typedef struct enet_stats {
   ENET_COMMON_STATS_STRUCT   COMMON;

   /* Following stats are physical errors/conditions */
   uint32_t     ST_RX_ALIGN;         /* Frame Alignment error    */
   uint32_t     ST_RX_FCS;           /* CRC error                */
   uint32_t     ST_RX_RUNT;          /* Runt packet received     */
   uint32_t     ST_RX_GIANT;         /* Giant packet received    */
   uint32_t     ST_RX_LATECOLL;      /* Late collision           */
   uint32_t     ST_RX_OVERRUN;       /* DMA overrun              */

   uint32_t     ST_TX_SQE;           /* Heartbeat lost           */
   uint32_t     ST_TX_DEFERRED;      /* Transmission deferred    */
   uint32_t     ST_TX_LATECOLL;      /* Late collision           */
   uint32_t     ST_TX_EXCESSCOLL;    /* Excessive collisions     */
   uint32_t     ST_TX_CARRIER;       /* Carrier sense lost       */
   uint32_t     ST_TX_UNDERRUN;      /* DMA underrun             */

   /* Following stats are collected by the ethernet driver  */
   uint32_t     ST_RX_COPY_SMALL;     /* Driver had to copy packet */
   uint32_t     ST_RX_COPY_LARGE;     /* Driver had to copy packet */
   uint32_t     ST_TX_COPY_SMALL;     /* Driver had to copy packet */
   uint32_t     ST_TX_COPY_LARGE;     /* Driver had to copy packet */

   uint32_t     RX_FRAGS_EXCEEDED;
   uint32_t     RX_PCBS_EXHAUSTED;
   uint32_t     RX_LARGE_BUFFERS_EXHAUSTED;

   uint32_t     TX_ALIGNED;
   uint32_t     TX_ALL_ALIGNED;

#if BSPCFG_ENABLE_ENET_HISTOGRAM
   uint32_t     RX_HISTOGRAM[ENET_HISTOGRAM_ENTRIES];  
   uint32_t     TX_HISTOGRAM[ENET_HISTOGRAM_ENTRIES];  
#endif
  
} ENET_STATS, * ENET_STATS_PTR;
#endif


/* forward declarations */
struct enet_mcb_struct;
struct enet_context_struct;
struct enet_param_struct;

typedef struct {
   uint32_t (_CODE_PTR_  INIT) (struct enet_context_struct *);
   uint32_t (_CODE_PTR_  STOP)(struct enet_context_struct *);
   uint32_t (_CODE_PTR_  SEND) (struct enet_context_struct *, PCB_PTR, uint32_t, uint32_t, uint32_t);
   bool (_CODE_PTR_  PHY_READ) (struct enet_context_struct *, uint32_t, uint32_t *, uint32_t);
   bool (_CODE_PTR_  PHY_WRITE) (struct enet_context_struct *,  uint32_t, uint32_t, uint32_t);
   uint32_t (_CODE_PTR_  JOIN) (struct enet_context_struct *, struct enet_mcb_struct *);
   uint32_t (_CODE_PTR_  REJOIN)(struct enet_context_struct *);
   uint32_t (_CODE_PTR_  MEDIACTL) (struct enet_context_struct *, uint32_t command_id, void *inout_param);
} ENET_MAC_IF_STRUCT, * ENET_MAC_IF_STRUCT_PTR;


typedef struct enet_pyh_if_struct {
   bool (_CODE_PTR_  DISCOVER) (struct enet_context_struct *);
   bool (_CODE_PTR_  INIT) (struct enet_context_struct *);
   uint32_t (_CODE_PTR_  SPEED)(struct enet_context_struct *);
   bool (_CODE_PTR_  STATUS) (struct enet_context_struct *);
   bool (_CODE_PTR_  DUPLEX)(struct enet_context_struct *);
} ENET_PHY_IF_STRUCT, * ENET_PHY_IF_STRUCT_PTR;



typedef struct enet_if_struct {
   const ENET_MAC_IF_STRUCT *    MAC_IF;        /* pointer to MAC interface struct */
   const ENET_PHY_IF_STRUCT *    PHY_IF;        /* pointer to PHY interface struct */
   unsigned char                         MAC_NUMBER;    /* MAC device number */
   unsigned char                         PHY_NUMBER;    /* MAC device number for communication with PHY */
   unsigned char                         PHY_ADDRESS;   /* PHY address */
   uint32_t                       PHY_MII_SPEED; /* PHY MII Speed (MDC - Management Data Clock) */
} ENET_IF_STRUCT, * ENET_IF_STRUCT_PTR;



typedef struct enet_param_struct {
   const ENET_IF_STRUCT         *ENET_IF;
   ENET_mode                     MODE;
   uint32_t                       OPTIONS;
   
   uint16_t                       NUM_TX_ENTRIES;
   uint16_t                       NUM_TX_BUFFERS;
   uint16_t                       TX_BUFFER_SIZE;

   uint16_t                       NUM_RX_ENTRIES;
   uint16_t                       NUM_RX_BUFFERS;
   uint16_t                       RX_BUFFER_SIZE;
   uint16_t                       NUM_RX_PCBS;
   
   uint16_t                       NUM_SMALL_BUFFERS;
   uint16_t                       NUM_LARGE_BUFFERS;
   void                         *MAC_PARAM;
    
} ENET_PARAM_STRUCT, * ENET_PARAM_STRUCT_PTR;


/*
 *      Generic format for most parameters that fit in an int
 */
typedef struct  _param
{
   int32_t          value;          /* The value of the parameter itself */
   uint8_t          fixed;          /* Hardware should not use auto select */
   uint8_t          disabled;       /* Disable the feature */
   uint32_t         flags;          /* Various specifc flags (if any) */
   void           *data;
   uint32_t         length;
}ENET_MEDIACTL_PARAM,* ENET_MEDIACTL_PARAM_PTR;

/*--------------------------------------------------------------------------*/
/*                        
**                            PROTOTYPES AND GLOBAL EXTERNS
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TAD_COMPILE__

extern const ENET_PARAM_STRUCT ENET_default_params[];

extern uint32_t        ENET_close(_enet_handle, uint16_t);
extern uint32_t        ENET_get_address(_enet_handle, _enet_address);
extern _mqx_uint      ENET_initialize(_mqx_uint, _enet_address, _mqx_uint,    _enet_handle *);
extern _mqx_uint      ENET_initialize_ex(const ENET_PARAM_STRUCT *,  _enet_address,  _enet_handle *);
extern uint32_t        ENET_open(_enet_handle, uint16_t, void (_CODE_PTR_)(PCB_PTR, void *), void *);
extern uint32_t        ENET_send(_enet_handle, PCB_PTR, uint16_t, _enet_address, uint32_t);
extern uint32_t        ENET_send_raw(_enet_handle, PCB_PTR);
extern uint32_t        ENET_shutdown(_enet_handle);
extern const char * ENET_strerror(_mqx_uint);
extern uint32_t        ENET_get_speed (_enet_handle);
extern uint32_t        ENET_get_MTU(_enet_handle);
extern uint32_t        ENET_get_mac_address(uint32_t, uint32_t,_enet_address);
extern bool        ENET_link_status(_enet_handle);
extern bool        ENET_read_mii(_enet_handle, uint32_t, uint32_t *, uint32_t);
extern bool        ENET_write_mii(_enet_handle, uint32_t, uint32_t, uint32_t);
extern uint32_t        ENET_get_phy_addr(_enet_handle);
extern uint32_t        ENET_mediactl(_enet_handle,uint32_t,void *);
extern bool        ENET_phy_registers(_enet_handle,uint32_t,uint32_t *);
extern _enet_handle   ENET_get_device_handle(uint32_t mac_number);
extern _enet_handle   ENET_get_next_device_handle(_enet_handle handle);
extern uint32_t        ENET_get_options(_enet_handle handle);

#if BSPCFG_ENABLE_ENET_STATS
extern ENET_STATS_PTR ENET_get_stats(_enet_handle);
#endif

extern uint32_t        ENET_join(_enet_handle, uint16_t, _enet_address);
extern uint32_t        ENET_leave(_enet_handle, uint16_t, _enet_address);

#endif

#ifdef __cplusplus
}
#endif

#endif

/* EOF */
