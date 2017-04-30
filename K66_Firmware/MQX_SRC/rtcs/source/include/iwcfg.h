#ifndef __iwcfg_h__
#define __iwcfg_h__
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
*   Definitions for WIFI device configurations.
*
*
*END************************************************************************/
#include <mqx.h>
#include <bsp.h>
#if !PLATFORM_SDK_ENABLED
#include <enet_wifi.h>
#endif


#define         MAX_RSSI_TABLE_SZ    5


typedef struct _iw_rssi_quanta{
  uint8_t max;
  uint8_t min;
} RSSI_QUANTA,* RSSI_QUANTA_PTR;


/***************************************
**
** Prototypes
**
*/

#ifdef __cplusplus
extern "C" {
#endif

uint32_t iwcfg_set_essid
    (
        uint32_t dev_num,
        char *essid
    );

uint32_t iwcfg_commit
    (
        uint32_t dev_num
    );

uint32_t iwcfg_set_mode
    (
        uint32_t dev_num,
        char *mode
    );

uint32_t iwcfg_set_wep_key 
    (
        uint32_t dev_num,
        char *wep_key,
        uint32_t key_len,
        uint32_t key_index
        
    );
    
uint32_t iwcfg_set_sec_type 
    (
        uint32_t dev_num,
        char  *sec_type
    );

uint32_t iwcfg_set_passphrase
    (
        uint32_t dev_num,
        char *passphrase
    );
uint32_t iwcfg_set_power 
    (
        uint32_t dev_num,
        uint32_t pow_val,
        uint32_t flags
        
    );

uint32_t iwcfg_set_scan
    (
        uint32_t dev_num,
        char *ssid
    );

uint32_t iwcfg_get_essid
    (
        uint32_t dev_num,
        char *essid /*[OUT]*/
    );

uint32_t iwcfg_get_mode
    (
        uint32_t dev_num,
        char *essid /*[OUT]*/
    );
    
uint32_t iwcfg_get_sectype
    (
        uint32_t dev_num,
        char *sectype /*[OUT]*/
    );  
uint32_t iwcfg_get_wep_key 
    (
        uint32_t dev_num,
        char *wep_key,
        uint32_t *key_index
  
    );
uint32_t iwcfg_get_passphrase 
    (
        uint32_t dev_num,
        char *passphrase
         
    );          

#ifdef __cplusplus
}
#endif

#endif
/* EOF */
