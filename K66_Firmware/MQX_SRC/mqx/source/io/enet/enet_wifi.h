/*HEADER**********************************************************************
*
* Copyright 2009 Freescale Semiconductor, Inc.
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
*   This file contains the definitions of constants,IOCTL'S and structures
*   required for the Wifi operation.
*
*
*END************************************************************************/
#ifndef enet_wifi_h
#define enet_wifi_h 1

#include "mqx.h"
#include "bsp.h"
#if MQX_USE_IO_OLD
#include "ioctl.h"
#else
#include "nio/ioctl.h"
#endif

/* IOCTL's to set various features of WIFI device. */
#if MQX_USE_IO_OLD
#define ENET_SET_MEDIACTL_COMMIT                                 _IO(IO_TYPE_MEDIACTL_WIFI,0x01)
#define ENET_SET_MEDIACTL_FREQ                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x02)
#define ENET_SET_MEDIACTL_MODE                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x03)
#define ENET_SET_MEDIACTL_SCAN                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x04)
#define ENET_SET_MEDIACTL_ESSID                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x05)
#define ENET_SET_MEDIACTL_RATE                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x06)
#define ENET_SET_MEDIACTL_RTS                                    _IO(IO_TYPE_MEDIACTL_WIFI,0x07)
#define ENET_SET_MEDIACTL_RETRY                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x08)
#define ENET_SET_MEDIACTL_ENCODE                                 _IO(IO_TYPE_MEDIACTL_WIFI,0x09)
#define ENET_SET_MEDIACTL_POWER                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x0A)
#define ENET_SET_MEDIACTL_SEC_TYPE                               _IO(IO_TYPE_MEDIACTL_WIFI,0x0B)
#define ENET_SET_MEDIACTL_PASSPHRASE                             _IO(IO_TYPE_MEDIACTL_WIFI,0x0C)
#define ENET_SET_MEDIACTL_DEBUG                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x0D)
#define ENET_MEDIACTL_VENDOR_SPECIFIC                            _IO(IO_TYPE_MEDIACTL_WIFI,0x81)
#define ENET_SET_MEDIACTL_TX_POWER                               _IO(IO_TYPE_MEDIACTL_WIFI,0xA0)
#define ENET_SET_MEDIACTL_ADHOC_MODE                             _IO(IO_TYPE_MEDIACTL_WIFI,0xA1)
/* IOCTL's to get various features of WIFI device. */
#define ENET_GET_MEDIACTL_NAME                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x0E)
#define ENET_GET_MEDIACTL_FREQ                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x0F)
#define ENET_GET_MEDIACTL_MODE                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x10)
#define ENET_GET_MEDIACTL_RANGE                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x11)
#define ENET_GET_MEDIACTL_WAP                                    _IO(IO_TYPE_MEDIACTL_WIFI,0x12)
#define ENET_GET_MEDIACTL_SCAN                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x13)
#define ENET_GET_MEDIACTL_ESSID                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x14)
#define ENET_GET_MEDIACTL_RATE                                   _IO(IO_TYPE_MEDIACTL_WIFI,0x15)
#define ENET_GET_MEDIACTL_RTS                                    _IO(IO_TYPE_MEDIACTL_WIFI,0x16)
#define ENET_GET_MEDIACTL_RETRY                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x17)
#define ENET_GET_MEDIACTL_ENCODE                                 _IO(IO_TYPE_MEDIACTL_WIFI,0x18)
#define ENET_GET_MEDIACTL_POWER                                  _IO(IO_TYPE_MEDIACTL_WIFI,0x19)
#define ENET_GET_MEDIACTL_SEC_TYPE                               _IO(IO_TYPE_MEDIACTL_WIFI,0x20)
#define ENET_MEDIACTL_IS_INITIALIZED                             _IO(IO_TYPE_MEDIACTL_WIFI,0x21)
#define ENET_GET_MEDIACTL_PASSPHRASE                             _IO(IO_TYPE_MEDIACTL_WIFI,0x22)

#else //MQX_USE_IO_OLD

#define ENET_SET_MEDIACTL_COMMIT                                 (IO_TYPE_MEDIACTL_WIFI|0x01)
#define ENET_SET_MEDIACTL_FREQ                                   (IO_TYPE_MEDIACTL_WIFI|0x02)
#define ENET_SET_MEDIACTL_MODE                                   (IO_TYPE_MEDIACTL_WIFI|0x03)
#define ENET_SET_MEDIACTL_SCAN                                   (IO_TYPE_MEDIACTL_WIFI|0x04)
#define ENET_SET_MEDIACTL_ESSID                                  (IO_TYPE_MEDIACTL_WIFI|0x05)
#define ENET_SET_MEDIACTL_RATE                                   (IO_TYPE_MEDIACTL_WIFI|0x06)
#define ENET_SET_MEDIACTL_RTS                                    (IO_TYPE_MEDIACTL_WIFI|0x07)
#define ENET_SET_MEDIACTL_RETRY                                  (IO_TYPE_MEDIACTL_WIFI|0x08)
#define ENET_SET_MEDIACTL_ENCODE                                 (IO_TYPE_MEDIACTL_WIFI|0x09)
#define ENET_SET_MEDIACTL_POWER                                  (IO_TYPE_MEDIACTL_WIFI|0x0A)
#define ENET_SET_MEDIACTL_SEC_TYPE                               (IO_TYPE_MEDIACTL_WIFI|0x0B)
#define ENET_SET_MEDIACTL_PASSPHRASE                             (IO_TYPE_MEDIACTL_WIFI|0x0C)
#define ENET_SET_MEDIACTL_DEBUG                                  (IO_TYPE_MEDIACTL_WIFI|0x0D)
/* IOCTL's to get various features of WIFI device. */
#define ENET_GET_MEDIACTL_NAME                                   (IO_TYPE_MEDIACTL_WIFI|0x0E)
#define ENET_GET_MEDIACTL_FREQ                                   (IO_TYPE_MEDIACTL_WIFI|0x0F)
#define ENET_GET_MEDIACTL_MODE                                   (IO_TYPE_MEDIACTL_WIFI|0x10)
#define ENET_GET_MEDIACTL_RANGE                                  (IO_TYPE_MEDIACTL_WIFI|0x11)
#define ENET_GET_MEDIACTL_WAP                                    (IO_TYPE_MEDIACTL_WIFI|0x12)
#define ENET_GET_MEDIACTL_SCAN                                   (IO_TYPE_MEDIACTL_WIFI|0x13)
#define ENET_GET_MEDIACTL_ESSID                                  (IO_TYPE_MEDIACTL_WIFI|0x14)
#define ENET_GET_MEDIACTL_RATE                                   (IO_TYPE_MEDIACTL_WIFI|0x15)
#define ENET_GET_MEDIACTL_RTS                                    (IO_TYPE_MEDIACTL_WIFI|0x16)
#define ENET_GET_MEDIACTL_RETRY                                  (IO_TYPE_MEDIACTL_WIFI|0x17)
#define ENET_GET_MEDIACTL_ENCODE                                 (IO_TYPE_MEDIACTL_WIFI|0x18)
#define ENET_GET_MEDIACTL_POWER                                  (IO_TYPE_MEDIACTL_WIFI|0x19)
#define ENET_GET_MEDIACTL_SEC_TYPE                               (IO_TYPE_MEDIACTL_WIFI|0x20)
#define ENET_MEDIACTL_IS_INITIALIZED                             (IO_TYPE_MEDIACTL_WIFI|0x21)
#define ENET_GET_MEDIACTL_PASSPHRASE                             (IO_TYPE_MEDIACTL_WIFI|0x22)
#endif //MQX_USE_IO_OLD

#if BSP_ENET_WIFI_WEB_PROV_ENABLED
/* Additional GainSpan Wifi driver specific IOCTL's  */
#define ENET_SET_WPS_ENABLE                                      _IO(IO_TYPE_MEDIACTL_WIFI,0x23)
#define ENET_SET_WEB_PROV_ENABLE                                 _IO(IO_TYPE_MEDIACTL_WIFI,0x24)
#define ENET_SET_WEB_PROV_PARAM_SSID                             _IO(IO_TYPE_MEDIACTL_WIFI,0x25)
#define ENET_SET_WEB_PROV_PARAM_CH                               _IO(IO_TYPE_MEDIACTL_WIFI,0x26)
#define ENET_SET_WEB_PROV_PARAM_USRNAME                          _IO(IO_TYPE_MEDIACTL_WIFI,0x27)
#define ENET_SET_WEB_PROV_PARAM_PWD                              _IO(IO_TYPE_MEDIACTL_WIFI,0x28)
#define ENET_GET_WEB_PROV_PARAM_USRNAME                          _IO(IO_TYPE_MEDIACTL_WIFI,0x29)
#define ENET_GET_WEB_PROV_PARAM_PWD                              _IO(IO_TYPE_MEDIACTL_WIFI,0x30)
#define ENET_SET_FW_UPGRADE                                      _IO(IO_TYPE_MEDIACTL_WIFI,0x31)
#endif

#define ENET_MEDIACTL_FREQ_AUTO                                  (0x00)/* Let the driver decides */
#define ENET_MEDIACTL_FREQ_FIXED                                 (0x01)/* Force a specific value */
/* Flags for encoding (along with the token) */
#define ENET_MEDIACTL_ENCODE_INDEX                               0x00FF  /* Token index (if needed) */
#define ENET_MEDIACTL_ENCODE_FLAGS                               0xFF00  /* Flags defined below */
#define ENET_MEDIACTL_ENCODE_MODE                                0xF000  /* Modes defined below */
#define ENET_MEDIACTL_ENCODE_DISABLED                            0x8000  /* Encoding disabled */
#define ENET_MEDIACTL_ENCODE_ENABLED                             0x0000  /* Encoding enabled */
#define ENET_MEDIACTL_ENCODE_RESTRICTED                          0x4000  /* Refuse non-encoded packets */
#define ENET_MEDIACTL_ENCODE_OPEN                                0x2000  /* Accept non-encoded packets */
#define ENET_MEDIACTL_ENCODE_NOKEY                               0x0800  /* Key is write only, so not present */
#define ENET_MEDIACTL_ENCODE_TEMP                                0x0400  /* Temporary key */
/* Retry limits and lifetime flags available */
#define ENET_MEDIACTL_RETRY_ON                                   0x0000  /* No details... */
#define ENET_MEDIACTL_RETRY_TYPE                                 0xF000  /* Type of parameter */
#define ENET_MEDIACTL_RETRY_LIMIT                                0x1000  /* Maximum number of retries*/
#define ENET_MEDIACTL_RETRY_LIFETIME                             0x2000  /* Maximum duration of retries in us */
#define ENET_MEDIACTL_RETRY_MODIFIER                             0x00FF  /* Modify a parameter */
#define ENET_MEDIACTL_RETRY_MIN                                  0x0001  /* Value is a minimum  */
#define ENET_MEDIACTL_RETRY_MAX                                  0x0002  /* Value is a maximum */
#define ENET_MEDIACTL_RETRY_RELATIVE                             0x0004  /* Value is not in seconds/ms/us */
#define ENET_MEDIACTL_RETRY_SHORT                                0x0010  /* Value is for short packets  */
#define ENET_MEDIACTL_RETRY_LONG                                 0x0020  /* Value is for long packets */

/* Power management flags available (along with the value, if any) */
#define ENET_MEDIACTL_POWER_ON                                   0x0000  /* No details... */
#define ENET_MEDIACTL_POWER_TYPE                                 0xF000  /* Type of parameter */
#define ENET_MEDIACTL_POWER_PERIOD                               0x1000  /* Value is a period/duration of  */
#define ENET_MEDIACTL_POWER_TIMEOUT                              0x2000  /* Value is a timeout (to go asleep) */
#define ENET_MEDIACTL_POWER_MODE                                 0x0F00  /* Power Management mode */
#define ENET_MEDIACTL_POWER_UNICAST_R                            0x0100  /* Receive only unicast messages */
#define ENET_MEDIACTL_POWER_MULTICAST_R                          0x0200  /* Receive only multicast messages */
#define ENET_MEDIACTL_POWER_ALL_R                                0x0300  /* Receive all messages though PM */
#define ENET_MEDIACTL_POWER_FORCE_S                              0x0400  /* Force PM procedure for sending unicast */
#define ENET_MEDIACTL_POWER_REPEATER                             0x0800  /* Repeat broadcast messages in PM period */
#define ENET_MEDIACTL_POWER_MODIFIER                             0x000F  /* Modify a parameter */
#define ENET_MEDIACTL_POWER_MIN                                  0x0001  /* Value is a minimum  */
#define ENET_MEDIACTL_POWER_MAX                                  0x0002  /* Value is a maximum */
#define ENET_MEDIACTL_POWER_RELATIVE                             0x0004  /* Value is not in seconds/ms/us */

/* Security types */
#define ENET_MEDIACTL_SECURITY_TYPE_NONE                         (0x00)
#define ENET_MEDIACTL_SECURITY_TYPE_WEP                          (0x01)
#define ENET_MEDIACTL_SECURITY_TYPE_WPA                          (0x02)
#define ENET_MEDIACTL_SECURITY_TYPE_WPA2                         (0x03)

#define ENET_MEDIACTL_MODE_AUTO                                  (0)
/* Single cell network */
#define ENET_MEDIACTL_MODE_ADHOC                                 (1)
/* Multi cell network, roaming, ... */
#define ENET_MEDIACTL_MODE_INFRA                                 (2)
/* Synchronisation master or Access Point */
#define ENET_MEDIACTL_MODE_MASTER                                (3)
/* Wireless Repeater (forwarder) */
#define ENET_MEDIACTL_MODE_REPEAT                                (4)
 /* Secondary master/repeater (backup) */
#define ENET_MEDIACTL_MODE_SECOND                                (5)
/* Passive monitor (listen only) */
#define ENET_MEDIACTL_MODE_MONITOR                               (6)
/* Mesh (IEEE 802.11s) network */
#define ENET_MEDIACTL_MODE_MESH                                  (7)


typedef struct __essid
{
    char  *essid;
    uint16_t flags;
    uint32_t length;
}ENET_ESSID,* ENET_ESSID_PTR;

typedef struct _scan_info
{
    uint8_t channel;
    uint8_t ssid_len;
    uint8_t rssi;
    uint8_t security_enabled;
    uint16_t beacon_period;
    uint8_t preamble;
    uint8_t bss_type;
    uint8_t bssid[6];
    uint8_t ssid[32];
} ENET_SCAN_INFO, * ENET_SCAN_INFO_PTR;

typedef struct _scan_list
{
    int32_t num_scan_entries;
    ENET_SCAN_INFO_PTR scan_info_list;
}ENET_SCAN_LIST,* ENET_SCAN_LIST_PTR;
#endif
/* EOF */

